/*
 * main.c
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

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
	sem_init(&sem_lecturaEscritura, 0, 1);
}

void obtenerDatosConfig(char ** argv) {
	configuracion = config_create(argv[1]);
	PUERTO = config_get_string_value(configuracion, "PUERTO");
	printf("PUERTO: %s\n", PUERTO);

	IP_MSP = config_get_string_value(configuracion, "IP_MSP");
	printf("IP MSP: %s\n", IP_MSP);

	PUERTO_MSP = config_get_string_value(configuracion, "PUERTO_MSP");
	printf("PUERTO MSP: %s\n", PUERTO_MSP);

	QUANTUM = config_get_int_value(configuracion, "QUANTUM");
	printf("QUANTUM: %d\n", QUANTUM);

	SYSCALLS = config_get_string_value(configuracion, "SYSCALLS");
	printf("PATH ARCHIVO SYSCALLS: %s\n", SYSCALLS);

	TAMANIO_STACK = config_get_int_value(configuracion, "TAMANIO_STACK");
	printf("TAMAÑO DEL STACK: %d\n", TAMANIO_STACK);

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

		destruir_segmento_MSP(consola_conectada->PID, consola_conectada->M);
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
					printf("NO SE ENCONTRO CONSOLA CONECTADA\n");
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
					int segmento_codigo = solicitar_crear_segmento(nuevoTCB,
							datos->tamanio);
					printf("ME MANDO UNA BASE %d\n", segmento_codigo);
					if (segmento_codigo < 0) {
						exit(0);
					}
					consola_conectada->M = segmento_codigo;
					int segmento_stack = solicitar_crear_segmento(nuevoTCB,
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
					consola_conectada->cantidad_hilos = 1;

					hilo_t * nuevo = malloc(sizeof(hilo_t));

					nuevo->tcb.PID = nuevoTCB->PID;
					nuevo->tcb.TID = nuevoTCB->TID;
					nuevo->tcb.KM = nuevoTCB->KM;
					nuevo->tcb.M = nuevoTCB->M;
					nuevo->tcb.tamanioSegmentoCodigo =
							nuevoTCB->tamanioSegmentoCodigo;
					nuevo->tcb.P = nuevoTCB->P;
					nuevo->tcb.X = nuevoTCB->X;
					nuevo->tcb.S = nuevoTCB->S;
					nuevo->tcb.registrosProgramacion[0] =
							nuevoTCB->registrosProgramacion[0];
					nuevo->tcb.registrosProgramacion[1] =
							nuevoTCB->registrosProgramacion[1];
					nuevo->tcb.registrosProgramacion[2] =
							nuevoTCB->registrosProgramacion[2];
					nuevo->tcb.registrosProgramacion[3] =
							nuevoTCB->registrosProgramacion[3];
					nuevo->tcb.registrosProgramacion[4] =
							nuevoTCB->registrosProgramacion[4];

					nuevo->cola = READY;

					list_add(HILOS_SISTEMA, nuevo);

					meter_en_ready(1, nuevoTCB);
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

	char * syscalls = extraer_syscalls(SYSCALLS);

	printf("\n      CONECTANDO CON LA MSP\n");

	printf("\n Solicitando segmentos principales en la MSP\n");

	TCB_struct * tcb_km = malloc(sizeof(TCB_struct));
	tcb_km->KM = 1;
	tcb_km->PID = pid_KM_boot;

	socket_MSP = crear_cliente(IP_MSP, PUERTO_MSP);
	if (socket_MSP < 0) {
		printf("FALLO al conectar con la MSP\n");
		exit(-1);
	}

	handshake_MSP(socket_MSP);

	int base_segmento_codigo = solicitar_crear_segmento(tcb_km,
			tamanio_codigo_syscalls);
	if (base_segmento_codigo < 0) {
		printf("Error al solicitar segmento de codigo para las syscalls\n");
		exit(-1);
	}
	escribir_memoria(tcb_km, base_segmento_codigo,
			(int) tamanio_codigo_syscalls, (void*) syscalls);
	free(syscalls);

	int base_segmento_stack = solicitar_crear_segmento(tcb_km, TAMANIO_STACK);
	if (base_segmento_stack < 0) {
		printf("Error al solicitar segmento de stack para las syscalls\n");
		exit(-1);
	}

	tcb_km->M = base_segmento_codigo;
	tcb_km->tamanioSegmentoCodigo = tamanio_codigo_syscalls;
	tcb_km->P = 0;
	tcb_km->S = base_segmento_stack;
	tcb_km->TID = 0;
	tcb_km->X = base_segmento_stack;

	tcb_km->registrosProgramacion[0] = 0;
	tcb_km->registrosProgramacion[1] = 0;
	tcb_km->registrosProgramacion[2] = 0;
	tcb_km->registrosProgramacion[3] = 0;
	tcb_km->registrosProgramacion[4] = 0;

	queue_push(block.prioridad_0, (void *) tcb_km);

	hilo_t * nuevo = malloc(sizeof(hilo_t));

	copiar_tcb(&nuevo->tcb, tcb_km);
	nuevo->tcb.PID = tcb_km->PID;
	nuevo->tcb.TID = tcb_km->TID;
	nuevo->tcb.KM = tcb_km->KM;
	nuevo->tcb.M = tcb_km->M;
	nuevo->tcb.tamanioSegmentoCodigo = tcb_km->tamanioSegmentoCodigo;
	nuevo->tcb.P = tcb_km->P;
	nuevo->tcb.X = tcb_km->X;
	nuevo->tcb.S = tcb_km->S;
	nuevo->tcb.registrosProgramacion[0] = tcb_km->registrosProgramacion[0];
	nuevo->tcb.registrosProgramacion[1] = tcb_km->registrosProgramacion[1];
	nuevo->tcb.registrosProgramacion[2] = tcb_km->registrosProgramacion[2];
	nuevo->tcb.registrosProgramacion[3] = tcb_km->registrosProgramacion[3];
	nuevo->tcb.registrosProgramacion[4] = tcb_km->registrosProgramacion[4];

	nuevo->cola = BLOCK;

	list_add(HILOS_SISTEMA, nuevo);

	loguear(BLOCK, tcb_km);

	sem_post(&sem_kmDisponible);

	t_log * logger = log_create("../panel/lg", "KERNEL", 1, LOG_LEVEL_DEBUG);

	printf("Esperando conexiones...\n");
	log_debug(logger, "ESPERANDO CONEXIONES");

	FD_ZERO(&consola_set);
	FD_ZERO(&CPU_set);
	pthread_t thread_planificador;
	pthread_t thread_loader;

	descriptor_mas_alto_consola = socket_gral;
	descriptor_mas_alto_cpu = socket_gral;

	socket_gral = crear_servidor(PUERTO, backlog);
	if (socket_gral < 0) {
		printf("No se pudo crear el servidor\n");
		exit(-1);
	}
	printf("Se ha creado el servidor exitosamente en el socket %d\n",
			socket_gral);

	while (1) {
		log_debug(logger, "Se esta esperando recibir la conexion!!!!");

		int socket_conectado = recibir_conexion(socket_gral);
		log_debug(logger, "SE RECIBIO UNA CONEXION! SOCKET: %d\n",
				socket_conectado);
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
			consola_conectada->termino_ejecucion = false;
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
			cpu_conectada->TID = -1;
			list_add(CPU_list, cpu_conectada);
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
	struct_bloqueado * tcb_bloqueado = malloc(sizeof(struct_bloqueado));
	tcb_bloqueado->id_recurso = dirSyscall;

	copiar_tcb(&tcb_bloqueado->tcb, tcb);

	list_add(block.prioridad_1, tcb_bloqueado);
	loguear(BLOCK, tcb);

	queue_push(SYS_CALL, tcb);
	sem_post(&sem_procesoListo);
	sem_post(&sem_procesoListo);
}

void crear_hilo(TCB_struct tcb, int socketCPU) {

	TCB_struct * nuevoTCB = malloc(sizeof(TCB_struct));
	printf("Por solicitarle segmento a la msp\n");
	int base_stack = solicitar_crear_segmento(&tcb, TAMANIO_STACK);
	printf("Me devolvio base %d\n", base_stack);
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
	printf("por enviarle el nuevo hilo a la cpu socket %d\n", socketCPU);
	if (enviar_datos(socketCPU, datos) < 0) {

	} else {

		printf("Datos enviados\n");
	}

}

void planificar_hilo_creado(TCB_struct * nuevoTCB) {

	hilo_t * nuevo = malloc(sizeof(hilo_t));

	copiar_tcb(&nuevo->tcb, nuevoTCB);
	nuevo->cola = READY;
	list_add(HILOS_SISTEMA, nuevo);

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
int solicitar_crear_segmento(TCB_struct * tcb, int tamanio_del_segmento) {

	sem_wait(&sem_lecturaEscritura);

	char * datos = malloc(2 * sizeof(int));
	memcpy(datos, &(tcb->PID), sizeof(int));
	memcpy(datos + sizeof(int), &tamanio_del_segmento, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(crear_segmento, (void*) datos,
			2 * sizeof(int));
	printf(
			"TAMANIO DEL SEGMENTO %d, PID %d, SOCKET MSP %d, PAQUETE CODOP %d, TAMANIO PAQUETE %d",
			tamanio_del_segmento, tcb->PID, socket_MSP,
			paquete->codigo_operacion, paquete->tamanio);

	enviar_datos(socket_MSP, paquete);
	free(datos);
	printf("SE VAN A RECIBIR DATOS EN EL SOCKET %d DE LA MSP\n", socket_MSP);
	t_datosAEnviar * respuesta = recibir_datos(socket_MSP);
	printf("RECIBI ALGO!\n");
	if (respuesta == NULL ) {
		printf("no se recibio una respuesta\n");
	}
	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta->datos, sizeof(int));
	int codop = respuesta->codigo_operacion;
	free(respuesta->datos);
	free(respuesta);

	sem_post(&sem_lecturaEscritura);

	if ((codop == error_memoriaLlena) || (codop == error_general)) {
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

	sem_wait(&sem_lecturaEscritura);

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
	printf("SE VAN A RECIBIR DATOS EN EL SOCKET %d DE LA MSP\n", socket_MSP);

	paquete = recibir_datos(socket_MSP);

	sem_post(&sem_lecturaEscritura);

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
		//exit(0);
	} else {
		meter_en_ready(1, tcb);
	}

}

void sacar_de_ejecucion(TCB_struct* tcb, bool waitear) {

	int PID = tcb->PID;
	int TID = tcb->TID;
	bool es_TCB(TCB_struct tcb_comparar) {
		return (tcb_comparar.PID == PID) && (tcb_comparar.TID == TID);
	}
	printf("SACANDO TCB DE EXEC \n");
	list_remove_and_destroy_by_condition(exec,
			(void*) es_TCB, free);

	/* Fijarse si hay algun hilo esperando joins
	 Si es el padre hay que abortar los hilos (hay que fijarse si el tid es el mismo que tiene guardada la consola
	 asociada).
	 Si hay una asociada, hay que eliminar el nodo y fijarse si el tid llamador no esta esperando a otro hilo.
	 (Creo que no puede estar esperando a otro hilo, este caso no esta implementado)
	 Si no está esperando a otro habria que consultar si va a exit o a ejecucion nuevamente.*/

	fijarse_joins(TID);
	struct_consola * consola_asociada = obtener_consolaAsociada(PID);
	if (consola_asociada->TID_padre == TID) {
		printf("TERMINO LA EJECUCION DEL TID PADRE\n");
		consola_asociada->termino_ejecucion = true;
		//consola_asociada->cantidad_hilos--;
		//matar_hijos(PID);
		t_datosAEnviar * paquete = crear_paquete(terminar_conexion, NULL, 0);
		if (enviar_datos(consola_asociada->socket_consola, paquete) < 0) {

		}
		free(paquete);
		printf("DESCONECTE LA CONSOLA ASOCIADA\n");
	}
	mandar_a_exit(tcb);

	//if (consola_asociada->cantidad_hilos == 0) {

	//	destruir_segmento_MSP(consola_asociada->PID, consola_asociada->M);

	//	t_datosAEnviar * paquete = crear_paquete(terminar_conexion, NULL, 0);
	//	if (enviar_datos(consola_asociada->socket_consola, paquete) < 0) {

	//	}
	//	free(paquete);
	//	printf("DESCONECTE LA CONSOLA ASOCIADA\n");
	//} else if (waitear && !(consola_asociada->cantidad_hilos < 1)) {

		//sem_post(&sem_CPU);
	//}

}

void matar_hijos(int PID) {
	printf("cantidad elementos ready %d\n", queue_size(ready.prioridad_1));
	matar_hijos_en_lista(PID, ready.prioridad_1->elements, READY);
	matar_hijos_en_lista(PID, block.prioridad_1, BLOCK);
	matar_hijos_en_lista(PID, SYS_CALL->elements, 8);		//TODO
	matar_hijos_en_lista(PID, hilos_join, 10);
	matar_hijo_en_diccionario(PID);
}

void matar_hijo_en_diccionario(int PID) {
	void matar_hijo(t_queue * bloqueado_por_recurso) {
		matar_hijos_en_lista(PID, bloqueado_por_recurso->elements, 8);
	}
	dictionary_iterator(dic_bloqueados, (void*) matar_hijo);
}

void matar_hijos_en_lista(int PID, t_list* lista, int tipo_lista) {
	TCB_struct * tcb;
	struct_bloqueado * block;
	struct_join * bloq;
	int cantidad, contador = 0;
	bool tiene_mismo_pid(TCB_struct * tcb) {
		printf("PID DEL ELEMENTO %d\n", tcb->PID);
		return tcb->PID == PID;
	}
	bool tiene_mismo_pid_block(struct_bloqueado * block) {
		return block->tcb.PID == PID;
	}
	bool tiene_mismo_pid_bloq(struct_join * estructura) {

		return estructura->tcb_llamador->PID == PID;
	}
	switch (tipo_lista) {

	case READY: //HACER WAIT DE PROCESO LISTO
		cantidad = list_count_satisfying(lista, (void*) tiene_mismo_pid);
		sem_wait(&sem_procesoListo);

                sem_wait(&sem_procesoListo);		
                while (contador < cantidad) {
			tcb = list_remove_by_condition(lista, (void*) tiene_mismo_pid);
			mandar_a_exit(tcb);
			contador++;
		}

		break;
	case BLOCK: //LISTA QUE TIENE TCB E ID RECURSO

		cantidad = list_count_satisfying(lista, (void*) tiene_mismo_pid_block);

		while (contador < cantidad) {
			block = list_remove_by_condition(lista,
					(void*) tiene_mismo_pid_block);
			tcb = &block->tcb;
			printf("PID DEL BLOQUEADO %d y el otro %d\n", tcb->PID,
					block->tcb.PID);
			mandar_a_exit(tcb);
			contador++;

		}
		break;
	case 8: //LISTA DE TCBS
		cantidad = list_count_satisfying(lista, (void*) tiene_mismo_pid);

		while (contador < cantidad) {
			tcb = list_remove_by_condition(lista, (void*) tiene_mismo_pid_bloq);
			mandar_a_exit(tcb);
			contador++;
		}

		break;
	case 10: //LISTA DE BLOQUEADOS (TCB LLAMADOR - TID A ESPERAR)

		cantidad = list_count_satisfying(lista, (void*) tiene_mismo_pid_bloq);
		while (contador < cantidad) {
			bloq = list_remove_by_condition(lista,
					(void*) tiene_mismo_pid_bloq);
			tcb = bloq->tcb_llamador;
			mandar_a_exit(tcb);
			contador++;
		}

		break;
	}

	/*
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
	 printf("Esperando a mandar a exit \n");
	 mandar_a_exit(tcb);

	 contador++;
	 }*/

}

void fijarse_joins(int tid) {

	printf("Me fijo joins\n");

	bool esta_esperando_tid(struct_join *estructura) {
		return estructura->tid_a_esperar == tid;
	}
	void desbloquear_por_join(struct_join * estructura) {
		if (esta_esperando_tid(estructura)) {
			TCB_struct * tcb_bloqueado = malloc(sizeof(TCB_struct));
			copiar_tcb(tcb_bloqueado, estructura->tcb_llamador);
			meter_en_ready(1, tcb_bloqueado);
		}
	}

	//list_remove_and_destroy_by_condition(hilos_join, (void*) esta_esperando_tid,
	//		free);
	//Como el filter contiene referencias a estructuras que contiene hilos_join, con liberar los elementos del
	//filter tendrían que dejar de estar los elementos en hilos_join

	list_map(hilos_join, (void*) desbloquear_por_join);
}

void finalizo_ejecucion(TCB_struct *tcb_fin) {

	if (tcb_fin->KM == 1) {
		printf("FINALIZO LA EJECUCION DE UNA INTERRUPCION\n");
		queue_push(block.prioridad_0, tcb_fin);
		loguear(BLOCK, tcb_fin);

		printf("TAMAÑO DE BLOQUEADOS %d\n", list_size(block.prioridad_1));
		printf("TID KM %d\n", tcb_fin->TID);
		bool tiene_mismo_tid(struct_bloqueado * estructura) {
			return estructura->tcb.TID == tcb_fin->TID;
		}
		struct_bloqueado * tcb_bloqueado = list_remove_by_condition(
				block.prioridad_1, (void*) tiene_mismo_tid);
		if (tcb_bloqueado == NULL ) {
			printf("NO SE ENCONTRO EL TCB BLOQUEADO\n");
		}

		TCB_struct * tcb_block = malloc(sizeof(TCB_struct));

		tcb_bloqueado->tcb.registrosProgramacion[0] =
				tcb_fin->registrosProgramacion[0];
		tcb_bloqueado->tcb.registrosProgramacion[1] =
				tcb_fin->registrosProgramacion[1];
		tcb_bloqueado->tcb.registrosProgramacion[2] =
				tcb_fin->registrosProgramacion[2];
		tcb_bloqueado->tcb.registrosProgramacion[3] =
				tcb_fin->registrosProgramacion[3];
		tcb_bloqueado->tcb.registrosProgramacion[4] =
				tcb_fin->registrosProgramacion[4];

		copiar_tcb(tcb_block, &tcb_bloqueado->tcb);

		free(tcb_bloqueado);

		meter_en_ready(1, tcb_block);
		sem_post(&sem_kmDisponible);
	} else {
		printf("FINALIZO LA EJECUCION DE UN TCB COMUNACHO\n");
		sacar_de_ejecucion(tcb_fin, true);
	}
}

void enviar_a_ejecucion(TCB_struct * tcb) {

	struct_CPU* cpu = list_find(CPU_list, (void*) CPU_esta_libre);
	while(cpu == NULL ) {
		sem_wait(&sem_CPU);
		cpu = list_find(CPU_list, (void*) CPU_esta_libre);
	}
	cpu->bit_estado = ocupado;
	cpu->PID = tcb->PID;
	cpu->TID = tcb->TID;

	if (tcb == NULL ) {
		printf("EL TCB ES NULO!!!\n");
		exit(-1);
	}
	list_add(exec, tcb);
	loguear(EXEC, tcb);

	printf("\n TCB!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! \n");
	printf("\n PID: %d \n", tcb->PID);
	printf("\n TID: %d \n", tcb->TID);
	printf("\n KM: %d \n", tcb->KM);
	printf("\n BASE SEGMENTO CODIGO: %d \n", tcb->M);
	printf("\n TAMANIO SEGMENTO CODIGO %d \n", tcb->tamanioSegmentoCodigo);
	printf("\n PUNTERO INSTRUCCION %d \n", tcb->P);
	printf("\n BASE STACK %d \n", tcb->X);
	printf("\n CURSOR STACK %d \n", tcb->S);
	printf("\n A: %d \n", tcb->registrosProgramacion[0]);
	printf("\n B: %d \n", tcb->registrosProgramacion[1]);
	printf("\n C: %d \n", tcb->registrosProgramacion[2]);
	printf("\n D: %d \n", tcb->registrosProgramacion[3]);
	printf("\n E: %d \n", tcb->registrosProgramacion[4]);

	void * mensaje = malloc(sizeof(TCB_struct) + sizeof(int));
	memcpy(mensaje, tcb, sizeof(TCB_struct));
	memcpy(mensaje + sizeof(TCB_struct), &QUANTUM, sizeof(int));
	printf("Se esta por enviar el hilo a ejecutar\n");
	t_datosAEnviar * paquete = crear_paquete(ejecutar, mensaje,
			sizeof(TCB_struct) + sizeof(int));
	enviar_datos(cpu->socket_CPU, paquete);
}

/*El dispatcher se encarga tanto de las llamadas al sistema como de los procesos que estan en la cola de ready*/
void dispatcher() {

	while (1) {
		sem_wait(&sem_procesoListo);
		sem_wait(&sem_CPU);
		sem_wait(&sem_procesoListo);
		printf("\nSe detectó un nuevo proceso!\n");
		if (!queue_is_empty(SYS_CALL)) {

			printf("ENTRE A EJECUTAR UNA INTERRUPCION!!!!!!!\n");
			sem_wait(&sem_kmDisponible);
			tcb_ejecutandoSysCall = (TCB_struct*) queue_pop(SYS_CALL);

			struct_bloqueado * tcb_bloqueado = obtener_bloqueado(
					tcb_ejecutandoSysCall->TID);
			TCB_struct * tcb_km = queue_pop(block.prioridad_0);
			copiarRegistros(tcb_km->registrosProgramacion,
					tcb_ejecutandoSysCall->registrosProgramacion);
			printf("PID A EJECUTAR %d\n", tcb_ejecutandoSysCall->PID);
			printf("TID A EJECUTAR %d\n", tcb_ejecutandoSysCall->TID);
			tcb_km->PID = tcb_ejecutandoSysCall->PID;
			tcb_km->TID = tcb_ejecutandoSysCall->TID;
			tcb_km->P = tcb_bloqueado->id_recurso;
			printf("COPIO EL PUNTERO DE INSTRUCCION: %d\n",
					tcb_bloqueado->id_recurso);
			//free(tcb_ejecutandoSysCall);
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

