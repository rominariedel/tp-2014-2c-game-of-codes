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

int crear_servidor(char *, int);
int crear_cliente(char*, char *);
t_datosAEnviar* crear_paquete(int cod_op, void * datos, int tamanio);
int enviar_datos(int socket, t_datosAEnviar * paquete);
t_datosAEnviar * recibir_datos(int socket);
int recibir_conexion(int socket);

#endif /* SOCKETS_H_ */

