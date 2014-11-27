#include "../CPU.h"


/*Funciones MSP*/

t_datosAEnviar* MSP_SolicitarMemoria(int PID,int direccionALeer, int cantidad, int codOperacion){
	char * datos = malloc(3 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &direccionALeer, sizeof(int));
	memcpy(datos + sizeof(int) *2, &cantidad, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(codOperacion, (void*) datos, sizeof(int) + sizeof(int) + sizeof(int));
	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);
	if(respuesta==NULL){

	}

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

t_datosAEnviar* MSP_SolicitarParametros(int punteroInstruccion, int tamanioParametros){
	int tamanio = tamanioParametros;
	char * datos = malloc(3 * sizeof (int));
	memcpy(datos, &PIDactual, sizeof(int));
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
	}else{
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
	free(respuesta);
	free(datos);

	return status;
}

int MSP_EscribirEnMemoria(int PID, int direccion, void * bytes, int tamanio) {

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
	int status = procesarRespuesta(respuesta);
	free(respuesta);
	if(status < 0){
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
		log_error(LOGCPU, "ERROR MSP: No se pudieron recibir datos MSP");
		return estado;
	}else{
	if(respuesta->codigo_operacion < 0){
		log_info(LOGCPU, "PROCESAR RESPUESTA : %d", respuesta->codigo_operacion);
		printf("PROCESAR RESPUESTA: %d", respuesta->codigo_operacion);
	switch(respuesta->codigo_operacion){
		case error_segmentationFault:
			estado = error_segmentationFault;
			log_error(LOGCPU, "ERROR MSP: Segmentation Fault");
			break;
		case error_memoriaLlena:
			estado = error_memoriaLlena;
			log_error(LOGCPU, "ERROR MSP: Memoria Llena");
			break;
	}
		log_error(LOGCPU, "ERROR MSP: Me devolvio algo negativo, no es ni Segmentation Fault, ni Memoria Llena");
		return estado;
	}}

	return estado;
}











