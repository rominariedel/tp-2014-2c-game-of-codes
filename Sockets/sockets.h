/*
 * sockets.h
 *
 *  Created on: 13/09/2014
 *      Author: utnso
 */

#ifndef SOCKETS_H_
#define SOCKETS_H_


/*PARA USAR LA LIBRERIA: Propiedades del proyecto >> C/C++ General >> Path and Symbols >> References
 * >>Tildar Sockets*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define tamanio_header (2*sizeof(int))


typedef struct{
	int codigo_operacion;
	void * datos;
	int tamanio;
}t_datosAEnviar;

enum mensajes {

	//Mensajes enviados

	crear_segmento = 1,
	escribir_en_memoria = 2,
	ejecucion_abortada = 3,
	imprimir_en_pantalla = 4,
	ingresar_cadena = 5,
	ejecutar = 6,
	devolucion_cadena = 7,
	terminar_conexion = 27,
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

int crear_servidor(char *, int);
int crear_cliente(char*, char *);
t_datosAEnviar* crear_paquete(int cod_op, void * datos, int tamanio);
int enviar_datos(int socket, t_datosAEnviar * paquete);
t_datosAEnviar * recibir_datos(int socket);
int recibir_conexion(int socket);

#endif /* SOCKETS_H_ */

