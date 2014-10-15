/*
 * auxiliares.c
 *
 *  Created on: 27/09/2014
 *      Author: utnso
 */

#include "auxiliares.h"

int obtener_TID(){
	return TID++;
}

int obtener_PID(){
	return PID++;
}


void crear_colas(){
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
}

void free_colas(){
	queue_destroy(NEW);
	queue_destroy(EXIT);
}

long tamanio_syscalls(FILE* syscalls){
	fseek(syscalls, 0, SEEK_END);
	return ftell(syscalls);
}

char * extraer_syscalls(){
	FILE* archivo_syscalls = fopen(SYSCALLS, "read");
	tamanio_codigo_syscalls = tamanio_syscalls(archivo_syscalls);
	char * syscalls = malloc(tamanio_codigo_syscalls);
	fread((void*) syscalls, 1, tamanio_codigo_syscalls, archivo_syscalls);
	fclose(archivo_syscalls);
	return syscalls;
}

void mover_a_exit(TCB_struct * tcb){
	queue_push(EXIT, tcb);
}


bool CPU_esta_libre(struct_CPU cpu){
	return cpu.bit_estado;
}


void planificar(TCB_struct tcb){
	queue_push(NEW, &tcb);
	queue_pop(NEW);
	queue_push(EXIT, &tcb);
}

