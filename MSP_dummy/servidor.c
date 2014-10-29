#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <sockets.h>
#define MAXCONEXIONES 10

enum mensajes {
	reservar_segmento = 1,
	escribir_en_memoria = 2,
	ejecucion_abortada = 3,
	imprimir_en_pantalla = 4,
	ingresar_cadena = 5,
	ejecutar = 6,
	devolucion_cadena = 7,
};

int main(void) {

	t_log * logger = log_create("Logging", "MSP_dummy", false, LOG_LEVEL_INFO);
	int listener; //Socket que recibe las conexiones
	int cliente; //Socket que se me conect�
	int clienteConectado; //Flag

	listener = crear_servidor("6667", MAXCONEXIONES);
	if (listener < 0) {
		log_error(logger, "FALLO en la creacion del servidor.", "ERROR");
	}
	log_info(logger, "Server conectado.", "INFO");

	while (1) {

		printf("Esperando conexiones\n");

		//Acepto una conexion
		cliente = recibir_conexion(listener);
		clienteConectado = 1;
		printf("Se conecto un cliente\n");
		t_datosAEnviar * datos;
		while (clienteConectado) {
			printf("Esperando mensaje\n");
			datos = recibir_datos(cliente);
			if (datos == NULL ) {
				log_error(logger, "No se recibieron los datos correctamente.",
						"ERROR");
				exit(-1);
			}
			printf("Se recibio un mensaje\n");
			//Recibo mensaje del cliente
			if (1) {
				switch (datos->codigo_operacion) {
				case reservar_segmento:
					printf("RESERVAR SEGMENTO\n");
					break;
				case escribir_en_memoria:
					printf("ESCRIBIR EN MEMORIA\n");
					break;

				}

				//Envio como respuesta una direccion 4
				free(datos);
				datos = malloc(sizeof(int));
				int aux = 4;
				memcpy(datos, &aux, sizeof(int));
				t_datosAEnviar * paquete = crear_paquete(0, datos, sizeof(int));
				enviar_datos(cliente, paquete);

			} else
				clienteConectado = 0;
		}
		printf("Se desconecto el cliente\n");

		//Cierro el socket del cliente
		close(cliente);
	}

	return 0;

}
