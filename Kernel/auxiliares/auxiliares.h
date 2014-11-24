/*
 * auxiliares.h
 *
 *  Created on: 27/09/2014
 *      Author: utnso
 */

#ifndef AUXILIARES_H_
#define AUXILIARES_H_

#include <stdio.h>
#include <stddef.h>
#include <commons/config.h>
#include <sockets.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <sys/select.h>
#include <commons/string.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/dictionary.h>
#include <panel.h>
#include <kernel.h>

/*          ESTRUCTURAS          */

typedef struct tcb {
	int PID;
	int TID;
	int KM;
	int M;
	int tamanioSegmentoCodigo;
	int P;
	int X;
	int S;
	int registrosProgramacion[5];
} TCB_struct;

typedef struct cpu {
	int socket_CPU;
	int bit_estado;
	int PID;
} struct_CPU;

typedef struct consola {
	int socket_consola;
	int PID;
	int cantidad_hilos;
	int TID_padre;
	bool termino_ejecucion;
} struct_consola;

typedef struct colas {
	t_queue * prioridad_0;
	t_queue * prioridad_1;
} t_colas_prioridades;

typedef struct listas {
	t_queue * prioridad_0;
	t_list * prioridad_1;
} t_lista_prioridades;

typedef struct bloqueado {
	int id_recurso;
	TCB_struct tcb;
} struct_bloqueado;

/*TIPOS DE BLOQUEADOS: manejo
 * INTERRUMPIDOS POR EJECUTAR SYSCALLS -> BLOCK.prioridad_1
 * ESPERANDO UN RECURSO -> dictionary >> key=recurso >> t_queue *
 * ESPERANDO A OTRO HILO (EJECUTO UN JOIN) -> hilos_join
 * */


typedef struct joins {
	TCB_struct * tcb_llamador;
	int tid_a_esperar;
} struct_join;

/*      SEMÁFOROS      */

sem_t sem_procesoListo;
sem_t sem_CPU;
sem_t mutex_entradaSalida;
sem_t sem_READY;
sem_t sem_kmDisponible;
sem_t sem_exit;

/*       FUNCIONES        */

int obtener_TID();
int obtener_PID();
TCB_struct * deserializar_TCB(char *);
void crear_colas();
void meter_en_ready(int, TCB_struct *);
TCB_struct * sacar_de_ready(int);
bool CPU_esta_libre(struct_CPU*);
void planificar(TCB_struct);
void free_colas();
void free_listas();
long tamanio_syscalls(FILE*);
void abortar(TCB_struct*); //VER
void crear_hilo(TCB_struct, int);
void finalizo_ejecucion(TCB_struct*);
void finalizo_quantum(TCB_struct*);
void interrumpir(TCB_struct*, int);
void planificador();
struct_consola * obtener_consolaConectada(int);
struct_consola * obtener_consolaAsociada(int);
struct_bloqueado * obtener_bloqueado(int);
void producir_salida_estandar(int, char*);
void producir_entrada_estandar(int, char*, int, int);
void devolver_entrada_aCPU(int);
void realizar_join(TCB_struct *, int);
char * extraer_syscalls(char *);
void planificar_hilo_creado(TCB_struct *);
void realizar_bloqueo(TCB_struct *, int);
void realizar_desbloqueo(int);
int chequear_proceso_abortado(TCB_struct *);

enum mensajes {

	//Mensajes enviados

	crear_segmento = 32,
	escribir_en_memoria = 2,
	ejecucion_abortada = 3,
	imprimir_en_pantalla = 4,
	ingresar_cadena = 5,
	ejecutar = 6,
	devolucion_cadena = 7,
	terminar_conexion = 27,
	soy_kernel = 29,
	destruir_segmento = 30,
	leer_memoria = 31, //Este todavía no se si tengo que usarlo

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
	planificar_nuevo_hilo = 28,

	//-->MSP
	operacion_exitosa = 1, //Esto  me lo va a mandar si destruyo bien los segmentos y con todas las otras operaciones
	error_memoriaLlena = -3,
	error_segmentationFault = -2,
	error_general = -1,

	//-->CONSOLA
	soy_consola = 18,
	codigo_consola = 25,
	se_produjo_entrada = 26,
};

#endif /* AUXILIARES_H_ */
