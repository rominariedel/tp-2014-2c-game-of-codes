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
	aux ++;
	return aux;
}

int obtener_PID() {
	PID = PID + 1;
	return PID;
}

void crear_colas() {
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
	hilos_join = list_create();
}

void free_listas() {
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

void mover_a_exit(TCB_struct * tcb) {
	queue_push(EXIT, tcb);
}

int CPU_esta_libre(struct_CPU cpu) {
	return cpu.bit_estado;
}

void planificar(TCB_struct tcb) {
	queue_push(NEW, &tcb);
	queue_pop(NEW);
	queue_push(EXIT, &tcb);
}

void planificador() {

	fd_set copia_set;
	while (1) {

		copia_set = CPU_set;
		int i = select(descriptor_mas_alto_cpu + 1, &copia_set, NULL, NULL,
				NULL );

		if (i == -1) {
			//error
			break;
		}

		int n_descriptor = 0;

		while (n_descriptor <= descriptor_mas_alto_cpu) {

			if (FD_ISSET(n_descriptor, &copia_set)) {
				t_datosAEnviar * datos;
				datos = recibir_datos(n_descriptor);
				int codigo_operacion = datos->codigo_operacion;

				TCB_struct* tcb = malloc(sizeof(TCB_struct));
				int * dirSysCall, *tamanio, *pid, *tid_llamador, *tid_a_esperar, *id_recurso;
				char * cadena;
				char * id_tipo;

				sem_init(&mutex_entradaSalida, 0, 1);

				switch (codigo_operacion) {

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
					memcpy(dirSysCall, datos->datos + sizeof(TCB_struct),
							sizeof(int));
					interrumpir(tcb, *dirSysCall);
					break;
				case creacion_hilo:
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					crear_hilo(*tcb);
					break;
				case entrada_estandar:
					pid = malloc(sizeof(int));
					id_tipo = malloc(datos->tamanio);
					tamanio = malloc(sizeof(int));
					memcpy(tamanio, &datos->tamanio, sizeof(int));
					memcpy(pid, datos->datos, sizeof(int));
					memcpy(id_tipo, datos->datos + sizeof(int), datos->tamanio);
					producir_entrada_estandar(*pid, id_tipo, n_descriptor,
							*tamanio);

					break;
				case salida_estandar:
					pid = malloc(sizeof(int));
					cadena = malloc(datos->tamanio - sizeof(int));
					memcpy(pid, datos->datos, sizeof(int));
					memcpy(cadena, datos->datos + sizeof(int),
							datos->tamanio - sizeof(int));
					producir_salida_estandar(*pid, cadena);
					break;
				case join: //TODO: preguntar si ese hilo llamador esta en ejecucion o que
					tid_llamador = malloc(sizeof(int));
					tid_a_esperar = malloc(sizeof(int));
					memcpy(tid_llamador, datos->datos, sizeof(int));
					memcpy(tid_a_esperar, datos->datos + sizeof(int),
							sizeof(int));
					realizar_join(*tid_llamador, *tid_a_esperar);
					break;
				case bloquear:
					id_recurso = malloc(sizeof(int));
					memcpy(tcb, datos->datos, sizeof(TCB_struct));
					memcpy(id_recurso, datos->datos + sizeof(TCB_struct), sizeof(int));
					break;
				case despertar:

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
	return list_find(BLOCK.prioridad_1, (void*) tiene_mismo_tid);
}

void producir_salida_estandar(int pid, char* cadena) {
	struct_consola * consola_asociada = obtener_consolaAsociada(pid);
	t_datosAEnviar * datos = malloc(sizeof(t_datosAEnviar));
	datos->codigo_operacion = imprimir_en_pantalla;
	datos->tamanio = string_length(cadena);
	datos->datos = (void*) cadena;

	enviar_datos(consola_asociada->socket_consola, datos);

	//TODO: chequear que se enviaron los datos

	free(datos);
}

void producir_entrada_estandar(int pid, char * id_tipo, int socket_CPU,
		int tamanio) {

	//Se hace un wait del mutex de entrada_salida al hacerse la solicitud de entrada y un post
	//cuando se le devuelve la entrada a la CPU, asi si otra CPU solicita entrada salida, espera a
	//que se libere el recurso compartido (la estructura de entrada salida). De esta forma las demas
	//CPUS que hagan otras solicitudes pueden ser planificadas

	sem_wait(&mutex_entradaSalida);
	entrada = malloc(sizeof(entrada_salida));
	entrada->cadena = malloc(tamanio);
	memcpy(&entrada->socket_CPU, &socket_CPU, sizeof(int));

	//Enviando la solicitud a la consola para el ingreso de datos
	struct_consola * consola_asociada = obtener_consolaAsociada(pid);
	t_datosAEnviar * datos_consola = crear_paquete(ingresar_cadena, id_tipo,
			tamanio);
	enviar_datos(consola_asociada->socket_consola, datos_consola);
	free(datos_consola);

}

void devolver_entrada_aCPU(int tamanio_datos) {
	//Esta funcion es invocada cuando la consola manda el mensaje de que ya se ingresaron los datos
	struct_CPU * CPU_asociada = obtener_CPUAsociada(entrada->socket_CPU);
	t_datosAEnviar * datos = crear_paquete(devolucion_cadena, entrada->cadena,
			tamanio_datos);
	enviar_datos(CPU_asociada->socket_CPU, datos);
	free(datos);
	free(entrada->cadena);
	free(entrada);
	sem_post(&mutex_entradaSalida);
}

void realizar_join(int tid_llamador, int tid_a_esperar) {
	struct_join * estructura = malloc(sizeof(struct_join));
	estructura->tid_a_esperar = tid_a_esperar;
	estructura->tid_llamador = tid_llamador;
	list_add(hilos_join, estructura);

	//TODO: aca hay que mandar un tcb a bloquear..
}
