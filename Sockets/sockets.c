/*
 * sdfs.c
 *
 *  Created on: 13/09/2014
 *      Author: utnso
 */

#include "sockets.h"

static char* serializar_paquete(t_datosAEnviar * paquete){
	char * paquete_corrido = malloc(paquete->tamanio + tamanio_header);
	memcpy(paquete_corrido, paquete, tamanio_header);
	memcpy(paquete_corrido + tamanio_header, paquete->datos, paquete->tamanio);
	return paquete_corrido;
}


static t_datosAEnviar* deserializar_header(char * buffer){
	t_datosAEnviar * paquete = malloc(tamanio_header);
	memcpy(&paquete->codigo_operacion, buffer, sizeof(int));
	memcpy(&paquete->tamanio, buffer + sizeof(int), sizeof(int));
	return paquete;
}

static void serializar_datos(char * buffer, t_datosAEnviar * datos_recibidos){
	datos_recibidos-> datos = malloc(datos_recibidos->tamanio);
	memcpy(datos_recibidos->datos, buffer, datos_recibidos->tamanio);
}



t_datosAEnviar* crear_paquete(int cod_op, void * datos, int tamanio){

	t_datosAEnviar * paquete = malloc(sizeof(t_datosAEnviar));

	paquete->codigo_operacion = cod_op;
	paquete->datos = malloc(tamanio);
	memcpy(paquete->datos, datos, tamanio);
	paquete->tamanio = tamanio;

	return paquete;
}

int crear_servidor(char * PUERTO, int backlog){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo);
	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	listen(listenningSocket, backlog);
	return listenningSocket;
}

int crear_cliente(char* IP, char * PUERTO){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);
	return serverSocket;
 }

int recibir_conexion(int socket){
	struct sockaddr_in addr;			// Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(socket, (struct sockaddr *) &addr, &addrlen);

	return socketCliente;
}

int enviar_datos(int socket, t_datosAEnviar * paquete){

	int cantidad_enviada;
	int enviando = 1;
	int offset = 0;
	char * buffer = serializar_paquete(paquete);
	int cantidad_total = paquete->tamanio + tamanio_header;
	while(enviando){
		cantidad_enviada = send(socket, buffer + offset, cantidad_total-offset, 0);
		if(cantidad_enviada==-1){
			//TODO: Loguear error
			return -1;
		}
		if(cantidad_enviada<cantidad_total){
			cantidad_total = cantidad_total - offset;
			offset= cantidad_enviada;
		}else
			enviando = 0;
	}
	return 0;
}

t_datosAEnviar * recibir_datos(int socket){

	//Recibe header
	char * buffer = malloc(tamanio_header);
	int tamanio_recibido_header = recv(socket, buffer, tamanio_header, MSG_WAITALL);
	if(tamanio_recibido_header <= 0){
		//TODO: loguear error
		return NULL;
	}

	//Copia header
	t_datosAEnviar * datos_recibidos = deserializar_header(buffer);
	free(buffer);

	//Recibe datos
	buffer = malloc(datos_recibidos ->tamanio);
	int tamanio_recibido_datos = recv(socket, buffer, datos_recibidos ->tamanio, MSG_WAITALL);
	if(tamanio_recibido_datos <0){
		//TODO: loguear error
		return NULL;
	}

	//Copia datos
	serializar_datos(buffer, datos_recibidos);
	free(buffer);

	return datos_recibidos;
}



