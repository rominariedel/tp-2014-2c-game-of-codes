/*
 * main.c
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#include "auxiliares/auxiliares.h"
#include "auxiliares/variables_globales.h"

#define pid_KM_boot 0

/*VARIABLES GLOBALES*/

int main(int argc, char ** argv) {

	printf("\n -------------  KERNEL  -------------\n");
	printf("    Iniciando...\n");
	crear_colas();
	iniciar_semaforos();
	obtenerDatosConfig(argv);
	TID = 0;
	PID = 0;

	pthread_t thread_boot;
	pthread_t thread_dispatcher;
	pthread_create(&thread_boot, NULL, (void*) &boot, NULL );
	pthread_create(&thread_dispatcher, NULL, (void*) &dispatcher, NULL );
	pthread_join(thread_boot, NULL );
	pthread_join(thread_dispatcher, NULL );
	config_destroy(configuracion);
	free_listas();
	return 0;
}

void iniciar_semaforos() {
	sem_init(&sem_procesoListo, 0, 0);
	sem_init(&sem_CPU, 0, 0);
	sem_init(&sem_READY, 0, 1);
	sem_init(&sem_kmDisponible, 0, 0);
	sem_init(&sem_exit, 0, 1);
	sem_init(&mutex_entradaSalida, 0, 1);
}

void obtenerDatosConfig(char ** argv) {
	configuracion = config_create(argv[1]);
	PUERTO = config_get_string_value(configuracion, "PUERTO");
	IP_MSP = config_get_string_value(configuracion, "IP_MSP");
	PUERTO_MSP = config_get_string_value(configuracion, "PUERTO_MSP");
	QUANTUM = config_get_int_value(configuracion, "QUANTUM");
	SYSCALLS = config_get_string_value(configuracion, "SYSCALLS");
	TAMANIO_STACK = config_get_int_value(configuracion, "TAMANIO_STACK");
	char * PANEL = config_get_string_value(configuracion, "PANEL");
	inicializar_panel(KERNEL, PANEL);
}

void liberar_recursos(struct_consola * consola_conectada) {

	//SI LA CONSOLA TIENE HILOS PLANIFICANDOSE O EJECUTANDOSE Y SE DESCONECTA, ABORTAN Y SE ELIMINAN SUS
	//SEGMENTOS DE STACK
	consola_conectada->termino_ejecucion = true;
	if (consola_conectada->cantidad_hilos > 0) {
		printf("Entre acaaaaa!\n");
		matar_hijos(consola_conectada->PID);
	}
	if (consola_conectada->cantidad_hilos == 0) {

		int tamanio = 2 * sizeof(int);
		void * datos = malloc(tamanio);
		memcpy(datos, &consola_conectada->PID, sizeof(int));
		memcpy(datos + sizeof(int), &consola_conectada->M, sizeof(int));
		t_datosAEnviar * paquete = crear_paquete(destruir_segmento, datos,
				tamanio);
		printf(
				"Se va a solicitar que se destruya el codigo base %d proceso %d tamanio %d\n",
				consola_conectada->M, consola_conectada->PID, tamanio);
		enviar_datos(socket_MSP, paquete);
		printf("SE HA SOLICITADO!!!\n");
		free(datos);
		free(paquete->datos);
		free(paquete);
	}

}

void loader() {
	fd_set copia_set;
	while (1) {
		struct timeval * timeout = malloc(sizeof(struct timeval));
		timeout->tv_sec = 4;
		timeout->tv_usec = 0;
		copia_set = consola_set;

		int i = select(descriptor_mas_alto_consola + 1, &copia_set, NULL, NULL,
				timeout);
		if (i == -1) {
			//ERROR
			exit(-1);
		}
		free(timeout);

		int n_descriptor = 0;

		while (n_descriptor <= descriptor_mas_alto_consola) {
			if (FD_ISSET(n_descriptor, &copia_set)) {
				struct_consola * consola_conectada = obtener_consolaConectada(
						n_descriptor);
				if (consola_conectada == NULL ) {
					break;
				}
				t_datosAEnviar * datos;
				datos = recibir_datos(n_descriptor);
				if (datos == NULL || datos->codigo_operacion == 0) {
					desconexion_consola(n_descriptor);
					liberar_recursos(consola_conectada);
					FD_CLR(n_descriptor, &consola_set);
					break;
				}
				TCB_struct * nuevoTCB;

				int * aux;
				switch (datos->codigo_operacion) {

				case codigo_consola:
					printf("COD OPERACION: CODIGO_CONSOLA\n");
					nuevoTCB = malloc(sizeof(TCB_struct));
					nuevoTCB->PID = consola_conectada->PID;
					nuevoTCB->TID = obtener_TID();
					consola_conectada->TID_padre = nuevoTCB->TID;
					printf("ESTOY SOLICITANDO SEGMENTO \n");
					int segmento_codigo = solicitar_segmento(nuevoTCB,
							datos->tamanio);
					printf("ME MANDO UNA BASE %d\n", segmento_codigo);
					if (segmento_codigo < 0) {
						exit(0);
					}
					consola_conectada->M = segmento_codigo;
					int segmento_stack = solicitar_segmento(nuevoTCB,
							TAMANIO_STACK);
					if (segmento_stack < 0) {
						exit(-1);
					}

					escribir_memoria(nuevoTCB, segmento_codigo, datos->tamanio,
							datos->datos);

					nuevoTCB->KM = 0;
					nuevoTCB->M = segmento_codigo;
					nuevoTCB->tamanioSegmentoCodigo = datos->tamanio;
					nuevoTCB->P = segmento_codigo;
					nuevoTCB->X = segmento_stack;
					nuevoTCB->S = segmento_stack;
					nuevoTCB->registrosProgramacion[0] = 0;
					nuevoTCB->registrosProgramacion[1] = 0;
					nuevoTCB->registrosProgramacion[2] = 0;
					nuevoTCB->registrosProgramacion[3] = 0;
					nuevoTCB->registrosProgramacion[4] = 0;
					printf("\nSe inicializo el TCB PADRE\n");
					meter_en_ready(1, nuevoTCB);
					consola_conectada->cantidad_hilos = 1;
					break;

				case se_produjo_entrada:
					aux = malloc(sizeof(int));
					memcpy(aux, &datos->tamanio, sizeof(int));
					memcpy(entrada->cadena, datos->datos, datos->tamanio);
					devolver_entrada_aCPU(*aux);

					break;
				}
				free(datos);
			}
			n_descriptor = n_descriptor + 1;
		}

	}
}

void handshake_MSP(int socketMSP) {
	t_datosAEnviar * datos = crear_paquete(soy_kernel, NULL, 0);
	enviar_datos(socketMSP, datos);
	free(datos);
}

void boot() {

	printf("\n    INICIANDO BOOT   \n");

	socket_gral = crear_servidor(PUERTO, backlog);
	if (socket_gral < 0) {
		printf("No se pudo crear el servidor\n");
		exit(-1);
	}
	printf("Se ha creado el servidor exitosamente\n");

	char * syscalls = extraer_syscalls(SYSCALLS);

	printf("\n      CONECTANDO CON LA MSP\n");
	socket_MSP = crear_cliente(IP_MSP, PUERTO_MSP);
	if (socket_MSP < 0) {
		printf("FALLO al conectar con la MSP\n");
		exit(-1);
	}

	handshake_MSP(socket_MSP);
	printf("\n Solicitando segmentos principales en la MSP\n");

	TCB_struct * tcb_km = malloc(sizeof(TCB_struct));
	tcb_km->KM = 1;

	int base_segmento_codigo = solicitar_segmento(tcb_km,
			tamanio_codigo_syscalls);
	if (base_segmento_codigo < 0) {
		printf("Error al solicitar segmento de codigo para las syscalls");
		exit(-1);
	}
	escribir_memoria(tcb_km, base_segmento_codigo,
			(int) tamanio_codigo_syscalls, (void*) syscalls);
	free(syscalls);

	int base_segmento_stack = solicitar_segmento(tcb_km, TAMANIO_STACK);
	if (base_segmento_stack < 0) {
		printf("Error al solicitar segmento de stack para las syscalls");
		exit(-1);
	}

	tcb_km->M = base_segmento_codigo;
	tcb_km->tamanioSegmentoCodigo = tamanio_codigo_syscalls;
	tcb_km->P = 0;
	tcb_km->PID = pid_KM_boot;
	tcb_km->S = base_segmento_stack;
	tcb_km->TID = 0;
	tcb_km->X = base_segmento_stack;

	tcb_km->registrosProgramacion[0] = 0;
	tcb_km->registrosProgramacion[1] = 0;
	tcb_km->registrosProgramacion[2] = 0;
	tcb_km->registrosProgramacion[3] = 0;
	tcb_km->registrosProgramacion[4] = 0;

	queue_push(block.prioridad_0, (void *) tcb_km);
	sem_post(&sem_kmDisponible);

	printf("Esperando conexiones...\n");

	FD_ZERO(&consola_set);
	FD_ZERO(&CPU_set);
	pthread_t thread_planificador;
	pthread_t thread_loader;

	descriptor_mas_alto_consola = 0;
	descriptor_mas_alto_cpu = 0;

	while (1) {

		int socket_conectado = recibir_conexion(socket_gral);
		int modulo_conectado = -1;
		t_datosAEnviar * datos = recibir_datos(socket_conectado);
		modulo_conectado = datos->codigo_operacion;

		if (modulo_conectado == soy_consola) {
			conexion_consola(socket_conectado);
			FD_SET(socket_conectado, &consola_set);
			struct_consola * consola_conectada = malloc(sizeof(struct_consola));
			int pid = obtener_PID();
			consola_conectada->PID = pid;
			consola_conectada->socket_consola = socket_conectado;
			consola_conectada->cantidad_hilos = 0;
			list_add(consola_list, consola_conectada);
			if (descriptor_mas_alto_consola == 0) {
				descriptor_mas_alto_consola = socket_conectado;
				pthread_create(&thread_loader, NULL, (void*) &loader, NULL );
			}
			if (descriptor_mas_alto_consola < socket_conectado) {
				descriptor_mas_alto_consola = socket_conectado;
			}

		} else if (modulo_conectado == soy_CPU) {
			conexion_cpu(socket_conectado);
			FD_SET(socket_conectado, &CPU_set);
			struct_CPU* cpu_conectada = malloc(sizeof(struct_CPU));
			cpu_conectada->PID = -1;
			cpu_conectada->bit_estado = libre;
			cpu_conectada->socket_CPU = socket_conectado;
			list_add(CPU_list, cpu_conectada);
			printf("LISTA DE CPUS ");
			hilos(CPU_list); //TODO: SACAR ESTE LOGUEO!!!!
			sem_post(&sem_CPU);
			if (descriptor_mas_alto_cpu == 0) {
				descriptor_mas_alto_cpu = socket_conectado;
				pthread_create(&thread_planificador, NULL,
						(void*) &planificador, NULL );
			}
			if (descriptor_mas_alto_cpu < socket_conectado) {
				descriptor_mas_alto_cpu = socket_conectado;
			}
		}

		free(datos->datos);
		free(datos);

	}
}

void interrumpir(TCB_struct * tcb, int dirSyscall) {
	queue_push(SYS_CALL, tcb);
	struct_bloqueado * tcb_bloqueado = malloc(sizeof(struct_bloqueado));
	tcb_bloqueado->id_recurso = dirSyscall;
	tcb_bloqueado->tcb = *tcb;
	list_add(block.prioridad_1, tcb_bloqueado);
	hilos(block.prioridad_1);
	sem_post(&sem_procesoListo);
	sem_post(&sem_procesoListo);
}

void crear_hilo(TCB_struct tcb, int socketCPU) {

	TCB_struct * nuevoTCB = malloc(sizeof(TCB_struct));

	int base_stack = solicitar_segmento(&tcb, TAMANIO_STACK);

	nuevoTCB->PID = tcb.PID;
	int tid = obtener_TID();
	nuevoTCB->X = base_stack;
	nuevoTCB->S = base_stack;
	nuevoTCB->KM = 0;
	nuevoTCB->PID = tcb.PID;
	nuevoTCB->TID = tid;
	nuevoTCB->M = tcb.M;
	nuevoTCB->tamanioSegmentoCodigo = tcb.tamanioSegmentoCodigo;
	nuevoTCB->P = tcb.P;
	copiarRegistros(nuevoTCB->registrosProgramacion, tcb.registrosProgramacion);

	t_datosAEnviar * datos = crear_paquete(0, nuevoTCB, sizeof(TCB_struct));
	enviar_datos(socketCPU, datos);

}

void planificar_hilo_creado(TCB_struct * nuevoTCB) {

	meter_en_ready(1, nuevoTCB);

	//Indico que la cantidad de hilos de un proceso aumentó
	struct_consola * consola_asociada = obtener_consolaAsociada(nuevoTCB->PID);
	consola_asociada->cantidad_hilos++;

}

void copiarRegistros(int registro1[5], int registro2[5]) {
	int n = 5;
	while (n >= 0) {
		registro1[n - 1] = registro2[n - 1];
		n--;
	}
}

/*Esta operacion le solicita a la MSP un segmento, retorna la direccion base del
 * segmento reservado*/
int solicitar_segmento(TCB_struct * tcb, int tamanio_del_segmento) {

	char * datos = malloc(2 * sizeof(int));
	memcpy(datos, &(tcb->PID), sizeof(int));
	memcpy(datos + sizeof(int), &tamanio_del_segmento, sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(crear_segmento, (void*) datos,
			2 * sizeof(int));

	enviar_datos(socket_MSP, paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socket_MSP);
	if (respuesta == NULL ) {
		printf("no se recibio una respuesta\n");
	}
	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta->datos, sizeof(int));
	free(respuesta->datos);
	free(respuesta);
	if ((*dir_base == error_memoriaLlena) || (*dir_base == error_general)) {
		struct_consola * consola_asociada = obtener_consolaAsociada(tcb->PID);
		//Copio el tid padre en el tcb para poder terminar la ejecucion del ese tid y que en consecuencia
		//aborte toodo el proceso
		tcb->TID = consola_asociada->TID_padre;
		abortar(tcb);
		return -1;
	}
	printf("Se recibio la direccion base: %d\n", *dir_base);

	return *dir_base;
}

void escribir_memoria(TCB_struct * tcb, int dir_logica, int tamanio,
		void * bytes) {

	char * datos = malloc((3 * sizeof(int)) + tamanio);

	memcpy(datos, &tcb->PID, sizeof(int));
	memcpy(datos + sizeof(int), &dir_logica, sizeof(int));
	memcpy(datos + (2 * sizeof(int)), bytes, tamanio);
	memcpy(datos + (2 * sizeof(int)) + tamanio, &tamanio, sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(escribir_en_memoria, datos,
			(3 * sizeof(int)) + tamanio);
	enviar_datos(socket_MSP, paquete);
	free(datos);
	free(paquete);

	paquete = recibir_datos(socket_MSP);
	if ((paquete->codigo_operacion == error_segmentationFault)
			|| (paquete->codigo_operacion == error_general)) {
		struct_consola * consola_asociada = obtener_consolaAsociada(tcb->PID);
		//Copio el tid padre en el tcb para poder terminar la ejecucion del ese tid y que en consecuencia
		//aborte toodo el proceso
		tcb->TID = consola_asociada->TID_padre;
		abortar(tcb);
	}

}

void finalizo_quantum(TCB_struct* tcb) {

	bool es_TCB(TCB_struct tcb_comparar) {
		return (tcb_comparar.PID == PID) && (tcb_comparar.TID == TID);
	}
	TCB_struct * tcb_exec = list_remove_by_condition(exec, (void*) es_TCB);
	free(tcb_exec);
	printf("Se saco el tcb de la cola de ejecucion\n");
	if (chequear_proceso_abortado(tcb) < 0) {
		//EL PADRE DE ESE PROCESO TERMINO
		exit(0);
	}
	meter_en_ready(1, tcb);

}

void sacar_de_ejecucion(TCB_struct* tcb, bool waitear) {

	int PID = tcb->PID;
	int TID = tcb->TID;
	bool es_TCB(TCB_struct tcb_comparar) {
		return (tcb_comparar.PID == PID) && (tcb_comparar.TID == TID);
	}
	TCB_struct * tcb_exec = list_remove_by_condition(exec, (void*) es_TCB);
	hilos(exec);
	free(tcb_exec);

	/* Fijarse si hay algun hilo esperando joins
	 Si es el padre hay que abortar los hilos (hay que fijarse si el tid es el mismo que tiene guardada la consola
	 asociada).
	 Si hay una asociada, hay que eliminar el nodo y fijarse si el tid llamador no esta esperando a otro hilo.
	 (Creo que no puede estar esperando a otro hilo, este caso no esta implementado)
	 Si no está esperando a otro habria que consultar si va a exit o a ejecucion nuevamente.*/

	fijarse_joins(TID);
	struct_consola * consola_asociada = obtener_consolaAsociada(PID);
	if (consola_asociada->TID_padre == TID) {
		consola_asociada->termino_ejecucion = true;
		matar_hijos(PID);
	}
	mandar_a_exit(tcb);

	if (consola_asociada->cantidad_hilos == 0) {

		int tamanio = 2 * sizeof(int);
		void * datos = malloc(tamanio);
		memcpy(datos, &consola_asociada->PID, sizeof(int));
		memcpy(datos + sizeof(int), &consola_asociada->M, sizeof(int));
		t_datosAEnviar * paquete = crear_paquete(destruir_segmento, datos,
				tamanio);
		printf(
				"Se va a solicitar que se destruya el codigo base %d proceso %d tamanio %d\n",
				consola_asociada->M, consola_asociada->PID, tamanio);
		enviar_datos(socket_MSP, paquete);
		printf("SE HA SOLICITADO!!!\n");
		free(datos);
		free(paquete->datos);
		free(paquete);

		paquete = crear_paquete(terminar_conexion, NULL, 0);
		if (enviar_datos(consola_asociada->socket_consola, paquete) < 0) {

		}
		free(paquete);
		printf("DESCONECTE LA CONSOLA ASOCIADA\n");
	}else if(waitear && !(consola_asociada->cantidad_hilos < 1)){

		sem_post(&sem_CPU);
	}


}

void matar_hijos(int PID) {
	printf("cantidad elementos ready %d\n", queue_size(ready.prioridad_1));
	matar_hijos_en_lista(PID, ready.prioridad_1->elements, true, false);
	matar_hijos_en_lista(PID, block.prioridad_1, false, true);
	matar_hijos_en_lista(PID, SYS_CALL->elements, false, false);
	matar_hijos_en_lista(PID, hilos_join, false, false);
	matar_hijo_en_diccionario(PID);
}

void matar_hijo_en_diccionario(int PID) {
	void matar_hijo(t_queue * bloqueado_por_recurso) {
		matar_hijos_en_lista(PID, bloqueado_por_recurso->elements, false,
				false);
	}
	dictionary_iterator(dic_bloqueados, (void*) matar_hijo);
}

void matar_hijos_en_lista(int PID, t_list* lista, bool es_ready,
		bool es_bloqueado) {
	int cantidad = 0;
	printf("1\n");
	bool tiene_mismo_pid(TCB_struct * tcb) {
		printf("PID DEL ELEMENTO %d\n", tcb->PID);
		return tcb->PID == PID;
	}
	bool tiene_mismo_pid_block(struct_bloqueado * block) {
		return block->tcb.PID == PID;
	}
	if (!es_bloqueado) {
		cantidad = list_count_satisfying(lista, (void*) tiene_mismo_pid);
	} else {
		cantidad = list_count_satisfying(lista, (void*) tiene_mismo_pid_block);
	}
	printf(
			"Cantidad de hijos que hay en la lista: %d\n Cantidad de elementos en la lista: %d\n",
			cantidad, list_size(lista));
	printf("PID A ELIMINAR %d\n", PID);
	printf("2\n");
	int contador = 0;
	while (contador < cantidad) {
		if (es_ready) {
			printf("3\n");
			sem_wait(&sem_procesoListo);
		}
		printf("4\n");
		TCB_struct * tcb;
		struct_bloqueado * block;
		if (!es_bloqueado) {
			tcb = list_remove_by_condition(lista, (void*) tiene_mismo_pid);
		} else {
			block = list_remove_by_condition(lista,
					(void*) tiene_mismo_pid_block);
			tcb = &block->tcb;
			printf("QUEDO UN PID: %d\n", tcb->PID);
		}
		mandar_a_exit(tcb);

		contador++;
	}

}

void fijarse_joins(int tid) {

	bool esta_esperando_tid(struct_join *estructura) {
		return estructura->tid_a_esperar == tid;
	}
	t_list * lista_bloqueados_por_tid = list_filter(hilos_join,
			(void*) esta_esperando_tid);

	void desbloquear_por_join(struct_join * estructura) {
		TCB_struct * tcb_bloqueado = malloc(sizeof(TCB_struct));
		memcpy(tcb_bloqueado, estructura->tcb_llamador, sizeof(TCB_struct));
		meter_en_ready(1, tcb_bloqueado);
	}

	if (lista_bloqueados_por_tid != NULL ) {
		list_map(hilos_join, (void*) desbloquear_por_join);
		list_destroy_and_destroy_elements(lista_bloqueados_por_tid, free);
		//Como el filter contiene referencias a estructuras que contiene hilos_join, con liberar los elementos del
		//filter tendrían que dejar de estar los elementos en hilos_join
	}
}

void finalizo_ejecucion(TCB_struct *tcb) {

	if (tcb->KM == 1) {
		queue_push(block.prioridad_0, tcb);
		struct_bloqueado * tcb_bloqueado = obtener_bloqueado(tcb->TID);
		tcb_bloqueado->tcb.registrosProgramacion[0] =
				tcb->registrosProgramacion[0];
		tcb_bloqueado->tcb.registrosProgramacion[1] =
				tcb->registrosProgramacion[1];
		tcb_bloqueado->tcb.registrosProgramacion[2] =
				tcb->registrosProgramacion[2];
		tcb_bloqueado->tcb.registrosProgramacion[3] =
				tcb->registrosProgramacion[3];
		tcb_bloqueado->tcb.registrosProgramacion[4] =
				tcb->registrosProgramacion[4];

		sem_post(&sem_kmDisponible);
		meter_en_ready(1, &tcb_bloqueado->tcb);
	} else {
		sacar_de_ejecucion(tcb, true);
	}
}

void enviar_a_ejecucion(TCB_struct * tcb) {

	struct_CPU* cpu = list_find(CPU_list, (void*) CPU_esta_libre);
	if (cpu == NULL ) {
		printf("FALLO. NO SE ENCONTRO CPU\n");
		exit(-1);
	}
	cpu->PID = tcb->PID;
	cpu->TID = tcb->TID;

	if (tcb == NULL ) {
		printf("EL TCB ES NULO!!!\n");
		exit(-1);
	}
	list_add(exec, tcb);
	printf("EXEC ");
	hilos(exec);
	void * mensaje = malloc(sizeof(TCB_struct) + sizeof(int));
	memcpy(mensaje, tcb, sizeof(TCB_struct));
	memcpy(mensaje + sizeof(TCB_struct), &QUANTUM, sizeof(int));
	printf("Se esta por enviar el hilo a ejecutar\n");
	t_datosAEnviar * paquete = crear_paquete(ejecutar, mensaje,
			sizeof(TCB_struct) + sizeof(int));
	enviar_datos(cpu->socket_CPU, paquete);
	cpu->bit_estado = ocupado;
}

/*El dispatcher se encarga tanto de las llamadas al sistema como de los procesos que estan en la cola de ready*/
void dispatcher() {

	while (1) {
		sem_wait(&sem_procesoListo);
		sem_wait(&sem_CPU);
		sem_wait(&sem_procesoListo);
		printf("\nSe detectó un nuevo proceso!\n");
		if (!queue_is_empty(SYS_CALL)) {
			tcb_ejecutandoSysCall = (TCB_struct*) queue_pop(SYS_CALL);

			struct_bloqueado * tcb_bloqueado = obtener_bloqueado(
					tcb_ejecutandoSysCall->TID);
			sem_wait(&sem_kmDisponible);
			TCB_struct * tcb_km = queue_pop(block.prioridad_0);
			printf("BLOQUEADOS PRIORIDAD 0 ");
			hilos(block.prioridad_0->elements);
			copiarRegistros(tcb_km->registrosProgramacion,
					tcb_ejecutandoSysCall->registrosProgramacion);

			tcb_km->PID = tcb_ejecutandoSysCall->PID;
			tcb_km->TID = tcb_ejecutandoSysCall->TID;
			tcb_km->P = tcb_bloqueado->id_recurso;
			printf("COPIO EL PUNTERO DE INSTRUCCION: %d\n",
					tcb_bloqueado->id_recurso);
			free(tcb_ejecutandoSysCall);
			enviar_a_ejecucion(tcb_km);

		} else {
			TCB_struct * tcb;

			if (!queue_is_empty(ready.prioridad_0)) {
				tcb = sacar_de_ready(0);
			} else {
				tcb = sacar_de_ready(1);
			}
			enviar_a_ejecucion(tcb);
		}
	}

}

