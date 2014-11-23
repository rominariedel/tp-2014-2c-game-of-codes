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

//FUNCIONES DEL KERNEL.C

void loader();
void boot();
void obtenerDatosConfig(char**);
void boot();
void escribir_memoria(TCB_struct *, int, int, void*);
void obtenerDatosConfig(char**);
void sacar_de_ejecucion(TCB_struct *);
int solicitar_segmento(TCB_struct * , int);
void iniciar_semaforos();
void enviar_a_ejecucion(TCB_struct *);
void dispatcher();
void copiarRegistros(int registro1[5], int registro2[5]);
void handshake_MSP(int socketMSP);
void fijarse_joins(int tid);
void matar_hijos(int PID);
void matar_hijos_en_lista(int, t_list*);
void matar_hijo_en_diccionario(int PID);
void mandar_a_exit(TCB_struct*);


#endif /* VARIABLES_GLOBALES_H_ */
