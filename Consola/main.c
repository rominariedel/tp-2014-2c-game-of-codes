/*
 * main.c
 *
 *  Created on: 04/09/2014
 *      Author: utnso
 */

#include <commons/config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>


typedef struct {
	char ip[15];
	char puerto[4];
} t_regConfig;//definicion de la estructura del archivo de configuracion para q sea mas facil leerlo
char * ESO_CONFIG;
int main(int argc, char ** argv){
	int puerto = 9;
	char* ip;

	t_config* config = config_create(argv[1]);
	ip = config_get_string_value(config, "IP");
	puerto = config_get_int_value(config, "PUERTO");

	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(ip, (char*) puerto, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion

	int kernelSocket;
	kernelSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	connect(kernelSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);	// No lo necesitamos mas

	printf("Conectado al Kernel. Ya se puede enviar el codigo ESO\n");

	// cargar el contenido del archivo con el codigo ESO;

	//send(kernelSocket, ACA VA LO QUE LE ENVIAMOS,ACA HAY QUE PONER CUANTO PESA LO UQE LE ENVIAMOS sizeof(char), 0);
	//recv(kernelSocket, &conf, sizeof(char), 0);

	//log_info(logger, "Se establecio conexion con la UMV");


	close(kernelSocket);

	return 0;
}
