/*
 * main.c
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#include "auxiliares/auxiliares.h"

#define pid_KM_boot 0

/*VARIABLES GLOBALES*/

enum bit_de_estado {
	libre, ocupado,
};

/*FUNCIONES*/
void loader();
void boot();
void obtenerDatosConfig(char**);
void agregar_hilo(t_queue*, TCB_struct); //VER
void boot();
void ejecutarSysCall();
void escribir_memoria(int, int, int, void*);
void obtenerDatosConfig(char**);
void sacar_de_ejecucion(TCB_struct *);
int solicitar_segmento(int, int);
void iniciar_semaforos();
void enviar_a_ejecucion(TCB_struct *);
int main(int argc, char ** argv) {

	printf("\n -------------  KERNEL  -------------\n");
	printf("\n    iniciando...\n");
	crear_colas();
	iniciar_semaforos();
	obtenerDatosConfig(argv);
	TID = 0;
	PID = 0;

	pthread_t thread_boot;
	pthread_create(&thread_boot, NULL, (void*) &boot, NULL );

	return 0;
}

void iniciar_semaforos() {
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_CPU, 0, 0);
	sem_init(&sem_syscalls, 0, 0);
}

void obtenerDatosConfig(char ** argv) {
	t_config * configuracion = config_create(argv[1]);
	PUERTO = config_get_string_value(configuracion, "PUERTO");
	IP_MSP = config_get_string_value(configuracion, "IP_MSP");
	PUERTO_MSP = config_get_string_value(configuracion, "PUERTO_MSP");
	QUANTUM = config_get_int_value(configuracion, "QUANTUM");
	SYSCALLS = config_get_string_value(configuracion, "SYSCALLS");
	TAMANIO_STACK = config_get_int_value(configuracion, "TAMANIO_STACK");
}

void loader() {

	fd_set copia_set;

	while (1) {
		copia_set = consola_set;
		int i = select(descriptor_mas_alto_consola + 1, &consola_set, NULL,
				NULL, NULL );

		if (i == -1) {
			//ERROR
			break;
		}

		int n_descriptor = 0;

		while (n_descriptor <= descriptor_mas_alto_consola) {
			if (FD_ISSET(n_descriptor, &copia_set)) {
				t_datosAEnviar * datos;
				datos = recibir_datos(n_descriptor);
				struct_consola * consola_conectada = obtener_consolaConectada(
						n_descriptor);
				TCB_struct * nuevoTCB;
				switch (datos->codigo_operacion) {
				case path_codigo_consola:
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
					nuevoTCB->registrosProgramacion.A = 0;
					nuevoTCB->registrosProgramacion.B = 0;
					nuevoTCB->registrosProgramacion.C = 0;
					nuevoTCB->registrosProgramacion.D = 0;
					nuevoTCB->registrosProgramacion.E = 0;

					agregar_hilo(NEW, *nuevoTCB);
					consola_conectada->cantidad_hilos = 1;
				}
			}
		}

	}
}

void boot() {

	printf("\n    INICIANDO BOOT   \n");
	char * syscalls = extraer_syscalls();
	socket_MSP = crear_cliente(IP_MSP, PUERTO_MSP);

	int base_segmento_codigo = solicitar_segmento(pid_KM_boot,
			tamanio_codigo_syscalls);
	//TODO: validar la base del segmento
	escribir_memoria(pid_KM_boot, base_segmento_codigo,
			(int) tamanio_codigo_syscalls, (void*) syscalls);
	free(syscalls);

	int base_segmento_stack = solicitar_segmento(pid_KM_boot, TAMANIO_STACK);
	//TODO: validar la base del stack
	tcb_km.KM = 1;
	tcb_km.M = base_segmento_codigo;
	tcb_km.tamanioSegmentoCodigo = tamanio_codigo_syscalls;
	tcb_km.P = 0;
	tcb_km.PID = pid_KM_boot;
	tcb_km.S = base_segmento_stack;
	tcb_km.TID = 0;
	tcb_km.X = base_segmento_stack;

	tcb_km.registrosProgramacion.A = 0;
	tcb_km.registrosProgramacion.B = 0;
	tcb_km.registrosProgramacion.C = 0;
	tcb_km.registrosProgramacion.D = 0;
	tcb_km.registrosProgramacion.E = 0;

	queue_push(BLOCK.prioridad_0, (void *) &tcb_km);

	printf("Esperando conexiones...");

	socket_gral = crear_servidor(PUERTO, backlog);
	//TODO: validar socket

	FD_ZERO(&consola_set);
	FD_ZERO(&CPU_set);
	pthread_t thread_planificador;
	pthread_t thread_loader;

	while (1) {

		int socket_conectado = recibir_conexion(socket_gral);
		int modulo_conectado = -1;
		t_datosAEnviar * datos = recibir_datos(socket_conectado);
		modulo_conectado = datos->codigo_operacion;

		if (modulo_conectado == soy_consola) {
			FD_SET(socket_conectado, &consola_set);
			struct_consola * consola_conectada = malloc(sizeof(struct_consola));
			int pid = obtener_PID();
			consola_conectada->PID = pid;
			consola_conectada->socket_consola = socket_conectado;
			consola_conectada->cantidad_hilos = 0;
			if (list_is_empty(consola_list)) {
				descriptor_mas_alto_consola = socket_conectado;
				pthread_create(&thread_loader, NULL, (void*) &loader, NULL );
			}
			list_add(consola_list, consola_conectada);

		} else if (modulo_conectado == soy_CPU) {
			FD_SET(socket_conectado, &CPU_set);
			struct_CPU* cpu_conectada = malloc(sizeof(struct_CPU));
			cpu_conectada->PID = -1;
			cpu_conectada->bit_estado = libre;
			cpu_conectada->socket_CPU = socket_conectado;
			list_add(CPU_list, cpu_conectada);

			sem_post(&sem_CPU);
			if (list_is_empty(CPU_list)) {
				descriptor_mas_alto_cpu = socket_conectado;
				pthread_create(&thread_planificador, NULL,
						(void*) &planificador, NULL );
			}
		}

		free(datos->datos);
		free(datos);

	}
}

void interrumpir(TCB_struct * tcb, int dirSyscall) {
	agregar_hilo(SYS_CALL, *tcb);
	struct_bloqueado tcb_bloqueado;
	tcb_bloqueado.id_recurso = dirSyscall;
	tcb_bloqueado.tcb = *tcb;
	list_add(BLOCK.prioridad_1, &tcb_bloqueado);
	sem_post(&sem_syscalls);
}

void ejecutarSysCall() {
	while (1) {
		sem_wait(&sem_syscalls);
		tcb_ejecutandoSysCall = (TCB_struct*) queue_peek(SYS_CALL);

		struct_bloqueado * tcb_bloqueado = obtener_bloqueado(tcb_ejecutandoSysCall->TID);
		tcb_km.registrosProgramacion =
				tcb_ejecutandoSysCall->registrosProgramacion;
		tcb_km.PID = tcb_ejecutandoSysCall->PID;

		tcb_km.P = tcb_bloqueado->id_recurso;

		enviar_a_ejecucion(&tcb_km);
	}
}

void crear_hilo(TCB_struct tcb) {

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
	reg_programacion nuevosRegistros = tcb.registrosProgramacion;
	nuevoTCB.registrosProgramacion = nuevosRegistros;
	agregar_hilo(READY.prioridad_1, nuevoTCB);

}

void agregar_hilo(t_queue * COLA, TCB_struct tcb) {

	queue_push(COLA, (void*) &tcb);

}

/*Esta operacion le solicita a la MSP un segmento, retorna la direccion base del
 * segmento reservado*/
int solicitar_segmento(int pid, int tamanio) {

	char * datos = malloc(2 * sizeof(int));
	memcpy(datos, &pid, sizeof(int));
	memcpy(datos + sizeof(int), &tamanio, sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(reservar_segmento, (void*) datos,
			2 * sizeof(int));

	enviar_datos(socket_MSP, paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socket_MSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta->datos, sizeof(int));
	//TODO: validar si no hay violacion de segmento

	return *dir_base;
}

void escribir_memoria(int pid, int dir_logica, int tamanio, void * bytes) {

	char * datos = malloc((3 * sizeof(int)) + tamanio);

	memcpy(datos, &pid, sizeof(int));
	memcpy(datos + sizeof(int), &dir_logica, sizeof(int));
	memcpy(datos + (2 * sizeof(int)), &tamanio, sizeof(int));
	memcpy(datos + (3 * sizeof(int)), bytes, tamanio);

	t_datosAEnviar * paquete = crear_paquete(escribir_en_memoria, datos,
			(3 * sizeof(int)) + sizeof(void*));
	enviar_datos(socket_MSP, paquete);
	free(datos);
	free(paquete);

}

void finalizo_quantum(TCB_struct* tcb) {
	sacar_de_ejecucion(tcb);
	agregar_hilo(READY.prioridad_1, *tcb);

}

void sacar_de_ejecucion(TCB_struct* tcb) {
	int PID = tcb->PID;
	int TID = tcb->TID;
	bool es_TCB(TCB_struct tcb_comparar) {
		return (tcb_comparar.PID == PID) && (tcb_comparar.TID == TID);
	}
	TCB_struct * tcb_exec = list_remove_by_condition(EXEC, (void*) es_TCB);
	free(tcb_exec);

}

void finalizo_ejecucion(TCB_struct *tcb) {
	sacar_de_ejecucion(tcb);
	agregar_hilo(EXIT, *tcb);
}

void abortar(TCB_struct* tcb) {
	sacar_de_ejecucion(tcb);
	agregar_hilo(EXIT, *tcb);
	//LOGUEAR que tuvo que abortar el hilo
}

void enviar_a_ejecucion(TCB_struct * tcb) {
	sem_wait(&sem_CPU);
	list_add(EXEC, tcb);
	struct_CPU* cpu = list_find(CPU_list, (void*) CPU_esta_libre);
	int op = ejecutar;
	void * mensaje = malloc(sizeof(TCB_struct) + sizeof(int));
	memcpy(mensaje, &op, sizeof(int));
	memcpy(mensaje + sizeof(int), tcb, sizeof(TCB_struct));
	enviar_datos(cpu->socket_CPU, mensaje);
}

void enviarAExec() {

	while (1) {
		sem_wait(&sem_ready);
		TCB_struct * tcb;
		if (!queue_is_empty(READY.prioridad_0)) {
			tcb = queue_pop(READY.prioridad_0);
		} else {
			tcb = queue_pop(READY.prioridad_1);
		}
		enviar_a_ejecucion(tcb);
	}
}

/*El dispatcher se encarga tanto de las llamadas al sistema como de los procesos que estan en la cola de ready*/
void dispatcher() {

	pthread_t thread_ejecucionDeSyscalls;
	pthread_create(&thread_ejecucionDeSyscalls, NULL, (void*) &ejecutarSysCall,
			NULL );

	pthread_t thread_flujoDeReadyAExec;
	pthread_create(&thread_flujoDeReadyAExec, NULL, (void*) &enviarAExec,
			NULL );
}
