/*
 * main.c
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#include<stdio.h>
#include<stddef.h>
#include <commons/config.h>
#include<commons/collections/list.h>
#include"sockets.h"

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
	int pid;
	int dir_logica;
	void* bytes;
	int tamanio;
}t_datosAEnviar;

/*VARIABLES GLOBALES*/

char * PUERTO;
char * IP_MSP;
char * PUERTO_MSP;
int QUANTUM;
char * SYSCALLS;
int TAMANIO_STACK;
int backlog;
int socket_MSP;
int socket_CPU;

colaPlanificacion * NEW;
colaPlanificacion * READY;
colaPlanificacion * BLOCK;
colaPlanificacion * EXEC;
colaPlanificacion * SYS_CALL;
TCB_struct tcb_km;



/*FUNCIONES*/

int obtener_TID(int);
void boot();
void obtenerDatosConfig(char**);
void agregar_hilo(colaPlanificacion *, TCB_struct);
int solicitar_segmento(int mensaje[2]);
int tamanio_syscalls(void*);
void * extraer_syscalls();
t_datosAEnviar crear_paquete(int, int, void*, int);
int escribir_memoria(int, int, void*, int);


int main(int argc, char ** argv){
	obtenerDatosConfig(argv);
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

	socket_MSP = crear_servidor(PUERTO_MSP, backlog);
	tcb_km.KM = 1;
	BLOCK->TCB=tcb_km;
	BLOCK->siguiente_TCB = NULL;
	void * syscalls = extraer_syscalls();
	int mensaje_codigo[2];
	mensaje_codigo[0] = pid_KM_boot;
	mensaje_codigo[1] = tamanio_syscalls(syscalls);

	int base_segmento_codigo = solicitar_segmento(mensaje_codigo);
	//TODO: validar la base del segmento
	escribir_memoria(pid_KM_boot, base_segmento_codigo, syscalls, mensaje_codigo[1]);
	free(syscalls);

	int mensaje_stack[2];
	mensaje_stack[0] = pid_KM_boot;
	mensaje_stack[1] = TAMANIO_STACK;
	int base_segmento_stack = solicitar_segmento(mensaje_stack);
	//TODO: validar la base del stack

	tcb_km.M = base_segmento_codigo;
	tcb_km.P = base_segmento_codigo;
	tcb_km.PID = pid_KM_boot;
	tcb_km.S = base_segmento_stack;
	tcb_km.TID = 0;
	tcb_km.X = base_segmento_stack;
	//TODO: inicializar los registros de programacion

}

void interrupcion(TCB_struct tcb, int dirSysCall){
	agregar_hilo(BLOCK, tcb);
	agregar_hilo(SYS_CALL, tcb);
}

/* la SYS_CALL se ejecuta siempre que hay una CPU disponible y haya algun elemento en la cola de
 * SYS_CALL*/

void ejecutarSysCall(int dirSysCall){


	tcb_km.registrosProgramacion = SYS_CALL->TCB.registrosProgramacion;
	tcb_km.PID = SYS_CALL->TCB.PID;

	tcb_km.P = dirSysCall;

	//TODO: se envia a ejecutar el tcb a alguna cpu libre
	agregar_hilo(BLOCK, tcb_km);

	SYS_CALL ->TCB.registrosProgramacion = tcb_km.registrosProgramacion;
	//TODO: desbloquear el tcb de usuario para ser re-planificado
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

int obtener_TID(int pid){
	return 0;
}

void agregar_hilo(colaPlanificacion * COLA, TCB_struct tcb){
	list_add((void*) COLA,(void*) &tcb);
}

void planificador(){
	//recibe conexiones de diferentes CPUs
}

/*Esta operacion le solicita a la MSP un segmento de STACK o CODIGO*/
int solicitar_segmento(int mensaje[2]){
	int todo_ok;
	int respuesta;
	int reservar_segmento = 1;
	send(socket_MSP, &(reservar_segmento), sizeof(int), 0);
	recv(socket_MSP, &todo_ok, sizeof(int), 0);
	//TODO: validar que se pueda reservar un segmento

	send(socket_MSP, mensaje, 2*(sizeof(int)), 0);
	recv(socket_MSP, &respuesta, sizeof(int), 0);
	//TODO:validar que se haya podido reservar el segmento, si no hay error de memoria llena

	return respuesta;
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

int escribir_memoria(int pid, int dir_logica, void* bytes, int tamanio){

	int todo_ok;
	int escribir_en_memoria = 2;
	int respuesta;

	send(socket_MSP, &escribir_en_memoria, sizeof(int), 0);
	recv(socket_MSP, &todo_ok, sizeof(int), 0);

	t_datosAEnviar paquete = crear_paquete(pid, dir_logica, bytes, tamanio);
	send(socket_MSP, &paquete, sizeof(t_datosAEnviar), 0);
	recv(socket_MSP, &respuesta, sizeof(int), 0);
	//TODO:validar si hay segmentation fault
	return respuesta;

}

t_datosAEnviar crear_paquete(int pid, int dir_logica, void* bytes, int tamanio){
	t_datosAEnviar paquete;
	paquete.pid = pid;
	paquete.dir_logica = dir_logica;
	void * bytes_a_enviar = malloc(tamanio);
	memcpy(bytes_a_enviar, bytes, tamanio);
	paquete.bytes = bytes_a_enviar;
	paquete.tamanio = tamanio;
	return paquete;
}
