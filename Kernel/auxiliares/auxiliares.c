/*
 * auxiliares.c
 *
 *  Created on: 27/09/2014
 *      Author: utnso
 */

#include "variables_globales.h"


int obtener_TID(){
	return TID++;
}

int obtener_PID(){
	return PID++;
}


void crear_colas(){
	NEW = queue_create();
	EXIT = queue_create();

	READY.prioridad_0 = queue_create();
	READY.prioridad_1 = queue_create();
	BLOCK.prioridad_0 = queue_create();

	SYS_CALL = queue_create();

	BLOCK.prioridad_1 = list_create();
	EXEC = list_create();
	CPU_list = list_create();
	consola_list = list_create();
}

void free_listas(){
	queue_destroy(NEW);
	queue_destroy(EXIT);

	queue_destroy(READY.prioridad_0);
	queue_destroy(READY.prioridad_1);
	queue_destroy(SYS_CALL);
	queue_destroy(BLOCK.prioridad_0);
	list_destroy_and_destroy_elements(BLOCK.prioridad_1, &free);
	list_destroy_and_destroy_elements(EXEC, &free);
	list_destroy_and_destroy_elements(CPU_list, &free);
	list_destroy_and_destroy_elements(consola_list, &free);


}

long tamanio_syscalls(FILE* syscalls){
	fseek(syscalls, 0, SEEK_END);
	int tamanio= ftell(syscalls);
	rewind(syscalls);
	return tamanio;
}

char * extraer_syscalls(){
	FILE* archivo_syscalls = fopen(SYSCALLS, "read");
	tamanio_codigo_syscalls = tamanio_syscalls(archivo_syscalls);
	char * syscalls = malloc(tamanio_codigo_syscalls); //MMAP
	fread((void*) syscalls, 1, tamanio_codigo_syscalls, archivo_syscalls);
	fclose(archivo_syscalls);
	return syscalls;
}

void mover_a_exit(TCB_struct * tcb){
	queue_push(EXIT, tcb);
}


int CPU_esta_libre(struct_CPU cpu){
	return cpu.bit_estado;
}


void planificar(TCB_struct tcb){
	queue_push(NEW, &tcb);
	queue_pop(NEW);
	queue_push(EXIT, &tcb);
}

void planificador(){

	fd_set copia_set;
	while(1){

		copia_set = CPU_set;
		int i = select(descriptor_mas_alto_cpu +1, &copia_set, NULL, NULL, NULL);

		if(i == -1){
			//error
			break;
		}

		int n_descriptor = 0;

		while(n_descriptor <= descriptor_mas_alto_cpu){

			if(FD_ISSET(n_descriptor, &copia_set)){
				t_datosAEnviar * datos;
				datos = recibir_datos(n_descriptor);
				int codigo_operacion = datos->codigo_operacion;

				TCB_struct* tcb = malloc(sizeof(TCB_struct));
				int * dirSysCall;
				int * pid;
				char * cadena;
				int * id_tipo;

				sem_init(&mutex_entradaSalida, 0, 1);
				sem_init(&sem_entrada, 0, 0);

				switch(codigo_operacion){

					case finaliza_quantum:
						memcpy(tcb, datos->datos, sizeof(TCB_struct));
						finalizo_quantum(tcb);
						break;
					case finaliza_ejecucion:
						memcpy(tcb, datos->datos, sizeof(TCB_struct));
						finalizo_ejecucion(tcb);
						break;
					case ejecucion_erronea:
						memcpy(tcb, datos->datos, sizeof(TCB_struct));
						abortar(tcb);
						break;
					case desconexion:
						memcpy(tcb, datos->datos, sizeof(TCB_struct));
						abortar(tcb);
						break;
					case interrupcion:
						dirSysCall = malloc(sizeof(int));
						memcpy(tcb, datos->datos, sizeof(TCB_struct));
						memcpy(dirSysCall, datos->datos + sizeof(TCB_struct),sizeof(int));
						interrumpir(tcb, *dirSysCall);
						break;
					case creacion_hilo:
						memcpy(tcb, datos->datos, sizeof(TCB_struct));
						crear_hilo(*tcb);
						break;
					case entrada_estandar:
						pid = malloc(sizeof(int));
						id_tipo = malloc(sizeof(int));
						memcpy(pid, datos->datos, sizeof(int));
						memcpy(id_tipo, datos->datos + sizeof(int), sizeof(int));
						producir_entrada_estandar(*pid, *id_tipo, n_descriptor);

						break;
					case salida_estandar:
						pid = malloc(sizeof(int));
						cadena = malloc(datos->tamanio - sizeof(int));
						memcpy(pid, datos->datos, sizeof(int));
						memcpy(cadena, datos->datos + sizeof(int), datos->tamanio - sizeof(int));
						producir_salida_estandar(*pid, cadena);
						break;
					case join:

						break;
					case bloquear:

						break;
					case despertar:

						break;

					}
				free(datos);
			}
			n_descriptor ++;
		}

	}
}

struct_consola * obtener_consolaConectada(int socket_consola){
	bool tiene_mismo_socket(struct_consola estructura){
		return estructura.socket_consola == socket_consola;
	}
	return list_find(consola_list, (void*)&tiene_mismo_socket);

}

struct_consola * obtener_consolaAsociada(int PID){
	bool tiene_mismo_pid(struct_consola estructura){
		return estructura.PID == PID;
	}
	return list_find(consola_list, (void*) &tiene_mismo_pid);
}

struct_CPU * obtener_CPUAsociada(int socket_cpu){
	bool tiene_mismo_socket(struct_CPU estructura){
		return estructura.socket_CPU == socket_cpu;
	}
	return list_find(CPU_list, (void*) &tiene_mismo_socket);
}

struct_bloqueado * obtener_bloqueado(int TID){
	bool tiene_mismo_tid(struct_bloqueado estructura){
		return estructura.tcb.TID == TID;
	}
	return list_find(BLOCK.prioridad_1, (void*)&tiene_mismo_tid);
}

void producir_salida_estandar(int pid, char* cadena){
	struct_consola * consola_asociada = obtener_consolaAsociada(pid);
	t_datosAEnviar * datos = malloc(sizeof(t_datosAEnviar));
	datos->codigo_operacion = imprimir_en_pantalla;
	datos->tamanio = string_length(cadena);
	datos->datos = (void*) cadena;

	enviar_datos(consola_asociada->socket_consola, datos);

	//TODO: chequear que se enviaron los datos

	free(datos);
}

void producir_entrada_estandar(int pid, int id_tipo, int socket_CPU){

	sem_wait(&mutex_entradaSalida);
	entrada = malloc(sizeof(entrada_salida));
	entrada->semaforo = sem_entrada;

	struct_consola * consola_asociada = obtener_consolaAsociada(pid);
	t_datosAEnviar * datos_consola = malloc(sizeof(t_datosAEnviar));
	datos_consola->codigo_operacion = ingresar_cadena;
	memcpy(datos_consola->datos, &id_tipo, sizeof(int));
	datos_consola->tamanio = sizeof(int);
	enviar_datos(consola_asociada->socket_consola, datos_consola);
	free(datos_consola);

	sem_wait(&sem_entrada);

	struct_CPU * CPU_asociada = obtener_CPUAsociada(socket_CPU);
	t_datosAEnviar * datos = malloc(sizeof(t_datosAEnviar));
	datos->codigo_operacion = devolucion_cadena;
	datos->datos = entrada->cadena;
	datos->tamanio = entrada->tamanio;
	enviar_datos(CPU_asociada->socket_CPU, datos);
	free(datos);
	free(entrada);
	sem_post(&sem_entrada);

	sem_post(&mutex_entradaSalida);

}

void recibir_cadena(void * cadena, int tamanio){
	entrada->cadena = malloc(tamanio);
	memcpy(&entrada->cadena, cadena, tamanio);
	entrada->tamanio = tamanio;
	sem_post(&sem_entrada);

}
