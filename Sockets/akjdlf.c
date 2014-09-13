/*
 * sdfs.c
 *
 *  Created on: 13/09/2014
 *      Author: utnso
 */

#include "sockets.h"


int crear_servidor(char * PUERTO, int backlog, int tamanioMaximo){
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


