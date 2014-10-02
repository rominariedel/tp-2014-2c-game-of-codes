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
	BLOCK->prioridad_0 = queue_create();

	SYS_CALL = queue_create();

	BLOCK->prioridad_1 = list_create();
	EXEC = list_create();
	CPU_list = list_create();
	consola_list = list_create();
}


int tamanio_syscalls(void* syscalls){
	int offset = ftell(syscalls);
	fseek(syscalls, offset, SEEK_SET);
	return sizeof(syscalls);
}

void * extraer_syscalls(){
	FILE* archivo_syscalls = fopen(SYSCALLS, "read");
	int tamanio = sizeof(archivo_syscalls);
	void * syscalls = malloc(tamanio);
	memcpy(syscalls, archivo_syscalls, tamanio);
	fclose(archivo_syscalls);
	return syscalls;
}
