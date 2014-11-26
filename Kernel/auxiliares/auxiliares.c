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
	int aux = TID;
	aux++;
	return aux;
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

void meter_en_ready(int prioridad, TCB_struct * tcb){
	sem_wait(&sem_READY);
	switch(prioridad){
	case 0:
		queue_push(ready.prioridad_0, tcb);
		break;
	case 1:
		printf("Se puso un elemento en la cola de ready prioridad 1!!!");
		queue_push(ready.prioridad_1, tcb);
		printf("cantidad elementos ready %d\n",queue_size(ready.prioridad_1));
		break;
	}
	sem_post(&sem_READY);
	sem_post(&sem_procesoListo);
}

TCB_struct * sacar_de_ready(int prioridad){
	printf("SE SOLICITO SACAR UN ELEMENTO DE READY!!!!\n");
	sem_wait(&sem_READY);
	TCB_struct * tcb = NULL;
	switch(prioridad){
	case 0:
		tcb = queue_pop(ready.prioridad_0);
		break;
	case 1:
		tcb = queue_pop(ready.prioridad_1);
		printf("Se saco un elemento prioridad 1 :(\n");
		break;
	}
	sem_post(&sem_READY);
	return tcb;
}

void liberar_cpu(int socket){
	struct_CPU * cpu = obtener_CPUAsociada(socket);
	cpu->bit_estado = libre;

	bool tiene_mismo_tid(TCB_struct * tcb){
		return tcb->TID == cpu->TID;
	}

	list_remove_by_condition(exec, (void*)tiene_mismo_tid);
	cpu->PID = -1;

	sem_post(&sem_CPU);
}

TCB_struct * obtener_tcbEnEjecucion(int TID){
	bool tiene_mismo_tid(TCB_struct * tcb){
		return tcb->TID == TID;
	}
	return list_remove_by_condition(exec, (void*)tiene_mismo_tid);
}


void abortar(TCB_struct* tcb) {
	sacar_de_ejecucion(tcb);
	//LOGUEAR que tuvo que abortar el hilo
}

void desconecto_cpu(int socket){
	//sem_wait(&sem_CPU); //TODO: ARREGLAR LA SINCRO ACA
	struct_CPU * cpu = obtener_CPUAsociada(socket);
	if(cpu == NULL){
		printf("No se encontro la CPU\n");
		exit(-1);
	}
	if(cpu->PID >= 0){

		struct_consola * consola = obtener_consolaAsociada(cpu->PID);
		TCB_struct * tcb = obtener_tcbEnEjecucion(cpu->TID);

		if(tcb->KM){
			void abortar_procesos(struct_consola * consola){
				t_datosAEnviar * datos = crear_paquete(terminar_conexion, NULL, 0);
				enviar_datos(consola->socket_consola, datos);
				free(datos);
			}
			list_iterate(consola_list, (void*)abortar_procesos);
		}else{
			t_datosAEnviar * datos = crear_paquete(se_desconecto_cpu, NULL, 0);
			enviar_datos(consola->socket_consola, datos);
			free(datos);

		}
		struct_bloqueado * bloqueado = malloc(sizeof(struct_bloqueado));
		bloqueado->tcb = *tcb;
		bloqueado->id_recurso = -1;
		list_add(block.prioridad_1, bloqueado);



		//TODO: NOTIFICAR CONSOLA
		//ABORTAR LA EJECUCION
		//CONTEMPLAR SI ESTA EJECUTANDO TCB KM
	}
	free(cpu);
}

void planificador() {

	fd_set copia_set;
	while (1) {

		struct timeval * timeout = malloc(sizeof(struct timeval));
		timeout->tv_sec = 4;
		timeout->tv_usec = 0;

		copia_set = CPU_set;
		int i = select(descriptor_mas_alto_cpu + 1, &copia_set, NULL, NULL,
				NULL );

		if (i == -1) {
			//error
			break;
		}
		free(timeout);
		int n_descriptor = 0;

		while (n_descriptor <= descriptor_mas_alto_cpu) {

			if (FD_ISSET(n_descriptor, &copia_set)) {
				t_datosAEnviar * datos;
				datos = recibir_datos(n_descriptor);

				if(datos == NULL){
					desconexion_cpu(n_descriptor);
					desconecto_cpu(n_descriptor);
					FD_CLR(n_descriptor, &CPU_set);
					break;
				}

				int codigo_operacion = datos->codigo_operacion;

				TCB_struct* tcb = malloc(sizeof(TCB_struct));
				int dirSysCall, tamanio, pid, tid_a_esperar,
						id_recurso;
				char * cadena;
				char * id_tipo;

				sem_init(&mutex_entradaSalida, 0, 1);
				printf("CODIGO OPERACION : %d\n", codigo_operacion);

				switch (codigo_operacion) {

				case finaliza_quantum:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					liberar_cpu(n_descriptor);
					finalizo_quantum(tcb);
					break;
				case finaliza_ejecucion:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					liberar_cpu(n_descriptor);
					finalizo_ejecucion(tcb);
					break;
				case ejecucion_erronea:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					liberar_cpu(n_descriptor);
					abortar(tcb);
					break;
				case interrupcion:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					memcpy(&dirSysCall, datos->datos + sizeof(TCB_struct),
							sizeof(int));
					liberar_cpu(n_descriptor);
					printf("LLEGO LA DIRECCIONNANANANANANANANANANA: %d\n", dirSysCall);
					interrumpir(tcb, dirSysCall);
					break;
				case creacion_hilo:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					crear_hilo(*tcb, n_descriptor);
					break;
				case planificar_nuevo_hilo: //Aca llega el TCB listo para planificar con su stack inicializado
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					planificar_hilo_creado(tcb);
					break;
				case entrada_estandar:
					id_tipo = malloc(sizeof(int));
					memcpy(&tamanio, datos->datos, sizeof(int));
					memcpy(&pid, datos->datos + sizeof(int), sizeof(int));
					memcpy(id_tipo, datos->datos + (2*sizeof(int)), sizeof(int));
					producir_entrada_estandar(pid, id_tipo, n_descriptor,
							tamanio);

					break;
				case salida_estandar:
					cadena = malloc(datos->tamanio - sizeof(int));
					memcpy(&pid, datos->datos, sizeof(int));
					memcpy(cadena, datos->datos + sizeof(int),
							datos->tamanio - sizeof(int));
					producir_salida_estandar(pid, cadena);

					break;
				case join:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					memcpy(&tid_a_esperar, datos->datos + sizeof(int),
							sizeof(int));
					realizar_join(tcb, tid_a_esperar);

					break;
				case bloquear:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					memcpy(&id_recurso, datos->datos + sizeof(TCB_struct),
							sizeof(int));
					liberar_cpu(n_descriptor);
					realizar_bloqueo(tcb, id_recurso);

					break;
				case despertar:
					memcpy(&id_recurso, datos->datos, sizeof(int));
					realizar_desbloqueo(id_recurso);

					break;
			}
				free(datos);

			}
			n_descriptor++;
		}

	}
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

void producir_salida_estandar(int pid, char* cadena) {
	struct_consola * consola_asociada = obtener_consolaAsociada(pid);
	t_datosAEnviar * datos = crear_paquete(imprimir_en_pantalla, cadena, string_length(cadena));

	enviar_datos(consola_asociada->socket_consola, datos);
	free(datos->datos);
	free(datos);
}

	/*Se hace un wait del mutex de entrada_salida al hacerse la solicitud de entrada y un post
	cuando se le devuelve la entrada a la CPU, asi si otra CPU solicita entrada salida, espera a
	que se libere el recurso compartido (la estructura de entrada salida). De esta forma las demas
	CPUS que hagan otras solicitudes pueden ser planificadas*/
void producir_entrada_estandar(int pid, char * id_tipo, int socket_CPU,
		int tamanio) {


	sem_wait(&mutex_entradaSalida);
	entrada = malloc(sizeof(entrada_salida));
	entrada->cadena = malloc(tamanio);
	memcpy(&entrada->socket_CPU, &socket_CPU, sizeof(int));

	void * buffer = malloc(1 + sizeof(int));
	memcpy(buffer, id_tipo, 1);
	memcpy(buffer + 1, &tamanio, sizeof(int));
	//Enviando la solicitud a la consola para el ingreso de datos
	struct_consola * consola_asociada = obtener_consolaAsociada(pid);
	t_datosAEnviar * datos_consola = crear_paquete(ingresar_cadena, buffer,
			tamanio);
	enviar_datos(consola_asociada->socket_consola, datos_consola);
	free(datos_consola);
	free(buffer);

}

	/*Esta funcion es invocada cuando la consola manda el mensaje de que ya se ingresaron los datos*/
void devolver_entrada_aCPU(int tamanio_datos) {
	struct_CPU * CPU_asociada = obtener_CPUAsociada(entrada->socket_CPU);
	t_datosAEnviar * datos = crear_paquete(devolucion_cadena, entrada->cadena,
			tamanio_datos + 1);
	printf("Devolviendo la cadena que es: %s y tiene largo %d\n", (char*)entrada->cadena, tamanio_datos);
	enviar_datos(CPU_asociada->socket_CPU, datos);
	free(datos);
	free(entrada->cadena);
	free(entrada);
	sem_post(&mutex_entradaSalida);
}

void realizar_join(TCB_struct * tcb, int tid_a_esperar) {
	struct_join * estructura = malloc(sizeof(struct_join));
	estructura->tid_a_esperar = tid_a_esperar;
	estructura->tcb_llamador = tcb;
	list_add(hilos_join, estructura);
}

/*Los bloqueados por recurso se manejan con un diccionario con colas. Cada recurso es una entrada en el diccionario.
 * Si existe la entrada de ese recurso, se obtiene la cola y se le agrega el tcb. De otra forma, se crea una nueva
 * cola y se la inserta en una nueva entrada en el diccionario.*/
void realizar_bloqueo(TCB_struct * tcb, int id_recurso){

	char * recurso = string_itoa(id_recurso);

	if(dictionary_has_key(dic_bloqueados, recurso)){
		t_queue * bloqueados_por_recurso = dictionary_get(dic_bloqueados, recurso);
		queue_push(bloqueados_por_recurso, tcb);
	}else{
		t_queue * nuevo_bloqueado = queue_create();
		queue_push(nuevo_bloqueado, tcb);
		dictionary_put(dic_bloqueados, recurso, nuevo_bloqueado);
		printf("SE AGREGO UN NUEVO RECURSO A BLOQUEAR %d\n", id_recurso);
	}
}

void realizar_desbloqueo(int id_recurso){

	char * recurso = string_itoa(id_recurso);

	if(dictionary_has_key(dic_bloqueados, recurso)){
		t_queue * bloqueados_por_recurso = dictionary_get(dic_bloqueados, recurso);
		TCB_struct * tcb = queue_pop(bloqueados_por_recurso);
		meter_en_ready(1, tcb);
	}else{
		//NO EXISTEN BLOQUEADOS DE ESE RECURSO
		exit(-1);
	}
}

int chequear_proceso_abortado(TCB_struct * tcb){
	struct_consola * consola = obtener_consolaAsociada(tcb->PID);
	if(consola->termino_ejecucion){
		mandar_a_exit(tcb);
		return -1;
	}
	return 0;
}

void mandar_a_exit(TCB_struct * tcb) {

	sem_wait(&sem_exit);

	//LIBERACION DE RECURSOS
	int tamanio = 2 * sizeof(int);
	void * datos = malloc(tamanio);
	memcpy(datos, &tcb->PID, sizeof(int));
	memcpy(datos + sizeof(int), &tcb->X, sizeof(int));
	printf("Se va a solicitar destruir otro segmento \n");
	t_datosAEnviar * paquete = crear_paquete(destruir_segmento, datos, tamanio);
	enviar_datos(socket_MSP, paquete);
	free(datos);
	free(paquete);

	struct_consola * consola_asociada = obtener_consolaAsociada(tcb->PID);

	if(consola_asociada == NULL){
		printf("LA CONSOLA ES NULA!!! Se solicito de PID %d y el tamaño de la lista es %d\n", tcb->PID, list_size(consola_list));
		exit(-1);
	}

	consola_asociada->cantidad_hilos--;


	queue_push(e_exit, tcb);

	sem_post(&sem_exit);
}

