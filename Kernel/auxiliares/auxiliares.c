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

	//READY.prioridad_0 = queue_create();
	//READY.prioridad_1 = queue_create();
//	BLOCK.prioridad_0 = queue_create();

	//SYS_CALL = queue_create();

//	BLOCK.prioridad_1 = list_create();
	//EXEC = list_create();
	//CPU_list = list_create();
	//consola_list = list_create();
}

void free_colas(){
	queue_destroy(NEW);
	queue_destroy(EXIT);
}

int tamanio_syscalls(void* syscalls){
	//int offset = ftell(syscalls);
	//fseek(syscalls, offset, SEEK_SET);
	return sizeof(syscalls);
}

FILE * extraer_syscalls(){
	FILE* archivo_syscalls = fopen(SYSCALLS, "read");
	int tamanio = sizeof(archivo_syscalls);
	FILE * syscalls = malloc(tamanio);
	memcpy(syscalls, archivo_syscalls, tamanio);
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

