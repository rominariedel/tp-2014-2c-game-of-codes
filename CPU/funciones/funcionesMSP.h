/*
 * funcionesMSP.h
 *
 *  Created on: 06/11/2014
 *      Author: utnso
 */

#ifndef FUNCIONESMSP_H_
#define FUNCIONESMSP_H_


/*Funciones MSP*/
t_datosAEnviar* MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion);
int MSP_CrearNuevoSegmento(int PID, int tamanioSegmento);
int MSP_DestruirSegmento(int PID, int registro);
t_datosAEnviar* MSP_SolicitarParametros(int PID,int punteroInstruccion, int cantidadParametros);
int MSP_EscribirEnMemoria(int PID, int direccion, void * bytes, int tamanio);
t_datosAEnviar* MSP_SolicitarMemoria(int PID,int direccionALeer, int cantidad, int codOperacion);
int procesarRespuesta(t_datosAEnviar* respuesta);
char* procesarRespuestaMSP(t_datosAEnviar * respuesta);

#endif /* FUNCIONESMSP_H_ */
