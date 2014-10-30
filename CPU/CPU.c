/*
 * CPU.c
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */

#include "CPU.h"


int main(int cantArgs, char** args){
	LOGCPU = log_create("/home/utnso/Escritorio/LogCPU", "CPU", 0, LOG_LEVEL_TRACE);

	log_trace(LOGCPU, "\n ------Bienvenido al CPU----- \n");
	printf("\n ------Bienvenido al CPU----- \n");
	log_trace(LOGCPU, "\n Cargar archivos configuración\n");
	printf("\n Cargando archivos configuración... \n");
	cargarArchivoConfiguracion(cantArgs,args);
	log_trace(LOGCPU, "IP MSP : %d \n", (int)IPMSP);
	printf("\n IP MSP : %d \n", (int)IPMSP);
	log_trace(LOGCPU, "\n PUERTO MSP : %d \n", (int)PUERTOMSP);
	printf("\n PUERTO MSP : %d \n", (int)PUERTOMSP);
	log_trace(LOGCPU,"\n IP KERNEL : %d \n", (int)IPKERNEL);
	printf("\n IP KERNEL : %d \n", (int)IPKERNEL);
	log_trace(LOGCPU,"\n PUERTO KERNEL : %d \n", (int)PUERTOKERNEL);
	printf("\n PUERTO KERNEL : %d \n", (int)PUERTOKERNEL);
	log_trace(LOGCPU,"\n RETARDO : %d \n", RETARDO);
	printf("\n RETARDO : %d \n", RETARDO);


	log_trace(LOGCPU,"\n Conectando con la MSP ...\n");
	printf("\n Conectando con la MSP ...\n\n\n");
	conectarConMSP();

	log_trace(LOGCPU, "Aviso a la MSP que soy_CPU");
	t_datosAEnviar * paqueteMSP = crear_paquete(soy_CPU,NULL,0);
	enviar_datos(socketMSP,paqueteMSP);

	log_trace(LOGCPU,"\n Conectando con el Kernel ...\n\n\n");
	printf("\n Conectando con el Kernel ...\n");
	conectarConKernel();

	log_trace(LOGCPU, "Aviso al Kernel que soy_CPU");
	t_datosAEnviar * paqueteKERNEL = crear_paquete(soy_CPU,NULL,0);
	enviar_datos(socketKernel,paqueteKERNEL);

	while(1)
	{
		log_trace(LOGCPU, "recibir TCB y quantum del Kernel");
		printf("Estoy a la espera de que el Kernel me mande el TCB y el quantum correspondiente");
		t_datosAEnviar *  datosKernel = recibir_datos(socketKernel);
		if (datosKernel == NULL){
			printf("Fallo al recibir TCB y quantum");
			log_error(LOGCPU, "Fallo al recibir TCB y quantum");
			exit(0);
		}

		//1.Cargar todos los datos del TCB actual y sus registros de programacion.

		quantum = recibirTCByQuantum(datosKernel);

		int quantumActual = 0;
		printf("\n quantum a ejecutar para %d es: %d \n",PIDactual, quantumActual);


		log_trace(LOGCPU, "Quantum a ejecutar para %d es: %d",PIDactual, quantum);

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

			log_trace(LOGCPU, "\n quantum = %d \n",quantumActual);
			printf("\n %d \n", quantumActual);
			//2. Usando el registro Puntero de Instrucción, le solicitará a la MSP la próxima instrucción a ejecutar.

			log_trace(LOGCPU, "\n Solicito a MSP proximaInstruccionAEJecutar \n");
			char* proximaInstruccionAEjecutar = MSP_SolicitarProximaInstruccionAEJecutar(PIDactual, punteroInstruccionActual);
			log_trace(LOGCPU, "\n Proxima Instruccion A Ejecutar: %p \n", *proximaInstruccionAEjecutar);

			// 	3. Interpretará la instrucción en BESO y realizará la operación que corresponda. Para conocer todas las instrucciones existentes y su propósito, ver el Anexo I: Especificación de ESO.
			log_trace(LOGCPU, "\n Espero %d segundos de retardo \n", RETARDO);
			usleep(RETARDO);

			log_trace(LOGCPU, "\n Interpretar y Ejecutar Instruccion \n");
			int respuesta = interpretarYEjecutarInstruccion(proximaInstruccionAEjecutar);

			log_trace(LOGCPU, "\n Registro A \n", A);
			log_trace(LOGCPU, "\n Registro B \n", B);
			log_trace(LOGCPU, "\n Registro C \n", C);
			log_trace(LOGCPU, "\n Registro D \n", D);
			log_trace(LOGCPU, "\n Registro E \n", E);


			// 4. Actualizará los registros de propósito general del TCB correspondientes según la especificación de la instrucción.

			actualizarRegistrosTCB();

			// 5. Incrementa el Puntero de Instrucción.

			if(respuesta ==-1){
				log_error(LOGCPU, "No se encontro la instruccion o no tiene los permisos necesarios");
				printf("No se encontro la instruccion o no tiene los permisos necesarios");

				log_trace(LOGCPU, "Devolver TCB %d al Kernel", PIDactual);
				log_error(LOGCPU, "Error al interpretar instruccion");
				devolverTCBactual(error_al_interpretar_instruccion);
				break;
			}else{
				log_trace(LOGCPU, "Incrementar punteroInstruccion %d", punteroInstruccionActual);
				punteroInstruccionActual =+ respuesta;}
				log_trace(LOGCPU, "punteroInstruccion: %d", punteroInstruccionActual);

			// 5.b Incrementar quantum
			quantumActual++;
			log_trace(LOGCPU, "Ejecutando %d de %d quantum", quantumActual, quantum);

		if(quantumActual == quantum && KMactual == 0){
			// 6. En caso que sea el último ciclo de ejecución del Quantum, devolverá el TCB actualizado al
			//proceso Kernel y esperará a recibir el TCB del próximo hilo a ejecutar. Si el TCB en cuestión
			//tuviera el flag KM (Kernel Mode) activado, se debe ignorar el valor del Quantum.
			log_trace(LOGCPU, "Se completo el quantum! %d == %d", quantumActual, quantum);
			log_trace(LOGCPU, "Devuelvo TCB %d al Kernel", PIDactual);
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
	printf("\n Desconectar CPU \n");
	exit(0);
}

void cargarRegistrosCPU(){
	A = TCBactual -> registrosProgramacion[0];
	B = TCBactual -> registrosProgramacion[1];
	C = TCBactual -> registrosProgramacion[2];
	D = TCBactual -> registrosProgramacion[3];
	E = TCBactual -> registrosProgramacion[4];
	log_trace(LOGCPU, "\n Registro A \n", A);
	log_trace(LOGCPU, "\n Registro B \n", B);
	log_trace(LOGCPU, "\n Registro C \n", C);
	log_trace(LOGCPU, "\n Registro D \n", D);
	log_trace(LOGCPU, "\n Registro E \n", E);

}

void actualizarRegistrosTCB(){
	TCBactual -> registrosProgramacion[0] = A;
	TCBactual -> registrosProgramacion[1] = B;
	TCBactual -> registrosProgramacion[2] = C;
	TCBactual -> registrosProgramacion[3] = D;
	TCBactual -> registrosProgramacion[4] = E;
}



int cargarDatosTCB(){

	log_trace(LOGCPU, "\n datos TCB actual \n");
	PIDactual = TCBactual->PID;
	log_trace(LOGCPU, "\n PID actual: %d \n", PIDactual);
	TIDactual = TCBactual -> TID;
	log_trace(LOGCPU, "\n TID actual: %d \n", TIDactual);
	KMactual = TCBactual -> KM;
	log_trace(LOGCPU, "\n KM actual: %d \n", KMactual);
	baseSegmentoCodigoActual = TCBactual -> baseSegmentoCodigo;
	log_trace(LOGCPU, "\n Base Segmento Codigo actual: %d \n", baseSegmentoCodigoActual);
	tamanioSegmentoCodigoActual = TCBactual -> tamanioSegmentoCodigo;
	log_trace(LOGCPU, "\n Tamanio Segmento Codigo actual: %d \n", tamanioSegmentoCodigoActual);
	punteroInstruccionActual = TCBactual -> punteroInstruccion;
	log_trace(LOGCPU, "\n Puntero actual: %d \n", punteroInstruccionActual);
	baseStackActual = TCBactual -> baseStack;
	log_trace(LOGCPU, "\n Base Stack actual: %d \n", baseStackActual);
	cursorStackActual = TCBactual -> cursorStack;
	log_trace(LOGCPU, "\n Cursor Stack actual: %d \n", cursorStackActual);

	log_trace(LOGCPU, "\n Cargar Registros de Programacion \n");
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
	log_trace(LOGCPU, "Devolviendo TCB actual");
	log_trace(LOGCPU, "Estado TCB actual");
	log_trace(LOGCPU, "\n datos TCB actual \n");
	log_trace(LOGCPU, "\n PID actual: %d \n", PIDactual);
	log_trace(LOGCPU, "\n TID actual: %d \n", TIDactual);
	log_trace(LOGCPU, "\n KM actual: %d \n", KMactual);
	log_trace(LOGCPU, "\n Base Segmento Codigo actual: %d \n", baseSegmentoCodigoActual);
	log_trace(LOGCPU, "\n Tamanio Segmento Codigo actual: %d \n", tamanioSegmentoCodigoActual);
	log_trace(LOGCPU, "\n Puntero actual: %d \n", punteroInstruccionActual);
	log_trace(LOGCPU, "\n Base Stack actual: %d \n", baseStackActual);
	log_trace(LOGCPU, "\n Cursor Stack actual: %d \n", cursorStackActual);
	log_trace(LOGCPU, "Estado registros");
	log_trace(LOGCPU, "\n Registro A \n", A);
	log_trace(LOGCPU, "\n Registro B \n", B);
	log_trace(LOGCPU, "\n Registro C \n", C);
	log_trace(LOGCPU, "\n Registro D \n", D);
	log_trace(LOGCPU, "\n Registro E \n", E);

	actualizarTCB();

	log_trace(LOGCPU, "Armando paquete con TCB actual PID %d, TID %d", PIDactual, TIDactual);
	int op = codOperacion; //ver porque lo estoy mandando... termino quantum, por interrupcion, por lo que sea
	void * mensaje = malloc(sizeof(t_TCB) + sizeof(int));
	memcpy(mensaje, &op, sizeof(int));
	memcpy(mensaje + sizeof(int), TCBactual, sizeof(t_TCB));
	int status = enviar_datos(socketKernel, mensaje);
	if(status == -1){
		log_trace(LOGCPU, "No se pudo devolver el TCB actual");
		perror("No se pudo devolver el TCBactual");}
	free(mensaje);
	log_trace(LOGCPU, "Se devolvio TCB al Kernel");
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
	log_trace(LOGCPU, "\n PID actual: %d \n", PIDactual);
	log_trace(LOGCPU, "\n TID actual: %d \n", TIDactual);
	log_trace(LOGCPU, "\n KM actual: %d \n", KMactual);
	log_trace(LOGCPU, "\n Base Segmento Codigo actual: %d \n", baseSegmentoCodigoActual);
	log_trace(LOGCPU, "\n Tamanio Segmento Codigo actual: %d \n", tamanioSegmentoCodigoActual);
	log_trace(LOGCPU, "\n Puntero actual: %d \n", punteroInstruccionActual);
	log_trace(LOGCPU, "\n Base Stack actual: %d \n", baseStackActual);
	log_trace(LOGCPU, "\n Cursor Stack actual: %d \n", cursorStackActual);
	log_trace(LOGCPU, "\n Registro A \n", A);
	log_trace(LOGCPU, "\n Registro B \n", B);
	log_trace(LOGCPU, "\n Registro C \n", C);
	log_trace(LOGCPU, "\n Registro D \n", D);
	log_trace(LOGCPU, "\n Registro E \n", E);


	log_trace(LOGCPU, "Se limpiaron los registros");

}



int interpretarYEjecutarInstruccion(char* instruccion){
	if(strcmp(instruccion,"LOAD")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_load));
		tparam_load* parametros = (tparam_load*) respuesta;
		log_trace(LOGCPU, "LOAD(%d,%c)",parametros->num, parametros->reg1);
		LOAD(parametros);
	return sizeof(tparam_load);
	}
	if(strcmp(instruccion,"GETM")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_getm));
		tparam_getm * parametros = (tparam_getm*)respuesta;
		log_trace(LOGCPU, "GETM(%c,%c)",parametros->reg1, parametros->reg2);
		GETM(parametros);
	return sizeof(tparam_getm); }
	if(strcmp(instruccion,"MOVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_movr));
		tparam_movr * parametros = (tparam_movr *) respuesta;
		log_trace(LOGCPU, "MOVR(%c,%c)",parametros->reg1, parametros->reg2);
		MOVR(parametros);
	return sizeof(tparam_movr); }
	if(strcmp(instruccion,"ADDR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_addr));
		tparam_addr* parametros = (tparam_addr*)respuesta;
		log_trace(LOGCPU, "ADDR(%c,%c)",parametros->reg1, parametros->reg2);
		ADDR(parametros);
	return sizeof(tparam_addr); }
	if(strcmp(instruccion,"SUBR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_subr));
		tparam_subr* parametros = (tparam_subr *) respuesta;
		log_trace(LOGCPU, "SUBR(%c,%c)",parametros->reg1, parametros->reg2);
		SUBR(parametros);
	return sizeof(tparam_subr); }
	if(strcmp(instruccion,"MULR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_mulr));
		tparam_mulr * parametros = (tparam_mulr *) respuesta;
		log_trace(LOGCPU, "MULR(%c,%c)",parametros->reg1, parametros->reg2);
		MULR(parametros);
	return sizeof(tparam_mulr); }
	if(strcmp(instruccion,"MODR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_modr));
		tparam_modr * parametros = (tparam_modr *) respuesta;
		log_trace(LOGCPU, "MODR(%c,%c)",parametros->reg1, parametros->reg2);
		MODR(parametros);
	return  sizeof(tparam_modr); }
	if(strcmp(instruccion,"DIVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_divr));
		tparam_divr* parametros = (tparam_divr *) respuesta;
		log_trace(LOGCPU, "DIVR(%c,%c)",parametros->reg1, parametros->reg2);
		DIVR(parametros);
	return sizeof(tparam_divr); }
	if(strcmp(instruccion,"INCR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_incr));
		tparam_incr* parametros = (tparam_incr *) respuesta;
		log_trace(LOGCPU, "INCR(%c)",parametros->reg1);
 		INCR(parametros);
	return sizeof(tparam_incr); }
	if(strcmp(instruccion,"DECR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_decr));
		tparam_decr* parametros = (tparam_decr *) respuesta;
		log_trace(LOGCPU, "DECR(%c)",parametros->reg1);
 		DECR(parametros);
	return sizeof(tparam_decr); }
	if(strcmp(instruccion,"COMP")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_comp));
		tparam_comp* parametros = (tparam_comp *) respuesta;
		log_trace(LOGCPU, "COMP(%c,%c)",parametros->reg1, parametros->reg2);
 		COMP(parametros);
	return sizeof(tparam_comp); }
	if(strcmp(instruccion,"CGEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cgeq));
		tparam_cgeq* parametros = (tparam_cgeq *) respuesta;
		log_trace(LOGCPU, "CGEQ(%c,%c)",parametros->reg1, parametros->reg2);
 		CGEQ(parametros);
	return  sizeof(tparam_cgeq); }
	if(strcmp(instruccion,"CLEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cleq));
		tparam_cleq* parametros = (tparam_cleq *) respuesta;
		log_trace(LOGCPU, "CLEQ(%c,%c)",parametros->reg1, parametros->reg2);
		CLEQ(parametros);
	return sizeof(tparam_cleq); }
	if(strcmp(instruccion,"GOTO")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_goto));
		tparam_goto* parametros = (tparam_goto *) respuesta;
		log_trace(LOGCPU, "GOTO(%c)",parametros->reg1);
		GOTO(parametros);
	return sizeof(tparam_goto); }
	if(strcmp(instruccion,"JMPZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jmpz));
		tparam_jmpz* parametros = (tparam_jmpz *) respuesta;
		log_trace(LOGCPU, "JMPZ(%d)",parametros->direccion);
 		JMPZ(parametros);
	return sizeof(tparam_jmpz); }
	if(strcmp(instruccion,"JPNZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jpnz));
		tparam_jpnz* parametros = (tparam_jpnz *) respuesta;
		log_trace(LOGCPU, "JPNZ(%d)",parametros->direccion);
 		JPNZ(parametros);
	return sizeof(tparam_jpnz); }
	if(strcmp(instruccion,"INTE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_inte));
		tparam_inte* parametros = (tparam_inte *) respuesta;
		log_trace(LOGCPU, "INTE(%d)",parametros->direccion);
 		INTE(parametros);
	return  sizeof(tparam_inte); }
	if(strcmp(instruccion,"NOPP")){
		log_trace(LOGCPU, "NOPP()");
		NOPP();
		return 0; }
	if(strcmp(instruccion,"PUSH")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_push));
		tparam_push* parametros = (tparam_push *) respuesta;
		log_trace(LOGCPU, "PUSH(%d,%d)",parametros->num1, parametros->num2);
		PUSH(parametros);
	return sizeof(tparam_push); }
	if(strcmp(instruccion,"TAKE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_take));
		tparam_take* parametros = (tparam_take *) respuesta;
		log_trace(LOGCPU, "PUSH(%d,%c)",parametros->numero, parametros->registro);
		TAKE(parametros);
	return sizeof(tparam_take); }
	if(strcmp(instruccion,"XXXX")){
		log_trace(LOGCPU,"XXXX()");
		XXXX();
		return 0;}
	if(strcmp(instruccion,"MALC") && KMactual == 1){
		log_trace(LOGCPU,"MALC()");
		MALC();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar MALC()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1; }
	if(strcmp(instruccion,"FREE") && KMactual == 1){
		log_trace(LOGCPU,"FREE()");
		FREE();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar FREE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"INNN") && KMactual == 1){
		log_trace(LOGCPU,"INNN()");
		INNN();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar INNN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"OUTN") && KMactual == 1){
		log_trace(LOGCPU,"OUTN()");
		OUTN();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar OUTN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");return -1;}
	if(strcmp(instruccion,"CREA") && KMactual == 1){
		log_trace(LOGCPU,"CREA()");
		CREA();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar CREA()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"JOIN") && KMactual == 1){
		log_trace(LOGCPU,"JOIN()");
		JOIN();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar JOIN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"BLOK") && KMactual == 1){
		log_trace(LOGCPU,"BLOK()");
		BLOK();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar BLOK()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}
	if(strcmp(instruccion,"WAKE") && KMactual == 1){
		log_trace(LOGCPU,"WAKE()");
		WAKE();
		return 0; }else{
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar WAKE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}

	return -1;

}

int recibirTCByQuantum(t_datosAEnviar *  datosKernel){

	log_trace(LOGCPU, "\n Desempaquetando Paquete \n");
	char* buffer  = malloc(datosKernel -> tamanio);
	memcpy(buffer,datosKernel->datos,datosKernel -> tamanio - sizeof(int));
	TCBactual = (t_TCB *) buffer;
	cargarDatosTCB(TCBactual);
	int quantum;
	memcpy(&quantum,datosKernel->datos + sizeof(t_TCB),sizeof(int));
	return quantum;

}

char* deserializarPaqueteMSP(t_datosAEnviar* paqueteMSP){
	log_trace(LOGCPU, "\n Deserializar paquete MSP \n");
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
	t_datosAEnviar * paquete = crear_paquete(codOperacion, (void*) datos, sizeof(int) + cantidad);         //TODO: ver si esta bien esta abstraccion

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	return respuesta;

}

char* MSP_SolicitarProximaInstruccionAEJecutar(int PID, int punteroInstruccion){
	int tamanio = 4;
	log_trace(LOGCPU, "\n Envio paquete a MSP \n");
	char * datos = malloc(3 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int));
	memcpy(datos + sizeof(int) + sizeof(int), &tamanio , sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 3 * sizeof(int));

	enviar_datos(socketMSP,paquete);
	free(datos);
	log_trace(LOGCPU, "\n Recibo Respuesta MSP \n");
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);
	if(respuesta == NULL){
		log_error(LOGCPU, "\n No se pudieron recibir datos MSP \n");
	}

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));

	char* proximaInstruccion = deserializarPaqueteMSP(respuesta);

	return proximaInstruccion;
}

char* MSP_SolicitarParametros(int punteroInstruccion, int cantidadParametros){
	char * datos = malloc(3 * sizeof (int));
	memcpy(datos, &PIDactual, sizeof(int));
	memcpy(datos + sizeof(int), &punteroInstruccion, sizeof(int)); //ese puntero instruccion es el punteroInstruccionActual + 4
	memcpy(datos + sizeof(int) + sizeof(int), &cantidadParametros, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(solicitarMemoria, (void*) datos, 3 * sizeof(int));
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

t_datosAEnviar * MSP_DestruirSegmento(int PID, int baseSegmento){
	char * datos = malloc(2 * sizeof (int));
	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &baseSegmento, sizeof(int));
	t_datosAEnviar * paquete = crear_paquete(destruirSegmento, (void*) datos, 2* sizeof(int));

	enviar_datos(socketMSP,paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socketMSP);

	int * dir_base = malloc(sizeof(int));
	memcpy(dir_base, respuesta -> datos, sizeof(int));

	return respuesta;

}

void MSP_EscribirEnMemoria(int PID, int direccion, void * bytes, int tamanio) {

	char * datos = malloc((3 * sizeof(int)) + tamanio);

	memcpy(datos, &PID, sizeof(int));
	memcpy(datos + sizeof(int), &direccion, sizeof(int));
	memcpy(datos + 2 * sizeof(int), bytes, tamanio);
	memcpy(datos + 2* sizeof(int) + tamanio, &tamanio, sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(escribirMemoria, datos,2 * sizeof(int) + tamanio);
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
	char * datos = malloc(sizeof (char));
	memcpy(datos, &codigo, sizeof(char));
	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos, sizeof(char));
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
	char * datos = malloc(sizeof(char) + sizeof (int));
	memcpy(datos, &codigo, sizeof(char));
	memcpy(datos + sizeof(char), &tamanioMaxCadena, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(entrada_estandar, (void*) datos, sizeof(int) + sizeof(char));
	enviar_datos(socketKernel,paquete);
	free(datos);
	t_datosAEnviar* respuesta = recibir_datos(socketKernel);
	return respuesta;
}

void KERNEL_MostrarNumeroPorConsola(int PID, int nro){
	char codigo = 'N';
	char * datos = malloc(sizeof (int) + sizeof(char));
	memcpy(datos, &codigo, sizeof(char));
	memcpy(datos + sizeof(char), &nro, sizeof(int));
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos, sizeof(int) + sizeof(char));
	enviar_datos(socketKernel,paquete);
	free(datos);
	free(paquete);
}


void KERNEL_MostrarCadenaPorConsola(int PID, char* cadena){
	char codigo = 'C';
	char * datos = malloc(string_length(cadena) + sizeof(char));
	memcpy(datos, &codigo, sizeof(char));
	memcpy(datos + sizeof(char), &cadena, string_length(cadena));
	t_datosAEnviar* paquete = crear_paquete(salida_estandar, (void*) datos, string_length(cadena) + sizeof(char));
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
	MSP_EscribirEnMemoria(PIDactual,A,cadena,respuesta->tamanio);
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

	A = TIDactual;
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


