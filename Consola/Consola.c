/*
 * main.c
 *
 *  Created on: 04/09/2014
 *      Author: utnso
 */

#include <commons/config.h>
#include <sockets.h>
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

enum mensajes{
	codigo_consola = 25,
};

char * extraer_data(char * path);
int tamanio_codigo;

int main(int argc, char ** argv){
	char* puerto = "1122";
	char* ip = "127.0.0.1";
	printf("INICIANDO CONSOLA\n");
	//t_config* config = config_create(argv[1]);
	//ip = config_get_string_value(config, "IP");
	//puerto = config_get_string_value(config, "PUERTO");

	char * buffer = extraer_data(argv[1]);

	int kernelSocket;
	printf("iniciando la creacion del cliente\n");
	kernelSocket = crear_cliente(ip, puerto);
	if(kernelSocket<0){
		printf("FALLO en la conexion con el kernel \n");
		return EXIT_FAILURE;
	}
	printf("Conectado al Kernel. Ya se puede enviar el codigo ESO\n");

	t_datosAEnviar * paquete = crear_paquete(codigo_consola, buffer, tamanio_codigo);
	if(enviar_datos(kernelSocket, paquete) < 0) {
		printf("FALLO al enviar datos al kernel\n");
		return EXIT_FAILURE;
	}
	printf("Se enviaron los primeros datos exitosamente\n");
	free(paquete);
	while(1){
		paquete = recibir_datos(kernelSocket);
		switch(paquete->codigo_operacion){
		 //ACCIONES
		}
		free(paquete);
	}


	close(kernelSocket);

	return 0;
}

long tamanio_archivo(FILE* archivo){
	fseek(archivo, 0, SEEK_SET);
	int tamanio= ftell(archivo);
	rewind(archivo);
	return tamanio;
}

char * extraer_data(char * path){
	printf("Extrayendo datos del archivo\n");
	FILE* archivo = fopen(path, "read");
	tamanio_codigo = tamanio_archivo(archivo);
	char * buffer = malloc(tamanio_codigo); //MMAP
	fread((void*) buffer, 1, tamanio_codigo, archivo);
	fclose(archivo);
	printf("Datos copiados exitosamente\n");
	return buffer;
}
