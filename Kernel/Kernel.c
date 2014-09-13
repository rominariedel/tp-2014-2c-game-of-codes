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

int obtener_TID(int);
void boot();
void obtenerDatosConfig(char**);
void agregar_hilo(colaPlanificacion *, TCB_struct);

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
	//aca hay que pedirle a la MSP un segmento de codigo para llenarlo con el
	//contenido compilado del Archivo de syscalls del ensamblador
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

	//se envia a ejecutar el tcb a alguna cpu libre
	agregar_hilo(BLOCK, tcb_km);

	SYS_CALL ->TCB.registrosProgramacion = tcb_km.registrosProgramacion;
	//desbloquear el tcb de usuario para ser re-planificado
}

TCB_struct crear_hilo(TCB_struct tcb){

	TCB_struct nuevoTCB;
	//Crear un segmento de stack en la MSP
	int tid = obtener_TID(tcb.PID);
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
