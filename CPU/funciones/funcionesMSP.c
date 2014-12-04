#include "../CPU.h"


/*Funciones MSP*/

t_datosAEnviar* MSP_SolicitarMemoria(int PID,int direccionALeer, int cantidad, int codOperacion){
	if(KMactual == 1){
		PID = PIDkm;
	}

	if(noPIDkm == -1){
		PID = PIDactual;
	}

	char * datos = malloc(3 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &direccionALeer, sizeof(int));
	memcpy(datos + sizeof(int) *2, &cantidad, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(codOperacion, (void*) datos, sizeof(int) + sizeof(int) + sizeof(int));
	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);
	return respuesta;
}

t_datosAEnviar* MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion){
	int tamanio = 4;
	log_info(LOGCPU, "  Envio paquete a MSP  ");
	char * datos = malloc(3 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int));
	memcpy(datos + sizeof(int) + sizeof(int), &tamanio , sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 3 * sizeof(int));
	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);
	return respuesta;
}

t_datosAEnviar* MSP_SolicitarParametros(int PID,int punteroInstruccion, int tamanioParametros){
	if(KMactual == 1){
		PID = PIDkm;
	}

	int tamanio = tamanioParametros;
	char * datos = malloc(3 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int)); //ese puntero instruccion es el punteroInstruccionActual + 4
	memcpy(datos + sizeof(int) + sizeof(int), &tamanio, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 3 * sizeof(int));
	enviar_datos(socketMSP,paquete);
	free(datos);
	free(paquete);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);
	return respuesta;
}

char* procesarRespuestaMSP(t_datosAEnviar * respuesta){
	char* respuestaMSP = calloc(respuesta->tamanio + 1, sizeof(char));
	log_info(LOGCPU, "Llegaron bien los parametros");
	memcpy(respuestaMSP, respuesta -> datos, respuesta->tamanio);
	return respuestaMSP;
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
	}else{if(status<0){
		printf("ERROR MSP al crear Nuevo Segmento\n");
		log_info(LOGCPU,"ERROR MSP al crear Nuevo Segmento");

	}

		return status;
	}

	free(respuesta);

	return *dir_base;

}

int MSP_DestruirSegmento(int PID, int baseSegmento){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &baseSegmento, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(destruirSegmento, (void*) datos, 2* sizeof(int));
	enviar_datos(socketMSP,paquete);

	t_datosAEnviar * respuesta = recibir_datos(socketMSP);
	procesarRespuesta(respuesta);

	int status = procesarRespuesta(respuesta);
	if(status < 0){
		printf("MSP: No se pudo destruir Segmento \n");
		log_error(LOGCPU,"No se pudo destruir Segmento \n");
	}
	free(respuesta);
	free(datos);

	return status;
}

int MSP_EscribirEnMemoria(int PID, int direccion, void * bytes, int tamanio) {

	char * datos = malloc((3 * sizeof(int))+ tamanio);

	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &direccion, sizeof(int));
	memcpy(datos + 2 * sizeof(int), bytes, tamanio);
	memcpy(datos + 2* sizeof(int) + tamanio, &tamanio, sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(escribirMemoria, datos,3 * sizeof(int) + tamanio);
	enviar_datos(socketMSP, paquete);
	free(datos);
	free(paquete);

	t_datosAEnviar* respuesta  = recibir_datos(socketMSP);
	int status = procesarRespuesta(respuesta);
	if(status < 0){
		printf("No se pudo Escribir en Memoria \n");
		log_error(LOGCPU,"No se pudo Escribir en Memoria \n");
		return status;
	}
	free(respuesta);
	return 0;
}

int procesarRespuesta(t_datosAEnviar* respuesta){
	log_info(LOGCPU, "Procesando respuesta MSP");
	int estado;
	estado = 0;
	if(respuesta == NULL){
		estado = ejecucion_erronea;
		printf("ERROR MSP: No se pudieron recibir datos MSP");
		log_error(LOGCPU, "ERROR MSP: No se pudieron recibir datos MSP");
		return estado;
	}else{
	if(respuesta->codigo_operacion < 0){
	switch(respuesta->codigo_operacion){
		case error_segmentationFault:
			estado = error_segmentationFault;
			printf("ERROR MSP: Segmentation Fault");
			log_error(LOGCPU, "ERROR MSP: Segmentation Fault");
			break;
		case error_memoriaLlena:
			estado = error_memoriaLlena;
			printf("ERROR MSP: Memoria Llena");
			log_error(LOGCPU, "ERROR MSP: Memoria Llena");
			break;
	}
	}}

	return estado;
}











