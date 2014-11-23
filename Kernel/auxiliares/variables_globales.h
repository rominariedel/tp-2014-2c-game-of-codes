/*
 * variables_globales.h
 *
 *  Created on: 18/10/2014
 *      Author: utnso
 */

#ifndef VARIABLES_GLOBALES_H_
#define VARIABLES_GLOBALES_H_

#include "auxiliares.h"

t_colas_prioridades READY;
t_lista_prioridades BLOCK;
t_list * EXEC;
t_queue * SYS_CALL;
t_queue * EXIT;
t_dictionary * dic_bloqueados;
t_list * hilos_join;

//TCB_struct * tcb_km;
TCB_struct * tcb_ejecutandoSysCall;
fd_set clientes_set;
fd_set CPU_set;
fd_set consola_set;
t_list * CPU_list;
t_list * consola_list;
t_config * configuracion;

typedef struct {
	int socket_CPU; //El Socket pertenece a la CPU que hizo la solicitud de entrada/salida
	void * cadena;
} entrada_salida;
entrada_salida * entrada;

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
long tamanio_codigo_syscalls;
int socket_gral;
int descriptor_mas_alto_consola;
int descriptor_mas_alto_cpu;

#endif /* VARIABLES_GLOBALES_H_ */
