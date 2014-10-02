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
#include <error.h>



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

/*Datos actuales*/
int PIDactual;
int TIDactual;
int KMactual;   //KM == 1 el programa puede ejecutar las instrucciones protegidas. esta en modo kernel.
int baseSegmentoCodigoActual;
int tamanioSegmentoCodigoActual;
int* punteroInstruccionActual;
int baseStackActual;
int cursorStackActual;

/*Registros CPU*/
char* A;
char* B;
char* C;
char* D;
char* E;
char* F;

/* Variables Globales */
int kernelSocket;
int socketMSP;

char* PUERTOMSP;
char* IPMSP;
char* PUERTOKERNEL;
char* IPKERNEL;
int RETARDO;


t_TCB* TCBactual;
int quantum;


/*Estados del CPU*/
int estaEjecutando = 0;
int matarCPU = 0;


/*Definicion de funciones*/
void cargarArchivoConfiguracion(int cantArgs, char** args);
void conectarConMSP();
void conectarConKernel();
int abortarEjecucion();
void cargarRegistrosCPU();
void actualizarRegistrosTCB();
int cargarDatosTCB();
int actualizarTCB();
void ejecutarInstruccion(int);
void devolverTCBactual();
void limpiarRegistros();

/*Funciones MSP*/
int MSP_SolicitarProximaInstruccionAEJecutar(int* punteroAInstruccion);
void* MSP_CrearNuevoSegmento(int PID, int tamanioSegmento);
void MSP_DestruirSegmento(int PID, void* segmento);


/*Instrucciones*/
char* LOAD(char* registro, int numero);
char* GETM(char* registro1, char* registro2);
char* MOVR(char* registro1, char* registro2);
char* ADDR(char* registro1, char* registro2);
char* SUBR(char* registro1, char* registro2);
char* MULR(char* registro1, char* registro2);
char* MODR(char* registro1, char* registro2);
char* DIVR(char* registro1, char* registro2);
char* INCR(char* registro);
char* DECR(char* registro);
char* COMP(char* registro1, char* registro2);
char* CGEQ(char* registro1, char* registro2);
char* CLEQ(char* registro1, char* registro2);
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

	//TODO: para poder ubicar donde empieza el main
	/*cargarArchivoConfiguracion(cantArgs,args);
	conectarConMSP();
	conectarConKernel();
	//proximaInstruccionaejecutar
	//ejecutar instruccion
*/

	while(1)
	{
		int quantumActual = 0;

		//estoy a la espera de que el kernel me mande el TCB y el quantum
		int recibidoTCBactual = recv(kernelSocket,TCBactual,sizeof(t_TCB),0);
		int recibidoquantum = recv(kernelSocket,quantum,sizeof(int),0);
		if (recibidoTCBactual == -1){error("error al recibir TCB");
		if (recibidoquantum == -1){error("error al recibir quantum");

		//1.Cargar todos los datos del TCB actual y sus registros de programacion.
		cargarDatosTCB();

		estaEjecutando = 1; //estado CPU

		while(quantumActual<quantum)
		{

			//2. Usando el registro Puntero de Instrucción, le solicitará a la MSP la próxima instrucción a ejecutar.

			int proximaInstruccionAEjecutar = MSP_SolicitarProximaInstruccionAEJecutar(punteroInstruccionActual);

			// 	3. Interpretará la instrucción en BESO y realizará la operación que corresponda. Para conocer todas las instrucciones existentes y su propósito, ver el Anexo I: Especificación de ESO.

			ejecutarInstruccion(proximaInstruccionAEjecutar);

			// 4. Actualizará los registros de propósito general del TCB correspondientes según la especificación de la instrucción.

			actualizarRegistrosCPU();

			// 5. Incrementa el Puntero de Instrucción.

			punteroInstruccionActual++;

			// Incrementar quantum

			quantumActual++;

			//TODO: considerar caso de que sea KM!!! no se tiene en cuenta el q
		}

		if(quantumActual == quantum){
			/* 6. En caso que sea el último ciclo de ejecución del Quantum, devolverá el TCB actualizado al
			proceso Kernel y esperará a recibir el TCB del próximo hilo a ejecutar. Si el TCB en cuestión
			tuviera el flag KM (Kernel Mode) activado, se debe ignorar el valor del Quantum.
			*/
			devolverTCBactual();
			limpiarRegistros();

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

	int estadoConexion = connect(socketMSP, mspInfo->ai_addr, mspInfo->ai_addrlen);

	if(estadoConexion == -1){
		error("no se pudo realizar la conexion con la MSP");
		abortarEjecucion();
	}
	//TODO: hacer log

	printf("Se conecto a la MSP");

	freeaddrinfo(mspInfo);	// No lo necesitamos mas
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

	int estadoConexion = connect(kernelSocket, kernelInfo->ai_addr, kernelInfo->ai_addrlen);

	if(estadoConexion == -1){
		error("no se pudo realizar la conexion con el Kernel");
		abortarEjecucion();
	}
	//TODO: hacer log

	printf("Se conecto al Kernel");

	freeaddrinfo(kernelInfo);	// No lo necesitamos mas

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

void actualizarRegistrosTCB(){
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
	actualizarRegistrosTCB();
	return 0;
}


void devolverTCBactual(){
	actualizarTCB();
	int status = send(kernelSocket,TCBactual,sizeof(t_TCB),0);
	if(status){printf("no se pudo devolver el TCBactual");}
}

void limpiarRegistros(){
	TIDactual = 0;
	KMactual = 0;
	baseSegmentoCodigoActual = NULL;
	tamanioSegmentoCodigoActual = 0;
	punteroInstruccionActual = NULL;
	baseStackActual = NULL;
	cursorStackActual = NULL;
	*A = 0;
	*B = 0;
	*C = 0;
	*D = 0;
	*E = 0;
	*F = 0;
}


/*Funciones MSP*/


int MSP_SolicitarProximaInstruccionAEJecutar(int* punteroInstruccion){
	int proximaInstruccionAEjecutar;
	int mensaje[2];
	mensaje[0]=1; //codigo de operacion 1 solicitar prox instruccion
	mensaje[1]= *punteroInstruccion;
	send(socketMSP,mensaje, sizeof(int[2]), 0);
	int status = recv(socketMSP, &proximaInstruccionAEjecutar, sizeof(char), 0);
	//me manda la instruccion a ejecutar en forma de numero, el valor de la instruccion en ASCII
	return proximaInstruccionAEjecutar;
}



void* MSP_CrearNuevoSegmento(int PID, int tamanioSegmento){
	void* nuevoSegmento;
	int mensaje[3];
	mensaje[0]=2; //codigo de operacion 2 crear segmento
	mensaje[1]= PID;
	mensaje[2]= tamanioSegmento;
	send(socketMSP,mensaje, sizeof(int[3]), 0);
	int status = recv(socketMSP, &nuevoSegmento, sizeof(char), 0);
	return nuevoSegmento;
}

void MSP_DestruirSegmento(int PID, void* segmento){
	int confirmacion;
	int mensaje[3];
	mensaje[0]=2; //codigo de operacion 3 destruir segmento
	mensaje[1]= PID;
	mensaje[2]= segmento;
	send(socketMSP,mensaje, sizeof(int[3]), 0);
	recv(socketMSP, confirmacion, sizeof(char), 0);
	if(confirmacion == -1){error("no se pudo destruir el segmento");}
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

char* SETM(int numero, char* registro1, char* registro2){
	//Pone tantos bytes desde el segundo registro, hacia la memoria apuntada por el primer registro
	//explicacion Gaston: pone en los n bytes del registro bx en la dirección de memoria apuntanda por el registro ax
	//(ax = numero que es una posición de memoria)

	//TODO: sigo sin entender.

	return registro2;
}


char* GETM(char* registro1, char* registro2){ //Obtiene el valor de memoria apuntado por el segundo registro. El valor obtenido lo asigna en el primer registro.
	registro1 = &registro2;
	return registro1;
}

/*
 * la diferencia entre GETM y MOVR seria que en GETM , le estoy asignando al primer registro la direccion de memoria que tiene el segundo registro?
 */

char* MOVR(char* registro1, char* registro2){ //Copia el valor del segundo registro hacia el primero
	*registro1 = *registro2;
	return registro1;
}

char* ADDR(char* registro1, char* registro2){ //Suma el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	*A = *registro1 + *registro2;
	return A;
}

char* SUBR(char* registro1, char* registro2){ //Resta el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	*A = *registro1 - *registro2;
	return A;
}

char* MULR(char* registro1, char* registro2){ //Multiplica el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	*A = (*registro1) * (*registro2);
	return A;
}

char* MODR(char* registro1, char* registro2){ //Obtiene el resto de la división del primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	*A = (*registro1) % (*registro2);
	return A;
}

char* DIVR(char* registro1, char* registro2){
	//Divide el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A; a menos que el segundo operando sea 0,
	//en cuyo caso tira error de division por cero

	if(*registro2 == 0){
		error("division por cero");
	}
	else {
		*A = (*registro1) % (*registro2);
	}
	return A;
}

char* INCR(char* registro){ //incrementar una unidad al registro
	*registro =+ 1;
	return registro;
}

char* DECR(char* registro){ //decrementar una unidad al registro
	*registro =- 1;
	return registro;
}

char* COMP(char* registro1, char* registro2){
	//Compara si el primer registro es igual al segundo. De ser verdadero, se almacena el valor 1. De lo
	//contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if(*registro1 == *registro2){
		*A = 1;
	}
	else{
		*A = 0;
	}
	return A;
}

char* CGEQ(char* registro1, char* registro2){
	//Compara si el primer registro es mayor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if(*registro1 >= *registro2){
		*A = 1;
	}
	else{
		*A = 0;
	}
	return A;
}

char* CLEQ(char* registro1, char* registro2){
	//Compara si el primer registro es menor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if(*registro1 <= *registro2){
		*A = 1;
	}
	else{
	    *A = 0;
	}
	return A;
}

char* GOTO(char* registro){
	//Altera el flujo de ejecución para ejecutar la instrucción apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.
	punteroInstruccionActual = &baseSegmentoCodigoActual + *registro;
	return registro;
}


void JMPZ(int direccion){
	//Altera el flujo de ejecución sólo si el valor del registro A es cero, para ejecutar la instrucción apuntada por la Dirección.
	//El valor es el desplazamiento desde el inicio del programa.

	if(A == 0){
		GOTO(direccion);
	}
}

void JPNZ(int direccion){
	// Altera el flujo de ejecución sólo si el valor del registro A no es cero, para ejecutar la instrucción apuntada por la Dirección.
	// El valor es el desplazamiento desde el inicio del programa.

	if(A != 0){
		GOTO(direccion);
	}
}

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

char* SHIF(int numero, char* registro){
	//Desplaza los bits del registro, tantas veces como se indique en el Número. De ser
	//desplazamiento positivo, se considera hacia la derecha. De lo contrario hacia la izquierda.
	if(numero < 0){
		*registro = *registro << numero;
	}
	if(numero > 0){
		*registro = *registro >> numero;
	}
	return registro;
}

void NOPP(){
	//NO HAGO NADA
}

void PUSH(int numeroA, int numeroB){
	//Apila los primeros bytes, indicado por el número, del registro hacia el stack. Modifica el valor del registro cursor de stack de forma acorde.
	//TODO: no entendi que hace
}

void TAKE(int numero, char* registro){
	//Desapila los primeros bytes, indicado por el número, del stack hacia el registro. Modifica el valor del registro de stack de forma acorde.
	//TODO:  no entiendo que seria apilar.
}

void XXXX(){
	// finaliza ejecucion del programa
	//TODO: hacer funcion. finalizarEjecucion();
}

/*Instrucciones Protegidas*/
/*    Solo si KM == 1     */


void MALC(){
	//Reserva una cantidad de memoria especificada por el registro A. La direccion de esta se
	//almacena en el registro A. Crea en la MSP un nuevo segmento del tamaño especificado asociado
	//al programa en ejecución.

	void* cantidadMemoria = *A;
	A = &cantidadMemoria;
	MSP_CrearNuevoSegmento(PIDactual, cantidadMemoria);

}

void FREE(){
	//Libera la memoria apuntada por el registro A. Solo se podrá liberar memoria alocada por la
	//instrucción de MALC. Destruye en la MSP el segmento indicado en el registro A.
	//TODO:
	MSP_DestruirSegmento(PIDactual, *A);
}

void INNN(){
	// Pide por consola del programa que se ingrese un número, con signo entre –2.147.483.648 y
	// 2.147.483.647. El mismo será almacenado en el registro A. Invoca al servicio correspondiente en
	// el proceso Kernel.
	int numero;
	printf("Ingrese un numero entre –2.147.483.648 y 2.147.483.647");
	scanf("%l", numero);
	*A = numero;
}

//TODO: a que refiere con invoca al servicio correspondiente en el proceso Kernel?

void INNC(){
	//Pide por consola del programa que se ingrese una cadena no más larga de lo indicado por el
	//registro B. La misma será almacenada en la posición de memoria apuntada por el registro A.
	//Invoca al servicio correspondiente en el proceso Kernel.
	char cadena[];
	printf("Ingrese una cadena no mas larga que %d", *B);
	scanf("%l", cadena);
	*A = cadena;
}



/*29. OUTN
Imprime por consola del programa el número, con signo almacenado en el registro A. Invoca al
servicio correspondiente en el proceso Kernel.
30. OUTC
Imprime por consola del programa una cadena de tamaño indicado por el registro B que se
encuentra en la direccion apuntada por el registro A. Invoca al servicio correspondiente en el
proceso Kernel.
31. CREA
Crea un hilo, hijo del TCB que ejecutó la llamada al sistema correspondiente. El nuevo hilo
tendrá su Program Counter apuntado al número almacenado en el registro B. El identificador del
nuevo hilo se almacena en el registro A.
Para lograrlo debe generar un nuevo TCB como copia del TCB actual, asignarle un nuevo TID
correlativo al actual, cargar en el Puntero de Instrucción la rutina donde comenzará a ejecutar el
nuevo hilo (registro B), pasarlo de modo Kernel a modo Usuario, duplicar el segmento de stack
desde la base del stack, hasta el cursor del stack. Asignar la base y cursor de forma acorde (tal
que la diferencia entre cursor y base se mantenga igual)13 y luego invocar al servicio
correspondiente en el proceso Kernel con el TCB recién generado.
32. JOIN
Bloquea el programa que ejecutó la llamada al sistema hasta que el hilo con el identificador
almacenado en el registro A haya finalizado. Invoca al servicio correspondiente en el proceso
Kernel.
33. BLOK
 */









