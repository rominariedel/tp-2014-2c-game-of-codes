/*
 * CPU.h
 *
 *  Created on: 24/10/2014
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include <commons/config.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <math.h>
#include <error.h>
#include <sys/socket.h>
#include <sockets.h>
#include "t_parametros.h"


/* Estructuras */

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


typedef struct {
	int PID;
	int TID;
	int KM;
	int M;
	int tamanioSegmentoCodigo;
	int P;
	int X;
	int S;
	int registrosProgramacion[5];
}TCB_struct;

/*Datos actuales*/
int PIDactual;
int TIDactual;
int KMactual;   //KM == 1 el programa puede ejecutar las instrucciones protegidas. esta en modo kernel.
int baseSegmentoCodigoActual;
int tamanioSegmentoCodigoActual;
int punteroInstruccionActual;
int baseStackActual;
int cursorStackActual;

/*Registros CPU*/
int A = 0;
int B = 0;
int C = 0;
int D = 0;
int E = 0;

/* Variables Globales */
int socketKernel;
int socketMSP;

char* PUERTOMSP;
char* IPMSP;
char* PUERTOKERNEL;
char* IPKERNEL;
int RETARDO;

t_TCB* TCBactual;
int quantum = 0;

t_log* LOGCPU;



/*Definicion de funciones*/
void cargarArchivoConfiguracion(int cantArgs, char** args);
void conectarConMSP();
void conectarConKernel();
void abortarEjecucion();
void cargarRegistrosCPU();
void actualizarRegistrosTCB();
int cargarDatosTCB();
int actualizarTCB();
void devolverTCBactual(int codigoOperacion);
void limpiarRegistros();
int recibirTCByQuantum(t_datosAEnviar *  datosKernel);
int* devolverRegistro(char registro);
int interpretarYEjecutarInstruccion(char* instruccion);
char* deserializarPaqueteMSP(t_datosAEnviar* paqueteMSP);
char* deserializarPaqueteKernel(t_datosAEnviar* paqueteKernel);

/*Funciones MSP*/
char* MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion);
int MSP_CrearNuevoSegmento(int PID, int tamanioSegmento);
t_datosAEnviar*  MSP_DestruirSegmento(int PID, int registro);
char* MSP_SolicitarParametros(int punteroInstruccion, int cantidadParametros);
void MSP_EscribirEnMemoria(int PID, int direccion, void * bytes, int tamanio);

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

/*Instrucciones*/
void LOAD(tparam_load*);
void SETM(tparam_setm*);
void GETM(tparam_getm*);
void MOVR(tparam_movr*);
void ADDR(tparam_addr*);
void SUBR(tparam_subr*);
void MULR(tparam_mulr*);
void MODR(tparam_modr*);
void DIVR(tparam_divr*);
void INCR(tparam_incr*);
void DECR(tparam_decr*);
void COMP(tparam_comp*);
void CGEQ(tparam_cgeq*);
void CLEQ(tparam_cleq*);
void GOTO(tparam_goto*);
void JMPZ(tparam_jmpz*);
void JPNZ(tparam_jpnz*);
void INTE(tparam_inte*);
void SHIF(tparam_shif*);
void NOPP();
void PUSH(tparam_push*);
void TAKE(tparam_take*);
void XXXX();

/*Instrucciones Protegidas*/
void MALC();
void FREE();
void INNN();
void INNC();
void OUTN();
void OUTC();
void CREA();
void JOIN();
void BLOK();
void WAKE();



enum mensajesMSP{
	/*enviar mensajes*/
	solicitarMemoria = 1,
	crearNuevoSegmento = 2,
	destruirSegmento = 3,
	escribirMemoria = 4,
	errorSegmentationFault = 17,

};

enum mensajesKernelCodOperacion{
	finaliza_quantum = 10,
	finaliza_ejecucion = 11,
	ejecucion_erronea = 12,
	desconexion = 13,
	interrupcion = 14,
	error_al_interpretar_instruccion = 15,
	crear_hilo = 15,
	planificar_hilo = 28,
	join = 22,
	bloquear = 23,
	despertar = 24,

	entrada_estandar = 20,
	salida_estandar = 21,

	soy_CPU = 19,
};

enum instruccionesCPU{
	_LOAD = 15,
	_GETM,
	_MOVR,
	_ADDR,
	_SUBR,
	_MULR,
	_MODR,
	_DIVR,
	_INCR,
	_DECR,
	_COMP,
	_CGEQ,
	_CLEQ,
	_GOTO,
	_JMPZ,
	_INTE,
	_FLCL,
	_SHIF,
	_NOPP,
	_PUSH,
	_TAKE,
	_XXXX,
};

enum instruccionesProtegidas{
	_MALC,
	_FREE,
	_INNN,
	_INNC,
	_OUTN,
	_OUTC,
	_CREA,
	_JOIN,
	_BLOK,
	_WAKE,
};





#endif /* CPU_H_ */
