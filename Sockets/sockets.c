/*
 * sdfs.c
 *
 *  Created on: 13/09/2014
 *      Author: utnso
 */

#include "sockets.h"

int suma(int cant_args, int arg_tamanio[cant_args]);

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

static void deserializar(char * buffer, t_datosAEnviar * datos_recibidos){
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
	deserializar(buffer, datos_recibidos);
	free(buffer);

	return datos_recibidos;
}


/* Recibe un número que indica la cantidad de elementos del paquete, un vector que indica el tamaño
 * de cada elemento del paquete, y un vector con punteros a los elementos del paquete.
 *
 * Los elementos de arg_tamanio y argumentos tienen que corresponderse (p.ej arg_tamanio[1] tiene que
 * tener el tamaño de argumentos[1]), y el buffer que retorna tiene copiados los elementos en orden
 * inverso al que se mandó en el vector*/

char* serializar_datos(int cant_args, int arg_tamanio[cant_args], void ** argumentos[cant_args]){
	int tamanio_total = suma(cant_args, arg_tamanio);
	char * buffer = malloc(tamanio_total);
	int offset = tamanio_total;
	while(cant_args > 0){
		offset = offset -arg_tamanio[cant_args-1];
		memcpy(buffer + offset, *(argumentos[cant_args-1]), arg_tamanio[cant_args-1]);
		cant_args --;
	}
	return buffer;
}

int suma(int cant_args, int arg_tamanio[cant_args]){
	int total = 0;
	while(cant_args){
		total = total + arg_tamanio[cant_args-1];
		cant_args --;
	}
	return total;
}

void * deserializar_datos(int cant_args, int arg_tamanio[cant_args], char * buffer){
	void * argumentos[cant_args];
	int offset = 0;
	while(cant_args>0){
		memcpy(argumentos[cant_args-1], buffer + offset, arg_tamanio[cant_args-1]);
		offset = offset + arg_tamanio[cant_args-1];
		cant_args --;
	}
	free(buffer);
	return argumentos[cant_args];
}
