/*
 * auxiliares.h
 *
 *  Created on: 27/09/2014
 *      Author: utnso
 */

#ifndef AUXILIARES_H_
#define AUXILIARES_H_

#include<stdio.h>
#include<stddef.h>
#include "commons/config.h"
#include<commons/collections/list.h>
#include"sockets.h"
#include<commons/collections/queue.h>
#include <sys/select.h>


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
	reg_programacion registrosProgramacion;
}TCB_struct;
typedef struct cola{
	TCB_struct TCB;
	struct cola * siguiente_TCB;
}colaPlanificacion;

typedef struct {
	int socket_CPU;
	int bit_estado;
	int PID;
} struct_CPU;

typedef struct{
	int socket_consola;
	int PID;
} struct_consola;


typedef struct {
	t_queue * prioridad_0;
	t_queue * prioridad_1;
} t_colas_prioridades;

typedef struct {
	t_queue * prioridad_0;
	t_list * prioridad_1;
}t_lista_prioridades;

t_colas_prioridades READY;
t_queue * NEW;
t_lista_prioridades * BLOCK;
t_list * EXEC;
t_queue * SYS_CALL;
t_queue * EXIT;

TCB_struct tcb_km;
fd_set clientes_set;
t_list * CPU_list;
t_list * consola_list;


int TID;
int PID;
char * SYSCALLS;
char * PUERTO;
char * IP_MSP;
char * PUERTO_MSP;
int QUANTUM;
int TAMANIO_STACK;
int backlog;
int socket_MSP;


int obtener_TID();
int obtener_PID();
TCB_struct * deserializar_TCB(char * datos);
void crear_colas();
void * extraer_syscalls();


#endif /* AUXILIARES_H_ */