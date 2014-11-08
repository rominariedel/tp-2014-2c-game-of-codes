/*
 * CPU.c
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */


#include "CPU.h"


int main(int cantArgs, char** args){
	LOGCPU = log_create("/home/utnso/Escritorio/LogCPU", "CPU", 0, LOG_LEVEL_TRACE);

	log_info(LOGCPU, "-------------  Bienvenido al CPU  -------------");
	printf("\n \n -------------  Bienvenido al CPU  -------------\n\n");
	log_info(LOGCPU, "Cargar archivos configuracion");
	printf("\n Cargando archivos configuracion... \n");

	cargarArchivoConfiguracion(cantArgs,args);

	log_info(LOGCPU, "IP MSP : %s",  IPMSP);
	printf("\n IP MSP : %s \n",  IPMSP);
	log_info(LOGCPU, "PUERTO MSP : %s",  PUERTOMSP);
	printf("\n PUERTO MSP : %s \n",  PUERTOMSP);
	log_info(LOGCPU,"IP KERNEL : %s ",  IPKERNEL);
	printf("\n IP KERNEL : %s \n",  IPKERNEL);
	log_info(LOGCPU,"PUERTO KERNEL : %s",  PUERTOKERNEL);
	printf("\n PUERTO KERNEL : %s \n",  PUERTOKERNEL);
	log_info(LOGCPU,"RETARDO : %d ", RETARDO);
	printf("\n RETARDO : %d \n", RETARDO);


	log_info(LOGCPU," Conectando con la MSP ...");
	printf("\n \n Conectando con la MSP ...\n\n\n");

	conectarConMSP();

	log_info(LOGCPU, "Aviso a la MSP que soy_CPU");

	t_datosAEnviar * paqueteMSP = malloc(sizeof(t_datosAEnviar));
	paqueteMSP = crear_paquete(soy_CPU,NULL,0);
	enviar_datos(socketMSP,paqueteMSP);
	free(paqueteMSP);

	log_info(LOGCPU,"\n \n Conectando con el Kernel ...\n\n\n");
	printf("\n Conectando con el Kernel ...\n");

	conectarConKernel();

	log_info(LOGCPU, "\n Aviso al Kernel que soy_CPU \n");

	t_datosAEnviar * paqueteKERNEL = malloc(sizeof(t_datosAEnviar));
	paqueteKERNEL = crear_paquete(soy_CPU,NULL,0);
	enviar_datos(socketKernel,paqueteKERNEL);
	free(paqueteKERNEL);

	while(1)
	{
		log_info(LOGCPU, "\n recibir TCB y quantum del Kernel \n");
		printf("\n Estoy a la espera de que el Kernel me mande el TCB y el quantum correspondiente \n");

		t_datosAEnviar *  datosKernel = recibir_datos(socketKernel);
		if (datosKernel == NULL){
			printf("Fallo al recibir TCB y quantum");
			log_error(LOGCPU, "Fallo al recibir TCB y quantum");
			free(datosKernel);
			exit(0);
		}

		//1.Cargar todos los datos del TCB actual y sus registros de programacion.

		quantum = recibirTCByQuantum(datosKernel);
		free(datosKernel);

		int quantumActual = 0;
		printf("\n quantum a ejecutar para %d es: %d \n",PIDactual, quantumActual);
		log_info(LOGCPU, "Quantum a ejecutar para %d es: %d",PIDactual, quantum);

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


		while(quantumActual<quantum || KMactual==1)
		{

			log_info(LOGCPU, "\n quantum = %d \n",quantumActual);
			printf("\n %d \n", quantumActual);
			//2. Usando el registro Puntero de Instrucción, le solicitará a la MSP la próxima instrucción a ejecutar.

			log_info(LOGCPU, "\n Solicito a MSP proximaInstruccionAEJecutar \n");
			char* proximaInstruccionAEjecutar = MSP_SolicitarProximaInstruccionAEJecutar(PIDactual, punteroInstruccionActual);
			log_info(LOGCPU, "\n Proxima Instruccion A Ejecutar: %p \n", *proximaInstruccionAEjecutar);

			// 	3. Interpretará la instrucción en BESO y realizará la operación que corresponda. Para conocer todas las instrucciones existentes y su propósito, ver el Anexo I: Especificación de ESO.
			log_info(LOGCPU, "\n Espero %d segundos de retardo \n", RETARDO);
			usleep(RETARDO);

			log_info(LOGCPU, "\n Interpretar y Ejecutar Instruccion \n");
			int respuesta = interpretarYEjecutarInstruccion(proximaInstruccionAEjecutar);

			log_info(LOGCPU, "\n Registro A \n", A);
			log_info(LOGCPU, "\n Registro B \n", B);
			log_info(LOGCPU, "\n Registro C \n", C);
			log_info(LOGCPU, "\n Registro D \n", D);
			log_info(LOGCPU, "\n Registro E \n", E);


			// 4. Actualizará los registros de propósito general del TCB correspondientes según la especificación de la instrucción.

			actualizarRegistrosTCB();

			// 5. Incrementa el Puntero de Instrucción.

			if(respuesta ==-1){
				log_error(LOGCPU, "No se encontro la instruccion o no tiene los permisos necesarios");
				printf("No se encontro la instruccion o no tiene los permisos necesarios");

				log_info(LOGCPU, "Devolver TCB %d al Kernel", PIDactual);
				log_error(LOGCPU, "Error al interpretar instruccion");
				devolverTCBactual(error_al_interpretar_instruccion);
				break;
			}else{
				log_info(LOGCPU, "Incrementar punteroInstruccion %d", punteroInstruccionActual);
				punteroInstruccionActual =+ respuesta;}
				log_info(LOGCPU, "punteroInstruccion: %d", punteroInstruccionActual);

			// 5.b Incrementar quantum
			quantumActual++;
			log_info(LOGCPU, "Ejecutando %d de %d quantum", quantumActual, quantum);

		if(quantumActual == quantum && KMactual == 0){
			// 6. En caso que sea el último ciclo de ejecución del Quantum, devolverá el TCB actualizado al
			//proceso Kernel y esperará a recibir el TCB del próximo hilo a ejecutar. Si el TCB en cuestión
			//tuviera el flag KM (Kernel Mode) activado, se debe ignorar el valor del Quantum.
			log_info(LOGCPU, "Se completo el quantum! %d == %d", quantumActual, quantum);
			log_info(LOGCPU, "Devuelvo TCB %d al Kernel", PIDactual);
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

	//config_destroy(configuracion);             ME TIRA BASURA CUANDO LO CORRO
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
	printf("\n Desconectar CPU \n");
	exit(0);
}

void cargarRegistrosCPU(){
	A = TCBactual -> registrosProgramacion[0];
	B = TCBactual -> registrosProgramacion[1];
	C = TCBactual -> registrosProgramacion[2];
	D = TCBactual -> registrosProgramacion[3];
	E = TCBactual -> registrosProgramacion[4];
	log_info(LOGCPU, "\n Registro A \n", A);
	log_info(LOGCPU, "\n Registro B \n", B);
	log_info(LOGCPU, "\n Registro C \n", C);
	log_info(LOGCPU, "\n Registro D \n", D);
	log_info(LOGCPU, "\n Registro E \n", E);

}

void actualizarRegistrosTCB(){
	TCBactual -> registrosProgramacion[0] = A;
	TCBactual -> registrosProgramacion[1] = B;
	TCBactual -> registrosProgramacion[2] = C;
	TCBactual -> registrosProgramacion[3] = D;
	TCBactual -> registrosProgramacion[4] = E;
}



int cargarDatosTCB(){

	log_info(LOGCPU, "\n datos TCB actual \n");
	PIDactual = TCBactual->PID;
	log_info(LOGCPU, "\n PID actual: %d \n", PIDactual);
	TIDactual = TCBactual -> TID;
	log_info(LOGCPU, "\n TID actual: %d \n", TIDactual);
	KMactual = TCBactual -> KM;
	log_info(LOGCPU, "\n KM actual: %d \n", KMactual);
	baseSegmentoCodigoActual = TCBactual -> baseSegmentoCodigo;
	log_info(LOGCPU, "\n Base Segmento Codigo actual: %d \n", baseSegmentoCodigoActual);
	tamanioSegmentoCodigoActual = TCBactual -> tamanioSegmentoCodigo;
	log_info(LOGCPU, "\n Tamanio Segmento Codigo actual: %d \n", tamanioSegmentoCodigoActual);
	punteroInstruccionActual = TCBactual -> punteroInstruccion;
	log_info(LOGCPU, "\n Puntero actual: %d \n", punteroInstruccionActual);
	baseStackActual = TCBactual -> baseStack;
	log_info(LOGCPU, "\n Base Stack actual: %d \n", baseStackActual);
	cursorStackActual = TCBactual -> cursorStack;
	log_info(LOGCPU, "\n Cursor Stack actual: %d \n", cursorStackActual);

	log_info(LOGCPU, "\n Cargar Registros de Programacion \n");
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
	log_info(LOGCPU, "Devolviendo TCB actual");
	log_info(LOGCPU, "Estado TCB actual");
	log_info(LOGCPU, "\n datos TCB actual \n");
	log_info(LOGCPU, "\n PID actual: %d \n", PIDactual);
	log_info(LOGCPU, "\n TID actual: %d \n", TIDactual);
	log_info(LOGCPU, "\n KM actual: %d \n", KMactual);
	log_info(LOGCPU, "\n Base Segmento Codigo actual: %d \n", baseSegmentoCodigoActual);
	log_info(LOGCPU, "\n Tamanio Segmento Codigo actual: %d \n", tamanioSegmentoCodigoActual);
	log_info(LOGCPU, "\n Puntero actual: %d \n", punteroInstruccionActual);
	log_info(LOGCPU, "\n Base Stack actual: %d \n", baseStackActual);
	log_info(LOGCPU, "\n Cursor Stack actual: %d \n", cursorStackActual);
	log_info(LOGCPU, "Estado registros");
	log_info(LOGCPU, "\n Registro A \n", A);
	log_info(LOGCPU, "\n Registro B \n", B);
	log_info(LOGCPU, "\n Registro C \n", C);
	log_info(LOGCPU, "\n Registro D \n", D);
	log_info(LOGCPU, "\n Registro E \n", E);

	actualizarTCB();

	log_info(LOGCPU, "Armando paquete con TCB actual PID %d, TID %d", PIDactual, TIDactual);
	int op = codOperacion; //ver porque lo estoy mandando... termino quantum, por interrupcion, por lo que sea
	void * mensaje = malloc(sizeof(t_TCB) + sizeof(int));
	memcpy(mensaje, &op, sizeof(int));
	memcpy(mensaje + sizeof(int), TCBactual, sizeof(t_TCB));
	int status = enviar_datos(socketKernel, mensaje);
	if(status == -1){
		log_info(LOGCPU, "No se pudo devolver el TCB actual");
		perror("No se pudo devolver el TCBactual");}
	free(mensaje);
	log_info(LOGCPU, "Se devolvio TCB al Kernel");

	//TODO: cada vez que devuelvo tcb es porq termino ejecucion? entonces poner limpiarRegistros
	limpiarRegistros();
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
	log_info(LOGCPU, "\n PID actual: %d \n", PIDactual);
	log_info(LOGCPU, "\n TID actual: %d \n", TIDactual);
	log_info(LOGCPU, "\n KM actual: %d \n", KMactual);
	log_info(LOGCPU, "\n Base Segmento Codigo actual: %d \n", baseSegmentoCodigoActual);
	log_info(LOGCPU, "\n Tamanio Segmento Codigo actual: %d \n", tamanioSegmentoCodigoActual);
	log_info(LOGCPU, "\n Puntero actual: %d \n", punteroInstruccionActual);
	log_info(LOGCPU, "\n Base Stack actual: %d \n", baseStackActual);
	log_info(LOGCPU, "\n Cursor Stack actual: %d \n", cursorStackActual);
	log_info(LOGCPU, "\n Registro A \n", A);
	log_info(LOGCPU, "\n Registro B \n", B);
	log_info(LOGCPU, "\n Registro C \n", C);
	log_info(LOGCPU, "\n Registro D \n", D);
	log_info(LOGCPU, "\n Registro E \n", E);


	log_info(LOGCPU, "Se limpiaron los registros");

}


int interpretarYEjecutarInstruccion(char* instruccion){
	if(strcmp(instruccion,"LOAD")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_load));
		tparam_load* parametrosLoad = (tparam_load*) respuesta;
		log_info(LOGCPU, "LOAD(%d,%c)",parametrosLoad->num, parametrosLoad->reg1);
		LOAD(parametrosLoad);
	return sizeof(tparam_load);
	}
	if(strcmp(instruccion,"GETM")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_getm));
		tparam_getm * parametros = (tparam_getm*)respuesta;
		log_info(LOGCPU, "GETM(%c,%c)",parametros->reg1, parametros->reg2);
		GETM(parametros);
	return sizeof(tparam_getm); }
	if(strcmp(instruccion,"MOVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_movr));
		tparam_movr * parametros = (tparam_movr *) respuesta;
		log_info(LOGCPU, "MOVR(%c,%c)",parametros->reg1, parametros->reg2);
		MOVR(parametros);
	return sizeof(tparam_movr); }
	if(strcmp(instruccion,"ADDR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_addr));
		tparam_addr* parametros = (tparam_addr*)respuesta;
		log_info(LOGCPU, "ADDR(%c,%c)",parametros->reg1, parametros->reg2);
		ADDR(parametros);
	return sizeof(tparam_addr); }
	if(strcmp(instruccion,"SUBR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_subr));
		tparam_subr* parametros = (tparam_subr *) respuesta;
		log_info(LOGCPU, "SUBR(%c,%c)",parametros->reg1, parametros->reg2);
		SUBR(parametros);
	return sizeof(tparam_subr); }
	if(strcmp(instruccion,"MULR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_mulr));
		tparam_mulr * parametros = (tparam_mulr *) respuesta;
		log_info(LOGCPU, "MULR(%c,%c)",parametros->reg1, parametros->reg2);
		MULR(parametros);
	return sizeof(tparam_mulr); }
	if(strcmp(instruccion,"MODR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_modr));
		tparam_modr * parametros = (tparam_modr *) respuesta;
		log_info(LOGCPU, "MODR(%c,%c)",parametros->reg1, parametros->reg2);
		MODR(parametros);
	return  sizeof(tparam_modr); }
	if(strcmp(instruccion,"DIVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_divr));
		tparam_divr* parametros = (tparam_divr *) respuesta;
		log_info(LOGCPU, "DIVR(%c,%c)",parametros->reg1, parametros->reg2);
		DIVR(parametros);
	return sizeof(tparam_divr); }
	if(strcmp(instruccion,"INCR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_incr));
		tparam_incr* parametros = (tparam_incr *) respuesta;
		log_info(LOGCPU, "INCR(%c)",parametros->reg1);
 		INCR(parametros);
	return sizeof(tparam_incr); }
	if(strcmp(instruccion,"DECR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_decr));
		tparam_decr* parametros = (tparam_decr *) respuesta;
		log_info(LOGCPU, "DECR(%c)",parametros->reg1);
 		DECR(parametros);
	return sizeof(tparam_decr); }
	if(strcmp(instruccion,"COMP")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_comp));
		tparam_comp* parametros = (tparam_comp *) respuesta;
		log_info(LOGCPU, "COMP(%c,%c)",parametros->reg1, parametros->reg2);
 		COMP(parametros);
	return sizeof(tparam_comp); }
	if(strcmp(instruccion,"CGEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cgeq));
		tparam_cgeq* parametros = (tparam_cgeq *) respuesta;
		log_info(LOGCPU, "CGEQ(%c,%c)",parametros->reg1, parametros->reg2);
 		CGEQ(parametros);
	return  sizeof(tparam_cgeq); }
	if(strcmp(instruccion,"CLEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cleq));
		tparam_cleq* parametros = (tparam_cleq *) respuesta;
		log_info(LOGCPU, "CLEQ(%c,%c)",parametros->reg1, parametros->reg2);
		CLEQ(parametros);
	return sizeof(tparam_cleq); }
	if(strcmp(instruccion,"GOTO")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_goto));
		tparam_goto* parametros = (tparam_goto *) respuesta;
		log_info(LOGCPU, "GOTO(%c)",parametros->reg1);
		GOTO(parametros);
	return sizeof(tparam_goto); }
	if(strcmp(instruccion,"JMPZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jmpz));
		tparam_jmpz* parametros = (tparam_jmpz *) respuesta;
		log_info(LOGCPU, "JMPZ(%d)",parametros->direccion);
 		JMPZ(parametros);
	return sizeof(tparam_jmpz); }
	if(strcmp(instruccion,"JPNZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jpnz));
		tparam_jpnz* parametros = (tparam_jpnz *) respuesta;
		log_info(LOGCPU, "JPNZ(%d)",parametros->direccion);
 		JPNZ(parametros);
	return sizeof(tparam_jpnz); }
	if(strcmp(instruccion,"INTE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_inte));
		tparam_inte* parametros = (tparam_inte *) respuesta;
		log_info(LOGCPU, "INTE(%d)",parametros->direccion);
 		INTE(parametros);
	return  sizeof(tparam_inte); }
	if(strcmp(instruccion,"NOPP")){
		log_info(LOGCPU, "NOPP()");
		NOPP();
		return 0; }
	if(strcmp(instruccion,"PUSH")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_push));
		tparam_push* parametros = (tparam_push *) respuesta;
		log_info(LOGCPU, "PUSH(%d,%c)",parametros->numero, parametros->registro);
		PUSH(parametros);
	return sizeof(tparam_push); }
	if(strcmp(instruccion,"TAKE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_take));
		tparam_take* parametros = (tparam_take *) respuesta;
		log_info(LOGCPU, "PUSH(%d,%c)",parametros->numero, parametros->registro);
		TAKE(parametros);
	return sizeof(tparam_take); }
	if(strcmp(instruccion,"XXXX")){
		log_info(LOGCPU,"XXXX()");
		XXXX();
		return 0;}
	if(strcmp(instruccion,"MALC") && KMactual == 1){
		log_info(LOGCPU,"MALC()");
		MALC();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar MALC()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1; }
	if(strcmp(instruccion,"FREE") && KMactual == 1){
		log_info(LOGCPU,"FREE()");
		FREE();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar FREE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"INNN") && KMactual == 1){
		log_info(LOGCPU,"INNN()");
		INNN();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar INNN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"OUTN") && KMactual == 1){
		log_info(LOGCPU,"OUTN()");
		OUTN();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar OUTN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");return -1;}
	if(strcmp(instruccion,"CREA") && KMactual == 1){
		log_info(LOGCPU,"CREA()");
		CREA();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar CREA()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"JOIN") && KMactual == 1){
		log_info(LOGCPU,"JOIN()");
		JOIN();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar JOIN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"BLOK") && KMactual == 1){
		log_info(LOGCPU,"BLOK()");
		BLOK();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar BLOK()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"WAKE") && KMactual == 1){
		log_info(LOGCPU,"WAKE()");
		WAKE();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar WAKE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}

	return -1;

}


int recibirTCByQuantum(t_datosAEnviar *  datosKernel){

	log_info(LOGCPU, "\n Desempaquetando Paquete \n");
	char* buffer  = malloc(datosKernel -> tamanio);
	memcpy(buffer,datosKernel->datos,datosKernel -> tamanio - sizeof(int));
	TCBactual = (t_TCB *) buffer;
	cargarDatosTCB(TCBactual);
	int quantum;
	memcpy(&quantum,datosKernel->datos + sizeof(t_TCB),sizeof(int));
	return quantum;

}

char* deserializarPaqueteMSP(t_datosAEnviar* paqueteMSP){
	log_info(LOGCPU, "\n Deserializar paquete MSP \n");
	char* buffer  = malloc(paqueteMSP->tamanio);
	memcpy(buffer,paqueteMSP->datos,paqueteMSP->tamanio);
	return buffer;
}

char* deserializarPaqueteKernel(t_datosAEnviar* paqueteKernel){
	char* buffer  = malloc(paqueteKernel->tamanio);
	memcpy(buffer,paqueteKernel->datos,paqueteKernel->tamanio);
	return buffer;
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





