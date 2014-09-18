/*
 * sdfs.c
 *
 *  Created on: 13/09/2014
 *      Author: utnso
 */

#include "sockets.h"



static char* serializar(t_datosAEnviar * paquete){
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

static void deserializar_datos(char * buffer, t_datosAEnviar * datos_recibidos){
	void * datos = malloc(datos_recibidos->tamanio);
	memcpy(datos_recibidos->datos, datos, datos_recibidos->tamanio);
	free(datos);
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
	hints.ai_family = AF_UNSPEC;		// No importa si uso IPv4 o IPv6
	hints.ai_flags = AI_PASSIVE;		// Asigna el address del localhost: 127.0.0.1
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE
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

int enviar_datos(int socket, t_datosAEnviar * paquete){

	int cantidad_enviada;
	int enviando = 1;
	int offset = 0;
	char * buffer = serializar(paquete);
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
	deserializar_datos(buffer, datos_recibidos);
	free(buffer);

	return datos_recibidos;
}
