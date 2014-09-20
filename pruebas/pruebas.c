/*
 * pruebas.c
 *
 *  Created on: 14/09/2014
 *      Author: utnso
 */

#include<stdio.h>
#include "sockets.h"
#include "commons/string.h"
int main(){

	char * palabra = "hola";
	char* otra_palabra = "mundo ";

	int tamanio[2];
	tamanio[0] = sizeof(*palabra) * string_length(palabra);
	tamanio[1] = sizeof(*otra_palabra) * string_length(otra_palabra);
	char* buffer = malloc(suma(2, (void*)tamanio[2]));
	char* args[2];
	args[0] = palabra;
	args[1] = otra_palabra;
	buffer = serializar_datos(2, tamanio, (void*)args);

	printf("afds, %s\n", buffer);
	free(buffer);

	int numero = 5;
	int otro_numero = 10;
	void * arg_num[2];
	arg_num[1] =  string_itoa(otro_numero);
	arg_num[0] =  string_itoa(numero);
	int tamanio_num[2];
	tamanio_num[0] = string_length(arg_num[0]);
	tamanio_num[1] = string_length(arg_num[1]);
	char * otro_buffer = malloc(tamanio_num[0] + tamanio_num[1]);
	otro_buffer = serializar_datos(2, tamanio_num, (void*)arg_num);
	printf("kjhkj %s\n", otro_buffer);
	free(otro_buffer);
	return EXIT_SUCCESS;
}
