/*
 * CPU.c
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */

#include "CPU.h"


int main(int cantArgs, char** args){

	printf("\n ------Bienvenido al CPU----- \n");
	cargarArchivoConfiguracion(cantArgs,args);
	printf("\n Cargando archivos configuración \n");
	conectarConMSP();
	printf("\n Conectado con MSP \n");
	conectarConKernel();
	printf("\n Conectado con Kernel \n");

	while(1)
	{
		//estoy a la espera de que el kernel me mande el TCB y el quantum
		printf("Estoy a la espera de que el Kernel me mande el TCB y el quantum correspondiente");
		int recibidoTCBactual = 0;
//		recibidoTCBactual = recv(kernelSocket,superMensaje,sizeof(t_TCB),0);
		int recibidoquantum = 0;
//		recibidoquantum = recv(kernelSocket,&quantum,sizeof(int),0);
		if (recibidoTCBactual == -1)perror("error al recibir TCB PUTO");
		if (recibidoquantum == -1)perror("error al recibir quantum");


		//1.Cargar todos los datos del TCB actual y sus registros de programacion.
		printf("Recibí datos del TCB actual y sus registros de programacion");
		//cargarDatosTCB();

		estaEjecutando = 1; //estado CPU

		int quantumActual = 0;
		printf("\n quantumActual es: %d \n", quantumActual);
		int quantum = -1;
		while(quantumActual<quantum && KMactual==1)
		{

			//2. Usando el registro Puntero de Instrucción, le solicitará a la MSP la próxima instrucción a ejecutar.
			int proximaInstruccionAEjecutar = 0;
			//int proximaInstruccionAEjecutar = MSP_SolicitarProximaInstruccionAEJecutar(PIDactual, &punteroInstruccionActual);

			// 	3. Interpretará la instrucción en BESO y realizará la operación que corresponda. Para conocer todas las instrucciones existentes y su propósito, ver el Anexo I: Especificación de ESO.
			int parametros[2];
			parametros[0] = 0;
			parametros[1] = 0;

			ejecutarInstruccion(proximaInstruccionAEjecutar,parametros);

			// 4. Actualizará los registros de propósito general del TCB correspondientes según la especificación de la instrucción.

			actualizarRegistrosTCB();

			// 5. Incrementa el Puntero de Instrucción.

			punteroInstruccionActual++;

			// Incrementar quantum

			quantumActual++;


		}

		if(quantumActual == quantum && KMactual==0){
			// 6. En caso que sea el último ciclo de ejecución del Quantum, devolverá el TCB actualizado al
			//proceso Kernel y esperará a recibir el TCB del próximo hilo a ejecutar. Si el TCB en cuestión
			//tuviera el flag KM (Kernel Mode) activado, se debe ignorar el valor del Quantum.

			devolverTCBactual();
			limpiarRegistros();

		}
		return 0;
	}

}

void cargarArchivoConfiguracion(int cantArgs, char** args){
	t_config* configuracion = config_create(args[1]);
	PUERTOMSP = config_get_string_value(configuracion, "PUERTO_MSP");
	IPMSP = config_get_string_value(configuracion, "IP_MSP");
	PUERTOKERNEL = config_get_string_value(configuracion, "PUERTO_KERNEL");
	IPKERNEL = config_get_string_value(configuracion, "IP_KERNEL");
	RETARDO = config_get_int_value(configuracion, "RETARDO");
}

void conectarConMSP()
{
	socketMSP = crear_cliente(IPMSP, PUERTOMSP);

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
		// error("no se pudo realizar la conexion con el Kernel");
		abortarEjecucion();
	}
	//TODO: hacer log

	printf("Se conecto al Kernel");

	freeaddrinfo(kernelInfo);	// No lo necesitamos mas

}


void abortarEjecucion(){
	int mensaje[2];
	mensaje[0]=  1; //abortar ejecucion, matar cpu
	//mensaje[1]= TCBactual; TCBactual no es un int. REVISAR
	int status = send(kernelSocket,mensaje, sizeof(int[2]), 0);
	if (status == -1){printf("No se pudo conectar con el Kernel");}

}

void cargarRegistrosCPU(){
	A = TCBactual -> registrosProgramacion[0];
	B = TCBactual -> registrosProgramacion[1];
	C = TCBactual -> registrosProgramacion[2];
	D = TCBactual -> registrosProgramacion[3];
	E = TCBactual -> registrosProgramacion[4];
}

void actualizarRegistrosTCB(){
	TCBactual -> registrosProgramacion[0] = A;
	TCBactual -> registrosProgramacion[1] = B;
	TCBactual -> registrosProgramacion[2] = C;
	TCBactual -> registrosProgramacion[3] = D;
	TCBactual -> registrosProgramacion[4] = E;
}

void recibirSuperMensaje(int superMensaje[12]){
	TCBactual -> TID = superMensaje[0];
	TCBactual -> KM = superMensaje[1];
	TCBactual -> baseSegmentoCodigo = superMensaje[2];
	TCBactual -> tamanioSegmentoCodigo = superMensaje[3];
	TCBactual -> punteroInstruccion = superMensaje[4];
	TCBactual -> baseStack = superMensaje[5];
	TCBactual -> cursorStack = superMensaje[6];
	TCBactual -> registrosProgramacion[0] = superMensaje[7];
	TCBactual -> registrosProgramacion[1] = superMensaje[8];
	TCBactual -> registrosProgramacion[2] = superMensaje[9];
	TCBactual -> registrosProgramacion[3] = superMensaje[10];
	TCBactual -> registrosProgramacion[4] = superMensaje[11];
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

void cargarTCBenSuperMensaje(t_TCB* TCBactual){
	superMensaje[0] = TCBactual -> TID;
	superMensaje[1] = TCBactual -> KM;
	superMensaje[2] = TCBactual -> baseSegmentoCodigo;
	superMensaje[3] = TCBactual -> tamanioSegmentoCodigo;
	superMensaje[4] = TCBactual -> punteroInstruccion;
	superMensaje[5] = TCBactual -> baseStack;
	superMensaje[6] = TCBactual -> cursorStack;
	superMensaje[7] = TCBactual -> registrosProgramacion[0];
	superMensaje[8] = TCBactual -> registrosProgramacion[1];
	superMensaje[9] = TCBactual -> registrosProgramacion[2];
	superMensaje[10] = TCBactual -> registrosProgramacion[3];
	superMensaje[11] = TCBactual -> registrosProgramacion[4];
}

void devolverTCBactual(){
	actualizarTCB();
	cargarTCBenSuperMensaje(TCBactual);
	int status = send(kernelSocket,superMensaje,sizeof(t_TCB),0);
	if(status){printf("no se pudo devolver el TCBactual");}
}

void limpiarRegistros(){
	TIDactual = 0;
	KMactual = 0;
	baseSegmentoCodigoActual = 0;
	tamanioSegmentoCodigoActual = 0;
	punteroInstruccionActual = 0;
	baseStackActual = 0;
	cursorStackActual = 0;
	A = 0;
	B = 0;
	C = 0;
	D = 0;
	E = 0;
	F = 0;
}

void ejecutarInstruccion(int proximaInstruccionAEjecutar, int parametros[2]){
	/*switch(proximaInstruccionAEjecutar)
	{
	case ((int)"LOAD"):
		LOAD(parametros[0],parametros[1]);
		break;
	case "GETM":
		GETM(parametros[0],parametros[1]);
		break;
	case "MOVR":
		MOVR(parametros[0],parametros[1]);
		break;
	case "ADDR":
		ADDR(parametros[0],parametros[1]);
		break;
	case "SUBR":
		SUBR(parametros[0],parametros[1]);
		break;
	case "MULR":
		MULR(parametros[0],parametros[1]);
		break;
	case "MODR":
		MODR(parametros[0],parametros[1]);
		break;
	case "DIVR":
		DIVR(parametros[0],parametros[1]);
		break;
	case "INCR":
		INCR(parametros[0]);
		break;
	case "DECR":
		DECR(parametros[0]);
		break;
	case "COMP":
		COMP(parametros[0],parametros[1]);
		break;
	case "CGEQ":
		CGEQ(parametros[0],parametros[1]);
		break;
	case "CLEQ":
		CLEQ(parametros[0],parametros[1]);
		break;
	case "GOTO":
		GOTO(parametros[0]);
		break;
	case "JMPZ":
		JMPZ(parametros[0],parametros[1]);
		break;
	case "JPNZ":
		JPNZ(parametros[0]);
		break;
	case "INTE":
		INTE(parametros[0]);
		break;
	case "FLCL":
		FLCL(parametros[0],parametros[1]);
		break;
	case "SHIF":
		SHIF(parametros[0],parametros[1]);
		break;
	case "NOPP":
		NOPP(parametros[0],parametros[1]);
		break;
	case "PUSH":
		PUSH(parametros[0],parametros[1]);
		break;
	case "TAKE":
		TAKE(parametros[0],parametros[1]);
		break;
	case "XXXX":
		XXXX(parametros[0],parametros[1]);
		break;

	if (KMactual == 1){
		case "MALC":
			MALC( );
			break;
		case "FREE":
			FREE( );
			break;
		case "INNN":
			INNN( );
			break;
		case "OUTN":
			OUTN( );
			break;
		case "CREA":
			CREA( );
			break;
		case "JOIN":
			JOIN( );
			break;
		case "BLOK":
			BLOK( );
			break;
		case "WAKE":
			WAKE( );
			break; */

}

/*Funciones MSP*/
/*
void MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion){

	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 2* sizeof(int));

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));
}

int* MSP_CrearNuevoSegmento(int PID, int tamanioSegmento){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &tamanioSegmento, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(crearNuevoSegmento, (void*) datos, 2* sizeof(int));

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));

	return  dir_base;
}

t_datosAEnviar * MSP_DestruirSegmento(int PID, int registro){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &registro, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(destruirSegmento, (void*) datos, 2* sizeof(int));

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));

	return respuesta;

}
*/

/*Codigo ESO*/

/*
El lenguaje que deberán interpretar consta de un bytecode de 4 bytes seguido de 0, 1, 2 o 3 operadores
de tipo registro (1 caracter, o sea 1 byte), número (1 entero, o sea 4 bytes) o direccion (1 entero, o sea 4
bytes). Los códigos de operación coinciden con su nombre en ASCII, por lo que el código para la
instrucción “MOVR”, es 1297045074 en un número entero, que en hexadecimal es 0x4d4f5652, que en
binario es: 01001101 (M) 01001111 (O)01010110 (V) 01010010 (R).
*/

void LOAD(char registro, int numero){ //Carga en el registro, el número dado.
	registro = numero;
}

void SETM(int numero, char registro1, char registro2){
	//Pone tantos bytes desde el segundo registro, hacia la memoria apuntada por el primer registro
	//explicacion Gaston: pone en los n bytes del registro bx en la dirección de memoria apuntanda por el registro ax
	//(ax = numero que es una posición de memoria)

	//TODO: sigo sin entender.
}


void GETM(char registro1, char registro2){ //Obtiene el valor de memoria apuntado por el segundo registro. El valor obtenido lo asigna en el primer registro.
	registro1 = registro2;
}

/*
 * la diferencia entre GETM y MOVR seria que en GETM , le estoy asignando al primer registro la direccion de memoria que tiene el segundo registro?
 */

void MOVR(char registro1, char registro2){ //Copia el valor del segundo registro hacia el primero
	 registro1 =  registro2;
}

void ADDR(char registro1, char registro2){ //Suma el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	A = registro1 + registro2;
}

void SUBR(char registro1, char registro2){ //Resta el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	 A =  registro1 -  registro2;
}

void MULR(char registro1, char registro2){ //Multiplica el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	 A = registro1 * registro2;
}

void MODR(char registro1, char registro2){ //Obtiene el resto de la división del primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	 A = ( registro1) % ( registro2);
}

void DIVR(char registro1, char registro2){
	//Divide el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A; a menos que el segundo operando sea 0,
	//en cuyo caso tira error de division por cero

	if( registro2 == 0){
		perror("division por cero");
	}
	else {
		 A = ( registro1) % ( registro2);
	}
}

void INCR(char registro){ //incrementar una unidad al registro
	 registro =+ 1;
}

void DECR(char registro){ //decrementar una unidad al registro
	 registro =- 1;
}

void COMP(char registro1, char registro2){
	//Compara si el primer registro es igual al segundo. De ser verdadero, se almacena el valor 1. De lo
	//contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if( registro1 ==  registro2){
		 A = 1;
	}
	else{
		 A = 0;
	}
}

void CGEQ(char registro1, char registro2){
	//Compara si el primer registro es mayor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if( registro1 >=  registro2){
		 A = 1;
	}
	else{
		 A = 0;
	}
}

void CLEQ(char registro1, char registro2){
	//Compara si el primer registro es menor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if( registro1 <=  registro2){
		 A = 1;
	}
	else{
	     A = 0;
	}
}

void GOTO(char registro){
	//Altera el flujo de ejecución para ejecutar la instrucción apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.
	//santarAInstruccion(registro);
}

//void saltarAInstruccion(int direccion){
//	punteroInstruccionActual = baseSegmentoCodigoActual +  direccion;
//}

//void JMPZ(int direccion){
	//Altera el flujo de ejecución sólo si el valor del registro A es cero, para ejecutar la instrucción apuntada por la Dirección.
	//El valor es el desplazamiento desde el inicio del programa.

	//if(A == 0){

		//saltarAInstruccion(direccion);
	//}
//}

void JPNZ(int direccion){
	// Altera el flujo de ejecución sólo si el valor del registro A no es cero, para ejecutar la instrucción apuntada por la Dirección.
	// El valor es el desplazamiento desde el inicio del programa.

	if(A != 0){
	//	saltarAInstruccion(direccion);
	}
}

void INTE(int  direccion){

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

void SHIF(int numero, char registro){
	//Desplaza los bits del registro, tantas veces como se indique en el Número. De ser
	//desplazamiento positivo, se considera hacia la derecha. De lo contrario hacia la izquierda.
	if(numero < 0){
		 registro =  registro << numero;
	}
	if(numero > 0){
		 registro =  registro >> numero;
	}
}

void NOPP(){
	//nO HAGO NADA
}

void PUSH(int numeroA, int numeroB){
	//Apila los primeros bytes, indicado por el número, del registro hacia el stack. Modifica el valor del registro cursor de stack de forma acorde.
	//TODO: no entendi que hace
}

void TAKE(int numero, char registro){
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
/*
	int cantidadMemoria =  A;
	int* base_segmento = malloc(sizeof(int*));
	base_segmento = MSP_CrearNuevoSegmento(PIDactual, cantidadMemoria);
	A = *base_segmento;
*/
}

void FREE(){
	//Libera la memoria apuntada por el registro A. Solo se podrá liberar memoria alocada por la
	//instrucción de MALC. Destruye en la MSP el segmento indicado en el registro A.
	//TODO:
//	MSP_DestruirSegmento(PIDactual,  A);
}
/*
void INNN(){
	// Pide por consola del programa que se ingrese un número, con signo entre –2.147.483.648 y
	// 2.147.483.647. El mismo será almacenado en el registro A. Invoca al servicio correspondiente en
	// el proceso Kernel.
	int numero;
	printf("Ingrese un numero entre –2.147.483.648 y 2.147.483.647");
	scanf("%l", numero);
	 A = numero;
}

//TODO: a que refiere con invoca al servicio correspondiente en el proceso Kernel?

void INNC(){
	//Pide por consola del programa que se ingrese una cadena no más larga de lo indicado por el
	//registro B. La misma será almacenada en la posición de memoria apuntada por el registro A.
	//Invoca al servicio correspondiente en el proceso Kernel.
	char cadena[];
	printf("Ingrese una cadena no mas larga que %d",  B);
	scanf("%l", cadena);
	 A = cadena;
}

void OUTN(){
	//Imprime por consola del programa el número, con signo almacenado en el registro A. Invoca al
	//servicio correspondiente en el proceso Kernel.
	printf("el numero almacenado en el registro A es: %d",  A);
}

void OUTC(){
	//Imprime por consola del programa una cadena de tamaño indicado por el registro B que se
	//encuentra en la direccion apuntada por el registro A. Invoca al servicio correspondiente en el
	//proceso Kernel.
	int tamanio = strlen(A);
	if(tamanio <=  B){
		printf("La cadena apuntada por el registro A es: %d",  A);
	}
}

*/
void CREA(){
	/*Crea un hilo, hijo del TCB que ejecutó la llamada al sistema correspondiente. El nuevo hilo
	tendrá su Program Counter apuntado al número almacenado en el registro B. El identificador del
	nuevo hilo se almacena en el registro A.
	Para lograrlo debe generar un nuevo TCB como copia del TCB actual, asignarle un nuevo TID
	correlativo al actual, cargar en el Puntero de Instrucción la rutina donde comenzará a ejecutar el
	nuevo hilo (registro B), pasarlo de modo Kernel a modo Usuario, duplicar el segmento de stack
	desde la base del stack, hasta el cursor del stack. Asignar la base y cursor de forma acorde (tal
	que la diferencia entre cursor y base se mantenga igual)13 y luego invocar al servicio
	correspondiente en el proceso Kernel con el TCB recién generado.*/

}

void JOIN(){
	//Bloquea el programa que ejecutó la llamada al sistema hasta que el hilo con el identificador
	//almacenado en el registro A haya finalizado. Invoca al servicio correspondiente en el proceso Kernel.
}

void BLOK(){
	//Bloquea el programa que ejecutó la llamada al sistema hasta que el recurso apuntado por B se libere.
	//La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema WAIT pre-compilada.

}

void WAKE(){
	//Desbloquea al primer programa bloqueado por el recurso apuntado por B.
	//La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema SIGNAL pre-compilada.
}
