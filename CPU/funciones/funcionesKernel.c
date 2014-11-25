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
	free(datos);
}

int KERNEL_IngreseNumeroPorConsola(int PID){
	//codOperacion = solicitarNumero
	int codigo = atoi("N");
	char * datos = malloc(sizeof(int) * 3);
	int tamanio = 4;
	memcpy(datos, &tamanio, sizeof(int));
	memcpy(datos + sizeof(int), &PIDactual, sizeof(int));
	memcpy(datos + sizeof(int)*2, &codigo, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos, sizeof(int) * 3);

	enviar_datos(socketKernel,paquete);
	free(datos);
	t_datosAEnviar* respuesta = recibir_datos(socketKernel);

	int* numero = malloc(sizeof(int));
	memcpy(numero, respuesta -> datos, sizeof(int));

	return *numero;
}

t_datosAEnviar* KERNEL_IngreseCadenaPorConsola(int PID, int tamanioMaxCadena){
	//codOperacion = solicitarCadena
	int codigo = atoi("C");
	char * datos = malloc(3* sizeof (int));
	memcpy(datos, &tamanioMaxCadena, sizeof(int));
	memcpy(datos + sizeof(int),&PIDactual, sizeof(int));
	memcpy(datos + (2*sizeof(int)), &codigo, sizeof(int));

	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos, sizeof(int) * 3);
	enviar_datos(socketKernel,paquete);
	free(datos);
	t_datosAEnviar* respuesta = recibir_datos(socketKernel);
	return respuesta;
}

void KERNEL_MostrarNumeroPorConsola(int PID, int nro){
	char * toia = string_itoa(nro);
	char * datos = malloc(sizeof (int) + string_length(toia));
	memcpy(datos, &PIDactual, sizeof(int));
	memcpy(datos + sizeof(int), toia, string_length(toia));
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos,string_length(toia) + sizeof(int));
	enviar_datos(socketKernel,paquete);
	free(datos);
	free(paquete);
}


void KERNEL_MostrarCadenaPorConsola(int PID, char* cadena){
	char * datos = malloc(sizeof (int) + string_length(cadena));
	memcpy(datos, &PIDactual, sizeof(int));
	memcpy(datos + sizeof(int), cadena, string_length(cadena));
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos,string_length(cadena) + sizeof(int));
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
	t_TCB* hiloNuevo = (t_TCB *) buffer;

	free(paquete);
	free(respuesta);
	return hiloNuevo;
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
	t_datosAEnviar* respuesta = MSP_SolicitarMemoria(TCB->TID, TCB->X, TCB->S - TCB->X, solicitarMemoria);
	void* stackACopiar = malloc(TCB->S  - TCB->X);
	memcpy(&stackACopiar, respuesta->datos , cursorStackActual - baseStackActual);
	MSP_EscribirEnMemoria(hiloNuevo->PID,hiloNuevo->X, stackACopiar, TCB->S - TCB->X);
	KERNEL_PlanificarHilo(hiloNuevo);
	free(stackACopiar);
}

