#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <sockets.h>
#include <pthread.h>
#define MAXCONEXIONES 10

enum mensajes {
	reservar_segmento = 1,
	escribir_en_memoria = 2,
	soy_kernel = 29,
	soy_cpu = 19,

	//MENSAJES QUE RECIBIRIA DE LA CPU. ESTOS HAY QUE HACERLOS COINCIDIR CON LOS QUE ESTAN EN KERNEL
	solicitarMemoria = 1,
	crearNuevoSegmento = 2,
	destruirSegmento = 3,
	escribirMemoria = 4,
	solicitarMemoriaP = 80,

};

int kernel_sock;
int cpu_sock;
t_log * logger;

void conectado_a_Kernel();
void conectado_a_cpu();

int main(void) {

	pthread_t hilo_kernel;
	pthread_t hilo_cpu;

	logger = log_create("Logging", "MSP_dummy", false, LOG_LEVEL_INFO);
	int listener; //Socket que recibe las conexiones
	int cliente; //Socket que se me conecto

	listener = crear_servidor("6667", MAXCONEXIONES);
	if (listener < 0) {
		log_error(logger, "FALLO en la creacion del servidor.", "ERROR");
	}
	log_info(logger, "Server conectado.", "INFO");

	while (1) {

		printf("Esperando conexiones\n");

		//Acepto una conexion
		cliente = recibir_conexion(listener);
		printf("Se conecto un cliente\n");
		t_datosAEnviar * datos;
		printf("Esperando mensaje\n");
		datos = recibir_datos(cliente);
		if (datos == NULL ) {
			log_error(logger, "No se recibieron los datos correctamente.",
					"ERROR");
			exit(-1);
		}

		printf("Se recibio un mensaje\n");
		//Recibo mensaje del cliente
		switch (datos->codigo_operacion) {
		case soy_kernel:
			kernel_sock = cliente;
			pthread_create(&hilo_kernel, NULL, (void*) conectado_a_Kernel,
					NULL );
			break;
		case soy_cpu:
			cpu_sock = cliente;
			pthread_create(&hilo_cpu, NULL, (void*) conectado_a_cpu, NULL );
			break;
		}


	}
		pthread_join(hilo_kernel, NULL);
		pthread_join(hilo_cpu, NULL);
		return 0;
}

void conectado_a_Kernel() {
	t_datosAEnviar * datos;
	while (1) {
		printf("Esperando mensaje\n");
		datos = recibir_datos(kernel_sock);
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
			enviar_datos(kernel_sock, paquete);

		}
	}

}

void conectado_a_cpu() {
	t_datosAEnviar * datos;
	while (1) {
		printf("Esperando mensaje\n");
		datos = recibir_datos(cpu_sock);
		if (datos == NULL ) {
			log_error(logger, "No se recibieron los datos correctamente.",
					"ERROR");
			exit(-1);
		}
		printf("Se recibio un mensaje\n");
		//Recibo mensaje del cliente
		int aux = 4;
		t_datosAEnviar * paquete;
		if (1) {
			switch (datos->codigo_operacion) {
			case crearNuevoSegmento:
				printf("RESERVAR SEGMENTO\n");
				//Envio como respuesta una direccion 4
				free(datos);
				datos = malloc(sizeof(int));
				memcpy(datos, &aux, sizeof(int));
				paquete = crear_paquete(0, datos, sizeof(int));
				enviar_datos(cpu_sock, paquete);
				break;
			case escribirMemoria:
				printf("ESCRIBIR EN MEMORIA\n");
				//Envio como respuesta una direccion 4
				free(datos);
				datos = malloc(sizeof(int));
				memcpy(datos, &aux, sizeof(int));
				paquete = crear_paquete(0, datos, sizeof(int));
				enviar_datos(cpu_sock, paquete);
				break;
			case solicitarMemoria:
				printf("SOLICITAR MEMORIA\n");
				char* instruccion1 = "INNN";
				char instruction[4];
				memcpy(instruction, instruccion1, 4);
				printf("instruccionAEjecutar: %s", instruction);
				void* datosMSP = malloc(4);
				memcpy(datosMSP,instruccion1,4);
				paquete = crear_paquete(0,datosMSP,4);
				enviar_datos(cpu_sock, paquete);
				break;

			case solicitarMemoriaP:
				printf("SOLICITAR PARAMETROS\n");
				char* par = "A5";
				void* dat = malloc(2);
				printf("parametros %s", par);
			//	printf("datos a enviar %s", dat);
				memcpy(dat, par,2);
				paquete = crear_paquete(0,dat,2);
				enviar_datos(cpu_sock, paquete);
				break;

			case destruirSegmento:
				break;


			}

		}
	}

}
