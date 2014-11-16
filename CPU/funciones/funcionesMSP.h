/*
 * funcionesMSP.h
 *
 *  Created on: 06/11/2014
 *      Author: utnso
 */

#ifndef FUNCIONESMSP_H_
#define FUNCIONESMSP_H_


/*Funciones MSP*/
char* MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion);
int MSP_CrearNuevoSegmento(int PID, int tamanioSegmento);
void MSP_DestruirSegmento(int PID, int registro);
char* MSP_SolicitarParametros(int punteroInstruccion, int cantidadParametros);
void MSP_EscribirEnMemoria(int PID, int direccion, void * bytes, int tamanio);
t_datosAEnviar* MSP_SolicitarMemoria(int PID,int direccionALeer, int cantidad, int codOperacion);
int procesarRespuesta(t_datosAEnviar* respuesta);

#endif /* FUNCIONESMSP_H_ */
