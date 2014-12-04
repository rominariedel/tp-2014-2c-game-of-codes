/*
 * auxiliares.c
 *
 *  Created on: 27/09/2014
 *      Author: utnso
 */

#include "variables_globales.h"

long tamanio_del_archivo(FILE* archivo) {
	fseek(archivo, 0, SEEK_END);
	int tamanio = ftell(archivo);
	rewind(archivo);
	return tamanio;
}

char * extraer_syscalls(char * PATH) {
	printf("Extrayendo datos del archivo\n");
	FILE* archivo = fopen(PATH, "read");
	tamanio_codigo_syscalls = tamanio_del_archivo(archivo);
	char * buffer = malloc(tamanio_codigo_syscalls); //MMAP
	fread((void*) buffer, 1, tamanio_codigo_syscalls, archivo);
	fclose(archivo);
	printf("Datos copiados exitosamente\n");
	return buffer;
}

int obtener_TID() {
	TID += 1;
	return TID;
}

int obtener_PID() {
	PID = PID + 1;
	return PID;
}

void crear_colas() {
	e_exit = queue_create();

	ready.prioridad_0 = queue_create();
	ready.prioridad_1 = queue_create();
	block.prioridad_0 = queue_create();

	SYS_CALL = queue_create();

	block.prioridad_1 = list_create();
	exec = list_create();
	CPU_list = list_create();
	consola_list = list_create();
	hilos_join = list_create();

	dic_bloqueados = dictionary_create();

	HILOS_SISTEMA = list_create();
	mallocs = list_create();
}

void free_listas() {
	queue_destroy(e_exit);

	queue_destroy(ready.prioridad_0);
	queue_destroy(ready.prioridad_1);
	queue_destroy(SYS_CALL);
	queue_destroy(block.prioridad_0);
	list_destroy_and_destroy_elements(block.prioridad_1, &free);
	list_destroy_and_destroy_elements(exec, &free);
	list_destroy_and_destroy_elements(CPU_list, &free);
	list_destroy_and_destroy_elements(consola_list, &free);

}

bool CPU_esta_libre(struct_CPU * cpu) {
	return (cpu->bit_estado == 0);
}

void loguear(t_cola cola, TCB_struct * tcb_log) {
	printf("KM DEL TCB A LOGUEAR %d\n", tcb_log->KM);
	bool tiene_tcb(hilo_t * hilo) {
		if (tcb_log->KM == 1) {
			return hilo->tcb.KM == 1;
		} else {

			return (hilo->tcb.TID == tcb_log->TID) && (hilo->tcb.KM != 1);
		}
	}

	hilo_t * hilo = list_find(HILOS_SISTEMA, (void*) tiene_tcb);
	if (hilo == NULL ) {
		printf("FALLO EN EN LOGUEAR, NO SE ENCONTRO HILO\n");
	}

	hilo->tcb.PID = tcb_log->PID;
	hilo->tcb.TID = tcb_log->TID;
	hilo->tcb.KM = tcb_log->KM;
	hilo->tcb.M = tcb_log->M;
	hilo->tcb.tamanioSegmentoCodigo = tcb_log->tamanioSegmentoCodigo;
	hilo->tcb.P = tcb_log->P;
	hilo->tcb.X = tcb_log->X;
	hilo->tcb.S = tcb_log->S;
	hilo->tcb.registrosProgramacion[0] = tcb_log->registrosProgramacion[0];
	hilo->tcb.registrosProgramacion[1] = tcb_log->registrosProgramacion[1];
	hilo->tcb.registrosProgramacion[2] = tcb_log->registrosProgramacion[2];
	hilo->tcb.registrosProgramacion[3] = tcb_log->registrosProgramacion[3];
	hilo->tcb.registrosProgramacion[4] = tcb_log->registrosProgramacion[4];

	hilo->cola = cola;

	hilos(HILOS_SISTEMA);
}

void meter_en_ready(int prioridad, TCB_struct * tcb) {
	sem_wait(&sem_READY);

	switch (prioridad) {
	case 0:
		queue_push(ready.prioridad_0, tcb);
		break;
	case 1:
		queue_push(ready.prioridad_1, tcb);
		break;
	}

	loguear(READY, tcb);
	sem_post(&sem_READY);
	sem_post(&sem_procesoListo);
	sem_post(&sem_procesoListo);
}

TCB_struct * sacar_de_ready(int prioridad) {
	sem_wait(&sem_READY);
	TCB_struct * tcb = NULL;

	printf("READY ");
	switch (prioridad) {
	case 0:
		tcb = queue_pop(ready.prioridad_0);
		break;
	case 1:
		tcb = queue_pop(ready.prioridad_1);
		break;
	}
	//loguear(READY, tcb);
	sem_post(&sem_READY);
	return tcb;
}

void liberar_cpu(int socket) {
	struct_CPU * cpu = obtener_CPUAsociada(socket);
	cpu->bit_estado = libre;

	bool tiene_mismo_tid(TCB_struct * tcb) {
		return tcb->TID == cpu->TID;
	}

	list_remove_and_destroy_by_condition(exec, (void*) tiene_mismo_tid, free);

	cpu->PID = -1;

	sem_post(&sem_CPU);
}

void abortar(TCB_struct* tcb) {
	finalizo_ejecucion(tcb);
}

void desconecto_cpu(int socket) {
	struct_CPU * cpu = obtener_CPUAsociada(socket);
	if (cpu == NULL ) {
		printf("No se encontro la CPU\n");
		exit(-1);
	}
	bool tiene_mismo_socket(struct_CPU *estructura) {
		return estructura->socket_CPU == socket;
	}
	if (cpu->PID >= 0) {
		printf("LA CPU ESTABA EJECUTANDO UN PROCESO\n");

		struct_consola * consola = obtener_consolaAsociada(cpu->PID);
		TCB_struct * tcb = obtener_tcbEjecutando(cpu->TID);
		if (consola->termino_ejecucion) {
			sacar_de_ejecucion(tcb, false);
		} else {
			if (tcb->KM) {
				void abortar_procesos(struct_consola * consola) {
					t_datosAEnviar * datos = crear_paquete(se_desconecto_cpu,
							NULL, 0);
					enviar_datos(consola->socket_consola, datos);
					free(datos);
				}
				list_iterate(consola_list, (void*) abortar_procesos);
			} else {
				t_datosAEnviar * datos = crear_paquete(se_desconecto_cpu, NULL,
						0);
				enviar_datos(consola->socket_consola, datos);
				free(datos);

			}
			sacar_de_ejecucion(tcb, false);
			//struct_bloqueado * bloqueado = malloc(sizeof(struct_bloqueado));
			//bloqueado->tcb = *tcb;
			//bloqueado->id_recurso = -1;
			//list_add(block.prioridad_1, bloqueado);

		}
	} else {
		sem_wait(&sem_CPU);
	}
	list_remove_and_destroy_by_condition(CPU_list, (void*) tiene_mismo_socket,
			free);
}

void planificador() {

	t_datosAEnviar * datos;
	fd_set copia_set;
	while (1) {

		struct timeval * timeout = malloc(sizeof(struct timeval));
		timeout->tv_sec = 1;
		timeout->tv_usec = 0;

		copia_set = CPU_set;
		int i = select(descriptor_mas_alto_cpu + 1, &copia_set, NULL, NULL,
				timeout);

		if (i == -1) {
			//error
			break;
		}
		free(timeout);
		int n_descriptor = 0;

		while (n_descriptor <= descriptor_mas_alto_cpu) {

			if (FD_ISSET(n_descriptor, &copia_set)) {
				datos = recibir_datos(n_descriptor);

				if (datos == NULL ) {
					desconexion_cpu(n_descriptor);
					desconecto_cpu(n_descriptor);
					FD_CLR(n_descriptor, &CPU_set);
					break;
				}

				int codigo_operacion = datos->codigo_operacion;

				TCB_struct* tcb = malloc(sizeof(TCB_struct));
				struct_CPU * cpu;
				int dirSysCall, tamanio, pid, tid_a_esperar, id_recurso, tid, base_segmento;
				char * cadena;
				char * id_tipo;

				printf("CODIGO OPERACION : %d\n", codigo_operacion);

				switch (codigo_operacion) {

				case finaliza_quantum:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					finalizo_quantum(tcb);
					liberar_cpu(n_descriptor);
					break;
				case finaliza_ejecucion:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					finalizo_ejecucion(tcb);
					liberar_cpu(n_descriptor);
					break;
				case ejecucion_erronea:
					liberar_cpu(n_descriptor);
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					abortar(tcb);
					//liberar_cpu(n_descriptor);
					break;
				case interrupcion:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					memcpy(&dirSysCall, datos->datos + sizeof(TCB_struct),
							sizeof(int));

					printf("\n INTERRUPCION TCB \n");
					printf("\n PID: %d \n", tcb->PID);
					printf("\n TID: %d \n", tcb->TID);
					printf("\n KM: %d \n", tcb->KM);
					printf("\n BASE SEGMENTO CODIGO: %d \n", tcb->M);
					printf("\n TAMANIO SEGMENTO CODIGO %d \n",
							tcb->tamanioSegmentoCodigo);
					printf("\n PUNTERO INSTRUCCION %d \n", tcb->P);
					printf("\n BASE STACK %d \n", tcb->X);
					printf("\n CURSOR STACK %d \n", tcb->S);
					printf("\n A: %d \n", tcb->registrosProgramacion[0]);
					printf("\n B: %d \n", tcb->registrosProgramacion[1]);
					printf("\n C: %d \n", tcb->registrosProgramacion[2]);
					printf("\n D: %d \n", tcb->registrosProgramacion[3]);
					printf("\n E: %d \n", tcb->registrosProgramacion[4]);

					t_hilo * hilo2 = (t_hilo*) obtener_hilo_asociado(tcb);
					if (obtener_hilo_asociado(tcb) == NULL ) {
						printf("\n\n\n NO SE ENCONTRO EL HILOX2 \n\n");
					}
					if (hilo2 == NULL ) {
						printf("NO SE ENCONTRO EL HILO!\n");
						exit(1);
					} else {
						instruccion_protegida("Interrupcion", hilo2);
					}
					printf("LLEGO LA DIRECCIONNANANANANANANANANANA: %d\n",
							dirSysCall);
					interrumpir(tcb, dirSysCall);
					liberar_cpu(n_descriptor);
					break;
				case creacion_hilo:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					t_hilo * hilo = (t_hilo*) obtener_hilo_asociado(tcb);
					if (hilo == NULL ) {
						printf("NO SE ENCONTRO EL HILO!\n");
					} else {

						instruccion_protegida("Crear_Hilo",
								(t_hilo*) obtener_hilo_asociado(tcb));
					}
					crear_hilo(*tcb, n_descriptor);
					break;
				case planificar_nuevo_hilo: //Aca llega el TCB listo para planificar con su stack inicializado
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					planificar_hilo_creado(tcb);
					break;
				case entrada_estandar:
					id_tipo = malloc(1);
					memcpy(&tamanio, datos->datos, sizeof(int));
					memcpy(&pid, datos->datos + sizeof(int), sizeof(int));
					memcpy(id_tipo, datos->datos + (2 * sizeof(int)), 1);

					cpu = obtener_CPUAsociada(n_descriptor);

					producir_entrada_estandar(cpu->PID, *id_tipo, n_descriptor,
							tamanio);

					break;
				case salida_estandar:
					cadena = malloc(datos->tamanio - sizeof(int) - sizeof(int));
					//memcpy(&pid, datos->datos, sizeof(int));
					//memcpy(&tid, datos->datos + sizeof(int), sizeof(int));
					memcpy(cadena, datos->datos + sizeof(int) + sizeof(int),
							datos->tamanio - sizeof(int) - sizeof(int));

					printf("\n\n\n\nCADENNAAAAAAAAAAAAAAAAAAAAAAAAAA: %s \n\n\n\n",cadena);
					cpu = obtener_CPUAsociada(n_descriptor);

					producir_salida_estandar(cpu->PID, cpu->TID, cadena);

					break;
				case join:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					memcpy(&tid_a_esperar, datos->datos + sizeof(TCB_struct),
							sizeof(int));
					instruccion_protegida("Join",
							(t_hilo*) obtener_hilo_asociado(tcb));
					log_debug(logger, "SE SOLICITO JOIN. SE BLOQUEA TID %d, ESPERANDO A TID %d", tcb->TID, tid_a_esperar);
					realizar_join(tcb, tid_a_esperar);

					break;
				case bloquear:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					memcpy(&id_recurso, datos->datos + sizeof(TCB_struct),
							sizeof(int));
					instruccion_protegida("Bloquear",
							(t_hilo*) obtener_hilo_asociado(tcb));
					//liberar_cpu(n_descriptor);
					realizar_bloqueo(tcb, id_recurso);

					break;
				case despertar:
					memcpy(&id_recurso, datos->datos, sizeof(int));
					//instruccion_protegida("Despertar",
					//		(t_hilo*) obtener_hilo_asociado(tcb));
					realizar_desbloqueo(id_recurso);

					break;
				case hago_malloc:
					memcpy(&pid, datos->datos, sizeof(int));
					memcpy(&tid, datos->datos + sizeof(int), sizeof(int));
					memcpy(&base_segmento, datos->datos + (2* sizeof(int)), sizeof(int));
					hizo_malloc(pid, tid, base_segmento);
					break;
				case hago_free:
					memcpy(&pid, datos->datos, sizeof(int));
					memcpy(&tid, datos->datos + sizeof(int), sizeof(int));
					memcpy(&base_segmento, datos->datos + (2* sizeof(int)), sizeof(int));
					hizo_free(pid, tid, base_segmento);
					break;
				}
				free(datos->datos);
				free(datos);

			}
			n_descriptor++;
		}

	}
}

void hizo_malloc(int pid, int tid, int base_segmento){
	malc_struct * malc = malloc(sizeof(malc_struct));
	malc->PID = pid;
	malc->TID = tid;
	malc->base_segmento = base_segmento;
	list_add(mallocs, malc);
}

void hizo_free(int pid, int tid, int base_segmento){
	bool find_malloc_struct(malc_struct * malc){
		return (malc->TID == tid && malc->base_segmento == base_segmento);
	}
	list_remove_and_destroy_by_condition(mallocs, (void*) find_malloc_struct, free);
}

void liberar_mallocs(int tid){

	void liberar_malloc(malc_struct * malc){
		if (malc->TID == tid){
			destruir_segmento_MSP(malc->PID, malc->base_segmento);
			hizo_free(malc->PID, malc->TID, malc->base_segmento);
		}
	}

	list_iterate(mallocs, (void*) liberar_malloc);

}

struct_consola * obtener_consolaConectada(int socket_consola) {
	bool tiene_mismo_socket(struct_consola * estructura) {
		return estructura->socket_consola == socket_consola;
	}
	return list_find(consola_list, (void*) tiene_mismo_socket);

}

struct_consola * obtener_consolaAsociada(int PID) {
	bool tiene_mismo_pid(struct_consola * estructura) {
		return estructura->PID == PID;
	}
	return list_find(consola_list, (void*) tiene_mismo_pid);
}

struct_CPU * obtener_CPUAsociada(int socket_cpu) {
	bool tiene_mismo_socket(struct_CPU *estructura) {
		return estructura->socket_CPU == socket_cpu;
	}
	return list_find(CPU_list, (void*) tiene_mismo_socket);
}

struct_bloqueado * obtener_bloqueado(int TID) {
	bool tiene_mismo_tid(struct_bloqueado * estructura) {
		return estructura->tcb.TID == TID;
	}
	return list_find(block.prioridad_1, (void*) tiene_mismo_tid);
}

void producir_salida_estandar(int pid, int tid, char* cadena) {
	printf("SALIDA ESTANDAR! \n PID: %d \n TID: %d \n CADENA_ %s\n", pid, tid,
			cadena);
	TCB_struct * tcb = obtener_tcbEjecutando(tid);
	if (tcb != NULL ) {
		instruccion_protegida("Salida_Estandar",
				(t_hilo*) obtener_hilo_asociado(tcb));
	} else {
		printf("NO se encontro tcb\n");
	}

	struct_consola * consola_asociada = obtener_consolaAsociada(pid);
	t_datosAEnviar * datos = crear_paquete(imprimir_en_pantalla, cadena,
			string_length(cadena));
	if (consola_asociada == NULL ) {
		printf("NO SE ENCONTRO CONSOLA ASOCIADA");
	}
	enviar_datos(consola_asociada->socket_consola, datos); //TODO: BUUUUUGGGG!!!!! NO ENCUENTRA
	//LA CPU ME ESTA MANDANDO PID 0 Y FIJARSE SI ME ESTA MANDANDO LA CADENA COMO UN INT O UN CHAR
	//SI ES UN NRO
	free(datos->datos);
	free(datos);
}

/*Se hace un wait del mutex de entrada_salida al hacerse la solicitud de entrada y un post
 cuando se le devuelve la entrada a la CPU, asi si otra CPU solicita entrada salida, espera a
 que se libere el recurso compartido (la estructura de entrada salida). De esta forma las demas
 CPUS que hagan otras solicitudes pueden ser planificadas*/
void producir_entrada_estandar(int pid, char id_tipo, int socket_CPU,
		int tamanio) {

	TCB_struct * tcb = obtener_tcbEjecutando(pid);
	if (tcb != NULL ) {
		instruccion_protegida("Entrada_Estandar",
				(t_hilo*) obtener_hilo_asociado(tcb));
	} else {
		printf("NO se encontro tcb\n");
	}

	sem_wait(&mutex_entradaSalida);
	printf("DESPUES DEL SEMAFORO DE ENTRADA SALIDA \n");
	entrada = malloc(sizeof(entrada_salida));
	entrada->cadena = malloc(tamanio);
	memcpy(&entrada->socket_CPU, &socket_CPU, sizeof(int));

	void * buffer = malloc(1 + sizeof(int));
	memcpy(buffer, &id_tipo, sizeof(int));
	memcpy(buffer + 1, &tamanio, sizeof(int));
	//Enviando la solicitud a la consola para el ingreso de datos

	printf("ENVIANDO DATOS A LA CONSOLA PARA ENTRADA ESTANDAR \n");
	struct_consola * consola_asociada = obtener_consolaAsociada(pid);
	t_datosAEnviar * datos_consola = crear_paquete(ingresar_cadena, buffer,
			tamanio);
	enviar_datos(consola_asociada->socket_consola, datos_consola);
	printf("DATOS ENVIADDOS a socket %d\n", consola_asociada->socket_consola);
	free(datos_consola);
	free(buffer);

}

/*Esta funcion es invocada cuando la consola manda el mensaje de que ya se ingresaron los datos*/
void devolver_entrada_aCPU(int tamanio_datos) {
	struct_CPU * CPU_asociada = obtener_CPUAsociada(entrada->socket_CPU);
	char* entradaAmostrar = malloc(tamanio_datos + 1);
	memcpy(entradaAmostrar, (char*)entrada->cadena, tamanio_datos );
	entradaAmostrar[tamanio_datos] = '\0';


	t_datosAEnviar * datos = crear_paquete(devolucion_cadena, entradaAmostrar,
			tamanio_datos + 1);
	printf("Devolviendo la cadena que es: %s y tiene largo %d\n",
			(char*) entradaAmostrar, tamanio_datos);
	enviar_datos(CPU_asociada->socket_CPU, datos);
	free(datos);
	free(entrada->cadena);
	free(entrada);
	sem_post(&mutex_entradaSalida);
}

void realizar_join(TCB_struct * tcb, int tid_a_esperar) {

	struct_join * estructura = malloc(sizeof(struct_join));
	estructura->tid_a_esperar = tid_a_esperar;
	estructura->tcb_llamador = tcb_ejecutandoSysCall;
	list_add(hilos_join, estructura);
	loguear(BLOCK, tcb);
}

/*Los bloqueados por recurso se manejan con un diccionario con colas. Cada recurso es una entrada en el diccionario.
 * Si existe la entrada de ese recurso, se obtiene la cola y se le agrega el tcb. De otra forma, se crea una nueva
 * cola y se la inserta en una nueva entrada en el diccionario.*/
void realizar_bloqueo(TCB_struct * tcb, int id_recurso) {

	char * recurso = string_itoa(id_recurso);

	if (dictionary_has_key(dic_bloqueados, recurso)) {
		t_queue * bloqueados_por_recurso = dictionary_get(dic_bloqueados,
				recurso);
		queue_push(bloqueados_por_recurso, tcb);
	} else {
		t_queue * nuevo_bloqueado = queue_create();
		queue_push(nuevo_bloqueado, tcb);
		dictionary_put(dic_bloqueados, recurso, nuevo_bloqueado);
		printf("SE AGREGO UN NUEVO RECURSO A BLOQUEAR %d\n", id_recurso);
	}
	loguear(BLOCK, tcb);
}

void realizar_desbloqueo(int id_recurso) {

	char * recurso = string_itoa(id_recurso);

	if (dictionary_has_key(dic_bloqueados, recurso)) {
		t_queue * bloqueados_por_recurso = dictionary_get(dic_bloqueados,
				recurso);
		TCB_struct * tcb = queue_pop(bloqueados_por_recurso);
		meter_en_ready(1, tcb);
	} else {
		//NO EXISTEN BLOQUEADOS DE ESE RECURSO, SIGUE COMO SI NADA
	}
}

int chequear_proceso_abortado(TCB_struct * tcb) {
	struct_consola * consola = obtener_consolaAsociada(tcb->PID);
	if (consola->termino_ejecucion) {
		printf("EL PROCESO FUE ABORTADO \n");
		sacar_de_ejecucion(tcb, false);
		if (consola->cantidad_hilos == 0) {

			destruir_segmento_MSP(consola->PID, consola->M);

			t_datosAEnviar * paquete = crear_paquete(terminar_conexion, NULL,
					0);
			if (enviar_datos(consola->socket_consola, paquete) < 0) {
				return -1;
			}
			free(paquete);
			return -1;
		}
		return 0;
	}
	return 0;
}

void destruir_segmento_MSP(int pid, int base_segmento) {

	sem_wait(&sem_lecturaEscritura);

	int tamanio = 2 * sizeof(int);
	void * datos = malloc(tamanio);
	memcpy(datos, &pid, sizeof(int));
	memcpy(datos + sizeof(int), &base_segmento, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(destruir_segmento, datos, tamanio);
	printf(
			"Se va a solicitar que se destruya el codigo base %d proceso %d tamanio %d\n",
			base_segmento, pid, tamanio);
	if (enviar_datos(socket_MSP, paquete) < 0) {
		printf("NO SE PUDO ENVIAR LOS DATOS A LA MSP\n");
	}
	printf("SE HA SOLICITADO!!!\n");
	free(datos);
	free(paquete->datos);
	free(paquete);

	sem_post(&sem_lecturaEscritura);

}

void mandar_a_exit(TCB_struct * tcb) {

	sem_wait(&sem_exit);
	liberar_mallocs(tcb->TID);
	destruir_segmento_MSP(tcb->PID, tcb->X);

	struct_consola * consola_asociada = obtener_consolaAsociada(tcb->PID);

	if (consola_asociada == NULL ) {
		printf(
				"LA CONSOLA ES NULA!!! Se solicito de PID %d y el tamaño de la lista es %d\n",
				tcb->PID, list_size(consola_list));
		exit(-1);
	}

	consola_asociada->cantidad_hilos--;

	queue_push(e_exit, tcb);
	loguear(EXIT, tcb);

	sem_post(&sem_exit);
}

TCB_struct * obtener_tcbEjecutando(int TID) {

	bool tiene_mismo_tid(TCB_struct * tcb) {
		return tcb->TID == TID;
	}

	return list_find(exec, (void*) tiene_mismo_tid);
}

hilo_t * obtener_hilo_asociado(TCB_struct * tcb_h) {
	printf("tid del tcb a buscar %d\n", tcb_h->TID);
	bool es_hilo(hilo_t * hilo) {
		printf("buscar hilo %d\n", hilo->tcb.TID);
		return hilo->tcb.TID == tcb_h->TID;
	}

	return list_find(HILOS_SISTEMA, (void*) es_hilo);

}

void copiar_tcb(TCB_struct * tcb_destino, TCB_struct * tcb_origen) {

	tcb_destino->PID = tcb_origen->PID;
	tcb_destino->TID = tcb_origen->TID;
	tcb_destino->KM = tcb_origen->KM;
	tcb_destino->M = tcb_origen->M;
	tcb_destino->tamanioSegmentoCodigo = tcb_origen->tamanioSegmentoCodigo;
	tcb_destino->P = tcb_origen->P;
	tcb_destino->X = tcb_origen->X;
	tcb_destino->S = tcb_origen->S;
	tcb_destino->registrosProgramacion[0] =
			tcb_origen->registrosProgramacion[0];
	tcb_destino->registrosProgramacion[1] =
			tcb_origen->registrosProgramacion[1];
	tcb_destino->registrosProgramacion[2] =
			tcb_origen->registrosProgramacion[2];
	tcb_destino->registrosProgramacion[3] =
			tcb_origen->registrosProgramacion[3];
	tcb_destino->registrosProgramacion[4] =
			tcb_origen->registrosProgramacion[4];

}
