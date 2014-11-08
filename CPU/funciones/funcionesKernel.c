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
	char codigo = 'N';
	char * datos = malloc(sizeof (char));
	memcpy(datos, &codigo, sizeof(char));
	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos, sizeof(char));
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
	char * datos = malloc(sizeof(char) + sizeof (int));
	memcpy(datos, &codigo, sizeof(char));
	memcpy(datos + sizeof(char), &tamanioMaxCadena, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos, sizeof(int) + sizeof(char));
	enviar_datos(socketKernel,paquete);
	free(datos);
	t_datosAEnviar* respuesta = recibir_datos(socketKernel);
	return respuesta;
}

void KERNEL_MostrarNumeroPorConsola(int PID, int nro){
	char codigo = 'N';
	char * datos = malloc(sizeof (int) + sizeof(char));
	memcpy(datos, &codigo, sizeof(char));
	memcpy(datos + sizeof(char), &nro, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos, sizeof(int) + sizeof(char));
	enviar_datos(socketKernel,paquete);
	free(datos);
	free(paquete);
}


void KERNEL_MostrarCadenaPorConsola(int PID, char* cadena){
	char codigo = 'C';
	char * datos = malloc(string_length(cadena) + sizeof(char));
	memcpy(datos, &codigo, sizeof(char));
	memcpy(datos + sizeof(char), &cadena, string_length(cadena));
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos, string_length(cadena) + sizeof(char));
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
	t_datosAEnviar* paquete = crear_paquete(bloquear, (void*) datos, sizeof(int));
	enviar_datos(socketKernel, paquete);
	free(datos);
	free(paquete);
}

void KERNEL_CrearHilo(t_TCB * TCB, int registro){
	t_TCB* hiloNuevo = KERNEL_CrearNuevoHilo(TCB);
		A = hiloNuevo->TID;
		hiloNuevo->punteroInstruccion = registro;
		t_datosAEnviar* respuesta = MSP_SolicitarMemoria(TCB->TID, TCB->baseStack, TCB->cursorStack - TCB->baseStack, solicitarMemoria);
		void* stackACopiar = malloc(TCB->cursorStack  - TCB->baseStack);
		memcpy(&stackACopiar, respuesta->datos , cursorStackActual - baseStackActual);
		MSP_EscribirEnMemoria(hiloNuevo->PID,hiloNuevo->baseStack, stackACopiar, TCB->cursorStack - TCB->baseStack);
		KERNEL_PlanificarHilo(hiloNuevo);
		free(stackACopiar);
}

