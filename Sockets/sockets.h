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

int crear_servidor(char *, int);
int crear_cliente(char*, char *);

#endif /* SOCKETS_H_ */

