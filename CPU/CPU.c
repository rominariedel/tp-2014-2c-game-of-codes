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
	printf(" \n\n  -------------  Bienvenido al CPU  -------------\n\n");
	log_info(LOGCPU, "Cargar archivos configuracion");
	printf("\n CARGAR ARCHIVOS CONFIGURACION \n");

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

	printf("\n -------------------------------------- \n ");

	log_info(LOGCPU," Conectando con la MSP ...");
	printf("\n \n CONECTANDO CON LA MSP \n\n\n");

	conectarConMSP();

	printf("\n -------------------------------------- \n ");

	log_info(LOGCPU, "Aviso a la MSP que soy_CPU");

	t_datosAEnviar * paqueteMSP = malloc(sizeof(t_datosAEnviar));
	paqueteMSP = crear_paquete(soy_CPU,NULL,0);
	enviar_datos(socketMSP,paqueteMSP);
	free(paqueteMSP);

	log_info(LOGCPU,"Conectando con el Kernel ...");
	printf("\n \n CONECTANDO CON EL KERNEL \n\n\n");

	conectarConKernel();


	printf("\n -------------------------------------- \n ");

	log_info(LOGCPU, "Aviso al Kernel que soy_CPU");

	t_datosAEnviar * paqueteKERNEL = malloc(sizeof(t_datosAEnviar));
	paqueteKERNEL = crear_paquete(soy_CPU,NULL,0);
	enviar_datos(socketKernel,paqueteKERNEL);
	free(paqueteKERNEL);

	while(1)
	{
		log_info(LOGCPU, "\n \n \n -------------ESPERANDO DATOS DEL KERNEL-------------\n \n \n");
		printf("\n -------------------------MAIN 0 \n " );
		log_info(LOGCPU, "recibir TCB y quantum del Kernel");
		printf("\n Estoy a la espera de que el Kernel me mande el TCB y el quantum correspondiente \n");

		t_datosAEnviar *  datosKernel = recibir_datos(socketKernel);
		if (datosKernel == NULL){
			printf("\n -------------------------MAIN 0 error datos kernel \n " );
			printf("Fallo al recibir TCB y quantum");
			log_error(LOGCPU, "Fallo al recibir TCB y quantum");
			free(datosKernel);
			exit(0);
		}

		//1.Cargar todos los datos del TCB actual y sus registros de programacion.
		printf("\n -------------------------MAIN 1 \n " );
		quantum = recibirTCByQuantum(datosKernel);
		free(datosKernel);

		printf("\n -------------------------MAIN 2 \n " );

		int quantumActual = 0;
		printf("\n quantum a ejecutar para PID: %d es: %d \n",PIDactual, quantum);
		log_info(LOGCPU, "Quantum a ejecutar para PID : %d es: %d",PIDactual, quantum);

		printf("\n -------------------------MAIN 3 \n " );
/*
		printf("\n Recibí datos del TCB actual y sus registros de programacion \n");
		printf("\n PID: %d \n", PIDactual);
		printf("\n TID: %d \n", TIDactual);
		printf("\n KM: %d \n",KMactual);
		printf("\n BASE SEGMENTO CODIGO: %d \n",baseSegmentoCodigoActual);
		printf("\n TAMANIO SEGMENTO CODIGO %d \n", tamanioSegmentoCodigoActual);
		printf("\n PUNTERO INSTRUCCION %d \n", punteroInstruccionActual);
		printf("\n BASE STACK %d \n",baseStackActual);
		printf("\n CURSOR STACK %d \n",cursorStackActual);
		printf("\n A: %d \n",A);
		printf("\n B: %d \n",B);
		printf("\n C: %d \n",C);
		printf("\n D: %d \n",D);
		printf("\n E: %d \n",E);
*/
		log_info(LOGCPU, "\n \n \n -------------EMPIEZO A EJECUTAR TCB-------------\n \n \n");

		finalizarEjecucion = 1;
		printf("\n -------------------------MAIN 4 \n " );
		while((quantumActual<quantum /*|| KMactual==1*/ ) && finalizarEjecucion) //TODO: sacar los comentarios
		{
			log_info(LOGCPU, "\n \n \n -------------QUANTUM ACTUAL: %d-------------\n \n \n", quantumActual);
			printf("\n -------------------------MAIN 5 ------------------------------------- \n " );
			log_info(LOGCPU, "quantum = %d ",quantumActual);
			printf("\n %d \n", quantumActual);
			//2. Usando el registro Puntero de Instrucción, le solicitará a la MSP la próxima instrucción a ejecutar.

			printf("\n -------------------------MAIN 6 \n " );

			log_info(LOGCPU, " Solicito a MSP proximaInstruccionAEJecutar ");
			char* proximaInstruccionAEjecutar = MSP_SolicitarProximaInstruccionAEJecutar(PIDactual, punteroInstruccionActual);
			log_info(LOGCPU, "Proxima Instruccion A Ejecutar: %p ", proximaInstruccionAEjecutar);

			printf("\n -------------------------MAIN 7 \n " );

			if(errorOperacionesConMemoria == -1){
				printf("\n -------------------------MAIN 7 ERROR MEMORIA \n " );
				devolverTCBactual(ejecucion_erronea);
				break;
			}

			printf("\n -------------------------MAIN 8 \n " );

			// 	3. Interpretará la instrucción en BESO y realizará la operación que corresponda.
			log_info(LOGCPU, " Espero %d segundos de retardo ", RETARDO);
			usleep(RETARDO);

			printf("\n -------------------------MAIN 9 \n " );

			log_info(LOGCPU, "Interpretar y Ejecutar Instruccion");
			int respuesta = interpretarYEjecutarInstruccion(proximaInstruccionAEjecutar);

			printf("\n -------------------------MAIN 10 \n " );
			//si hubo algun error con las operaciones con memoria tengo que terminar la ejecucion!!

			if(errorOperacionesConMemoria == -1){

				printf("\n -------------------------MAIN 11 ERROR MEMORIA OPERACIOOOOOOOON \n " );
				devolverTCBactual(ejecucion_erronea);
				break;
			}

			printf("\n -------------------------MAIN 11 \n " );

			log_info(LOGCPU, "  Registro A : %d  ", A);
			log_info(LOGCPU, "  Registro B : %d  ", B);
			log_info(LOGCPU, "  Registro C : %d  ", C);
			log_info(LOGCPU, "  Registro D : %d  ", D);
			log_info(LOGCPU, "  Registro E : %d  ", E);

			// 4. Actualizará los registros de propósito general del TCB correspondientes según la especificación de la instrucción.

			actualizarRegistrosTCB();

			// 5. Incrementa el Puntero de Instrucción.

			printf("\n -------------------------MAIN 12 \n " );
			printf(" \n\n\n\n\n RESPUESTA: %d", respuesta);
			if(respuesta ==-1){
				log_info(LOGCPU, "\n \n \n -------------NO SE ENCONTRO O NO TIENE PERMISOS!-------------\n \n \n");

				printf("\n -------------------------MAIN 13 ERROR RESPUESTA \n " );
				log_error(LOGCPU, "No se encontro la instruccion o no tiene los permisos necesarios");
				printf("\n No se encontro la instruccion o no tiene los permisos necesarios\n");

				log_info(LOGCPU, "Devolver TCB %d al Kernel", PIDactual);
				log_error(LOGCPU, "Error al interpretar instruccion");
				devolverTCBactual(ejecucion_erronea);
				printf("\n -------------------------MAIN 14 ERROR RESPUESTA \n " );
				break;
			}else{
				printf("\n -------------------------MAIN 13 BIEN \n " );
				log_info(LOGCPU, "Incrementar punteroInstruccion %d", punteroInstruccionActual);
				punteroInstruccionActual += (respuesta + 4) ;
				printf("\n -------------------------MAIN 14 BIEN \n " );
			}
				log_info(LOGCPU, "punteroInstruccion: %d", punteroInstruccionActual);

			printf("\n -------------------------MAIN 15 \n " );
			if(finalizarEjecucion == -1){
				printf("\n -------------------------MAIN 15 ERROR, FINALIZAR EJECUCION \n " );
				log_info(LOGCPU, "finalizo EJECUCION!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				devolverTCBactual(finaliza_ejecucion);
				break;
			}

			printf("\n -------------------------MAIN 16 \n " );
			// 5.b Incrementar quantum
			quantumActual++;
			log_info(LOGCPU, "Ejecutando %d de %d quantum", quantumActual, quantum);
			printf("\n Ejecutando %d de %d quantum \n", quantumActual, quantum);
			printf("\n -------------------------MAIN 17 \n " );

		if(quantumActual == quantum && KMactual == 0){

			printf("\n -------------------------MAIN 18   TERMINO QUANTUUUUUUUUUUUUUMMM \n " );
			// 6. En caso que sea el último ciclo de ejecución del Quantum, devolverá el TCB actualizado al
			//proceso Kernel y esperará a recibir el TCB del próximo hilo a ejecutar. Si el TCB en cuestión
			//tuviera el flag KM (Kernel Mode) activado, se debe ignorar el valor del Quantum.
			log_info(LOGCPU, "Se completo el quantum! %d == %d", quantumActual, quantum);
			log_info(LOGCPU, "Devuelvo TCB %d al Kernel", PIDactual);
			log_info(LOGCPU, "SE TERMINO EL QUANTUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
			devolverTCBactual(finaliza_quantum);
			limpiarRegistros();
			actualizarTCB();
		}
		printf("\n -------------------------MAIN 19 NO ME ENTRO AL QUANTUM == QUANTUM ACTUAL \n " );
	  }
		printf("\n -------------------------MAIN 20 SALIO DEL WHILE QUANTUM \n " );
	}
	printf("\n -------------------------MAIN 21  SALIO DEL WHILE 1 \n " );

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
	log_info(LOGCPU, "  Registro A %d  ", A);
	log_info(LOGCPU, "  Registro B %d  ", B);
	log_info(LOGCPU, "  Registro C %d  ", C);
	log_info(LOGCPU, "  Registro D %d  ", D);
	log_info(LOGCPU, "  Registro E %d  ", E);

}

void actualizarRegistrosTCB(){
	TCBactual -> registrosProgramacion[0] = A;
	TCBactual -> registrosProgramacion[1] = B;
	TCBactual -> registrosProgramacion[2] = C;
	TCBactual -> registrosProgramacion[3] = D;
	TCBactual -> registrosProgramacion[4] = E;
}



int cargarDatosTCB(){

	log_info(LOGCPU, "  datos TCB actual  ");
	PIDactual = TCBactual->PID;
	log_info(LOGCPU, "  PID actual: %d  ", PIDactual);
	TIDactual = TCBactual -> TID;
	log_info(LOGCPU, "  TID actual: %d  ", TIDactual);
	KMactual = TCBactual -> KM;
	log_info(LOGCPU, "  KM actual: %d  ", KMactual);
	baseSegmentoCodigoActual = TCBactual -> M;
	log_info(LOGCPU, "  Base Segmento Codigo actual: %d  ", baseSegmentoCodigoActual);
	tamanioSegmentoCodigoActual = TCBactual -> tamanioSegmentoCodigo;
	log_info(LOGCPU, "  Tamanio Segmento Codigo actual: %d  ", tamanioSegmentoCodigoActual);
	punteroInstruccionActual = TCBactual -> P;
	log_info(LOGCPU, "  Puntero actual: %d  ", punteroInstruccionActual);
	baseStackActual = TCBactual -> X;
	log_info(LOGCPU, "  Base Stack actual: %d  ", baseStackActual);
	cursorStackActual = TCBactual -> S;
	log_info(LOGCPU, "  Cursor Stack actual: %d  ", cursorStackActual);

	log_info(LOGCPU, "  Cargar Registros de Programacion  ");
	cargarRegistrosCPU();

	return 0;
}

int actualizarTCB(){
	TCBactual -> TID = TIDactual;
	TCBactual -> KM = KMactual;
	TCBactual -> M = baseSegmentoCodigoActual;
	TCBactual -> tamanioSegmentoCodigo = tamanioSegmentoCodigoActual;
	TCBactual -> P = punteroInstruccionActual;
	TCBactual -> X = baseStackActual;
	TCBactual -> S = cursorStackActual;
	actualizarRegistrosTCB();
	return 0;
}


void devolverTCBactual(int codOperacion){
	log_info(LOGCPU, "Devolviendo TCB actual");
	log_info(LOGCPU, "Estado TCB actual");
	log_info(LOGCPU, "  PID actual: %d  ", PIDactual);
	log_info(LOGCPU, "  TID actual: %d  ", TIDactual);
	log_info(LOGCPU, "  KM actual: %d  ", KMactual);
	log_info(LOGCPU, "  Base Segmento Codigo actual: %d  ", baseSegmentoCodigoActual);
	log_info(LOGCPU, "  Tamanio Segmento Codigo actual: %d  ", tamanioSegmentoCodigoActual);
	log_info(LOGCPU, "  Puntero actual: %d  ", punteroInstruccionActual);
	log_info(LOGCPU, "  Base Stack actual: %d  ", baseStackActual);
	log_info(LOGCPU, "  Cursor Stack actual: %d  ", cursorStackActual);
	log_info(LOGCPU, "Estado registros");
	log_info(LOGCPU, "  Registro A %d  ", A);
	log_info(LOGCPU, "  Registro B %d  ", B);
	log_info(LOGCPU, "  Registro C %d  ", C);
	log_info(LOGCPU, "  Registro D %d  ", D);
	log_info(LOGCPU, "  Registro E %d  ", E);

	actualizarTCB();

	log_info(LOGCPU, "Armando paquete con TCB actual PID %d, TID %d", PIDactual, TIDactual);
	void * mensaje = malloc(sizeof(t_TCB));
	memcpy(mensaje, TCBactual, sizeof(t_TCB));
	t_datosAEnviar* paquete = crear_paquete(codOperacion, mensaje, sizeof(t_TCB));
	int status = enviar_datos(socketKernel, paquete);
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
	log_info(LOGCPU, "  PID actual: %d  ", PIDactual);
	log_info(LOGCPU, "  TID actual: %d  ", TIDactual);
	log_info(LOGCPU, "  KM actual: %d  ", KMactual);
	log_info(LOGCPU, "  Base Segmento Codigo actual: %d  ", baseSegmentoCodigoActual);
	log_info(LOGCPU, "  Tamanio Segmento Codigo actual: %d  ", tamanioSegmentoCodigoActual);
	log_info(LOGCPU, "  Puntero actual: %d  ", punteroInstruccionActual);
	log_info(LOGCPU, "  Base Stack actual: %d  ", baseStackActual);
	log_info(LOGCPU, "  Cursor Stack actual: %d  ", cursorStackActual);
	log_info(LOGCPU, "  Registro A %d  ", A);
	log_info(LOGCPU, "  Registro B %d  ", B);
	log_info(LOGCPU, "  Registro C %d   ", C);
	log_info(LOGCPU, "  Registro D %d   ", D);
	log_info(LOGCPU, "  Registro E %d   ", E);


	log_info(LOGCPU, "Se limpiaron los registros");

}


int interpretarYEjecutarInstruccion(char* instruccion){
	printf("\n INSTRUCCION A EJECUTAR: %s \n", instruccion);
	//printf("\n INSTRUCCION A EJECUTAR: %d \n", instruccion);
	log_info(LOGCPU, "\n INSTRUCCION A EJECUTAR: %d \n", instruccion);
	if(0 == strcmp(instruccion,"LOAD")){
		printf("ME METI EN LOAD\n");
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_load));
		tparam_load* parametrosLoad = (tparam_load*) respuesta;
		log_info(LOGCPU, "LOAD(%c,%d)",parametrosLoad->reg1, parametrosLoad->num);
		LOAD(parametrosLoad);
	return sizeof(tparam_load);
	}
	if(0 == strcmp(instruccion,"GETM")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_getm));
		tparam_getm * parametros = (tparam_getm*)respuesta;
		log_info(LOGCPU, "GETM(%c,%c)",parametros->reg1, parametros->reg2);
		GETM(parametros);
	return sizeof(tparam_getm); }
	if(0 == strcmp(instruccion,"MOVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_movr));
		tparam_movr * parametros = (tparam_movr *) respuesta;
		log_info(LOGCPU, "MOVR(%c,%c)",parametros->reg1, parametros->reg2);
		MOVR(parametros);
	return sizeof(tparam_movr); }
	if(0 == strcmp(instruccion,"ADDR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_addr));
		tparam_addr* parametros = (tparam_addr*)respuesta;
		log_info(LOGCPU, "ADDR(%c,%c)",parametros->reg1, parametros->reg2);
		ADDR(parametros);
	return sizeof(tparam_addr); }
	if(0 == strcmp(instruccion,"SUBR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_subr));
		tparam_subr* parametros = (tparam_subr *) respuesta;
		log_info(LOGCPU, "SUBR(%c,%c)",parametros->reg1, parametros->reg2);
		SUBR(parametros);
	return sizeof(tparam_subr); }
	if(0 == strcmp(instruccion,"MULR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_mulr));
		tparam_mulr * parametros = (tparam_mulr *) respuesta;
		log_info(LOGCPU, "MULR(%c,%c)",parametros->reg1, parametros->reg2);
		MULR(parametros);
	return sizeof(tparam_mulr); }
	if(0 == strcmp(instruccion,"MODR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_modr));
		tparam_modr * parametros = (tparam_modr *) respuesta;
		log_info(LOGCPU, "MODR(%c,%c)",parametros->reg1, parametros->reg2);
		MODR(parametros);
	return  sizeof(tparam_modr); }
	if(0 == strcmp(instruccion,"DIVR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_divr));
		tparam_divr* parametros = (tparam_divr *) respuesta;
		log_info(LOGCPU, "DIVR(%c,%c)",parametros->reg1, parametros->reg2);
		DIVR(parametros);
	return sizeof(tparam_divr); }
	if(0 == strcmp(instruccion,"INCR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_incr));
		tparam_incr* parametros = (tparam_incr *) respuesta;
		log_info(LOGCPU, "INCR(%c)",parametros->reg1);
 		INCR(parametros);
	return sizeof(tparam_incr); }
	if(0 == strcmp(instruccion,"DECR")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_decr));
		tparam_decr* parametros = (tparam_decr *) respuesta;
		log_info(LOGCPU, "DECR(%c)",parametros->reg1);
 		DECR(parametros);
	return sizeof(tparam_decr); }
	if(0 == strcmp(instruccion,"COMP")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_comp));
		tparam_comp* parametros = (tparam_comp *) respuesta;
		log_info(LOGCPU, "COMP(%c,%c)",parametros->reg1, parametros->reg2);
 		COMP(parametros);
	return sizeof(tparam_comp); }
	if(0 == strcmp(instruccion,"CGEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cgeq));
		tparam_cgeq* parametros = (tparam_cgeq *) respuesta;
		log_info(LOGCPU, "CGEQ(%c,%c)",parametros->reg1, parametros->reg2);
 		CGEQ(parametros);
	return  sizeof(tparam_cgeq); }
	if(0 == strcmp(instruccion,"CLEQ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cleq));
		tparam_cleq* parametros = (tparam_cleq *) respuesta;
		log_info(LOGCPU, "CLEQ(%c,%c)",parametros->reg1, parametros->reg2);
		CLEQ(parametros);
	return sizeof(tparam_cleq); }
	if(0 == strcmp(instruccion,"GOTO")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, 1); //TODO cambiar a sizeof(tparam_goto
		tparam_goto* parametros = (tparam_goto *) respuesta;
		log_info(LOGCPU, "GOTO(%c)",parametros->reg1);
		GOTO(parametros);
	return sizeof(tparam_goto); }
	if(0 == strcmp(instruccion,"JMPZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jmpz));
		tparam_jmpz* parametros = (tparam_jmpz *) respuesta;
		log_info(LOGCPU, "JMPZ(%d)",parametros->direccion);
 		JMPZ(parametros);
	return sizeof(tparam_jmpz); }
	if(0 == strcmp(instruccion,"JPNZ")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jpnz));
		tparam_jpnz* parametros = (tparam_jpnz *) respuesta;
		log_info(LOGCPU, "JPNZ(%d)",parametros->direccion);
 		JPNZ(parametros);
	return sizeof(tparam_jpnz); }
	if(0 == strcmp(instruccion,"INTE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_inte));
		tparam_inte* parametros = (tparam_inte *) respuesta;
		log_info(LOGCPU, "INTE(%d)",parametros->direccion);
 		INTE(parametros);
	return  sizeof(tparam_inte); }
	if(0 == strcmp(instruccion,"NOPP")){
		log_info(LOGCPU, "NOPP()");
		NOPP();
		return 0; }
	if(0 == strcmp(instruccion,"PUSH")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_push));
		tparam_push* parametros = (tparam_push *) respuesta;
		log_info(LOGCPU, "PUSH(%d,%c)",parametros->numero, parametros->registro);
		PUSH(parametros);
	return sizeof(tparam_push); }
	if(0 == strcmp(instruccion,"TAKE")){
		char* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_take));
		tparam_take* parametros = (tparam_take *) respuesta;
		log_info(LOGCPU, "PUSH(%d,%c)",parametros->numero, parametros->registro);
		TAKE(parametros);
	return sizeof(tparam_take); }
	if(0 == strcmp(instruccion,"XXXX")){
		log_info(LOGCPU,"XXXX()");

		XXXX();
		return 0;}
	if(0 == strcmp(instruccion,"MALC") && KMactual == 1){
		log_info(LOGCPU,"MALC()");
		MALC();
		return 0; }else{
			if(0 == strcmp(instruccion,"MALC")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar MALC()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1; }}
	if(0 == strcmp(instruccion,"FREE") && KMactual == 1){
		log_info(LOGCPU,"FREE()");
		FREE();
		return 0; }else{
			if(0 == strcmp(instruccion,"FREE")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar FREE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}}
	if(0 == strcmp(instruccion,"INNN") && KMactual == 1){
		log_info(LOGCPU,"INNN()");
		INNN();
		return 0; }else{
			if(0 == strcmp(instruccion,"INNN")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar INNN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}}
	if(0 == strcmp(instruccion,"OUTN") && KMactual == 1){
		log_info(LOGCPU,"OUTN()");
		OUTN();
		return 0; }else{
			if(0 == strcmp(instruccion,"OUTN")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar OUTN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}}
	if(0 == strcmp(instruccion,"CREA\0")&& KMactual == 1){
		printf("\n---------------CREA()------------------\n");
		log_info(LOGCPU,"CREA()");
		CREA();
		return 0; }else{
			if(0 == strcmp(instruccion,"CREA")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar CREA()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}}
	if(0 == strcmp(instruccion,"JOIN") && KMactual == 1){
		printf("\n---------------JOIN()------------------\n");
		log_info(LOGCPU,"JOIN()");
		JOIN();
		return 0; }else{
			if(0 == strcmp(instruccion,"JOIN")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar JOIN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}}
	if(0 == strcmp(instruccion,"BLOK") && KMactual == 1){
		log_info(LOGCPU,"BLOK()");
		BLOK();
		return 0; }else{
			if(0 == strcmp(instruccion,"BLOK")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar BLOK()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}}
	if(0 == strcmp(instruccion,"WAKE") && KMactual == 1){
		log_info(LOGCPU,"WAKE()");
		WAKE();
		return 0; }else{
			if(0 == strcmp(instruccion,"WAKE")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar WAKE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -1;}}
	return -1;

}


int recibirTCByQuantum(t_datosAEnviar *  datosKernel){

	log_info(LOGCPU, "  Desempaquetando Paquete  ");
	char* buffer  = malloc(datosKernel -> tamanio);
	memcpy(buffer,datosKernel->datos,sizeof(t_TCB));
	TCBactual = (t_TCB *) buffer;
	cargarDatosTCB(TCBactual);
	memcpy(&quantum,datosKernel->datos + sizeof(t_TCB),sizeof(int));
	return quantum;

}

char* deserializarPaqueteMSP(t_datosAEnviar* paqueteMSP){
	log_info(LOGCPU, "  Deserializar paquete MSP  ");
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
	case 'M':
		return &baseSegmentoCodigoActual;
		break;
	}
	return 0;
}

void abortar(int codigoOperacion){
	devolverTCBactual(codigoOperacion);
	finalizarEjecucion = -1;
}



