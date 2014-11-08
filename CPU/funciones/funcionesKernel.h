/*
 * funcionesKernel.h
 *
 *  Created on: 06/11/2014
 *      Author: utnso
 */


#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

typedef struct {
	int PID;
	int TID;
	int KM;
	int baseSegmentoCodigo;
	int tamanioSegmentoCodigo;
	int punteroInstruccion;
	int baseStack;
	int cursorStack;
	int registrosProgramacion[5];
}t_TCB;

t_TCB* TCBactual;


/*Funciones Kernel*/
void KERNEL_ejecutarRutinaKernel(int codOperacion, int direccion);
int KERNEL_IngreseNumeroPorConsola(int PID);
t_datosAEnviar* KERNEL_IngreseCadenaPorConsola(int PID, int tamanioMaxCadena);
void KERNEL_MostrarNumeroPorConsola(int PID, int nro);
void KERNEL_MostrarCadenaPorConsola(int PID, char* cadena);
t_TCB* KERNEL_CrearNuevoHilo(t_TCB* TCB);
void KERNEL_PlanificarHilo(t_TCB* hiloNuevo);
void KERNEL_JoinTCB(t_TCB* TCB, int TIDabloquear);
void KERNEL_BloquearTCB(t_TCB* TCB, int recursoABloquear);
void KERNEL_WakePrograma(int recurso);
void KERNEL_CrearHilo(t_TCB * TCB, int registro);

#endif /* FUNCIONESKERNEL_H_ */
