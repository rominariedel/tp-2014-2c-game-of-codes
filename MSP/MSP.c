/*
 * main.c
 *
 *  Created on: 08/09/2014
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <pthread.h>


typedef struct {
		int sizeMemory;
} t_regConfigMSP;

char* MSP_CONFIG;


int main (int cantArgs, char** args)
{
	int sizeMem;
	t_config* configuracion = config_create(args[1]);
	sizeMem = config_get_int_value(configuracion, "sizeMemoria");
	printf("tamanio Memoria = " %d /n, sizeMem);

	/*t_config* configuracion = config_create(args[1]);
	logi = log_create(args[2], "UMV", 0, LOG_LEVEL_TRACE);
	int sizeMem = config_get_int_value(configuracion, "sizeMemoria");

	memPpal = malloc (sizeMem);	//El tamaño va por configuracion
	char* aux = (char*) memPpal;
	finMemPpal = memPpal + sizeMem; //El tamaño va por configuracion
	for(i=0; i<sizeMem; i++){aux[i] = 0;}

	rhConsola = pthread_create(&consola, NULL, mainConsola, NULL);
	rhEsperarConexiones = pthread_create(&esperarConexiones, NULL, mainEsperarConexiones, NULL);

	pthread_join(consola, NULL);
	pthread_join(esperarConexiones, NULL);

	printf("%d",rhConsola);
		printf("%d",rhEsperarConexiones);

		exit(0);*/

}

