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
	int A;
	int B;
	int C;
	int D;
	int E;
}reg_programacion;

typedef struct {
	int PID;
	int TID;
	int KM;
	int M;
	int tamanioSegmentoCodigo;
	int P;
	int X;
	int S;
	reg_programacion registrosProgramacion;
}TCB_struct;

/*Tipos parametros instruccion*/

typedef struct{
	char reg1;
	char reg2;
}tparam_load;

typedef struct{
	char reg1;
	char reg2;
}tparam_getm;

typedef struct{
	char reg1;
	char reg2;
}tparam_addr;

typedef struct{
	char reg1;
	char reg2;
}tparam_subr;

typedef struct{
	char reg1;
	char reg2;
}tparam_mulr;

typedef struct{
	char reg1;
	char reg2;
}tparam_modr;

typedef struct{
	char reg1;
	char reg2;
}tparam_divr;

typedef struct{
	char reg1;
}tparam_incr;

typedef struct{
	char reg1;
}tparam_decr;

typedef struct{
	char reg1;
	char reg2;
}tparam_comp;

typedef struct{
	char reg1;
	char reg2;
}tparam_cgeq;

typedef struct{
	char reg1;
	char reg2;
}tparam_cleq;

typedef struct{
	char reg1;
	char reg2;
}tparam_divr;

typedef struct{
	char reg1;
}tparam_goto;

typedef struct{
	int direccion;
}tparam_jmpz;

typedef struct{
	int direccion;
}tparam_jpnz;

typedef struct{
	int direccion;
}tparam_inte;

typedef struct{
	int numero;
	char registro;
}tparam_shif;

typedef struct{
	int num1;
	int num2;
}tparam_push;

typedef struct{
	int numero;
	char registro;
}tparam_take;


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

int superMensaje[13];
t_TCB* TCBactual;
int quantum = 0;


/*Estados del CPU*/
int estaEjecutando = 0;
int matarCPU = 0;


/*Definicion de funciones*/
void cargarArchivoConfiguracion(int cantArgs, char** args);
void conectarConMSP();
void conectarConKernel();
void abortarEjecucion();
void cargarRegistrosCPU();
void actualizarRegistrosTCB();
int cargarDatosTCB();
int actualizarTCB();
//void ejecutarInstruccion(char instruccion[4], int parametros[3]);
void devolverTCBactual(int codigoOperacion);
void limpiarRegistros();
void recibirTCByQuantum(t_datosAEnviar *  datosKernel);
t_TCB* desempaquetarTCB(char* buffer);
void inicializarTCB(t_TCB* tcb);
int interpretarInstruccion(char * proximaInstruccionAEjecutar);
void saltarAInstruccion(int direccion);
int* devolverRegistro(char registro);
int calcularCantidadDeParametrosParaLaInstruccion(char instruccion[4]);
int interpretarYEjecutarInstruccion(char* instruccion);

/*Funciones MSP*/
char* MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion);
int* MSP_CrearNuevoSegmento(int PID, int tamanioSegmento);
t_datosAEnviar*  MSP_DestruirSegmento(int PID, int registro);
char* MSP_SolicitarParametros(int punteroInstruccion, int cantidadParametros);


/*Instrucciones*/
void LOAD(tparam_load);
void GETM(char registro1, char registro2);
void MOVR(char registro1, char registro2);
void ADDR(char registro1, char registro2);
void SUBR(char registro1, char registro2);
void MULR(char registro1, char registro2);
void MODR(char registro1, char registro2);
void DIVR(char registro1, char registro2);
void INCR(char registro);
void DECR(char registro);
void COMP(char registro1, char registro2);
void CGEQ(char registro1, char registro2);
void CLEQ(char registro1, char registro2);
void GOTO(char registro);
void JMPZ(int direccion);
void JPNZ(int direccion);
void INTE(int direccion);
void SHIF(int numero, char registro);
void NOPP();
void PUSH(int numeroA, int numeroB);
void TAKE(int numero, char registro);
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

};

enum mensajesKernelCodOperacion{
	finaliza_quantum = 10,
	finaliza_ejecucion = 11,
	ejecucion_erronea = 12,
	desconexion = 13,
	interrupcion = 14,
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
