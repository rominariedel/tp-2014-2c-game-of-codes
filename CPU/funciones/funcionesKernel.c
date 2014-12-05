/*
 * funcionesKernel.c
 *
 *  Created on: 06/11/2014
 *      Author: utnso
 */
/* Funciones Kernel*/

#include "../CPU.h"

void KERNEL_ejecutarRutinaKernel(int codOperacion, int direccion){
	char* datos = malloc(sizeof(t_TCB) + sizeof(int));
	log_info(LOGCPU, "\n ESTOY EN INTERRUPCION \n");
	log_info(LOGCPU, "Incrementar punteroInstruccion %d", punteroInstruccionActual);
	punteroInstruccionActual += 8;
	log_info(LOGCPU, "Puntero Instruccion actual: %d", punteroInstruccionActual);
	actualizarTCB();
	memcpy(datos, TCBactual,sizeof(t_TCB));
	memcpy(datos + sizeof(t_TCB), &direccion, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(codOperacion, (void*) datos, sizeof(t_TCB) + sizeof(int));
	enviar_datos(socketKernel, paquete);
	free(paquete);
	//free(datos);
}

int KERNEL_IngreseNumeroPorConsola(int PID){
	//codOperacion = solicitarNumero
	char codigo = 'N';
	char * datos = malloc(sizeof(int) * 2 + sizeof(char));
	int tamanio = 4;
	memcpy(datos, &tamanio, sizeof(int));
	memcpy(datos + sizeof(int), &PIDactual, sizeof(int));
	memcpy(datos + sizeof(int)*2, &codigo, sizeof(char));
	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos, sizeof(int) * 2 + sizeof(char));

	enviar_datos(socketKernel,paquete);
	free(datos);
	t_datosAEnviar* respuesta = recibir_datos(socketKernel);
	int* numero = malloc(sizeof(int));
	memcpy(numero, respuesta -> datos, sizeof(int));

	return *numero;
}

t_datosAEnviar* KERNEL_IngreseCadenaPorConsola(int PID, int tamanioMaxCadena){
	//codOperacion = solicitarCadena
	char codigo = 'C';
	char * datos = malloc(2* sizeof (int) + sizeof(char));
	memcpy(datos, &tamanioMaxCadena, sizeof(int));
	memcpy(datos + sizeof(int),&PIDactual, sizeof(int));
	memcpy(datos + (2*sizeof(int)), &codigo, sizeof(char));

	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos, sizeof(int) * 2 + sizeof(char));
	enviar_datos(socketKernel,paquete);
	free(datos);
	t_datosAEnviar* respuesta = recibir_datos(socketKernel);
	return respuesta;
}

void KERNEL_MostrarNumeroPorConsola(int PID, int nro){            //TODO PREGUNTAR ROMI SI VA PID TID O NOOOOOOOOO!!!!!!
	log_info(LOGCPU, "----------------------------------------------------------------------------------------numero a mostrar: %d", nro);
	char * toia = string_itoa(nro);
	char * datos = malloc(sizeof (int) *2 + string_length(toia));
	memcpy(datos,&PIDactual, sizeof(int));
	memcpy(datos+sizeof(int), &TIDactual, sizeof(int));
	memcpy(datos + sizeof(int)*2, toia, string_length(toia));
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos,string_length(toia) +2* sizeof(int));
	enviar_datos(socketKernel,paquete);
	free(datos);
	free(paquete);
}


void KERNEL_MostrarCadenaPorConsola(int PID, char* cadena){

	printf("\n\n\n\n CADENAAAAAAAAAAAAA: %s\n\n\n\n",cadena);
	char * datos = malloc(sizeof (int) * 2 + string_length(cadena) + 1);
	memcpy(datos, &PIDactual, sizeof(int));
	memcpy(datos+sizeof(int), &TIDactual, sizeof(int));
	memcpy(datos + sizeof(int)*2, cadena, string_length(cadena));
	datos[sizeof (int) * 2 + string_length(cadena)] = '\0';
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos,string_length(cadena) + 2* sizeof(int) + 1);
	enviar_datos(socketKernel,paquete);
	free(datos);
	free(paquete);
}

t_TCB* KERNEL_CrearNuevoHilo(t_TCB* TCB){
	char * datos = malloc(sizeof(t_TCB));
	actualizarTCB();
	memcpy(datos, TCB, sizeof(t_TCB));
	t_datosAEnviar* paquete = crear_paquete(crear_hilo, (void*)datos, sizeof(t_TCB));
	enviar_datos(socketKernel, paquete);

	t_datosAEnviar * respuesta = recibir_datos(socketKernel);

	char* buffer  = malloc(respuesta -> tamanio);
	memcpy(buffer,respuesta->datos, respuesta -> tamanio);

/*	TCBactual -> TID = TIDactual;
		TCBactual -> KM = KMactual;
		TCBactual -> M = baseSegmentoCodigoActual;
		TCBactual -> tamanioSegmentoCodigo = tamanioSegmentoCodigoActual;
		TCBactual -> P = punteroInstruccionActual;
		TCBactual -> X = baseStackActual;
		TCBactual -> S = cursorStackActual;

	t_TCB* hiloNuevo = (t_TCB *) buffer;
*/

	free(paquete);
	free(respuesta);
	free(respuesta->datos);
	return (t_TCB*)buffer;
}

void KERNEL_PlanificarHilo(t_TCB* hiloNuevo){
	char * datos = malloc(sizeof(t_TCB));
	memcpy(datos, hiloNuevo, sizeof(t_TCB));
	t_datosAEnviar* paquete = crear_paquete(planificar_hilo, (void*) datos, sizeof(t_TCB));
	enviar_datos(socketKernel, paquete);
	free(datos);
	free(paquete);
}

void KERNEL_JoinTCB(t_TCB* TCB, int TIDabloquear){
	char * datos = malloc(sizeof(t_TCB) + sizeof(int));
	memcpy(datos, TCB, sizeof(t_TCB));
	memcpy(datos + sizeof(t_TCB) , &TIDabloquear, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(join, (void*) datos, sizeof(t_TCB) + sizeof(int));
	enviar_datos(socketKernel, paquete);
	free(datos);
	free(paquete);
}

void KERNEL_BloquearTCB(t_TCB* TCB, int recursoABloquear){
	actualizarTCB();
	char * datos = malloc(sizeof(t_TCB) + sizeof(int));
	memcpy(datos, TCB, sizeof(t_TCB));
	memcpy(datos + sizeof(t_TCB) , &recursoABloquear, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(bloquear, (void*) datos, sizeof(t_TCB) + sizeof(int));
	enviar_datos(socketKernel, paquete);
	free(datos);
	free(paquete);
}

void KERNEL_WakePrograma(int recurso){
	char * datos = malloc(sizeof(t_TCB));
	memcpy(datos, &recurso, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(despertar, (void*) datos, sizeof(int));
	enviar_datos(socketKernel, paquete);
	free(datos);
	free(paquete);
}

void KERNEL_CrearHilo(t_TCB * TCB, int registro){
	t_TCB* hiloNuevo = KERNEL_CrearNuevoHilo(TCB);
	A = hiloNuevo->TID;
	hiloNuevo->P = registro;
	printf("\n TAMANIO CREA: %d \n",TCB->S - TCB->X );
	noPIDkm = -1;
	t_datosAEnviar* respuesta = MSP_SolicitarMemoria(TCB->PID, TCB->X, TCB->S - TCB->X, solicitarMemoria);
	int status= procesarRespuesta(respuesta);
	if(status < 0){
		finalizarEjecucion = -1;
	}else{
		printf("STACK A COPIAR TAMANIO %d \n", cursorStackActual - baseStackActual);
		void* stackACopiar = malloc(cursorStackActual - baseStackActual);
		memcpy(stackACopiar, respuesta->datos , cursorStackActual - baseStackActual);
		MSP_EscribirEnMemoria(hiloNuevo->PID,hiloNuevo->X, stackACopiar, TCB->S - TCB->X);
		int status2= procesarRespuesta(respuesta);
		if(status2 < 0){
			finalizarEjecucion = -1;
		}
		KERNEL_PlanificarHilo(hiloNuevo);
		free(stackACopiar);
	}
}

