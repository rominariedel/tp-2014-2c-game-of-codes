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
#include <commons/config.h>
#include <commons/collections/list.h>

int tamanioMemoria;
int puerto;
int cantidadSwap;
int sust_pags;
char* MSP_CONFIG;


int main (void)
{
	cargarArchivoConfiguracion();


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
	return 0;
}

void cargarArchivoConfiguracion(void){
	t_config* configuracion = config_create("config.txt");

	tamanioMemoria = config_get_int_value(configuracion, "CANTIDAD_MEMORIA");
	printf("Tamanio Memoria =  %d /n", tamanioMemoria);

	puerto= config_get_int_value(configuracion, "PUERTO");
	printf("Puerto =  %d /n", puerto);

	cantidadSwap = config_get_int_value(configuracion, "CANTIDAD_SWAP");
	printf("Cantidad Swap =  %d /n", cantidadSwap);

	sust_pags = config_get_int_value(configuracion, "SUST_PAGS");
	printf("Algoritmo de Sustitución de Páginas =  %d /n", sust_pags);
	}

