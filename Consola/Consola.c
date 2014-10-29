/*
 * main.c
 *
 *  Created on: 04/09/2014
 *      Author: utnso
 */

#include <commons/config.h>
#include <sockets.h>
#include <commons/log.h>

enum mensajes {
	imprimir_en_pantalla = 4, ingresar_cadena = 5, codigo_consola = 25,
};

char * extraer_data(char * path);
int tamanio_codigo;

int main(int argc, char ** argv) {

	char* puerto, *ip;
	t_log * logger = log_create("Logging", "Consola", false, LOG_LEVEL_INFO);

	printf("----------------CONSOLA------------------\n");

	//Extrayendo datos del archivo de configuracion y del codigo
	t_config* config = config_create(argv[1]);
	ip = config_get_string_value(config, "IP");
	puerto = config_get_string_value(config, "PUERTO");
	char * buffer = extraer_data(argv[2]);

	log_info(logger, "Tamanio codigo %d.", tamanio_codigo, "INFO");

	//Conectando al kernel
	int kernelSocket;
	kernelSocket = crear_cliente(ip, puerto);
	if (kernelSocket < 0) {
		log_error(logger, "FALLO en la conexion con el kernel.", "ERROR");
		return EXIT_FAILURE;
	}

	printf("Conectado al Kernel. Ya se puede enviar el codigo ESO\n");
	log_info(logger, "Se realizo la conexion con el kernel.", "INFO");

	t_datosAEnviar * paquete = crear_paquete(codigo_consola, buffer,
			tamanio_codigo);
	if (enviar_datos(kernelSocket, paquete) < 0) {
		log_error(logger, "FALLO al enviar datos al kernel.", "ERROR");
		return EXIT_FAILURE;
	}
	log_info(logger, "Se enviaron los primeros datos exitosamente.", "INFO");
	free(paquete);

	//Comienza la recepcion de datos
	while (1) {
		paquete = recibir_datos(kernelSocket);
		char * datos_a_imprimir;
		switch (paquete->codigo_operacion) {
		case imprimir_en_pantalla:
			datos_a_imprimir = malloc(paquete->tamanio);
			memcpy(datos_a_imprimir, paquete->datos, paquete->tamanio);
			printf(
					"Se ha solicitado salida estandar de los siguientes datos:\n %s\n",
					datos_a_imprimir);
			break;
		case ingresar_cadena:
			break;
		}
		free(paquete);
	}

	close(kernelSocket);

	return 0;
}

long tamanio_archivo(FILE* archivo) {
	fseek(archivo, 0, SEEK_END);
	int tamanio = ftell(archivo);
	rewind(archivo);
	return tamanio;
}

char * extraer_data(char * path) {
	printf("Extrayendo datos del archivo\n");
	FILE* archivo = fopen(path, "read");
	tamanio_codigo = tamanio_archivo(archivo);
	char * buffer = malloc(tamanio_codigo); //MMAP
	fread((void*) buffer, 1, tamanio_codigo, archivo);
	fclose(archivo);
	printf("Datos copiados exitosamente\n");
	return buffer;
}
