/*
 * main.c
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#include<stdio.h>
#include<stddef.h>
#include "commons/config.h"
#include<commons/collections/list.h>
#include"sockets.h"
#include<commons/collections/queue.h>
#include <sys/select.h>

#define pid_KM_boot 0

/*ESTRUCTURAS*/

typedef struct {
	int A;
	int B;
	int C;
	int D;
	int E;
}reg_programacion;

typedef struct {
	int PID;
	int TID;
	int KM;
	int M;
	int tamanioSegmentoCodigo;
	int P;
	int X;
	int S;
	reg_programacion * registrosProgramacion;
}TCB_struct;

typedef struct cola{
	TCB_struct TCB;
	struct cola * siguiente_TCB;
}colaPlanificacion;

typedef struct {
	t_queue * prioridad_0;
	t_queue * prioridad_1;
} t_colas_prioridades;

typedef struct {
	int socket_CPU;
	int bit_estado;
	int PID;
} struct_CPU;

typedef struct{
	int socket_consola;
	int PID;
} struct_consola;

/*VARIABLES GLOBALES*/

enum bit_de_estado{
	libre,
	ocupado,
};
enum mensajes{

	//Mensajes enviados

	reservar_segmento = 1,
	escribir_en_memoria = 2,
	ejecucion_abortada = 3,
	imprimir_en_pantalla = 4,
	ingresar_cadena = 5,

	//Mensajes recibidos

	finaliza_quantum = 10,
	finaliza_ejecucion = 11,
	ejecucion_erronea = 12,
	desconecta_CPU = 13,
	desconecta_consola = 14,
	ejecuta_syscall = 15,
	creacion_hilo = 16,
	error_memoriaLlena = 17,
	error_segmentationFault = 18,
	soy_consola = 19,
	soy_CPU = 20,
	entrada_estandar = 21,
	salida_estandar = 22,
	join = 23,
	bloquear = 24,
	despertar = 25,
};

char * PUERTO;
char * IP_MSP;
char * PUERTO_MSP;
int QUANTUM;
char * SYSCALLS;
int TAMANIO_STACK;
int backlog;
int socket_MSP;
int TID;
int PID;

/*COLAS DE PLANIFICACIÓN*/
t_colas_prioridades READY;
t_queue * NEW;
t_colas_prioridades BLOCK;
t_queue * EXEC;
t_queue * SYS_CALL;

TCB_struct tcb_km;
fd_set clientes_set;
t_list * CPU_list;
t_list * consola_list;

/*FUNCIONES*/

int obtener_PID();
int obtener_TID();
void boot();
void obtenerDatosConfig(char**);
void agregar_hilo(t_queue* , TCB_struct);
int solicitar_segmento(int mensaje[2]);
int tamanio_syscalls(void*);
void * extraer_syscalls();
void escribir_memoria(int, int, int, void*);
void crear_colas();
void ejecutarSysCall(int dirSyscall);
int es_CPU(int socket);


int main(int argc, char ** argv){

	crear_colas();
	obtenerDatosConfig(argv);
	TID = 0;
	PID = 0;
	boot();



	return 0;
}

void obtenerDatosConfig(char ** argv){
	t_config * configuracion = config_create(argv[1]);
	PUERTO = config_get_string_value(configuracion, "PUERTO");
	IP_MSP = config_get_string_value(configuracion, "IP_MSP");
	PUERTO_MSP = config_get_string_value(configuracion, "PUERTO_MSP");
	QUANTUM = config_get_int_value(configuracion, "QUANTUM");
	SYSCALLS = config_get_string_value(configuracion, "SYSCALLS");
	TAMANIO_STACK = config_get_int_value(configuracion, "TAMANIO_STACK");
}

void boot(){

	socket_MSP = crear_cliente(IP_MSP, PUERTO_MSP);

	void * syscalls = extraer_syscalls();
	int mensaje_codigo[2];
	mensaje_codigo[0] = pid_KM_boot;
	mensaje_codigo[1] = tamanio_syscalls(syscalls);

	int base_segmento_codigo = solicitar_segmento(mensaje_codigo);
	//TODO: validar la base del segmento
	escribir_memoria(pid_KM_boot, base_segmento_codigo, mensaje_codigo[1], syscalls);
	free(syscalls);

	int mensaje_stack[2];
	mensaje_stack[0] = pid_KM_boot;
	mensaje_stack[1] = TAMANIO_STACK;
	int base_segmento_stack = solicitar_segmento(mensaje_stack);
	//TODO: validar la base del stack
	tcb_km.KM = 1;
	tcb_km.M = base_segmento_codigo;
	tcb_km.tamanioSegmentoCodigo = mensaje_codigo[1];
	tcb_km.P = 0;
	tcb_km.PID = pid_KM_boot;
	tcb_km.S = base_segmento_stack;
	tcb_km.TID = 0;
	tcb_km.X = base_segmento_stack;
	//TODO: inicializar los registros de programacion

	queue_push(BLOCK.prioridad_0, (void *) &tcb_km);

	int socket_gral = crear_servidor(PUERTO,backlog);
	//TODO: validar socket

	FD_ZERO(&clientes_set);
	FD_SET(socket_gral, &clientes_set);

	fd_set copia_set;
	int descriptor_mas_alto = socket_gral;

	while(1){
		copia_set = clientes_set;

		int i = select(descriptor_mas_alto + 1, &copia_set, NULL, NULL, NULL);
		if (i == -1) {
					//ERROR
					break;
		}

		int n_descriptor = 0;

		while (n_descriptor <= descriptor_mas_alto) {

			if (FD_ISSET(n_descriptor,&copia_set)) {

				if (n_descriptor == socket_gral) {
					int socket_conectado = recibir_conexion(socket_gral);
					FD_SET(socket_conectado, &clientes_set);
					int modulo_conectado = -1;
					t_datosAEnviar * datos = recibir_datos(socket_conectado);
					modulo_conectado = datos->codigo_operacion;

					if(modulo_conectado == soy_consola){
						struct_consola consola_conectada;
						int pid = obtener_PID();
						consola_conectada.PID = pid;
						consola_conectada.socket_consola = socket_conectado;
						list_add(consola_list, &consola_conectada);
						//loader(consola_conectada); El PID ya esta asignado y es el que tiene que usar el
						//loader para crear el TCB


					}else if(modulo_conectado == soy_CPU){
						struct_CPU cpu_conectada;
						cpu_conectada.PID = -1;
						cpu_conectada.bit_estado = libre;
						cpu_conectada.socket_CPU = socket_conectado;
						list_add(CPU_list, &cpu_conectada);
					}


				}
				else{
					t_datosAEnviar * datos;
					datos = recibir_datos(n_descriptor);
					int codigo_operacion = datos->codigo_operacion;
					if(es_CPU(n_descriptor)){
						switch(codigo_operacion){
						//TODO:Mensajes que envia la CPU
						}
					}else{
						switch(codigo_operacion){
						//TODO:Mensajes que envia la consola
						}
					}

				}
			}
			n_descriptor++;
		}


	}
}

void interrupcion(TCB_struct tcb, int dirSyscall){
	agregar_hilo(BLOCK.prioridad_1, tcb);
	agregar_hilo(SYS_CALL, tcb);
	//Esperar a que haya alguna cpu libre
	ejecutarSysCall(dirSyscall);
}

/* la SYS_CALL se ejecuta siempre que hay una CPU disponible y haya algun elemento en la cola de
 * SYS_CALL*/

void ejecutarSysCall(int dirSyscall){

	TCB_struct * tcb_user = queue_peek(SYS_CALL);
	tcb_km.registrosProgramacion = tcb_user->registrosProgramacion;
	tcb_km.PID = tcb_user->PID;

	tcb_km.P = dirSyscall;

	//TODO: se envia a ejecutar el tcb a alguna cpu libre
	agregar_hilo(BLOCK.prioridad_0, tcb_km);

	tcb_user->registrosProgramacion = tcb_km.registrosProgramacion;
	queue_pop(BLOCK.prioridad_1);
	//TODO: re-planificar el tcb_user
}

TCB_struct crear_hilo(TCB_struct tcb){

	TCB_struct nuevoTCB;
	int mensaje[2];
	mensaje[0] = tcb.PID;
	mensaje[1] = TAMANIO_STACK;

	int base_stack = solicitar_segmento(mensaje);


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
	reg_programacion nuevosRegistros = *tcb.registrosProgramacion;
	nuevoTCB.registrosProgramacion = &nuevosRegistros;
	return nuevoTCB;
}

int obtener_TID(){
	return TID++;
}

int obtener_PID(){
	return PID++;
}
void agregar_hilo(t_queue * COLA, TCB_struct tcb){

	queue_push(COLA, (void*)&tcb);

}

void planificador(){
	//recibe conexiones de diferentes CPUs
}

/*Esta operacion le solicita a la MSP un segmento, retorna la direccion base del
 * segmento reservado*/
int solicitar_segmento(int mensaje[2]){

	char * datos = malloc(2 * sizeof(int));
	memcpy(datos, &mensaje[0], sizeof(int));
	memcpy(datos + sizeof(int), &mensaje[1], sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(reservar_segmento, (void*) datos, 2*sizeof(int));

	enviar_datos(socket_MSP, paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socket_MSP);

	int dir_base =(int) malloc(sizeof(int));
	memcpy(&dir_base, respuesta->datos, sizeof(int));
	//TODO: validar si no hay violacion de segmento

	return dir_base;
}

void * extraer_syscalls(){
	FILE* archivo_syscalls = fopen(SYSCALLS, "read");
	int tamanio = sizeof(archivo_syscalls);
	void * syscalls = malloc(tamanio);
	memcpy(syscalls, archivo_syscalls, tamanio);
	fclose(archivo_syscalls);
	return syscalls;
}

int tamanio_syscalls(void* syscalls){
	int offset = ftell(syscalls);
	fseek(syscalls, offset, SEEK_SET);
	return sizeof(syscalls);
}

void escribir_memoria(int pid, int dir_logica, int tamanio, void * bytes){

	char * datos = malloc((3 * sizeof(int)) + sizeof(void*));

	memcpy(datos, &pid, sizeof(int));
	memcpy(datos + sizeof(int), &dir_logica, sizeof(int));
	memcpy(datos + (2*sizeof(int)), &tamanio, sizeof(int));
	memcpy(datos + (3*sizeof(int)), bytes, sizeof(void*));

	t_datosAEnviar * paquete = crear_paquete(escribir_en_memoria, datos, (3*sizeof(int)) + sizeof(void*));
	enviar_datos(socket_MSP, paquete);
	free(datos);

	t_datosAEnviar * respuesta = recibir_datos(socket_MSP);

	int rta =(int) malloc(sizeof(int));
	memcpy(&rta, respuesta->datos, sizeof(int));


	//TODO:validar si hay segmentation fault

}

void crear_colas(){
	NEW = queue_create();

	READY.prioridad_0 = queue_create();
	READY.prioridad_1 = queue_create();

	BLOCK.prioridad_0 = queue_create();
	BLOCK.prioridad_1 = queue_create();

	EXEC = queue_create();
	SYS_CALL = queue_create();

	CPU_list = list_create();
	consola_list = list_create();
}

int es_CPU(int socket){

	bool tiene_mismo_socket(struct_CPU estructura){
		return estructura.socket_CPU == socket;
	}

	void * elemento = list_find(CPU_list, (void*)tiene_mismo_socket);
	return (elemento != NULL);
}
