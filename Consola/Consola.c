/*
 * main.c
 *
 *  Created on: 04/09/2014
 *      Author: utnso
 */

#include <commons/config.h>
#include <sockets.h>
#include <commons/log.h>
#include <math.h>
#include <commons/string.h>

enum mensajes {
	soy_consola = 18,
	imprimir_en_pantalla = 4,
	ingresar_cadena = 5,
	codigo_consola = 25,
	se_produjo_entrada = 26,
	terminar_conexion = 27,
	se_desconecto_cpu = 45,
};

char * extraer_data(char * path);
void evaluar_ingreso(void *);
void ingresar_cadena_menorA(int);
void ingresar_numero();

int tamanio_codigo;
int kernelSocket;

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
	kernelSocket = crear_cliente(ip, puerto);
	if (kernelSocket < 0) {
		log_error(logger, "FALLO en la conexion con el kernel.", "ERROR");
		return EXIT_FAILURE;
	}
	t_datosAEnviar * paquete;
	printf("Conectado al Kernel. Ya se puede enviar el codigo ESO. Socket %d\n", kernelSocket);
	log_info(logger, "Se realizo la conexion con el kernel.", "INFO");

	//AUTENTICACION
	paquete = crear_paquete(soy_consola, NULL, 0);
	if(enviar_datos(kernelSocket, paquete)>=0){
	printf("SE ENVIARON DATOS\n");
	}else{
		printf("NO\n");
	}
	free(paquete);


	paquete = crear_paquete(codigo_consola, buffer,
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
		if(paquete == NULL){
			printf("FALLO al recibir datos \n");
			return EXIT_FAILURE;
		}
		void * datos;
		char * datos_a_imprimir;
		void * solicitud_ingreso;
		switch (paquete->codigo_operacion) {
		case imprimir_en_pantalla:
			datos_a_imprimir = malloc(paquete->tamanio + 1 );
			memcpy(datos_a_imprimir, paquete->datos, paquete->tamanio);
			datos_a_imprimir[paquete->tamanio + 1] = '\0';
			printf(
					"Se ha solicitado salida estandar de los siguientes datos:\n %s\n",
					datos_a_imprimir);
			break;
		case ingresar_cadena:
			solicitud_ingreso = malloc(paquete->tamanio);
			memcpy(solicitud_ingreso, paquete->datos, paquete->tamanio);
			evaluar_ingreso(solicitud_ingreso);
			break;
		case terminar_conexion:
			free(paquete);
			printf("SE TERMINO LA CONEXION! \n");
			datos = crear_paquete(0, NULL, 0);
			enviar_datos(kernelSocket,datos);
			free(datos);
			return EXIT_SUCCESS;
		case se_desconecto_cpu:
			printf("EXCEPCION: Se produjo una desconexión de la cpu que estaba ejecutando el proceso. Abortando.\n");
			datos = crear_paquete(0, NULL, 0);
			enviar_datos(kernelSocket,datos);
			free(datos);
			return EXIT_SUCCESS;
		}
		free(paquete);
	}

	close(kernelSocket);

	return 0;
}

void evaluar_ingreso(void * solicitud) {
	printf("EVALUANDO INGRESO \n");
	char primera_letra;
	memcpy(&primera_letra, solicitud, sizeof(char));
	//memcpy(&primera_letra, solicitud, sizeof(int));
	printf("solicitud : %c", primera_letra);
	if (primera_letra == 'N') {
		//Se ha solicitado que se ingrese un numero entre 0 y 2³¹
		ingresar_numero();
	} else if (primera_letra == 'C' ) {
		//Se ha solicitado que se ingrese una cadena de longitud menor que el segundo parametro
		int segunda_letra;

		memcpy(&segunda_letra, solicitud + sizeof(char), sizeof(int));
		printf("SEGUNDA LETRA : %d", segunda_letra);
		ingresar_cadena_menorA(segunda_letra);
	}
}

void ingresar_cadena_menorA(int tamanio) {
	printf("6 ingresar cadena menor \n");
	int recibido_not_success = 1;

	while (recibido_not_success) {

		char * cadena = malloc(tamanio);
		printf("Ingrese una cadena con menos de %d caracteres\n", tamanio);
		scanf("%s", cadena);
		int largo_cadena = string_length(cadena);
		if ((largo_cadena <= tamanio) && (largo_cadena > 0)) {
			t_datosAEnviar * datos = crear_paquete(se_produjo_entrada, cadena,
					largo_cadena);
			printf("Se ingresaron datos de tamanio %d\n", largo_cadena);
			enviar_datos(kernelSocket, datos);
			recibido_not_success = 0;
		} else {
			printf("La cadena no es valida\n");
		}

	}
}

void ingresar_numero() {
	int recibido_not_success = 1;

	while (recibido_not_success) {
		int numero;
		printf("Ingrese un numero entre 0 y 2147483648\n");
		scanf("%d", &numero);
		if ((numero > 0) && (numero <= pow(2, 31))) {
			t_datosAEnviar * datos = crear_paquete(se_produjo_entrada, &numero,
					sizeof(int));
			enviar_datos(kernelSocket, datos);
			recibido_not_success = 0;
		} else {
			//Si no se ingreso un numero correcto
			printf(
					"El numero ingresado no esta dentro de los rangos permitidos\n");
		}
	}
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
