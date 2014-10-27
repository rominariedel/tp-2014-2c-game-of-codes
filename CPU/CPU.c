/*
 * CPU.c
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */

#include "CPU.h"


int main(int cantArgs, char** args){

	printf("\n ------Bienvenido al CPU----- \n");
	printf("\n Cargando archivos configuración... \n");
	cargarArchivoConfiguracion(cantArgs,args);

	printf("\n IP MSP : %d \n", (int)IPMSP);
	printf("\n PUERTO MSP : %d \n", (int)PUERTOMSP);
	printf("\n IP KERNEL : %d \n", (int)IPKERNEL);
	printf("\n PUERTO KERNEL : %d \n", (int)PUERTOKERNEL);
	printf("\n RETARDO : %d \n", RETARDO);


	printf("\n Conectando con la MSP ...\n");
	conectarConMSP();

	printf("\n Conectando con el Kernel ...\n");
	conectarConKernel();

	while(1)
	{
		printf("Estoy a la espera de que el Kernel me mande el TCB y el quantum correspondiente");
		t_datosAEnviar *  datosKernel = recibir_datos(socketKernel);
		if (datosKernel == NULL){
			perror("Fallo al recibir TCB y quantum");
			abortarEjecucion();
		}


		recibirTCByQuantum(datosKernel);
		//TODO: ver ocmo me envia el quantum


		//1.Cargar todos los datos del TCB actual y sus registros de programacion.
		printf("Recibí datos del TCB actual y sus registros de programacion");
		cargarDatosTCB();

		int quantumActual = 1;
		printf("\n quantumActual es: %d \n", quantumActual);

		while(quantumActual>quantum && KMactual==1)
		{

			printf("\n %d \n", quantumActual);
			//2. Usando el registro Puntero de Instrucción, le solicitará a la MSP la próxima instrucción a ejecutar.
			char* proximaInstruccionAEjecutar = MSP_SolicitarProximaInstruccionAEJecutar(PIDactual, punteroInstruccionActual);


			// 	3. Interpretará la instrucción en BESO y realizará la operación que corresponda. Para conocer todas las instrucciones existentes y su propósito, ver el Anexo I: Especificación de ESO.

			int instruccion = interpretarInstruccion(proximaInstruccionAEjecutar);
		//	ejecutarInstruccion(instruccion);

			// 4. Actualizará los registros de propósito general del TCB correspondientes según la especificación de la instrucción.

			actualizarRegistrosTCB();

			// 5. Incrementa el Puntero de Instrucción.

			punteroInstruccionActual++; //TODO

			// Incrementar quantum

			quantumActual++;


		}
		if((quantum - quantumActual)== 0){
			//termino su ejecucion
			devolverTCBactual(finaliza_ejecucion);
			limpiarRegistros();
			break;
		}

		if(quantumActual == quantum && KMactual==0){
			// 6. En caso que sea el último ciclo de ejecución del Quantum, devolverá el TCB actualizado al
			//proceso Kernel y esperará a recibir el TCB del próximo hilo a ejecutar. Si el TCB en cuestión
			//tuviera el flag KM (Kernel Mode) activado, se debe ignorar el valor del Quantum.

			devolverTCBactual(finaliza_quantum);
			limpiarRegistros();
			break;
		}

	}
	return 0;
}

void cargarArchivoConfiguracion(int cantArgs, char** args){
	t_config* configuracion = config_create(args[1]);
	PUERTOMSP = config_get_string_value(configuracion, "PUERTO_MSP");
	IPMSP = config_get_string_value(configuracion, "IP_MSP");
	PUERTOKERNEL = config_get_string_value(configuracion, "PUERTO_KERNEL");
	IPKERNEL = config_get_string_value(configuracion, "IP_KERNEL");
	RETARDO = config_get_int_value(configuracion, "RETARDO");
}

void conectarConMSP(){

	socketMSP = crear_cliente(IPMSP, PUERTOMSP);

	if(socketMSP == -1){
		perror("\n No se pudo realizar la conexion con la MSP");
		abortarEjecucion();
	}else {
		printf("\n Se conectó con la MSP, IP: %s , PUERTO: %s \n", IPMSP, PUERTOMSP);
	}
}

void conectarConKernel(){

	socketKernel = crear_cliente(IPKERNEL, PUERTOKERNEL);

	if(socketKernel == -1){
		perror("no se pudo realizar la conexion con el Kernel");
		abortarEjecucion();
	}else {
		printf("\n Se conectó con el Kernel, IP: %s , PUERTO: %s \n", IPKERNEL, PUERTOKERNEL);
	}
}


void abortarEjecucion(){
	printf("Desconectar CPU");

	exit(0); //TODO
	//TODO: abortar ejecucion, limpiar registros y enviar TCB a Kernel
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


void devolverTCBactual(int codOperacion){
	actualizarTCB();
	int op = codOperacion; //TODO: ver porque lo estoy mandando... termino quantum, por interrupcion, por lo que sea
	void * mensaje = malloc(sizeof(t_TCB) + sizeof(int));
	memcpy(mensaje, &op, sizeof(int));
	memcpy(mensaje + sizeof(int), TCBactual, sizeof(t_TCB));
	int status = enviar_datos(socketKernel, mensaje);
	if(status == -1){perror("No se pudo devolver el TCBactual");}
	free(mensaje);

	//TODO: ENVIAR QUANTUM TMB!! decirle al Kernel cuanto le quedo por ejecutar..
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
}

void limpiarTCBactual(t_TCB* tcb){
		 tcb -> PID = 0;
		 tcb -> TID = 0;
		 tcb ->   KM = 0;
		 tcb ->   baseSegmentoCodigo = 0;
		 tcb ->  tamanioSegmentoCodigo = 0;
		 tcb -> punteroInstruccion = 0;
		 tcb ->  baseStack = 0;
		 tcb ->  cursorStack = 0;
		 tcb ->  registrosProgramacion[0] = 0;
		 tcb ->  registrosProgramacion[1] = 0;
		 tcb ->  registrosProgramacion[2] = 0;
		 tcb ->  registrosProgramacion[3] = 0;
		 tcb ->  registrosProgramacion[4] = 0;
}

//TODO:este seria el ejecutar instruccion. o un interpretar y ejecutar instruccion
void interpretarYEjecutarInstruccion(char* instruccion){
	if(strcmp(instruccion,"LOAD")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_load));
		tparam_load* parametros = respuesta;
		LOAD(parametros);
	};
	if(strcmp(instruccion,"GETM")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_getm));
		tparam_getm* parametros = respuesta;
		GETM(parametros);
	};
	if(strcmp(instruccion,"MOVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_movr));
		tparam_movr* parametros = respuesta;
		MOVR(parametros);
	};
	if(strcmp(instruccion,"ADDR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_addr));
		tparam_addr* parametros = respuesta;
		ADDR(parametros);
	};
	if(strcmp(instruccion,"SUBR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_subr));
		tparam_subr* parametros = respuesta;
		SUBR(parametros);
	};
	if(strcmp(instruccion,"MULR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_mulr));
		tparam_mulr* parametros = respuesta;
		MULR(parametros);
	};
	if(strcmp(instruccion,"MODR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_modr));
		tparam_modr* parametros = respuesta;
		MODR(parametros);
	};
	if(strcmp(instruccion,"DIVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_divr));
		tparam_divr* parametros = respuesta;
		DIVR(parametros);
	};
	if(strcmp(instruccion,"INCR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_incr));
		tparam_incr* parametros = respuesta;
		INCR(parametros);
	};
	if(strcmp(instruccion,"DECR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_decr));
		tparam_decr* parametros = respuesta;
		DECR(parametros);
	};
	if(strcmp(instruccion,"COMP")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_comp));
		tparam_comp* parametros = respuesta;
		COMP(parametros);
	};
	if(strcmp(instruccion,"CGEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cgeq));
		tparam_cgeq* parametros = respuesta;
		CGEQ(parametros);
	};
	if(strcmp(instruccion,"CLEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cleq));
		tparam_cleq* parametros = respuesta;
		CLEQ(parametros);
	};
	if(strcmp(instruccion,"GOTO")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_goto));
		tparam_goto* parametros = respuesta;
		GOTO(parametros);
	};
	if(strcmp(instruccion,"JMPZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jmpz));
		tparam_jmpz* parametros = respuesta;
		JMPZ(parametros);
	}

	if(strcmp(instruccion,"JPNZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jpnz));
		tparam_goto* parametros = respuesta;
		GOTO(parametros);
	};
	if(strcmp(instruccion,"INTE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_inte));
		tparam_inte* parametros = respuesta;
		INTE(parametros);
	};
	if(strcmp(instruccion,"NOPP")){NOPP();};
	if(strcmp(instruccion,"PUSH")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_push));
		tparam_push* parametros = respuesta;
		PUSH(parametros);
	};
	if(strcmp(instruccion,"TAKE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_take));
		tparam_take* parametros = respuesta;
		TAKE(parametros);
	};
	if(strcmp(instruccion,"XXXX")){XXXX();};
	if(strcmp(instruccion,"MALC")){MALC();};
	if(strcmp(instruccion,"FREE")){FREE();};
	if(strcmp(instruccion,"INNN")){INNN();};
	if(strcmp(instruccion,"OUTN")){OUTN();};
	if(strcmp(instruccion,"CREA")){CREA();};
	if(strcmp(instruccion,"JOIN")){JOIN();};
	if(strcmp(instruccion,"BLOK")){BLOK();};
	if(strcmp(instruccion,"WAKE")){WAKE();};

}


t_TCB* desempaquetarTCB(char* buffer){
	t_TCB* tcb = malloc(sizeof(t_TCB));
	tcb -> TID = buffer[0];
	tcb -> KM = buffer[1];
	tcb -> baseSegmentoCodigo = buffer[2];
	tcb -> tamanioSegmentoCodigo = buffer[3];
	tcb -> punteroInstruccion = buffer[4];
	tcb -> baseStack = buffer[5];
	tcb -> cursorStack = buffer[6];
	tcb -> registrosProgramacion[0] = buffer[7];
	tcb -> registrosProgramacion[1] = buffer[8];
	tcb -> registrosProgramacion[2] = buffer[9];
	tcb -> registrosProgramacion[3] = buffer[10];
	tcb -> registrosProgramacion[4] = buffer[11];

	return tcb;
}

void recibirTCByQuantum(t_datosAEnviar *  datosKernel){
	//TODO: recibir quantum
	//TODO: se hace asi???????
	int tamanioBuffer = sizeof(TCB_struct) + sizeof(int);
	char* buffer  = malloc(tamanioBuffer);
	memcpy(buffer,datosKernel,tamanioBuffer);
	printf("el contenido del Buffer es: %s", buffer);
	TCBactual = desempaquetarTCB(buffer);
	cargarDatosTCB(TCBactual);

}

char* deserializarPaqueteMSP(t_datosAEnviar* paqueteMSP){
	//TODO: hacer que desarme el paquete que me mando la MSP

	char* buffer  = malloc(paqueteMSP->tamanio);
	memcpy(buffer,paqueteMSP->datos,paqueteMSP->tamanio);
	return buffer;
}


/*Funciones MSP*/

char* MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion){

	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 2* sizeof(int));

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));

	char* proximaInstruccion = deserializarPaqueteMSP(respuesta);

	return proximaInstruccion;
}

char* MSP_SolicitarParametros(int punteroInstruccion, int cantidadParametros){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PIDactual, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int)); //ese puntero instruccion es el punteroInstruccionActual + 4
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 2* sizeof(int));
	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));

	char* parametros = deserializarPaqueteMSP(respuesta);

	return parametros;

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

int* devolverRegistro(char registro){

	switch(registro){
	case 'A':
		return &A;
		break;
	case 'B':
		return &B;
		break;
	case 'C':
		return &C;
		break;
	case 'D':
		return &D;
		break;
	case 'E':
		return &E;
		break;
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

void LOAD(tparam_load* parametrosLoad){ //Carga en el registro, el número dado.
	&(devolverRegistro(parametrosLoad->reg1)) == parametrosLoad->num;
}

void SETM(tparam_setm* parametrosSetm){
	//Pone tantos bytes desde el segundo registro, hacia la memoria apuntada por el primer registro
	//explicacion Gaston: pone en los n bytes del registro bx en la dirección de memoria apuntanda por el registro ax
	//(ax = numero que es una posición de memoria)

	//TODO: sigo sin entender.
}


void GETM(tparam_getm* parametrosGetm){ //Obtiene el valor de memoria apuntado por el segundo registro. El valor obtenido lo asigna en el primer registro.
	registro1 = registro2; //TODO: PEDIR A MSP
}


void MOVR(tparam_movr* parametrosMovr){ //Copia el valor del segundo registro hacia el primero
	 registro1 =  registro2;
}

void ADDR(tparam_addr* parametrosAddr){ //Suma el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	A = registro1 + registro2;
}

void SUBR(tparam_subr* parametrosSubr){ //Resta el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	 A =  registro1 -  registro2;
}

void MULR(tparam_mulr* parametrosMulr){ //Multiplica el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	 A = registro1 * registro2;
}

void MODR(tparam_modr* parametrosModr){ //Obtiene el resto de la división del primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	 A = ( registro1) % ( registro2);
}

void DIVR(tparam_divr* parametrosDivr){
	//Divide el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A; a menos que el segundo operando sea 0,
	//en cuyo caso tira error de division por cero

	if( registro2 == 0){
		perror("division por cero");
	}
	else {
		 A = ( registro1) % ( registro2);
	}
}

void INCR(tparam_incr* parametrosIncr){ //incrementar una unidad al registro
	 registro =+ 1;
}

void DECR(tparam_decr* parametrosDecr){ //decrementar una unidad al registro
	 registro =- 1;
}

void COMP(tparam_comp* parametrosComp){
	//Compara si el primer registro es igual al segundo. De ser verdadero, se almacena el valor 1. De lo
	//contrario el valor 0. El resultado de la operación se almacena en el registro A.
	if( registro1 ==  registro2){
		 A = 1;
	}
	else{
		 A = 0;
	}
}

void CGEQ(tparam_cgeq* parametrosCgeq){
	//Compara si el primer registro es mayor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	A = registro1 >=  registro2;
}

void CLEQ(tparam_cleq* parametrosCleq){
	//Compara si el primer registro es menor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	 A = registro1 <=  registro2;
}

void saltarAInstruccion(int direccion){
	//TODO: ver si esta bien.
	punteroInstruccionActual = baseSegmentoCodigoActual +  direccion;
}

void GOTO(tparam_goto* parametrosGoto){
	//Altera el flujo de ejecución para ejecutar la instrucción apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.
	saltarAInstruccion(devolverRegistro(registro));
}


void JMPZ(tparam_jmpz* parametrosJmpz){
	//Altera el flujo de ejecución sólo si el valor del registro A es cero, para ejecutar la instrucción apuntada por la Dirección.
	//El valor es el desplazamiento desde el inicio del programa.

	if(A == 0){

		saltarAInstruccion(direccion);
	}
}

void JPNZ(tparam_jpnz* parametrosJpnz){
	// Altera el flujo de ejecución sólo si el valor del registro A no es cero, para ejecutar la instrucción apuntada por la Dirección.
	// El valor es el desplazamiento desde el inicio del programa.

	if(A != 0){
		saltarAInstruccion(direccion);
	}
}

void INTE(tparam_inte* parametrosInte){

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

void SHIF(tparam_shif* parametrosShif){
	//Desplaza los bits del registro, tantas veces como se indique en el Número. De ser
	//desplazamiento positivo, se considera hacia la derecha. De lo contrario hacia la izquierda.
	if(numero < 0){
		 devolverRegistro(registro) =  devolverRegistro(registro) << numero;
	}
	if(numero > 0){
		devolverRegistro(registro)  =  devolverRegistro(registro)  >> numero;
	}
}

void NOPP(){
	//NO HAGO NADA
}

void PUSH(tparam_push* parametrosPush){
	//Apila los primeros bytes, indicado por el número, del registro hacia el stack. Modifica el valor del registro cursor de stack de forma acorde.
	//TODO: no entendi que hace
}

void TAKE(tparam_take* parametrosTake){
	//Desapila los primeros bytes, indicado por el número, del stack hacia el registro. Modifica el valor del registro de stack de forma acorde.
	//TODO:  no entiendo que seria apilar.
}

void XXXX(){
	//Finaliza la ejecucion
	devolverTCBactual(interrupcion); //es interrupcion? o simplemente termino su ejecucion.
	abortarEjecucion();
	//TODO:ver si falta hacer algo mas
}




/*Instrucciones Protegidas*/
/*    Solo si KM == 1     */


void MALC(){
	//Reserva una cantidad de memoria especificada por el registro A. La direccion de esta se
	//almacena en el registro A. Crea en la MSP un nuevo segmento del tamaño especificado asociado
	//al programa en ejecución.

	int cantidadMemoria =  A;
	int* base_segmento = malloc(sizeof(int*));
	base_segmento = MSP_CrearNuevoSegmento(PIDactual, cantidadMemoria);
	A = &base_segmento;
	free(base_segmento);

}

void FREE(){
	//Libera la memoria apuntada por el registro A. Solo se podrá liberar memoria alocada por la
	//instrucción de MALC. Destruye en la MSP el segmento indicado en el registro A.
	//TODO: que no sea ninguno de los que crea el LOADER
	MSP_DestruirSegmento(PIDactual,  A);
}

void INNN(){
	// Pide por consola del programa que se ingrese un número, con signo entre –2.147.483.648 y
	// 2.147.483.647. El mismo será almacenado en el registro A. Invoca al servicio correspondiente en
	// el proceso Kernel.

	//TODO: pedirle al kernel
	int numero;
	printf("Ingrese un numero entre –2.147.483.648 y 2.147.483.647");
	scanf("%l", numero);
	 A = numero;
}


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

	//TODO: kernel
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
