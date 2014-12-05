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
#include <stdbool.h>
#include <string.h>
#include <commons/log.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
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
#include "funciones/bibliotecas.h"
#include "t_parametros.h"
#include "variablesGlobales.h"
#include "funciones/funcionesMSP.h"
#include "funciones/funcionesKernel.h"
#include "funciones/instruccionesESO.h"
#include "logsObligatorios.h"



/* Estructuras */

typedef struct{
	t_TCB* TCB;
	t_cola cola;
}hilo_t;


/*Datos actuales*/
int PIDactual;
int TIDactual;
int KMactual;   //KM == 1 el programa puede ejecutar las instrucciones protegidas. esta en modo kernel.
int baseSegmentoCodigoActual;
int tamanioSegmentoCodigoActual;
int punteroInstruccionActual;
int baseStackActual;
int cursorStackActual;
int PIDkm;

/*Registros CPU*/
int A;
int B;
int C;
int D;
int E;
int quantum;


/* Variables Globales */
int socketKernel;
int socketMSP;

char* PUERTOMSP;
char* IPMSP;
char* PUERTOKERNEL;
char* IPKERNEL;
int RETARDO;

t_log* LOGCPU;

int finalizarEjecucion;
int ejecutoInterrupcion;
int aumentoPuntero;
int noPIDkm;

//int errorOperacionesConMemoria;
//int errorMemoria;



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
void abortar(int codOperacion);

enum operaciones{
	operacionExitosa = 1,
};

enum mensajesMSP{
	/*enviar mensajes*/
	solicitarMemoria = 31,
	crearNuevoSegmento = 32,
	destruirSegmento = 30,
	escribirMemoria = 2,
	error_memoriaLlena = -3,
	error_segmentationFault = -2,
	no_llego_respuesta = 90,
};

enum mensajesKernelCodOperacion{
	finaliza_quantum = 10,
	finaliza_ejecucion = 11,
	ejecucion_erronea = -12,
	desconexion = 13,
	interrupcion = 14,
	error_al_interpretar_instruccion = 1010,
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
	_LOAD,
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
