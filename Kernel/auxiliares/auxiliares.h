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
#include <commons/config.h>
#include<commons/collections/list.h>
#include <sockets.h>
#include<commons/collections/queue.h>
#include <sys/select.h>
#include <commons/string.h>
#include <pthread.h>
#include <semaphore.h>

enum mensajes {

	//Mensajes enviados

	reservar_segmento = 1,
	escribir_en_memoria = 2,
	ejecucion_abortada = 3,
	imprimir_en_pantalla = 4,
	ingresar_cadena = 5,
	ejecutar = 6,
	devolucion_cadena = 7,

	//Mensajes recibidos

	//-->CPU
	finaliza_quantum = 10,
	finaliza_ejecucion = 11,
	ejecucion_erronea = 12,
	desconexion = 13,
	interrupcion = 14,
	creacion_hilo = 15,
	soy_CPU = 19,
	entrada_estandar = 20,
	salida_estandar = 21,
	join = 22,
	bloquear = 23,
	despertar = 24,

	//-->MSP
	error_memoriaLlena = 16,
	error_segmentationFault = 17,

	//-->CONSOLA
	soy_consola = 18,
	codigo_consola = 25,
	se_produjo_entrada = 26,
};

/*          ESTRUCTURAS          */

typedef struct {
	int A;
	int B;
	int C;
	int D;
	int E;
} reg_programacion;

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
} TCB_struct;

typedef struct {
	int socket_CPU;
	int bit_estado;
	int PID;
} struct_CPU;

typedef struct {
	int socket_consola;
	int PID;
	int cantidad_hilos;
} struct_consola;

typedef struct {
	t_queue * prioridad_0;
	t_queue * prioridad_1;
} t_colas_prioridades;

typedef struct {
	t_queue * prioridad_0;
	t_list * prioridad_1;
} t_lista_prioridades;

typedef struct {
	int id_recurso;
	TCB_struct tcb;
} struct_bloqueado;

/*      SEMÁFOROS      */

sem_t sem_procesoListo;
sem_t sem_CPU;
sem_t mutex_entradaSalida;
sem_t sem_entrada;
/*       FUNCIONES        */

int obtener_TID();
int obtener_PID();
TCB_struct * deserializar_TCB(char * datos);
void crear_colas();
int CPU_esta_libre(struct_CPU cpu);
void planificar(TCB_struct);
void free_colas();
void free_listas();
long tamanio_syscalls(FILE*);
void abortar(TCB_struct*); //VER
void crear_hilo(TCB_struct);
void finalizo_ejecucion(TCB_struct*);
void finalizo_quantum(TCB_struct*);
void interrumpir(TCB_struct*, int);
void planificador();
struct_consola * obtener_consolaConectada(int);
struct_consola * obtener_consolaAsociada(int PID);
struct_bloqueado * obtener_bloqueado(int TID);
void producir_salida_estandar(int pid, char* cadena);
void producir_entrada_estandar(int pid, char* id_tipo, int socket_CPU, int tamanio);
void devolver_entrada_aCPU(int tamanio_datos);

#endif /* AUXILIARES_H_ */
