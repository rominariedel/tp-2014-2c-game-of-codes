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
}t_datosAEnviar_memoria;

typedef struct {
	t_queue * prioridad_0;
	t_queue * prioridad_1;
} t_colas_prioridades;

typedef struct {
	int id_CPU;
	int bit_estado;
	int PID;
} lista_CPUs;


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
	soy_cpu = 20,
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

t_colas_prioridades READY;
t_queue * NEW;
t_colas_prioridades BLOCK;
t_queue * EXEC;
t_queue * SYS_CALL;
TCB_struct tcb_km;



/*FUNCIONES*/

int obtener_TID(int);
int boot();
void obtenerDatosConfig(char**);
void agregar_hilo(t_queue* , TCB_struct);
int solicitar_segmento(int mensaje[2]);
int tamanio_syscalls(void*);
void * extraer_syscalls();
t_datosAEnviar_memoria crear_paquete(int, int, void*, int);
int escribir_memoria(int, int, void*, int);
void crear_colas();
void ejecutarSysCall(int dirSyscall);



int main(int argc, char ** argv){
	crear_colas();
	obtenerDatosConfig(argv);
	TID = 0;
	int socket_gral = boot();
	//TODO: validar socket

	while(1){
		int modulo_conectado = -1;
		recv(socket_gral, &modulo_conectado, sizeof(int), 0);
		if(modulo_conectado == 19){
			//acciones de la consola
		}else if(modulo_conectado == 20){
			//acciones de CPU
		}
	}


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

int boot(){

	socket_MSP = crear_cliente(IP_MSP, PUERTO_MSP);

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
	return socket_gral;
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

int obtener_TID(int pid){
	TID ++;
	return TID;
}

void agregar_hilo(t_queue * COLA, TCB_struct tcb){

	queue_push(COLA, (void*)&tcb);

}

void planificador(){
	//recibe conexiones de diferentes CPUs
}

/*Esta operacion le solicita a la MSP un segmento de STACK o CODIGO, retortna la direccion base del
 * segmento reservado*/
int solicitar_segmento(int mensaje[2]){
	int todo_ok;
	int respuesta;
	send(socket_MSP, (void*) reservar_segmento, sizeof(int), 0);
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
	int respuesta;

	send(socket_MSP, (void*) escribir_en_memoria, sizeof(int), 0);
	recv(socket_MSP, &todo_ok, sizeof(int), 0);

	t_datosAEnviar_memoria paquete = crear_paquete(pid, dir_logica, bytes, tamanio);
	send(socket_MSP, &paquete, sizeof(t_datosAEnviar_memoria), 0);
	recv(socket_MSP, &respuesta, sizeof(int), 0);
	//TODO:validar si hay segmentation fault
	return respuesta;

}

t_datosAEnviar_memoria crear_paquete(int pid, int dir_logica, void* bytes, int tamanio){
	t_datosAEnviar_memoria paquete;
	paquete.pid = pid;
	paquete.dir_logica = dir_logica;
	void * bytes_a_enviar = malloc(tamanio);
	memcpy(bytes_a_enviar, bytes, tamanio);
	paquete.bytes = bytes_a_enviar;
	paquete.tamanio = tamanio;
	return paquete;
}

void crear_colas(){
	NEW = queue_create();

	READY.prioridad_0 = queue_create();
	READY.prioridad_1 = queue_create();

	BLOCK.prioridad_0 = queue_create();
	BLOCK.prioridad_1 = queue_create();

	EXEC = queue_create();
	SYS_CALL = queue_create();
}
