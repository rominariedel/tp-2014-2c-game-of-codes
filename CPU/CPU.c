/*
 * CPU.c
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <commons/log.h>
#include <pthread.h>
#include <string.h>
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



/* Estructuras */

typedef struct {
	int PID;
	int TID;
	int KM;
	int baseSegmentoCodigo;
	int tamanioSegmentoCodigo;
	int* punteroInstruccion;
	int baseStack;
	int cursorStack;
	int registrosProgramacion[5];
}t_TCB;

/* FLAGS */
bool ZERO_DIV;

/*Registros CPU*/
char* A;
char* B;
char* C;
char* D;
char* E;
char* F;

t_TCB* TCBactual;


/* Variables Globales */
int kernelSocket;
int socketMSP;
int superMensaje[11];

char* PUERTOMSP;
char* IPMSP;
char* PUERTOKERNEL;
char* IPKERNEL;
int RETARDO;

int punteroInstruccion;
t_TCB* tcbActivo;
int quantum;


/*Datos TCB actual*/
int PIDactual;
int TIDactual;
int KMactual;
int baseSegmentoCodigoActual;
int tamanioSegmentoCodigoActual;
int* punteroInstruccionActual;
int baseStackActual;
int cursorStackActual;

/*Estados del CPU*/
int estaEjecutando = 0;


/*Definicion de funciones*/
void cargarArchivoConfiguracion(int cantArgs, char** args);
void conectarConMSP();
void conectarConKernel();
int abortarEjecucion();
void cargarRegistrosCPU();
void cargarRegistrosTCB();
int cargarDatosTCB();
int actualizarTCB();
//int MSP_SolicitarProximaInstruccionAEJecutar(int* punteroAInstruccion);
char* LOAD(char* registro, int numero);
char* GETM(char* registroA, char* registroB);
char* MOVR(char* registroA, char* registroB);
char* ADDR(char* registroA, char* registroB);
char* SUBR(char* registroA, char* registroB);
char* MULR(char* registroA, char* registroB);
char* MODR(char* registroA, char* registroB);
char* DIVR(char* registroA, char* registroB);
char* INCR(char* registro);
char* DECR(char* registro);
char* COMP(char* registroA, char* registroB);
char* CGEQ(char* registroA, char* registroB);
char* CLEQ(char* registroA, char* registroB);
char* GOTO(char* registro);
void JMPZ(int nro, char* registro);
void INTE(int* direccion);
void FLCL();
void SHIF(int numero, char* registro);
void NOPP();
void PUSH(int numeroA, int numeroB);
void TAKE(int numero, char* registro);
void XXXX();



int main(int cantArgs, char** args){
	//char* instruccionAEjecutar = malloc(1);

	/*cargarArchivoConfiguracion(cantArgs,args);
	conectarConMSP();
	conectarConKernel();
	//proximaInstruccionaejecutar
	//ejecutar instruccion
*/


	while(1)
	{
		int quantumActual = 1;
//		int recibido = recv(kernelSocket,tcbActivo,sizeof(t_TCB),0);
//		int recibidoquantum = recv(kernelSocket,quantum,sizeof(int),0);

		//1.Cargar los registros de la CPU con los datos del TCB a ejecutar.

		cargarDatosTCB();

		estaEjecutando = 1;

		while(quantumActual<=quantum)
		{

			//MSP_SolicitarProximaInstruccionAEJecutar(punteroInstruccion);


			/*

			2. Usando el registro Puntero de Instrucción, le solicitará a la MSP la próxima instrucción a
			ejecutar.
			3. Interpretará la instrucción en BESO y realizará la operación que corresponda. Para conocer
			todas las instrucciones existentes y su propósito, ver el Anexo I: Especificación de ESO.
			4. Actualizará los registros de propósito general del TCB correspondientes según la especificación
			de la instrucción.
			5. Incrementa el Puntero de Instrucción.
			6. En caso que sea el último ciclo de ejecución del Quantum, devolverá el TCB actualizado al
			proceso Kernel y esperará a recibir el TCB del próximo hilo a ejecutar. Si el TCB en cuestión
			tuviera el flag KM (Kernel Mode) activado, se debe ignorar el valor del Quantum.
			7. Volver al punto 1).*/
			/*log_trace(logger, "Ejecuto rafaga del programa %d", pcb->pid);
			log_trace(logger, "Calculo instruccion a buscar");
			instruccionABuscar = UMV_solicitarBytes(pcb->pid,pcb->indiceCodigo,pcb->programCounter*8,sizeof(t_intructions));
			instruccionAEjecutar = realloc(instruccionAEjecutar, instruccionABuscar->offset);
			log_trace(logger, "Busco instruccion a ejecutar");
			instruccionAEjecutar = UMV_solicitarBytes(pcb->pid,pcb->segmentoCodigo,instruccionABuscar->start,instruccionABuscar->offset);
			*/
		}
	}
}

void cargarArchivoConfiguracion(int cantArgs, char** args){
	t_config* configuracion = config_create(args[1]);
	PUERTOMSP = config_get_string_value(configuracion, "PUERTO_MSP");
	IPMSP = config_get_string_value(configuracion, "IP_MSP");
	PUERTOKERNEL = config_get_string_value(configuracion, "PUERTO_KERNEL");
	IPKERNEL = config_get_string_value(configuracion, "IP_KERNEL");
	RETARDO = config_get_string_value(configuracion, "RETARDO");
}

void conectarConMSP()
{
	struct addrinfo hintsMSP;
	struct addrinfo *mspInfo;
	//char id = 1;
	//char conf = 0;

	memset(&hintsMSP, 0, sizeof(hintsMSP));
	hintsMSP.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hintsMSP.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IPMSP, PUERTOMSP, &hintsMSP, &mspInfo);	// Carga en serverInfo los datos de la conexion
	socketMSP = socket(mspInfo->ai_family, mspInfo->ai_socktype, mspInfo->ai_protocol);

	connect(socketMSP, mspInfo->ai_addr, mspInfo->ai_addrlen);
	freeaddrinfo(mspInfo);	// No lo necesitamos mas

	//send(socketMSP, &id, sizeof(char), 0);
	//recv(socketMSP, &conf, sizeof(char), 0);

}

void conectarConKernel()
{
	struct addrinfo hintsKernel;
	struct addrinfo *kernelInfo;

	memset(&hintsKernel, 0, sizeof(hintsKernel));
	hintsKernel.ai_family = AF_UNSPEC;		// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hintsKernel.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

	getaddrinfo(IPKERNEL, PUERTOKERNEL, &hintsKernel, &kernelInfo);	// Carga en serverInfo los datos de la conexion
	kernelSocket = socket(kernelInfo->ai_family, kernelInfo->ai_socktype, kernelInfo->ai_protocol);

	if(kernelSocket == -1){
		abortarEjecucion();
	}

	/*while (a == -1){
		a = connect(kernelSocket, kernelInfo->ai_addr, kernelInfo->ai_addrlen);
	}
	log_info(logger, "Se establecio conexion con el kernel por el socket %d", kernelSocket);
	recv(kernelSocket, &quantum, sizeof(int), 0);
	log_trace(logger, "Recibo valor de quantum: %d", quantum);
	recv(kernelSocket, &retardo, sizeof(int), 0);
	log_trace(logger, "Recibo valor de retardo: %d", retardo);
	freeaddrinfo(kernelInfo);	// No lo necesitamos mas*/

}

int abortarEjecucion(){
	//avisar al kernel que hay que matar al CPU
	return 0;
}

void cargarRegistrosCPU(){
	*A = TCBactual -> registrosProgramacion[0];
	*B = TCBactual -> registrosProgramacion[1];
	*C = TCBactual -> registrosProgramacion[2];
	*D = TCBactual -> registrosProgramacion[3];
	*E = TCBactual -> registrosProgramacion[4];
}

void cargarRegistrosTCB(){
	TCBactual -> registrosProgramacion[0] = *A;
	TCBactual -> registrosProgramacion[1] = *B;
	TCBactual -> registrosProgramacion[2] = *C;
	TCBactual -> registrosProgramacion[3] = *D;
	TCBactual -> registrosProgramacion[4] = *E;
}

int cargarDatosTCB(){
	TIDactual = TCBactual -> TID;
	KMactual = TCBactual -> KM;
	baseSegmentoCodigoActual = TCBactual -> baseSegmentoCodigo;
	tamanioSegmentoCodigoActual = TCBactual -> tamanioSegmentoCodigo;
	punteroInstruccionActual = TCBactual -> punteroInstruccion;
	baseStackActual = TCBactual -> baseStack;
	cursorStackActual = TCBactual -> cursorStack;
	cargarRegistrosCPU();
	return 0;
}

int actualizarTCB(){
	TCBactual -> TID = TIDactual;
	TCBactual -> KM = KMactual;
	TCBactual -> baseSegmentoCodigo = baseSegmentoCodigoActual;
	TCBactual -> tamanioSegmentoCodigo = tamanioSegmentoCodigoActual;
	TCBactual -> punteroInstruccion = punteroInstruccionActual;
	TCBactual -> baseStack = baseStackActual;
	TCBactual -> cursorStack = cursorStackActual;
	cargarRegistrosCPU();
	return 0;
}

int MSP_SolicitarProximaInstruccionAEJecutar(int* punteroInstruccion){
	char confirmacion;
	int proximaInstruccionAEjecutar;
	int mensaje[2];
	mensaje[0]=1; //codigo de operacion 1 solicitar prox instruccion
	mensaje[1]= *punteroInstruccion;
	send(socketMSP,mensaje, sizeof(int[2]), 0);
	/*int status = */recv(socketMSP, &proximaInstruccionAEjecutar, sizeof(char), 0);
	if(confirmacion != 0){

	//TODO: terminar
	}

	return 0;
}


/*Codigo ESO*/

/*
El lenguaje que deberán interpretar consta de un bytecode de 4 bytes seguido de 0, 1, 2 o 3 operadores
de tipo registro (1 caracter, o sea 1 byte), número (1 entero, o sea 4 bytes) o direccion (1 entero, o sea 4
bytes). Los códigos de operación coinciden con su nombre en ASCII, por lo que el código para la
instrucción “MOVR”, es 1297045074 en un número entero, que en hexadecimal es 0x4d4f5652, que en
binario es: 01001101 (M) 01001111 (O)01010110 (V) 01010010 (R).
*/

char* LOAD(char* registro, int numero){ //Carga en el registro, el número dado.
	*registro = numero;
	return registro;
}

char* GETM(char* registroA, char* registroB){ //Obtiene el valor de memoria apuntado por el segundo registro. El valor obtenido lo asigna en el primer registro.
	*registroA = *registroB;
	return registroA;
}

//TODO: SETM no entiendo que tiene que hacer

char* MOVR(char* registroA, char* registroB){ //Copia el valor del segundo registro hacia el primero
	//no es lo mismo que getm?
	return registroA;
}

char* ADDR(char* registroA, char* registroB){ //Suma el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	char* aux = registroA;
	*registroA = *aux + *registroB;
	return registroA;
}

char* SUBR(char* registroA, char* registroB){ //Resta el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	char* aux = registroA;
	*registroA = *aux - *registroB;
	return registroA;
}

char* MULR(char* registroA, char* registroB){ //Multiplica el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	char* aux = registroA;
	*registroA = (*aux) * (*registroB);
	return registroA;
}

char* MODR(char* registroA, char* registroB){ //Obtiene el resto de la división del primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	char* aux = registroA;
	*registroA = (*aux) % (*registroB);
	return registroA;
}

char* DIVR(char* registroA, char* registroB){
	//Divide el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A; a menos que el segundo operando sea 0,
	//en cuyo caso se asigna el flag de ZERO_DIV y no se hace la operación.
	if(*registroB == 0){
		*registroB = ZERO_DIV;
	} else {
		char* aux = registroA;
		*registroA = (*aux) % (*registroB);
		}
	return registroA;
}

char* INCR(char* registro){ //incrementar una unidad al registro
	*registro =+ 1;
	return registro;
}

char* DECR(char* registro){ //decrementar una unidad al registro
	*registro =- 1;
	return registro;
}

char* COMP(char* registroA, char* registroB){
	//Compara si el primer registro es igual al segundo. De ser verdadero, se almacena el valor 1. De lo
	//contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if(*registroA == *registroB){
		*registroA = 1;
	}
	*registroA = 0;
	return registroA;
}

char* CGEQ(char* registroA, char* registroB){
	//Compara si el primer registro es mayor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if(*registroA >= *registroB){
		*registroA = 1;
	}
	*registroA = 0;
	return registroA;
}

char* CLEQ(char* registroA, char* registroB){
	//Compara si el primer registro es menor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if(*registroA <= *registroB){
		*registroA = 1;
	}
	registroA = 0;
	return registroA;
}

char* GOTO(char* registro){
	//Altera el flujo de ejecución para ejecutar la instrucción apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.
	//TODO: REVISAR EL TEMA DE LOS REGISTROS DE CPU.
	*punteroInstruccionActual = *registro;
	return registro;
}

void JMPZ(int nro, char* registro){
	//Altera el flujo de ejecución, solo si el valor del registro A es cero, para ejecutar la instrucción
	//apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.
	//TODO: no tendria que tener por parametro un registro???? YO SE LO AGREGO
	if(registro == 0){
		GOTO(registro);
	}

}

//TODO: JMPZ y JPNZ son lo mismo

void INTE(int* direccion){

	//INTERRUMPIR EJECUCION PROGRAMA
	//BLOQUEAR HILO
	//EJECUTAR RUTINA KERNEL


	/*Interrumpe la ejecución del programa para ejecutar la rutina del kernel que se encuentra en la
	posición apuntada por la direccion. El ensamblador admite ingresar una cadena indicando el
	nombre, que luego transformará en el número correspondiente. Los posibles valores son
	“MALC”, “FREE”, “INNN”, “INNC”, “OUTN”, “OUTC”, “BLOK”, “WAKE”, “CREA” y “JOIN”. Invoca al
	servicio correspondiente en el proceso Kernel. Notar que el hilo en cuestión debe bloquearse
	tras una interrupción.*/


}

void FLCL(){ //limpia registro de flags
	//TODO: ver tema flags del CPU
}

void SHIF(int numero, char* registro){
	//Desplaza los bits del registro, tantas veces como se indique en el Número. De ser
	//desplazamiento positivo, se considera hacia la derecha. De lo contrario hacia la izquierda.
/*
	double auxNum = pow(2.0,(double)numero);

	if(numero < 0){
		char* aux = registro;
		*registro = *aux * auxNum;

	}
	if(numero > 0){
		char* aux = registro;
		*registro = *aux / auxNum;
	}
*/
	}

void NOPP(){
	//NO HAGO NADA
}

void PUSH(int numeroA, int numeroB){
	//Apila los primeros bytes, indicado por el número, del registro hacia el stack. Modifica el valor del registro cursor de stack de forma acorde.
	//TODO: no entendi que hace
}

void TAKE(int numero, char* registro){
	//TODO:  no entiendo que seria apilar.
}

void XXXX(){

	//TODO: hacer funcion. finalizarEjecucion();
}
