/*
 * CPU.h
 *
 *  Created on: 24/10/2014
 *      Author: utnso
 */

#ifndef CPU_H_
#define CPU_H_


#include <string.h>
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
#include "funciones/bibliotecas.h"
#include "t_parametros.h"
#include "funciones/funcionesMSP.h"
#include "funciones/funcionesKernel.h"
#include "funciones/instruccionesESO.h"

/* Estructuras */


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


enum mensajesMSP{
	/*enviar mensajes*/
	solicitarMemoria = 1,
	solicitarMemoriaP = 80,
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
