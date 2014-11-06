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

enum bit_de_estado {
	libre, ocupado,
};

/*FUNCIONES*/
void loader();
void boot();
void obtenerDatosConfig(char**);
void boot();
void escribir_memoria(int, int, int, void*);
void obtenerDatosConfig(char**);
void sacar_de_ejecucion(TCB_struct *);
int solicitar_segmento(int, int);
void iniciar_semaforos();
void enviar_a_ejecucion(TCB_struct *);
void dispatcher();
void copiarRegistros(int registro1[5], int registro2[5]);
void handshake_MSP(int socketMSP);

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
}

void obtenerDatosConfig(char ** argv) {
	configuracion = config_create(argv[1]);
	PUERTO = config_get_string_value(configuracion, "PUERTO");
	IP_MSP = config_get_string_value(configuracion, "IP_MSP");
	PUERTO_MSP = config_get_string_value(configuracion, "PUERTO_MSP");
	QUANTUM = config_get_int_value(configuracion, "QUANTUM");
	SYSCALLS = config_get_string_value(configuracion, "SYSCALLS");
	TAMANIO_STACK = config_get_int_value(configuracion, "TAMANIO_STACK");
}

void loader() {
	printf("\nSE INICIO EL LOADER\n");
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
				printf("Tamaño de lista de consolas %d\n",
						list_size(consola_list));
				printf("Descriptor actual %d\n", n_descriptor);
				if (consola_conectada == NULL ) {
					printf("La consola no se creo correctamente o no existe\n");
					exit(-1);
				}
				printf("Se encontro una consola activa de PID %d\n",
						consola_conectada->PID);
				t_datosAEnviar * datos;
				datos = recibir_datos(n_descriptor);
				TCB_struct * nuevoTCB;

				int * aux;
				switch (datos->codigo_operacion) {

				case codigo_consola:
					printf("COD OPERACION: CODIGO_CONSOLA\n");
					nuevoTCB = malloc(sizeof(TCB_struct));
					nuevoTCB->PID = consola_conectada->PID;
					nuevoTCB->TID = obtener_TID();
					int segmento_codigo = solicitar_segmento(nuevoTCB->PID,
							datos->codigo_operacion);

					int segmento_stack = solicitar_segmento(nuevoTCB->PID,
							TAMANIO_STACK);

					//TODO finalizar conexion si hubo un problema al solicitar un segmento.

					escribir_memoria(nuevoTCB->PID, segmento_codigo,
							datos->tamanio, datos->datos);

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
					queue_push(READY.prioridad_1, nuevoTCB); //TODO: ESTO LO PONGO ACA PARA PROBAR LA CPU
					consola_conectada->cantidad_hilos = 1;
					sem_post(&sem_procesoListo); //TODO: LO MISMO ACA
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

void handshake_MSP(int socketMSP){
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
	printf("\n Solicitando segmento para las syscalls de tamanio %d\n",
			(int) tamanio_codigo_syscalls);
	int base_segmento_codigo = solicitar_segmento(pid_KM_boot,
			tamanio_codigo_syscalls);
	//TODO: validar la base del segmento
	escribir_memoria(pid_KM_boot, base_segmento_codigo,
			(int) tamanio_codigo_syscalls, (void*) syscalls);

	free(syscalls);
	printf("\n Solicitando segmento para el stack del tcb km\n");
	int base_segmento_stack = solicitar_segmento(pid_KM_boot, TAMANIO_STACK);
	//TODO: validar la base del stack
	printf("Creando TCB Modo Kernel\n");

	tcb_km = malloc(sizeof(TCB_struct));
	tcb_km->KM = 1;
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

	printf("Bloqueando TCB KM\n");

	queue_push(BLOCK.prioridad_0, (void *) tcb_km);

	printf("Esperando conexiones...\n");

	FD_ZERO(&consola_set);
	FD_ZERO(&CPU_set);
	pthread_t thread_planificador;
	pthread_t thread_loader;

	descriptor_mas_alto_consola = 0;
	descriptor_mas_alto_cpu = 0;

	while (1) {

		int socket_conectado = recibir_conexion(socket_gral);
		printf("Se recibio una conexion!\n");
		int modulo_conectado = -1;
		t_datosAEnviar * datos = recibir_datos(socket_conectado);
		modulo_conectado = datos->codigo_operacion;

		if (modulo_conectado == soy_consola) {
			printf("Se conecto una consola\n");
			FD_SET(socket_conectado, &consola_set);
			struct_consola * consola_conectada = malloc(sizeof(struct_consola));
			int pid = obtener_PID();
			printf("Se va a agregar una consola de PID: %d y socket: %d\n", pid,
					socket_conectado);
			consola_conectada->PID = pid;
			consola_conectada->socket_consola = socket_conectado;
			consola_conectada->cantidad_hilos = 0;
			list_add(consola_list, consola_conectada);
			if (list_size(consola_list) == 0) {
				printf("No se agrego la consola correctamente\n");
				exit(-1);
			}
			printf("Se agrego una consola de PID: %d\n",
					consola_conectada->PID);
			if (descriptor_mas_alto_consola == 0) {
				descriptor_mas_alto_consola = socket_conectado;
				printf("Es la primera consola que se conecta \n");
				pthread_create(&thread_loader, NULL, (void*) &loader, NULL );
			}
			if (descriptor_mas_alto_consola < socket_conectado) {
				descriptor_mas_alto_consola = socket_conectado;
			}

		} else if (modulo_conectado == soy_CPU) {
			printf("Se conecto una CPU\n");
			FD_SET(socket_conectado, &CPU_set);
			struct_CPU* cpu_conectada = malloc(sizeof(struct_CPU));
			cpu_conectada->PID = -1;
			cpu_conectada->bit_estado = libre;
			cpu_conectada->socket_CPU = socket_conectado;
			list_add(CPU_list, cpu_conectada);

			sem_post(&sem_CPU);
			if (descriptor_mas_alto_cpu == 0) {
				printf("Es la primera CPU que se conecta\n");
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
	struct_bloqueado tcb_bloqueado;
	tcb_bloqueado.id_recurso = dirSyscall;
	tcb_bloqueado.tcb = *tcb;
	list_add(BLOCK.prioridad_1, &tcb_bloqueado);
	sem_post(&sem_procesoListo);
}

void crear_hilo(TCB_struct tcb, int socketCPU) {

	//TODO: llega la solicitud del nuevo hilo, creo el tcb con el segmento de stack asignado y se
	//lo envio a la CPU. Despues me llega otro mensaje de la cpu con el tcb completado a planificar

	TCB_struct nuevoTCB;

	int base_stack = solicitar_segmento(tcb.PID, TAMANIO_STACK);

	nuevoTCB.PID = tcb.PID;
	int tid = obtener_TID(tcb.PID);
	nuevoTCB.X = base_stack;
	nuevoTCB.S = base_stack;
	nuevoTCB.KM = 0;
	nuevoTCB.PID = tcb.PID;
	nuevoTCB.TID = tid;
	nuevoTCB.M = tcb.M;
	nuevoTCB.tamanioSegmentoCodigo = tcb.tamanioSegmentoCodigo;
	nuevoTCB.P = tcb.P;
	copiarRegistros(nuevoTCB.registrosProgramacion, tcb.registrosProgramacion);

	t_datosAEnviar * datos = crear_paquete(0, &nuevoTCB, sizeof(TCB_struct));
	enviar_datos(socketCPU, datos);

}

void planificar_hilo_creado(TCB_struct * nuevoTCB){

	queue_push(READY.prioridad_1, &nuevoTCB);
	sem_post(&sem_procesoListo);

		//Indico que la cantidad de hilos de un proceso aumentó
	struct_consola * consola_asociada = obtener_consolaAsociada(nuevoTCB->PID);
	consola_asociada->cantidad_hilos++;

}

void copiarRegistros(int registro1[5], int registro2[5]) {
	int n = 5;
	while (n > 0) {
		registro1[n - 1] = registro2[n - 1];
		n--;
	}
}

/*Esta operacion le solicita a la MSP un segmento, retorna la direccion base del
 * segmento reservado*/
int solicitar_segmento(int pid, int tamanio_del_segmento) {

	char * datos = malloc(2 * sizeof(int));
	memcpy(datos, &pid, sizeof(int));
	memcpy(datos + sizeof(int), &tamanio_del_segmento, sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(crear_segmento, (void*) datos,
			2 * sizeof(int));

	enviar_datos(socket_MSP, paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socket_MSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta->datos, sizeof(int));
	free(respuesta->datos);
	free(respuesta);
	if (*dir_base == error_segmentationFault) {
		//ERROR
		return -1;
	}
	printf("Se recibio la direccion base: %d\n", *dir_base);

	return *dir_base;
}

void escribir_memoria(int pid, int dir_logica, int tamanio, void * bytes) {

	char * datos = malloc((3 * sizeof(int)) + tamanio);

	memcpy(datos, &pid, sizeof(int));
	memcpy(datos + sizeof(int), &dir_logica, sizeof(int));
	memcpy(datos + (2 * sizeof(int)), bytes, tamanio);
	memcpy(datos + (2 * sizeof(int)) + tamanio, &tamanio, sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(escribir_en_memoria, datos,
			(3 * sizeof(int)) + tamanio);
	enviar_datos(socket_MSP, paquete);
	free(datos);
	free(paquete);

}

void finalizo_quantum(TCB_struct* tcb) {
	sacar_de_ejecucion(tcb);
	queue_push(READY.prioridad_1, tcb);
	sem_post(&sem_procesoListo);

}

void sacar_de_ejecucion(TCB_struct* tcb) {
	int PID = tcb->PID;
	int TID = tcb->TID;
	bool es_TCB(TCB_struct tcb_comparar) {
		return (tcb_comparar.PID == PID) && (tcb_comparar.TID == TID);
	}
	TCB_struct * tcb_exec = list_remove_by_condition(EXEC, (void*) es_TCB);
	free(tcb_exec); //TODO: sacar de ejecucion, va a exit------

	sem_post(&sem_CPU);

}

void finalizo_ejecucion(TCB_struct *tcb) {
	sacar_de_ejecucion(tcb);
	queue_push(EXIT, tcb);
}

void abortar(TCB_struct* tcb) {
	sacar_de_ejecucion(tcb);
	queue_push(EXIT, tcb);

	struct_consola * consola_asociada = obtener_consolaAsociada(tcb->PID);
	consola_asociada->cantidad_hilos--;
	if (consola_asociada->cantidad_hilos == 0) {
		//Si no quedan mas hilos de esa consola hay que terminar la conexion.
		t_datosAEnviar * paquete = crear_paquete(terminar_conexion, NULL, 0);
		enviar_datos(consola_asociada->socket_consola, paquete);
		free(paquete);
		close(consola_asociada->socket_consola);
		free(consola_asociada);
	}
	//LOGUEAR que tuvo que abortar el hilo
}

void enviar_a_ejecucion(TCB_struct * tcb) {

	printf(
			"\nEsperando la activacion de una CPU para enviar a ejecutar un hilo. \n");
	sem_wait(&sem_CPU);
	list_add(EXEC, tcb);
	struct_CPU* cpu = list_find(CPU_list, (void*) CPU_esta_libre);
	if (cpu == NULL ) {
		printf("FALLO. NO SE ENCONTRO CPU\n");
		exit(-1);
	}
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
		printf("\nSe detectó un nuevo proceso!\n");
		if (!queue_is_empty(SYS_CALL)) {
			tcb_ejecutandoSysCall = (TCB_struct*) queue_peek(SYS_CALL);

			struct_bloqueado * tcb_bloqueado = obtener_bloqueado(
					tcb_ejecutandoSysCall->TID);
			copiarRegistros(tcb_km->registrosProgramacion,
					tcb_ejecutandoSysCall->registrosProgramacion);
			tcb_km->PID = tcb_ejecutandoSysCall->PID;

			tcb_km->P = tcb_bloqueado->id_recurso;

			enviar_a_ejecucion(tcb_km);

		} else {
			TCB_struct * tcb;
			if (!queue_is_empty(READY.prioridad_0)) {
				tcb = queue_pop(READY.prioridad_0);
			} else {
				tcb = queue_pop(READY.prioridad_1);
			}
			enviar_a_ejecucion(tcb);
		}
	}

}

