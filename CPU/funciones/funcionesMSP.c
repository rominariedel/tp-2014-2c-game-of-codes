#include "../CPU.h"


/*Funciones MSP*/

t_datosAEnviar* MSP_SolicitarMemoria(int PID,int direccionALeer, int cantidad, int codOperacion){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &direccionALeer, cantidad);
	t_datosAEnviar * paquete = crear_paquete(codOperacion, (void*) datos, sizeof(int) + cantidad);

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	return respuesta;
}

char* MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion){
	int tamanio = 4;
	log_info(LOGCPU, "  Envio paquete a MSP  ");
	char * datos = malloc(3 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int));
	memcpy(datos + sizeof(int) + sizeof(int), &tamanio , sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 3 * sizeof(int));

	printf("1 \n");

	enviar_datos(socketMSP,paquete);
	printf("2 \n");
	free(datos);
	printf("3 \n");
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);
	printf("4 \n");
	log_info(LOGCPU, "  Recibo Respuesta MSP  ");
	printf("5 \n");
	int status = procesarRespuesta(respuesta);
	printf("6 \n");
	char* proximaInstruccion = calloc(4, sizeof(char));
	printf("7 \n");
	if(status == 0){
		printf("8 \n");
		memcpy(proximaInstruccion, respuesta -> datos, 4);
	}else{
		if(status <0){
			abortar(ejecucion_erronea);
			printf("9 \n");
		}
	}
	printf("10 \n");
	free(respuesta);
	printf("11 \n");
	printf("\n ME ESTA MANDANDO BIEN LA INSTRUCCION PROXIMA!!!!!!!!!!!! \n");

	return proximaInstruccion;
}

char* MSP_SolicitarParametros(int punteroInstruccion, int tamanioParametros){
	char * datos = malloc(3 * sizeof (int));
	memcpy(datos, &PIDactual, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int)); //ese puntero instruccion es el punteroInstruccionActual + 4
	memcpy(datos + sizeof(int) + sizeof(int), &tamanioParametros, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 3 * sizeof(int));
	enviar_datos(socketMSP,paquete);
	free(datos);
	free(paquete);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int status = procesarRespuesta(respuesta);

	char* parametros = calloc(respuesta->tamanio, sizeof(char));
	if(status == 0){
		memcpy(parametros, respuesta -> datos, respuesta->tamanio);
	}else{
		devolverTCBactual(ejecucion_erronea);
	}

	free(respuesta);

	return parametros;

}

int MSP_CrearNuevoSegmento(int PID, int tamanioSegmento){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &tamanioSegmento, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(crearNuevoSegmento, (void*) datos, 2* sizeof(int));

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int status = procesarRespuesta(respuesta);

	int * dir_base = malloc(sizeof(int));
	if(status == 0){
		memcpy(dir_base, respuesta -> datos, sizeof(int));
	}else{
		memcpy(dir_base,&status, 1);
	}

	free(respuesta);

	return *dir_base;

}

void MSP_DestruirSegmento(int PID, int baseSegmento){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &baseSegmento, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(destruirSegmento, (void*) datos, 2* sizeof(int));
	enviar_datos(socketMSP,paquete);

	t_datosAEnviar * respuesta = recibir_datos(socketMSP);
	procesarRespuesta(respuesta);
	free(respuesta);
	free(datos);
}

void MSP_EscribirEnMemoria(int PID, int direccion, void * bytes, int tamanio) {

	char * datos = malloc((3 * sizeof(int)) + tamanio);

	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &direccion, sizeof(int));
	memcpy(datos + 2 * sizeof(int), bytes, tamanio);
	memcpy(datos + 2* sizeof(int) + tamanio, &tamanio, sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(escribirMemoria, datos,2 * sizeof(int) + tamanio);
	enviar_datos(socketMSP, paquete);
	free(datos);
	free(paquete);

	t_datosAEnviar* respuesta  = recibir_datos(socketMSP);
	procesarRespuesta(respuesta);
	free(respuesta);
}

int procesarRespuesta(t_datosAEnviar* respuesta){
	log_info(LOGCPU, "Procesando respuesta MSP");
	int estado;
	errorOperacionesConMemoria = 0;
	errorMemoria = 0;
	estado = 0;
	if(respuesta == NULL){
		errorOperacionesConMemoria = -1;
		errorMemoria = no_llego_respuesta;
		estado = -1;
		log_error(LOGCPU, "ERROR MSP: No se pudieron recibir datos MSP");
		return estado;
	}else{
	if(respuesta->codigo_operacion < 0){
	switch(respuesta->codigo_operacion){
		case error_segmentationFault:
			devolverTCBactual(error_segmentationFault);
			errorOperacionesConMemoria = -1;
			errorMemoria = error_segmentationFault;
			estado = -1;
			log_error(LOGCPU, "ERROR MSP: Segmentation Fault");
			break;
		case error_memoriaLlena:
			devolverTCBactual(error_memoriaLlena);
			errorOperacionesConMemoria = -1;
			errorMemoria = error_memoriaLlena;
			estado = -1;
			log_error(LOGCPU, "ERROR MSP: Memoria Llena");
			break;
	}
		estado = -1;
		errorMemoria = -5; //POR DECIR ALGUN NRO NEGATIVO
		log_error(LOGCPU, "ERROR MSP: Me devolvio algo negativo, no es ni Segmentation Fault, ni Memoria Llena");
		return estado;
	}}

	return estado;
}
