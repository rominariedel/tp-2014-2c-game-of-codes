/*
 * CPU.c
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */

#include "CPU.h"


int main(int cantArgs, char** args){
	LOGGER = log_create(NULL, "CPU", 0, LOG_LEVEL_TRACE);

	log_trace(LOGGER, "\n ------Bienvenido al CPU----- \n");
	printf("\n ------Bienvenido al CPU----- \n");
	log_trace(LOGGER, "\n Cargar archivos configuración\n");
	printf("\n Cargando archivos configuración... \n");
	cargarArchivoConfiguracion(cantArgs,args);

	log_trace(LOGGER, "IP MSP : %d \n", (int)IPMSP);
	printf("\n IP MSP : %d \n", (int)IPMSP);
	log_trace(LOGGER, "\n PUERTO MSP : %d \n", (int)PUERTOMSP);
	printf("\n PUERTO MSP : %d \n", (int)PUERTOMSP);
	log_trace(LOGGER,"\n IP KERNEL : %d \n", (int)IPKERNEL);
	printf("\n IP KERNEL : %d \n", (int)IPKERNEL);
	log_trace(LOGGER,"\n PUERTO KERNEL : %d \n", (int)PUERTOKERNEL);
	printf("\n PUERTO KERNEL : %d \n", (int)PUERTOKERNEL);
	log_trace(LOGGER,"\n RETARDO : %d \n", RETARDO);
	printf("\n RETARDO : %d \n", RETARDO);


	log_trace(LOGGER,"\n Conectando con la MSP ...\n");
	printf("\n Conectando con la MSP ...\n");
	conectarConMSP();

	log_trace(LOGGER,"\n Conectando con el Kernel ...\n");
	printf("\n Conectando con el Kernel ...\n");
	conectarConKernel();

	log_trace(LOGGER, "Aviso al Kernel que soy_CPU");
	t_datosAEnviar * paquete = crear_paquete(soy_CPU,NULL,0);
	enviar_datos(socketKernel,paquete);

	while(1)
	{
		log_trace(LOGGER, "recibir TCB y quantum del Kernel");
		printf("Estoy a la espera de que el Kernel me mande el TCB y el quantum correspondiente");
		t_datosAEnviar *  datosKernel = recibir_datos(socketKernel);
		if (datosKernel == NULL){
			printf("Fallo al recibir TCB y quantum");
			log_error(LOGGER, "Fallo al recibir TCB y quantum");
			//TODO: hacer que insista en recibir en TCB y quantum
			// break; ??? hace que vuelva al WHILE(1) ??
		}

		//1.Cargar todos los datos del TCB actual y sus registros de programacion.

		quantum = recibirTCByQuantum(datosKernel);

		int quantumActual = 0;
		printf("\n quantum a ejecutar para %d es: %d \n",PIDactual, quantumActual);


		log_trace(LOGGER, "Quantum a ejecutar para %d es: %d",PIDactual, quantum);

		printf("Recibí datos del TCB actual y sus registros de programacion");
		printf("PID: %d \n", PIDactual);
		printf("TID: %d \n", TIDactual);
		printf("KM: %d \n",KMactual);
		printf("BASE SEGMENTO CODIGO: %d \n",baseSegmentoCodigoActual);
		printf("TAMANIO SEGMENTO CODIGO %d \n", tamanioSegmentoCodigoActual);
		printf("PUNTERO INSTRUCCION %d \n", punteroInstruccionActual);
		printf("BASE STACK %d \n",baseStackActual);
		printf("CURSOR STACK %d \n",cursorStackActual);
		printf("A: %d \n",A);
		printf("B: %d \n",B);
		printf("C: %d \n",C);
		printf("D: %d \n",D);
		printf("E: %d \n",E);


		while(quantumActual>quantum || KMactual==1)
		{

			log_trace(LOGGER, "\n quantum = %d \n",quantumActual);
			printf("\n %d \n", quantumActual);
			//2. Usando el registro Puntero de Instrucción, le solicitará a la MSP la próxima instrucción a ejecutar.

			log_trace(LOGGER, "\n Solicito a MSP proximaInstruccionAEJecutar \n");
			char* proximaInstruccionAEjecutar = MSP_SolicitarProximaInstruccionAEJecutar(PIDactual, punteroInstruccionActual);
			log_trace(LOGGER, "\n Proxima Instruccion A Ejecutar: %p \n", *proximaInstruccionAEjecutar);

			// 	3. Interpretará la instrucción en BESO y realizará la operación que corresponda. Para conocer todas las instrucciones existentes y su propósito, ver el Anexo I: Especificación de ESO.
			log_trace(LOGGER, "\n Espero %d segundos de retardo \n", RETARDO);
			usleep(RETARDO);

			log_trace(LOGGER, "\n Interpretar y Ejecutar Instruccion \n");
			int respuesta = interpretarYEjecutarInstruccion(proximaInstruccionAEjecutar);

			log_trace(LOGGER, "\n Registro A \n", A);
			log_trace(LOGGER, "\n Registro B \n", B);
			log_trace(LOGGER, "\n Registro C \n", C);
			log_trace(LOGGER, "\n Registro D \n", D);
			log_trace(LOGGER, "\n Registro E \n", E);


			// 4. Actualizará los registros de propósito general del TCB correspondientes según la especificación de la instrucción.

			actualizarRegistrosTCB();

			// 5. Incrementa el Puntero de Instrucción.

			if(respuesta ==-1){
				log_error(LOGGER, "No se encontro la instruccion o no tiene los permisos necesarios");
				printf("No se encontro la instruccion o no tiene los permisos necesarios");

				log_trace(LOGGER, "Devolver TCB %d al Kernel", PIDactual);
				log_error("Error al interpretar instruccion");
				devolverTCBactual(error_al_interpretar_instruccion);
				break;
			}else{
				log_trace(LOGGER, "Incrementar punteroInstruccion %d", punteroInstruccionActual);
				punteroInstruccionActual =+ respuesta;}
				log_trace(LOGGER, "punteroInstruccion: %d", punteroInstruccionActual);

			// 5.b Incrementar quantum
			quantumActual++;
			log_trace(LOGGER, "Ejecutando %d de %d quantum", quantumActual, quantum);

		if(quantumActual == quantum && KMactual == 0){
			// 6. En caso que sea el último ciclo de ejecución del Quantum, devolverá el TCB actualizado al
			//proceso Kernel y esperará a recibir el TCB del próximo hilo a ejecutar. Si el TCB en cuestión
			//tuviera el flag KM (Kernel Mode) activado, se debe ignorar el valor del Quantum.
			log_trace(LOGGER, "Se completo el quantum! %d == %d", quantumActual, quantum);
			log_trace(LOGGER, "Devuelvo TCB %d al Kernel", PIDactual);
			devolverTCBactual(finaliza_quantum);
			limpiarRegistros();
			actualizarTCB();
			break;
		}
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


void abortarEjecucion(){ //este abortarEjecucion es solo para el principio, cuando intenta conectarse con la MSP y el Kernel
	printf("Desconectar CPU");
	exit(0);
}

void cargarRegistrosCPU(){
	A = TCBactual -> registrosProgramacion[0];
	B = TCBactual -> registrosProgramacion[1];
	C = TCBactual -> registrosProgramacion[2];
	D = TCBactual -> registrosProgramacion[3];
	E = TCBactual -> registrosProgramacion[4];
	log_trace(LOGGER, "\n Registro A \n", A);
	log_trace(LOGGER, "\n Registro B \n", B);
	log_trace(LOGGER, "\n Registro C \n", C);
	log_trace(LOGGER, "\n Registro D \n", D);
	log_trace(LOGGER, "\n Registro E \n", E);

}

void actualizarRegistrosTCB(){
	TCBactual -> registrosProgramacion[0] = A;
	TCBactual -> registrosProgramacion[1] = B;
	TCBactual -> registrosProgramacion[2] = C;
	TCBactual -> registrosProgramacion[3] = D;
	TCBactual -> registrosProgramacion[4] = E;
}



int cargarDatosTCB(){

	log_trace(LOGGER, "\n datos TCB actual \n");
	PIDactual = TCBactual->PID;
	log_trace(LOGGER, "\n PID actual: %d \n", PIDactual);
	TIDactual = TCBactual -> TID;
	log_trace(LOGGER, "\n TID actual: %d \n", TIDactual);
	KMactual = TCBactual -> KM;
	log_trace(LOGGER, "\n KM actual: %d \n", KMactual);
	baseSegmentoCodigoActual = TCBactual -> baseSegmentoCodigo;
	log_trace(LOGGER, "\n Base Segmento Codigo actual: %d \n", baseSegmentoCodigoActual);
	tamanioSegmentoCodigoActual = TCBactual -> tamanioSegmentoCodigo;
	log_trace(LOGGER, "\n Tamanio Segmento Codigo actual: %d \n", tamanioSegmentoCodigoActual);
	punteroInstruccionActual = TCBactual -> punteroInstruccion;
	log_trace(LOGGER, "\n Puntero actual: %d \n", punteroInstruccionActual);
	baseStackActual = TCBactual -> baseStack;
	log_trace(LOGGER, "\n Base Stack actual: %d \n", baseStackActual);
	cursorStackActual = TCBactual -> cursorStack;
	log_trace(LOGGER, "\n Cursor Stack actual: %d \n", cursorStackActual);

	log_trace(LOGGER, "\n Cargar Registros de Programacion \n");
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
	log_trace(LOGGER, "Devolviendo TCB actual");
	log_trace(LOGGER, "Estado TCB actual");
	log_trace(LOGGER, "\n datos TCB actual \n");
	log_trace(LOGGER, "\n PID actual: %d \n", PIDactual);
	log_trace(LOGGER, "\n TID actual: %d \n", TIDactual);
	log_trace(LOGGER, "\n KM actual: %d \n", KMactual);
	log_trace(LOGGER, "\n Base Segmento Codigo actual: %d \n", baseSegmentoCodigoActual);
	log_trace(LOGGER, "\n Tamanio Segmento Codigo actual: %d \n", tamanioSegmentoCodigoActual);
	log_trace(LOGGER, "\n Puntero actual: %d \n", punteroInstruccionActual);
	log_trace(LOGGER, "\n Base Stack actual: %d \n", baseStackActual);
	log_trace(LOGGER, "\n Cursor Stack actual: %d \n", cursorStackActual);
	log_trace(LOGGER, "Estado registros");
	log_trace(LOGGER, "\n Registro A \n", A);
	log_trace(LOGGER, "\n Registro B \n", B);
	log_trace(LOGGER, "\n Registro C \n", C);
	log_trace(LOGGER, "\n Registro D \n", D);
	log_trace(LOGGER, "\n Registro E \n", E);

	actualizarTCB();

	log_trace(LOGGER, "Armando paquete con TCB actual PID %d, TID %d", PIDactual, TIDactual);
	int op = codOperacion; //ver porque lo estoy mandando... termino quantum, por interrupcion, por lo que sea
	void * mensaje = malloc(sizeof(t_TCB) + sizeof(int));
	memcpy(mensaje, &op, sizeof(int));
	memcpy(mensaje + sizeof(int), TCBactual, sizeof(t_TCB));
	int status = enviar_datos(socketKernel, mensaje);
	if(status == -1){
		log_trace(LOGGER, "No se pudo devolver el TCB actual");
		perror("No se pudo devolver el TCBactual");}
	free(mensaje);
	log_trace(LOGGER, "Se devolvio TCB al Kernel");
	//TODO: volver al WHILE(1)
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
	log_trace(LOGGER, "\n PID actual: %d \n", PIDactual);
	log_trace(LOGGER, "\n TID actual: %d \n", TIDactual);
	log_trace(LOGGER, "\n KM actual: %d \n", KMactual);
	log_trace(LOGGER, "\n Base Segmento Codigo actual: %d \n", baseSegmentoCodigoActual);
	log_trace(LOGGER, "\n Tamanio Segmento Codigo actual: %d \n", tamanioSegmentoCodigoActual);
	log_trace(LOGGER, "\n Puntero actual: %d \n", punteroInstruccionActual);
	log_trace(LOGGER, "\n Base Stack actual: %d \n", baseStackActual);
	log_trace(LOGGER, "\n Cursor Stack actual: %d \n", cursorStackActual);
	log_trace(LOGGER, "\n Registro A \n", A);
	log_trace(LOGGER, "\n Registro B \n", B);
	log_trace(LOGGER, "\n Registro C \n", C);
	log_trace(LOGGER, "\n Registro D \n", D);
	log_trace(LOGGER, "\n Registro E \n", E);


	log_trace(LOGGER, "Se limpiaron los registros");

}



int interpretarYEjecutarInstruccion(char* instruccion){
	if(strcmp(instruccion,"LOAD")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_load));
		tparam_load* parametros = malloc(sizeof(tparam_load));
		memcpy(parametros, respuesta, sizeof(tparam_load));
		log_trace(LOGGER, "LOAD(%d,%c)",parametros->num, parametros->reg1);
		LOAD(parametros);
	return sizeof(tparam_load);
	}
	if(strcmp(instruccion,"GETM")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_getm));
		tparam_getm * parametros = malloc(sizeof(tparam_getm));
		memcpy(parametros, respuesta , sizeof(tparam_getm));
		log_trace(LOGGER, "GETM(%c,%c)",parametros->reg1, parametros->reg2);
		GETM(parametros);
	return sizeof(tparam_getm); }
	if(strcmp(instruccion,"MOVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_movr));
		tparam_movr * parametros = malloc(sizeof(tparam_movr));
		memcpy(parametros, respuesta , sizeof(tparam_movr));
		log_trace(LOGGER, "MOVR(%c,%c)",parametros->reg1, parametros->reg2);
		MOVR(parametros);
	return sizeof(tparam_movr); }
	if(strcmp(instruccion,"ADDR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_addr));
		tparam_addr* parametros = malloc(sizeof(tparam_addr));
		memcpy(parametros, respuesta , sizeof(tparam_addr));
		log_trace(LOGGER, "ADDR(%c,%c)",parametros->reg1, parametros->reg2);
		ADDR(parametros);
	return sizeof(tparam_addr); }
	if(strcmp(instruccion,"SUBR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_subr));
		tparam_subr* parametros = malloc(sizeof(tparam_subr));
		memcpy(parametros, respuesta , sizeof(tparam_subr));
		log_trace(LOGGER, "SUBR(%c,%c)",parametros->reg1, parametros->reg2);
		SUBR(parametros);
	return sizeof(tparam_subr); }
	if(strcmp(instruccion,"MULR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_mulr));
		tparam_mulr * parametros = malloc(sizeof(tparam_mulr));
		memcpy(parametros, respuesta , sizeof(tparam_mulr));
		log_trace(LOGGER, "MULR(%c,%c)",parametros->reg1, parametros->reg2);
		MULR(parametros);
	return sizeof(tparam_mulr); }
	if(strcmp(instruccion,"MODR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_modr));
		tparam_modr * parametros = malloc(sizeof(tparam_modr));
		memcpy(parametros, respuesta , sizeof(tparam_modr));
		log_trace(LOGGER, "MODR(%c,%c)",parametros->reg1, parametros->reg2);
		MODR(parametros);
	return  sizeof(tparam_modr); }
	if(strcmp(instruccion,"DIVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_divr));
		tparam_divr* parametros = malloc(sizeof(tparam_divr));
		memcpy(parametros, respuesta , sizeof(tparam_divr));
		log_trace(LOGGER, "DIVR(%c,%c)",parametros->reg1, parametros->reg2);
		DIVR(parametros);
	return sizeof(tparam_divr); }
	if(strcmp(instruccion,"INCR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_incr));
		tparam_incr* parametros = malloc(sizeof(tparam_incr));
		memcpy(parametros, respuesta , sizeof(tparam_incr));
		log_trace(LOGGER, "INCR(%c)",parametros->reg1);
 		INCR(parametros);
	return sizeof(tparam_incr); }
	if(strcmp(instruccion,"DECR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_decr));
		tparam_decr* parametros = malloc(sizeof(tparam_decr));
		memcpy(parametros, respuesta , sizeof(tparam_decr));
		log_trace(LOGGER, "DECR(%c)",parametros->reg1);
 		DECR(parametros);
	return sizeof(tparam_decr); }
	if(strcmp(instruccion,"COMP")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_comp));
		tparam_comp* parametros = malloc(sizeof(tparam_comp));
		memcpy(parametros, respuesta , sizeof(tparam_comp));
		log_trace(LOGGER, "COMP(%c,%c)",parametros->reg1, parametros->reg2);
 		COMP(parametros);
	return sizeof(tparam_comp); }
	if(strcmp(instruccion,"CGEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cgeq));
		tparam_cgeq* parametros = malloc(sizeof(tparam_cgeq));
		memcpy(parametros, respuesta , sizeof(tparam_cgeq));
		log_trace(LOGGER, "CGEQ(%c,%c)",parametros->reg1, parametros->reg2);
 		CGEQ(parametros);
	return  sizeof(tparam_cgeq); }
	if(strcmp(instruccion,"CLEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cleq));
		tparam_cleq* parametros = malloc(sizeof(tparam_cleq));
		memcpy(parametros, respuesta , sizeof(tparam_cleq));
		log_trace(LOGGER, "CLEQ(%c,%c)",parametros->reg1, parametros->reg2);
		CLEQ(parametros);
	return sizeof(tparam_cleq); }
	if(strcmp(instruccion,"GOTO")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_goto));
		tparam_goto* parametros = malloc(sizeof(tparam_goto));
		memcpy(parametros,respuesta, sizeof(tparam_goto));
		log_trace(LOGGER, "GOTO(%c)",parametros->reg1);
		GOTO(parametros);
	return sizeof(tparam_goto); }
	if(strcmp(instruccion,"JMPZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jmpz));
		tparam_jmpz* parametros = malloc(sizeof(tparam_jmpz));
		memcpy(parametros, respuesta , sizeof(tparam_jmpz));
		log_trace(LOGGER, "JMPZ(%d)",parametros->direccion);
 		JMPZ(parametros);
	return sizeof(tparam_jmpz); }
	if(strcmp(instruccion,"JPNZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jpnz));
		tparam_jpnz* parametros = malloc(sizeof(tparam_jpnz));
		memcpy(parametros, respuesta , sizeof(tparam_jpnz));
		log_trace(LOGGER, "JPNZ(%d)",parametros->direccion);
 		JPNZ(parametros);
	return sizeof(tparam_jpnz); }
	if(strcmp(instruccion,"INTE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_inte));
		tparam_inte* parametros = malloc(sizeof(tparam_inte));
		memcpy(parametros, respuesta , sizeof(tparam_inte));
		log_trace(LOGGER, "INTE(%d)",parametros->direccion);
 		INTE(parametros);
	return  sizeof(tparam_inte); }
	if(strcmp(instruccion,"NOPP")){
		log_trace(LOGGER, "NOPP()");
		NOPP();
		return 0; }
	if(strcmp(instruccion,"PUSH")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_push));
		tparam_push* parametros = malloc(sizeof(tparam_push));
		memcpy(parametros, respuesta , sizeof(tparam_push));
		log_trace(LOGGER, "PUSH(%d,%d)",parametros->num1, parametros->num2);
		PUSH(parametros);
	return sizeof(tparam_push); }
	if(strcmp(instruccion,"TAKE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_take));
		tparam_take* parametros = malloc(sizeof(tparam_take));
		memcpy(parametros, respuesta , sizeof(tparam_take));
		log_trace(LOGGER, "PUSH(%d,%c)",parametros->numero, parametros->registro);
		TAKE(parametros);
	return sizeof(tparam_take); }
	if(strcmp(instruccion,"XXXX")){
		log_trace(LOGGER,"XXXX()");
		XXXX();
		return 0;}
	if(strcmp(instruccion,"MALC") && KMactual == 1){
		log_trace(LOGGER,"MALC()");
		MALC();
		return 0; }else{
			log_error(LOGGER, "PID: %d KM = %d, no tiene permiso para ejecutar MALC()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1; }
	if(strcmp(instruccion,"FREE") && KMactual == 1){
		log_trace(LOGGER,"FREE()");
		FREE();
		return 0; }else{
			log_error(LOGGER, "PID: %d KM = %d, no tiene permiso para ejecutar FREE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"INNN") && KMactual == 1){
		log_trace(LOGGER,"INNN()");
		INNN();
		return 0; }else{
			log_error(LOGGER, "PID: %d KM = %d, no tiene permiso para ejecutar INNN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"OUTN") && KMactual == 1){
		log_trace(LOGGER,"OUTN()");
		OUTN();
		return 0; }else{
			log_error(LOGGER, "PID: %d KM = %d, no tiene permiso para ejecutar OUTN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");return -1;}
	if(strcmp(instruccion,"CREA") && KMactual == 1){
		log_trace(LOGGER,"CREA()");
		CREA();
		return 0; }else{
			log_error(LOGGER, "PID: %d KM = %d, no tiene permiso para ejecutar CREA()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"JOIN") && KMactual == 1){
		log_trace(LOGGER,"JOIN()");
		JOIN();
		return 0; }else{
			log_error(LOGGER, "PID: %d KM = %d, no tiene permiso para ejecutar JOIN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"BLOK") && KMactual == 1){
		log_trace(LOGGER,"BLOK()");
		BLOK();
		return 0; }else{
			log_error(LOGGER, "PID: %d KM = %d, no tiene permiso para ejecutar BLOK()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"WAKE") && KMactual == 1){
		log_trace(LOGGER,"WAKE()");
		WAKE();
		return 0; }else{
			log_error(LOGGER, "PID: %d KM = %d, no tiene permiso para ejecutar WAKE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}

	return -1;

}

int recibirTCByQuantum(t_datosAEnviar *  datosKernel){
	//TODO: verificar que Romi me envie tmb el quantum, y que vaya dsp del tcb.

	log_trace(LOGGER, "\n Desempaquetando Paquete \n");

	char* buffer  = malloc(datosKernel -> tamanio);
	memcpy(buffer,datosKernel,datosKernel -> tamanio);
	TCBactual = desempaquetarTCB(buffer);
	cargarDatosTCB(TCBactual);
	int quantum = buffer[12];
	return quantum;

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

char* deserializarPaqueteMSP(t_datosAEnviar* paqueteMSP){
	log_trace(LOGGER, "\n Deserializar paquete MSP \n");
	char* buffer  = malloc(paqueteMSP->tamanio);
	memcpy(buffer,paqueteMSP->datos,paqueteMSP->tamanio);
	return buffer;
}

char* deserializarPaqueteKernel(t_datosAEnviar* paqueteKernel){
	char* buffer  = malloc(paqueteKernel->tamanio);
	memcpy(buffer,paqueteKernel->datos,paqueteKernel->tamanio);
	return buffer;
}

/*Funciones MSP*/

t_datosAEnviar* MSP_SolicitarMemoria(int PID,int direccionALeer, int cantidad, int codOperacion){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &direccionALeer, cantidad);
	t_datosAEnviar * paquete = crear_paquete(codOperacion, (void*) datos, 2* sizeof(int));         //TODO: ver si esta bien esta abstraccion

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	return respuesta;

}

char* MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion){

	log_trace(LOGGER, "\n Envio paquete a MSP \n");
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 2* sizeof(int));  //TODO: 2 * sizeof(int) ??? o 4 ??? por los 4 caracteres de las instrucciones

	enviar_datos(socketMSP,paquete);
	free(datos);
	log_trace(LOGGER, "\n Recibo Respuesta MSP \n");
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);
	if(respuesta == NULL){
		log_error(LOGGER, "\n No se pudieron recibir datos MSP \n");
	}

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));

	char* proximaInstruccion = deserializarPaqueteMSP(respuesta);

	return proximaInstruccion;
}

char* MSP_SolicitarParametros(int punteroInstruccion, int cantidadParametros){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PIDactual, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, cantidadParametros); //ese puntero instruccion es el punteroInstruccionActual + 4
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 2* sizeof(int));
	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));

	char* parametros = deserializarPaqueteMSP(respuesta);

	return parametros;

}

int MSP_CrearNuevoSegmento(int PID, int tamanioSegmento){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &tamanioSegmento, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(crearNuevoSegmento, (void*) datos, 2* sizeof(int));

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));

	if(*dir_base == errorSegmentationFault){
		//ERROR
		return -1;
	}

	return *dir_base;

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

void MSP_EscribirEnMemoria(int PID, int direccion, int tamanio, void * bytes) {

	char * datos = malloc((3 * sizeof(int)) + tamanio);

	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &direccion, sizeof(int));
	memcpy(datos + (2 * sizeof(int)), &tamanio, sizeof(int));
	memcpy(datos + (3 * sizeof(int)), bytes, tamanio);

	t_datosAEnviar * paquete = crear_paquete(escribirMemoria, datos,(3 * sizeof(int)) + sizeof(void*));
	enviar_datos(socketMSP, paquete);
	free(datos);
	free(paquete);

}

/* Funciones Kernel*/

void KERNEL_ejecutarRutinaKernel(int direccion){
	devolverTCBactual(interrupcion);
	//TODO: volverAl  WHILE(1)
}

int KERNEL_IngreseNumeroPorConsola(int PID){
	//codOperacion = solicitarNumero
	char codigo = 'N';
	char * datos = malloc(sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &codigo, sizeof(char));
	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos,sizeof(int) + sizeof(char));
	enviar_datos(socketKernel,paquete);
	free(datos);
	t_datosAEnviar* respuesta = recibir_datos(socketKernel);

	int* numero = malloc(sizeof(int));
	memcpy(numero, respuesta -> datos, sizeof(int));

	return *numero;
}

t_datosAEnviar* KERNEL_IngreseCadenaPorConsola(int PID, int tamanioMaxCadena){
	//codOperacion = solicitarCadena
	char codigo = 'C';
	char * datos = malloc(sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &codigo, sizeof(char));
	memcpy(datos + sizeof(int) + sizeof(char), &tamanioMaxCadena, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos,2 * sizeof(int) + sizeof(char));
	enviar_datos(socketKernel,paquete);
	free(datos);
	t_datosAEnviar* respuesta = recibir_datos(socketKernel);
	return respuesta;
}

void KERNEL_MostrarNumeroPorConsola(int PID, int nro){
	char codigo = 'N';
	char * datos = malloc(sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &codigo, sizeof(char));
	memcpy(datos + sizeof(int) + sizeof(char), &nro, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos,2 * sizeof(int) + sizeof(char));
	enviar_datos(socketKernel,paquete);
	free(datos);
	free(paquete);
}


void KERNEL_MostrarCadenaPorConsola(int PID, char* cadena){
	char codigo = 'C';
	char * datos = malloc(sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &codigo, sizeof(char));
	memcpy(datos + sizeof(int) + sizeof(char), &cadena, sizeof(cadena));
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos,sizeof(cadena) +sizeof(int) + sizeof(char));
	enviar_datos(socketKernel,paquete);
	free(datos);
	free(paquete);
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
	*(devolverRegistro(parametrosLoad->reg1)) = parametrosLoad->num;
}

void SETM(tparam_setm* parametrosSetm){
	//Pone tantos bytes desde el segundo registro, hacia la memoria apuntada por el primer registro

	//explicacion Gaston: pone en los n bytes del registro bx en la dirección de memoria apuntanda por el registro ax
	//(ax = numero que es una posición de memoria)


}


void GETM(tparam_getm* parametrosGetm){ //Obtiene el valor de memoria apuntado por el segundo registro. El valor obtenido lo asigna en el primer registro.
	t_datosAEnviar* respuesta = MSP_SolicitarMemoria(PIDactual, *(devolverRegistro(parametrosGetm->reg2)), sizeof(int), solicitarMemoria);
	char* buffer = malloc(sizeof(respuesta));
	buffer = deserializarPaqueteMSP(respuesta);
	memcpy(devolverRegistro(parametrosGetm->reg1),devolverRegistro(parametrosGetm->reg2), sizeof(buffer));
}

void MOVR(tparam_movr* parametrosMovr){ //Copia el valor del segundo registro hacia el primero
	 *devolverRegistro(parametrosMovr->reg1) = *devolverRegistro(parametrosMovr->reg2);
}

void ADDR(tparam_addr* parametrosAddr){ //Suma el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	A = *devolverRegistro(parametrosAddr->reg1) + *devolverRegistro(parametrosAddr->reg2);
}

void SUBR(tparam_subr* parametrosSubr){ //Resta el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	 A =  *devolverRegistro(parametrosSubr->reg1) -  *devolverRegistro(parametrosSubr->reg2);
}

void MULR(tparam_mulr* parametrosMulr){ //Multiplica el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	 A = (*devolverRegistro(parametrosMulr->reg1)) * (*devolverRegistro(parametrosMulr->reg2));
}

void MODR(tparam_modr* parametrosModr){ //Obtiene el resto de la división del primer registro con el segundo registro. El resultado de la operación se almacena en el registro A.
	 A = ( *devolverRegistro(parametrosModr->reg1)) % ( *devolverRegistro(parametrosModr->reg2));
}

void DIVR(tparam_divr* parametrosDivr){
	//Divide el primer registro con el segundo registro. El resultado de la operación se almacena en el registro A; a menos que el segundo operando sea 0,
	//en cuyo caso tira error de division por cero
	//TODO: en el caso que haya error de division por cero.. devuelvo el TCB? y espero a que me mande otro? o sigo con este?

	if( *devolverRegistro(parametrosDivr->reg2) == 0){
		perror("division por cero");
	}
	else {
		 A = ( *devolverRegistro(parametrosDivr->reg1)) % ( *devolverRegistro(parametrosDivr->reg2));
	}
}

void INCR(tparam_incr* parametrosIncr){ //incrementar una unidad al registro
	 *devolverRegistro(parametrosIncr->reg1) =+ 1;
}

void DECR(tparam_decr* parametrosDecr){ //decrementar una unidad al registro
	*devolverRegistro(parametrosDecr->reg1) =- 1;
}

void COMP(tparam_comp* parametrosComp){
	//Compara si el primer registro es igual al segundo. De ser verdadero, se almacena el valor 1. De lo
	//contrario el valor 0. El resultado de la operación se almacena en el registro A.
	A = (*devolverRegistro(parametrosComp->reg1) ==  *devolverRegistro(parametrosComp->reg2));
}

void CGEQ(tparam_cgeq* parametrosCgeq){
	//Compara si el primer registro es mayor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	A = *devolverRegistro(parametrosCgeq->reg1) >=  *devolverRegistro(parametrosCgeq->reg2);
}

void CLEQ(tparam_cleq* parametrosCleq){
	//Compara si el primer registro es menor o igual al segundo. De ser verdadero, se almacena el valor 1. De lo contrario el valor 0. El resultado de la operación se almacena en el registro A.
	 A = *devolverRegistro(parametrosCleq->reg1) <=  *devolverRegistro(parametrosCleq->reg2);
}


void GOTO(tparam_goto* parametrosGoto){
	//Altera el flujo de ejecución para ejecutar la instrucción apuntada por el registro. El valor es el desplazamiento desde el inicio del programa.

	punteroInstruccionActual = *devolverRegistro(parametrosGoto->reg1);
}


void JMPZ(tparam_jmpz* parametrosJmpz){
	//Altera el flujo de ejecución sólo si el valor del registro A es cero, para ejecutar la instrucción apuntada por la Dirección.
	//El valor es el desplazamiento desde el inicio del programa.

	if(A == 0){
		punteroInstruccionActual = (parametrosJmpz->direccion);
	}
}

void JPNZ(tparam_jpnz* parametrosJpnz){
	// Altera el flujo de ejecución sólo si el valor del registro A no es cero, para ejecutar la instrucción apuntada por la Dirección.
	// El valor es el desplazamiento desde el inicio del programa.

	if(A != 0){
		punteroInstruccionActual = (parametrosJpnz->direccion);
	}
}

void INTE(tparam_inte* parametrosInte){

	XXXX();
	KERNEL_ejecutarRutinaKernel(parametrosInte->direccion);
	//TODO:ver que mas hay que hacer. si simplemente es devolverle el TCB al Kernel, no hace falta usar esta funcion. alcanza con devolverTCB()


	/*
	cuando el proceso CPU notifique al Kernel que un hilo desea ejecutar una llamada al
	sistema que requiere su atención (INTE), el Kernel recibirá el TCB de este hilo cargado y la dirección en
	memoria de la llamada a ejecutar y lo encolará en el estado BLOCK5. Luego agregará una entrada en la
	cola de llamadas al sistema con el TCB en cuestión.*/

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
	if(parametrosShif->numero < 0){
		*devolverRegistro(parametrosShif->registro) =  *devolverRegistro(parametrosShif->registro) << parametrosShif->numero;
	}
	if(parametrosShif->numero > 0){
		*devolverRegistro(parametrosShif->registro) =  *devolverRegistro(parametrosShif->registro)  >> parametrosShif->numero;
	}
}

void NOPP(){
	//NO HAGO NADA
}

void PUSH(tparam_push* parametrosPush){
	//Apila los primeros bytes, indicado por el número, del registro hacia el stack. Modifica el valor del registro cursor de stack de forma acorde.
	//TODO: Gaston me iba a averiguar
}

void TAKE(tparam_take* parametrosTake){
	//Desapila los primeros bytes, indicado por el número, del stack hacia el registro. Modifica el valor del registro de stack de forma acorde.
	//TODO:  Preguntar Gaston
}

void XXXX(){
	//Finaliza la ejecucion
	devolverTCBactual(finaliza_ejecucion);
}




/*Instrucciones Protegidas*/
/*    Solo si KM == 1     */


void MALC(){
	//Reserva una cantidad de memoria especificada por el registro A. La direccion de esta se
	//almacena en el registro A. Crea en la MSP un nuevo segmento del tamaño especificado asociado
	//al programa en ejecución.
	A = MSP_CrearNuevoSegmento(PIDactual, A);
}

void FREE(){
	//Libera la memoria apuntada por el registro A. Solo se podrá liberar memoria alocada por la
	//instrucción de MALC. Destruye en la MSP el segmento indicado en el registro A.
	//segmento que no sea ninguno de los que crea el LOADER

	if((A =! baseSegmentoCodigoActual) || (A =! baseStackActual)){
		MSP_DestruirSegmento(PIDactual,  A);
	}
}

//TODO:que es "invoca al servicio correspondiente en el proceso Kernel??? mis KERNEL_Ingreseblabla??

void INNN(){
	// Pide por consola del programa que se ingrese un número, con signo entre –2.147.483.648 y
	// 2.147.483.647. El mismo será almacenado en el registro A. Invoca al servicio correspondiente en
	// el proceso Kernel.

	A = KERNEL_IngreseNumeroPorConsola(PIDactual);
}


void INNC(){
	//Pide por consola del programa que se ingrese una cadena no más larga de lo indicado por el
	//registro B. La misma será almacenada en la posición de memoria apuntada por el registro A.
	//Invoca al servicio correspondiente en el proceso Kernel.

	t_datosAEnviar* respuesta = KERNEL_IngreseCadenaPorConsola(PIDactual, B);
	char cadena[respuesta->tamanio];
	memcpy(cadena,&respuesta,respuesta->tamanio);
	MSP_EscribirEnMemoria(PIDactual,A,respuesta->tamanio,cadena);
}

void OUTN(){
	//Imprime por consola del programa el número, con signo almacenado en el registro A. Invoca al
	//servicio correspondiente en el proceso Kernel.
	KERNEL_MostrarNumeroPorConsola(PIDactual, A);
}

void OUTC(){
	//Imprime por consola del programa una cadena de tamaño indicado por el registro B que se
	//encuentra en la direccion apuntada por el registro A. Invoca al servicio correspondiente en el
	//proceso Kernel.
	t_datosAEnviar* respuesta = MSP_SolicitarMemoria(PIDactual, A, B, solicitarMemoria);
	char cadena[respuesta->tamanio];
	memcpy(cadena, respuesta->datos, respuesta->tamanio);
	KERNEL_MostrarCadenaPorConsola(PIDactual, cadena);
}


void CREA(){

	/*Crea un hilo, hijo del TCB que ejecutó la llamada al sistema correspondiente. El nuevo hilo
	tendrá su Program Counter apuntado al número almacenado en el registro B. El identificador del
	nuevo hilo se almacena en el registro A.
	Para lograrlo debe generar un nuevo TCB como copia del TCB actual, asignarle un nuevo TID
	correlativo al actual, cargar en el Puntero de Instrucción la rutina donde comenzará a ejecutar el
	nuevo hilo (registro B), pasarlo de modo Kernel a modo Usuario, duplicar el segmento de stack
	desde la base del stack, hasta el cursor del stack. Asignar la base y cursor de forma acorde (tal
	que la diferencia entre cursor y base se mantenga igual) y luego invocar al servicio
	correspondiente en el proceso Kernel con el TCB recién generado.*/

	//TODO: mandar mail a Gaston

	t_TCB * nuevoTCB = malloc(sizeof(t_TCB));

	nuevoTCB->PID = PIDactual;
	nuevoTCB->TID = TIDactual + 1;
	nuevoTCB->KM = 0;
	nuevoTCB->baseSegmentoCodigo = 0;
	nuevoTCB->tamanioSegmentoCodigo = 0;
	nuevoTCB->punteroInstruccion = B;
	nuevoTCB->baseStack = 0;
	nuevoTCB->cursorStack = 0;
	nuevoTCB->registrosProgramacion[0]=0;
	nuevoTCB->registrosProgramacion[1]=0;
	nuevoTCB->registrosProgramacion[2]=0;
	nuevoTCB->registrosProgramacion[3]=0;
	nuevoTCB->registrosProgramacion[4]=0;

	//A = &nuevoTCB;
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


