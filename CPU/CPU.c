/*
 * CPU.c
 *
 *  Created on: 12/09/2014
 *      Author: utnso
 */


#include "CPU.h"

int main(int cantArgs, char** args){

	LOGCPU = log_create(args[2], "CPU", 0, LOG_LEVEL_TRACE);

	//inicializar_panel(CPU, "LOGOBLIGATORIOS");

	log_info(LOGCPU, "\n -------------  -------------  Bienvenido al CPU  -------------  ------------- \n");
	printf(" \n\n  -------------  Bienvenido al CPU  -------------\n\n");
	log_info(LOGCPU, "CARGAR ARCHIVOS CONFIGURACION");
	printf("\n CARGAR ARCHIVOS CONFIGURACION \n");

	cargarArchivoConfiguracion(cantArgs,args);

	log_info(LOGCPU, " --------------------------------------");
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
	log_info(LOGCPU, " -------------------------------------- \n");
	printf("\n -------------------------------------- \n ");

	log_info(LOGCPU,"Iniciando conexion con la MSP");
	printf("\n INICIANDO CONEXION CON LA MSP \n");

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
		log_info(LOGCPU, "recibir TCB y quantum del Kernel");
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
/*		hilo_t* hiloAEjecutar;
		hiloAEjecutar->tcb = TCBactual;
		hiloAEjecutar->cola = EXEC;
		comienzo_ejecucion(hiloAEjecutar, quantum);
*/

		free(datosKernel);

		log_info(LOGCPU,"Recibí datos del TCB actual y sus registros de programacion \n");
		log_info(LOGCPU,"PID: %d \n", PIDactual);
		log_info(LOGCPU,"TID: %d \n", TIDactual);
		log_info(LOGCPU,"KM: %d \n",KMactual);
		log_info(LOGCPU,"BASE SEGMENTO CODIGO: %d \n",baseSegmentoCodigoActual);
		log_info(LOGCPU,"TAMANIO SEGMENTO CODIGO %d \n", tamanioSegmentoCodigoActual);
		log_info(LOGCPU,"PUNTERO INSTRUCCION %d \n", punteroInstruccionActual);
		log_info(LOGCPU,"BASE STACK %d \n",baseStackActual);
		log_info(LOGCPU,"CURSOR STACK %d \n",cursorStackActual);
		log_info(LOGCPU,"A: %d",A);
		log_info(LOGCPU,"B: %d",B);
		log_info(LOGCPU,"C: %d",C);
		log_info(LOGCPU,"D: %d",D);
		log_info(LOGCPU,"E: %d",E);

		printf("Recibí datos del TCB actual y sus registros de programacion \n");
		printf("PID: %d \n", PIDactual);
		printf("TID: %d \n", TIDactual);
		printf("KM: %d \n", KMactual);
		printf("BASE SEGMENTO CODIGO: %d \n", baseSegmentoCodigoActual);
		printf("TAMANIO SEGMENTO CODIGO %d \n", tamanioSegmentoCodigoActual);
		printf("PUNTERO INSTRUCCION %d \n", punteroInstruccionActual);
		printf("BASE STACK %d \n", baseStackActual);
		printf("CURSOR STACK %d \n", cursorStackActual);
		printf("A: %d \n", A);
		printf("B: %d \n", B);
		printf("C: %d \n", C);
		printf("D: %d \n", D);
		printf("E: %d \n", E);



		int quantumActual = 0;
		printf("\n Quantum a ejecutar para PID: %d es: %d \n",PIDactual, quantum);
		log_info(LOGCPU, "Quantum a ejecutar para PID : %d es: %d",PIDactual, quantum);

		printf("\n \n \n -------------EMPIEZO A EJECUTAR TCB-------------\n \n \n");
		log_info(LOGCPU, "\n -------------EMPIEZO A EJECUTAR TCB-------------\n");


		ejecutoInterrupcion = 0;
		finalizarEjecucion = 1;
		aumentoPuntero = 1;
		while((quantumActual<quantum || KMactual==1 ) && finalizarEjecucion)
		{
			aumentoPuntero = 1;
			log_info(LOGCPU, "\n \n \n -------------QUANTUM ACTUAL: %d-------------\n \n \n", quantumActual);
			printf("\n -------------------------- %d ------------------------------------------------- \n", quantumActual);

			//2. Usando el registro Puntero de Instrucción, le solicitará a la MSP la próxima instrucción a ejecutar.
			log_info(LOGCPU, "Solicito a MSP proximaInstruccionAEJecutar ");
			log_info(LOGCPU, "Puntero Instruccion Actual: %d", punteroInstruccionActual);

			t_datosAEnviar* respuesta = malloc(sizeof(t_datosAEnviar));
			if(KMactual == 1){
				log_info(LOGCPU, "Leo el archivo de SYSCALL");
				respuesta = MSP_SolicitarProximaInstruccionAEJecutar(0, punteroInstruccionActual);
			}else{
				respuesta = MSP_SolicitarProximaInstruccionAEJecutar(PIDactual, punteroInstruccionActual);
			}

			char* proximaInstruccionAEjecutar = malloc(5);
			int status = procesarRespuesta(respuesta);
			if(status < 0){
				printf("Error al Solicitar Proxima Instruccion a Ejecutar \n");
				log_error(LOGCPU,"Error al Solicitar Proxima Instruccion a Ejecutar");
				abortar(ejecucion_erronea);
				break;
				}else{
					log_info(LOGCPU, "Recibo Instruccion a ejecutar  ");
					log_info(LOGCPU, "Proxima Instruccion A Ejecutar: %p ", proximaInstruccionAEjecutar);
					printf("Recibo Instruccion a ejecutar \n");
					printf("Proxima Instruccion A Ejecutar: %s \n", proximaInstruccionAEjecutar);
					memcpy(proximaInstruccionAEjecutar, respuesta -> datos, 4);
					proximaInstruccionAEjecutar[4] = '\0';
			}
			free(respuesta);

			// 	3. Interpretará la instrucción en BESO y realizará la operación que corresponda.
			log_info(LOGCPU, " Espero %d segundos de retardo ", RETARDO);

			sleep(RETARDO / 100);

			log_info(LOGCPU, "Interpretar y Ejecutar Instruccion");

			int statusEjecutar = interpretarYEjecutarInstruccion(proximaInstruccionAEjecutar);
			if(statusEjecutar < 0 || finalizarEjecucion == -1){
				abortar(ejecucion_erronea);
				break;
			}else{
				if(aumentoPuntero != -1){
				log_info(LOGCPU, "Incrementar punteroInstruccion %d", punteroInstruccionActual);
				log_info(LOGCPU, "Incrementar los 4 + respuesta : %d", statusEjecutar);
				printf( "Puntero Instruccion Actual: %d \n", punteroInstruccionActual);
				printf("Aumentar %d al puntero de instruccion \n", statusEjecutar);
				punteroInstruccionActual += (statusEjecutar + 4);
				printf( "AUMENTAR Puntero Instruccion Actual: %d\n", punteroInstruccionActual);
				log_info(LOGCPU, "Puntero Instruccion Actual: %d\n", punteroInstruccionActual);
				log_info(LOGCPU, "punteroInstruccion: %d", punteroInstruccionActual);
				}
				if(finalizarEjecucion == -2){ //ES PARA CUANDO SE EJECUTA XXXX()
					abortar(finaliza_ejecucion);
					printf("\n \n \n -------------FINALIZO EJECUCION TCB-------------\n \n \n");
					log_info(LOGCPU, "\n -------------EMPIEZO A EJECUTAR TCB-------------\n");
					break;
				}
				if(finalizarEjecucion == -3){
					printf("SE MANDO INTERRUPCION ");
					//fin_ejecucion();
					break;
				}
			}

			//Muestro como quedan los registros.
			log_info(LOGCPU, "Registro A : %d  ", A);
			log_info(LOGCPU, "Registro B : %d  ", B);
			log_info(LOGCPU, "Registro C : %d  ", C);
			log_info(LOGCPU, "Registro D : %d  ", D);
			log_info(LOGCPU, "Registro E : %d  ", E);

			log_info(LOGCPU,"Puntero M: %d",baseSegmentoCodigoActual);
			log_info(LOGCPU,"Puntero P: %d", punteroInstruccionActual);
			log_info(LOGCPU,"Puntero X: %d",baseStackActual);
			log_info(LOGCPU,"Puntero S: %d",cursorStackActual);



/*
			t_registros_cpu* listaRegistros = malloc(sizeof(t_registros_cpu));
			listaRegistros->registros_programacion[0]= A;
			listaRegistros->registros_programacion[1]= B;
			listaRegistros->registros_programacion[2]= C;
			listaRegistros->registros_programacion[3]= D;
			listaRegistros->registros_programacion[4]= E;
			listaRegistros->I = PIDactual;
			listaRegistros->K = KMactual;
			listaRegistros->M = baseSegmentoCodigoActual;
			listaRegistros->P = punteroInstruccionActual;
			listaRegistros->S = cursorStackActual;
			listaRegistros->X = baseStackActual;
			cambio_registros(*listaRegistros);
*/

			// 4. Actualizará los registros de propósito general del TCB correspondientes según la especificación de la instrucción.

			actualizarRegistrosTCB();

			// 5. Incrementar quantum
			quantumActual++;
			log_info(LOGCPU, "Ejecutando %d de %d quantum", quantumActual, quantum);
			printf("\n Ejecutando %d de %d quantum \n", quantumActual, quantum);

		if(quantumActual == quantum && KMactual == 0){

			// 6. En caso que sea el último ciclo de ejecución del Quantum, devolverá el TCB actualizado al
			//proceso Kernel y esperará a recibir el TCB del próximo hilo a ejecutar. Si el TCB en cuestión
			//tuviera el flag KM (Kernel Mode) activado, se debe ignorar el valor del Quantum.
			log_info(LOGCPU, "Se completo el quantum! %d == %d", quantumActual, quantum);
			devolverTCBactual(finaliza_quantum);
			limpiarRegistros();
			actualizarTCB();
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
		log_info(LOGCPU, " --------------------------------------");
		log_error(LOGCPU, "NO SE PUDO REALIZAR LA CONEXION CON LA MSP");
		log_info(LOGCPU, " --------------------------------------");
		abortarEjecucion();
	}else {
		log_info(LOGCPU,"Se conecto correctamente con la MSP");
		log_info(LOGCPU, "IP MSP: %s , PUERTO MSP: %s \n", IPMSP, PUERTOMSP);
		printf("\n Se conectó con la MSP, IP: %s , PUERTO: %s \n", IPMSP, PUERTOMSP);
	}
}

void conectarConKernel(){

	socketKernel = crear_cliente(IPKERNEL, PUERTOKERNEL);

	if(socketKernel == -1){
		perror("\n no se pudo realizar la conexion con el Kernel \n");
		log_info(LOGCPU, " --------------------------------------");
		log_error(LOGCPU, "NO SE PUDO REALIZAR LA CONEXION CON LA MSP");
		log_info(LOGCPU, " --------------------------------------");
		abortarEjecucion();
	}else {
		log_info(LOGCPU,"SE CONECTO CORRECTAMENTE CON EL KERNEL");
		log_info(LOGCPU,"IP Kernel: %s , PUERTO Kernel: %s \n",IPKERNEL, PUERTOKERNEL);
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
/*	log_info(LOGCPU, "  Registro A %d  ", A);
	log_info(LOGCPU, "  Registro B %d  ", B);
	log_info(LOGCPU, "  Registro C %d  ", C);
	log_info(LOGCPU, "  Registro D %d  ", D);
	log_info(LOGCPU, "  Registro E %d  ", E);
*/
}

void actualizarRegistrosTCB(){
	TCBactual -> registrosProgramacion[0] = A;
	TCBactual -> registrosProgramacion[1] = B;
	TCBactual -> registrosProgramacion[2] = C;
	TCBactual -> registrosProgramacion[3] = D;
	TCBactual -> registrosProgramacion[4] = E;
}



int cargarDatosTCB(){

	//log_info(LOGCPU, "  datos TCB actual  ");
	PIDactual = TCBactual->PID;
	//log_info(LOGCPU, "  PID actual: %d  ", PIDactual);
	TIDactual = TCBactual -> TID;
	//log_info(LOGCPU, "  TID actual: %d  ", TIDactual);
	KMactual = TCBactual -> KM;
	//log_info(LOGCPU, "  KM actual: %d  ", KMactual);
	baseSegmentoCodigoActual = TCBactual -> M;
	//log_info(LOGCPU, "  Base Segmento Codigo actual: %d  ", baseSegmentoCodigoActual);
	tamanioSegmentoCodigoActual = TCBactual -> tamanioSegmentoCodigo;
	//log_info(LOGCPU, "  Tamanio Segmento Codigo actual: %d  ", tamanioSegmentoCodigoActual);
	punteroInstruccionActual = TCBactual -> P;
	//log_info(LOGCPU, "  Puntero actual: %d  ", punteroInstruccionActual);
	baseStackActual = TCBactual -> X;
	//log_info(LOGCPU, "  Base Stack actual: %d  ", baseStackActual);
	cursorStackActual = TCBactual -> S;
	//log_info(LOGCPU, "  Cursor Stack actual: %d  ", cursorStackActual);

	//log_info(LOGCPU, "  Cargar Registros de Programacion  ");
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
	log_info(LOGCPU, "Armando paquete con TCB actual PID %d, TID %d");
	void * mensaje = malloc(sizeof(t_TCB));
	memcpy(mensaje, TCBactual, sizeof(t_TCB));
	t_datosAEnviar* paquete = crear_paquete(codOperacion, mensaje, sizeof(t_TCB));
	int status = enviar_datos(socketKernel, paquete);
	if(status == -1){
		log_info(LOGCPU, "No se pudo devolver el TCB actual");
		perror("No se pudo devolver el TCBactual");}
	free(paquete->datos);
	free(paquete);
	free(mensaje);
	log_info(LOGCPU, "Se devolvio TCB al Kernel");

//	fin_ejecucion();

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
	log_info(LOGCPU, "\n INSTRUCCION A EJECUTAR: %s \n", instruccion);

	//t_list* list_parametros = list_create();

	if(0 == strcmp(instruccion,"LOAD")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_load));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				tparam_load* parametros = (tparam_load*) procesarRespuestaMSP(respuesta);
				free(respuesta);
				log_info(LOGCPU, "LOAD(%c,%d)",parametros->reg1, parametros->num);
				printf( "LOAD(%c,%d)",parametros->reg1, parametros->num);
				//list_add(list_parametros, (void*)&parametros->reg1);
				//list_add(list_parametros,  (void*)&parametros->num);

				LOAD(parametros);
				return sizeof(tparam_load);
			}
	}

	if(0 == strcmp(instruccion,"GETM")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_getm));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
				return status;
				}else{
					tparam_getm* parametros = (tparam_getm*) procesarRespuestaMSP(respuesta);
					free(respuesta);
					log_info(LOGCPU, "GETM(%c,%c)",parametros->reg1, parametros->reg2);
					printf(  "GETM(%c,%c)",parametros->reg1, parametros->reg2);

					////list_add(list_parametros,  (void*)&parametros->reg1);
					////list_add(list_parametros,  (void*)&parametros->reg2);

					GETM(parametros);
					return sizeof(tparam_getm);
				}
	}

	if(0 == strcmp(instruccion, "SETM")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_setm));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
				return status;
				}else{

					t_datosAEnviar* respuesta1 = MSP_SolicitarMemoria(PIDactual,punteroInstruccionActual + 4, 4, solicitarMemoria);
					int* numeroSETM = malloc(respuesta1->tamanio);
					memcpy(numeroSETM, respuesta->datos, 4);

					t_datosAEnviar* respuesta2 = MSP_SolicitarMemoria(PIDactual,punteroInstruccionActual + 8, 1, solicitarMemoria);
					char* reg1SETM = malloc(respuesta2->tamanio);
					memcpy(reg1SETM, respuesta2->datos, 1);

					t_datosAEnviar* respuesta3 = MSP_SolicitarMemoria(PIDactual,punteroInstruccionActual + 9, 1, solicitarMemoria);
					char* reg2SETM = malloc(respuesta3->tamanio);
					memcpy(reg2SETM, respuesta3->datos, 1);

					tparam_setm* parametros = malloc(sizeof(tparam_setm));
					parametros->num = *numeroSETM;
					parametros->reg1 = reg1SETM[0];
					parametros->reg2 = reg2SETM[0];
					log_info(LOGCPU, "SETM(%d,%c,%c)",parametros->num, parametros->reg1, parametros->reg2);
					printf(  "SETM(%d,%c,%c)",parametros->num, parametros->reg1, parametros->reg2);
					////list_add(list_parametros,  (void*)&parametros->num);
					////list_add(list_parametros,  (void*)&parametros->reg1);
					////list_add(list_parametros,  (void*)&parametros->reg2);

					SETM(parametros);
					return sizeof(tparam_setm);
				}


/*
					t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_setm));
					char* parametrosSETM = malloc(respuesta->tamanio);
					parametrosSETM = procesarRespuestaMSP(respuesta);
					tparam_setm* parametros = (tparam_setm*) procesarRespuestaMSP(respuesta);
					free(respuesta);
					printf("PARAMETROS SETM: %s", procesarRespuestaMSP(respuesta));

					log_info(LOGCPU, "SETM(%d,%c,%c)",parametros->num, parametros->reg1, parametros->reg2);
					SETM(parametros);*/
	}
	if(0 == strcmp(instruccion,"MOVR")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_movr));
		int status = procesarRespuesta(respuesta);
			if(status < 0){
				return status;
				}else{
					tparam_movr * parametros = (tparam_movr *) procesarRespuestaMSP(respuesta);
					log_info(LOGCPU, "MOVR(%c,%c)",parametros->reg1, parametros->reg2);
					printf( "MOVR(%c,%c)",parametros->reg1, parametros->reg2);
					////list_add(list_parametros,  (void*)&parametros->reg1);
					////list_add(list_parametros,  (void*)&parametros->reg2);

					MOVR(parametros);
					return sizeof(tparam_movr);
				}
	}
	if(0 == strcmp(instruccion,"ADDR")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_addr));
		int status = procesarRespuesta(respuesta);
				if(status < 0){
					return status;
					}else{
						tparam_addr* parametros = (tparam_addr*) procesarRespuestaMSP(respuesta);
						log_info(LOGCPU, "ADDR(%c,%c)",parametros->reg1, parametros->reg2);
						printf( "ADDR(%c,%c)",parametros->reg1, parametros->reg2);
						////list_add(list_parametros,  (void*)(&parametros->reg1));
						//list_add(list_parametros,  (void*)&parametros->reg2);

						ADDR(parametros);
						return sizeof(tparam_addr);
					}
	}
	if(0 == strcmp(instruccion,"SUBR")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_subr));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
				return status;
				}else{
					tparam_subr* parametros = (tparam_subr *) procesarRespuestaMSP(respuesta);
					log_info(LOGCPU, "SUBR(%c,%c)",parametros->reg1, parametros->reg2);
					printf(  "SUBR(%c,%c)",parametros->reg1, parametros->reg2);
					//list_add(list_parametros,  (void*)&parametros->reg1);
					//list_add(list_parametros,  (void*)&parametros->reg2);

					SUBR(parametros);
					return sizeof(tparam_subr);
				}
	}
	if(0 == strcmp(instruccion,"MULR")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_mulr));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
				return status;
				}else{
					tparam_mulr * parametros = (tparam_mulr *) procesarRespuestaMSP(respuesta);
					log_info(LOGCPU, "MULR(%c,%c)",parametros->reg1, parametros->reg2);
					printf(  "MULR(%c,%c)",parametros->reg1, parametros->reg2);

					//list_add(list_parametros,  (void*)&parametros->reg1);
					//list_add(list_parametros,  (void*)&parametros->reg2);

					MULR(parametros);
					return sizeof(tparam_mulr);
				}
	}
	if(0 == strcmp(instruccion,"MODR")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_modr));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
				return status;
				}else{
					tparam_modr * parametros = (tparam_modr *)  procesarRespuestaMSP(respuesta);
					log_info(LOGCPU, "MODR(%c,%c)",parametros->reg1, parametros->reg2);
					printf( "MODR(%c,%c)",parametros->reg1, parametros->reg2);
					//list_add(list_parametros,  (void*)&parametros->reg1);
					//list_add(list_parametros,  (void*)&parametros->reg2);

					MODR(parametros);
					return  sizeof(tparam_modr);
				}
	}
	if(0 == strcmp(instruccion,"DIVR")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_divr));
		int status = procesarRespuesta(respuesta);
				if(status < 0){
						return status;
						}else{
							tparam_divr* parametros = (tparam_divr *) procesarRespuestaMSP(respuesta);
							log_info(LOGCPU, "DIVR(%c,%c)",parametros->reg1, parametros->reg2);
							printf("DIVR(%c,%c)",parametros->reg1, parametros->reg2);
							//list_add(list_parametros,  (void*)&parametros->reg1);
							//list_add(list_parametros,  (void*)&parametros->reg2);

							DIVR(parametros);
							return sizeof(tparam_divr);
						}
	}
	if(0 == strcmp(instruccion,"INCR")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_incr));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				tparam_incr* parametros = (tparam_incr *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "INCR(%c)",parametros->reg1);
				printf("INCR(%c)",parametros->reg1);
				//list_add(list_parametros,  (void*)&parametros->reg1);

				INCR(parametros);
				return sizeof(tparam_incr);
			}
	}
	if(0 == strcmp(instruccion,"DECR")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_decr));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				tparam_decr* parametros = (tparam_decr *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "DECR(%c)",parametros->reg1);
				printf("DECR(%c)",parametros->reg1);
				//list_add(list_parametros,  (void*)&parametros->reg1);


				DECR(parametros);
				return sizeof(tparam_decr);
			}
	}
	if(0 == strcmp(instruccion,"COMP")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_comp));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				tparam_comp* parametros = (tparam_comp *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "COMP(%c,%c)",parametros->reg1, parametros->reg2);
				printf( "COMP(%c,%c)",parametros->reg1, parametros->reg2);
				//list_add(list_parametros,  (void*)&parametros->reg1);
				//list_add(list_parametros,  (void*)&parametros->reg2);

				COMP(parametros);
				return sizeof(tparam_comp);
			}
	}
	if(0 == strcmp(instruccion,"CGEQ")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cgeq));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				tparam_cgeq* parametros = (tparam_cgeq *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "CGEQ(%c,%c)",parametros->reg1, parametros->reg2);
				printf("CGEQ(%c,%c)",parametros->reg1, parametros->reg2);

				//list_add(list_parametros,  (void*)&parametros->reg1);
				//list_add(list_parametros,  (void*)&parametros->reg2);

				CGEQ(parametros);
				return  sizeof(tparam_cgeq);
			}
	}
	if(0 == strcmp(instruccion,"CLEQ")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_cleq));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				tparam_cleq* parametros = (tparam_cleq *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "CLEQ(%c,%c)",parametros->reg1, parametros->reg2);
				printf( "CLEQ(%c,%c)",parametros->reg1, parametros->reg2);
				//list_add(list_parametros,  (void*)&parametros->reg1);
				//list_add(list_parametros,  (void*)&parametros->reg2);

				CLEQ(parametros);
				return sizeof(tparam_cleq);
			}
	}
	if(0 == strcmp(instruccion,"GOTO")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, 1);
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				tparam_goto* parametros = (tparam_goto *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "GOTO(%c)",parametros->reg1);
				printf( "GOTO(%c)",parametros->reg1);
				aumentoPuntero = -1;

				//list_add(list_parametros,  (void*)&parametros->reg1);

				GOTO(parametros);
				return 0;
			}
	}
	if(0 == strcmp(instruccion,"JMPZ")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jmpz));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				tparam_jmpz* parametros = (tparam_jmpz *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "JMPZ(%d)",parametros->direccion);
				printf( "JMPZ(%d)",parametros->direccion);

				//list_add(list_parametros,  (void*)&parametros->direccion);

				JMPZ(parametros);
				return sizeof(tparam_jmpz);
			}
	}
	if(0 == strcmp(instruccion,"JPNZ")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_jpnz));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				tparam_jpnz* parametros = (tparam_jpnz *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "JPNZ(%d)",parametros->direccion);
				printf("JPNZ(%d)",parametros->direccion);
				//list_add(list_parametros,  (void*)&parametros->direccion);

				JPNZ(parametros);
				return sizeof(tparam_jpnz);
			}
	}
	if(0 == strcmp(instruccion,"INTE")){
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_inte));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
		tparam_inte* parametros = (tparam_inte *) procesarRespuestaMSP(respuesta);
		log_info(LOGCPU, "INTE(%d)",parametros->direccion);
		printf("INTE(%d)",parametros->direccion);

		//list_add(list_parametros,  (void*)&parametros->direccion);

 		INTE(parametros);
 		return  sizeof(tparam_inte);
		}

	}
	if(0 == strcmp(instruccion, "SHIF")){
		t_datosAEnviar* respuesta1 = MSP_SolicitarMemoria(PIDactual,punteroInstruccionActual + 4, 4, solicitarMemoria);
		int status = procesarRespuesta(respuesta1);
		if (status < 0) {
			return status;
		} else {
			int* numeroSHIF = malloc(respuesta1->tamanio);
			memcpy(numeroSHIF, respuesta1->datos, 4);
			t_datosAEnviar* respuesta2 = MSP_SolicitarMemoria(PIDactual,punteroInstruccionActual + 8, 1, solicitarMemoria);
			int status2 = procesarRespuesta(respuesta2);
			free(respuesta1);
			char* reg1SHIF = malloc(respuesta2->tamanio);
			if (status2 < 0) {
				return status2;
			} else {
				memcpy(reg1SHIF, respuesta2->datos, 1);
				free(respuesta2);
				tparam_shif* parametros = malloc(sizeof(tparam_shif));
				parametros->numero = *numeroSHIF;
				parametros->registro = reg1SHIF[0];
				log_info(LOGCPU, "SHIF(%d,%c)", parametros->numero,parametros->registro);
				printf("SHIF(%d,%c)", parametros->numero,parametros->registro);
				//list_add(list_parametros,  (void*)&parametros->numero);
				//list_add(list_parametros,  (void*)&parametros->registro);

				SHIF(parametros);
				return sizeof(tparam_shif);
			}
		}
/*
		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_shif));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
		tparam_shif* parametros = (tparam_shif *) procesarRespuestaMSP(respuesta);
		log_info(LOGCPU, "SHIF(%d,%c)",parametros->numero,parametros->registro);
 		SHIF(parametros);
 		return  sizeof(tparam_shif);*/

	}
	if(0 == strcmp(instruccion,"NOPP")){
		log_info(LOGCPU, "NOPP()");
		printf("NOPP()");
		NOPP();
		return 0; }
	if(0 == strcmp(instruccion,"PUSH")){
		t_datosAEnviar* respuesta1 = MSP_SolicitarMemoria(PIDactual,punteroInstruccionActual + 4, 4, solicitarMemoria);
		int status = procesarRespuesta(respuesta1);
		if (status < 0) {
			return status;
		} else {
			int* numeroPUSH = malloc(respuesta1->tamanio);
			memcpy(numeroPUSH, respuesta1->datos, 4);
			t_datosAEnviar* respuesta2 = MSP_SolicitarMemoria(PIDactual,punteroInstruccionActual + 8, 1, solicitarMemoria);
			int status2 = procesarRespuesta(respuesta2);
			free(respuesta1);
			char* reg1PUSH = malloc(respuesta2->tamanio);
			if (status2 < 0) {
				return status2;
			} else {
				memcpy(reg1PUSH, respuesta2->datos, 1);
				free(respuesta2);
				tparam_push* parametros = malloc(sizeof(tparam_push));
				parametros->numero = *numeroPUSH;
				parametros->registro = reg1PUSH[0];
				log_info(LOGCPU, "PUSH(%d,%c)", parametros->numero,	parametros->registro);
				printf("PUSH(%d,%c)", parametros->numero,	parametros->registro);

				//list_add(list_parametros,  (void*)&parametros->numero);
				//list_add(list_parametros,  (void*)&parametros->registro);

				PUSH(parametros);
		}

		return sizeof(tparam_push);

/*		t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_push));
				int status = procesarRespuesta(respuesta);
				if(status < 0){
					return status;
					}else{
				tparam_push* parametros = (tparam_push *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "PUSH(%d,%c)",parametros->numero,parametros->registro);
		 		PUSH(parametros);
		 		return  sizeof(tparam_push);*/
		}
	}
	if(0 == strcmp(instruccion,"TAKE")){
	/*	t_datosAEnviar* respuesta = MSP_SolicitarParametros(punteroInstruccionActual + 4, sizeof(tparam_take));
		int status = procesarRespuesta(respuesta);
		if(status < 0){
			return status;
			}else{
				printf("----------------------------------------------------------------RESPUESTA: %p", respuesta->datos);
				log_info(LOGCPU, "------------------------------------------------------RESPUESTA: %p", respuesta->datos);
				tparam_take* parametros = (tparam_take *) procesarRespuestaMSP(respuesta);
				log_info(LOGCPU, "TAKE(%d,%c)",parametros->numero, parametros->registro);
				TAKE(parametros);
				return sizeof(tparam_take);
			}*/
		t_datosAEnviar* respuesta1 = MSP_SolicitarMemoria(PIDactual,
				punteroInstruccionActual + 4, 4, solicitarMemoria);
		int status = procesarRespuesta(respuesta1);
		if (status < 0) {
			return status;
		} else {
			int* numeroTAKE = malloc(respuesta1->tamanio);
			memcpy(numeroTAKE, respuesta1->datos, 4);
			t_datosAEnviar* respuesta2 = MSP_SolicitarMemoria(PIDactual,
					punteroInstruccionActual + 8, 1, solicitarMemoria);
			int status2 = procesarRespuesta(respuesta2);
			free(respuesta1);
			char* reg1TAKE = malloc(respuesta2->tamanio);
			if (status2 < 0) {
				return status2;
			} else {
				memcpy(reg1TAKE, respuesta2->datos, 1);
				free(respuesta2);
				tparam_take* parametros = malloc(sizeof(tparam_take));
				parametros->numero = *numeroTAKE;
				parametros->registro = reg1TAKE[0];
				log_info(LOGCPU, "TAKE(%d,%c)", parametros->numero,	parametros->registro);
				printf("TAKE(%d,%c)", parametros->numero,	parametros->registro);

				//list_add(list_parametros,  (void*)&parametros->numero);
				//list_add(list_parametros,  (void*)&parametros->registro);

				TAKE(parametros);
			}

			return sizeof(tparam_take);
		}
	}
	if(0 == strcmp(instruccion,"XXXX")){
		log_info(LOGCPU,"XXXX()");
		printf("XXXX()");
		XXXX();
		return 0;}
	if(0 == strcmp(instruccion,"MALC") && KMactual == 1){
		log_info(LOGCPU,"MALC()");
		printf("MALC()");
		MALC();
		return 0; }else{
			if(0 == strcmp(instruccion,"MALC")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar MALC()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -12; }}
	if(0 == strcmp(instruccion,"FREE") && KMactual == 1){
		log_info(LOGCPU,"FREE()");
		printf("FREE()");
		FREE();
		return 0; }else{
			if(0 == strcmp(instruccion,"FREE")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar FREE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -12;}}
	if(0 == strcmp(instruccion,"INNN") && KMactual == 1){
		log_info(LOGCPU,"INNN()");
		printf("INNN()");
		INNN();
		return 0; }else{
			if(0 == strcmp(instruccion,"INNN")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar INNN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -12;}}
	if(0 == strcmp(instruccion,"INNC") && KMactual == 1){
			log_info(LOGCPU,"INNC()");
			printf("INNC()");
			INNC();
			return 0; }else{
				if(0 == strcmp(instruccion,"INNC")){
				log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar INNC()", PIDactual, KMactual);
				printf("no tiene permiso para ejecutar esta instruccion");
				return -12;}}
	if(0 == strcmp(instruccion,"OUTN") && KMactual == 1){
		log_info(LOGCPU,"OUTN()");
		printf("OUTN()");
		OUTN();
		return 0; }else{
			if(0 == strcmp(instruccion,"OUTN")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar OUTN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -12;}}
	if(0 == strcmp(instruccion,"OUTC") && KMactual == 1){
		log_info(LOGCPU,"OUTC()");
		printf("OUTC()");
		OUTC();
		return 0; }else{
			if(0 == strcmp(instruccion,"OUTC")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar OUTC()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -12;}}
	if(0 == strcmp(instruccion,"CREA\0")&& KMactual == 1){
		printf("\n---------------CREA()------------------\n");
		log_info(LOGCPU,"CREA()");
		printf("CREA()");
		CREA();
		return 0; }else{
			if(0 == strcmp(instruccion,"CREA")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar CREA()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -12;}}
	if(0 == strcmp(instruccion,"JOIN") && KMactual == 1){
		log_info(LOGCPU,"JOIN()");
		printf("JOIN()");
		JOIN();
		return 0; }else{
			if(0 == strcmp(instruccion,"JOIN")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar JOIN()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -12;}}
	if(0 == strcmp(instruccion,"BLOK") && KMactual == 1){
		log_info(LOGCPU,"BLOK()");
		printf("BLOK()");
		BLOK();
		return 0; }else{
			if(0 == strcmp(instruccion,"BLOK")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar BLOK()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -12;}}
	if(0 == strcmp(instruccion,"WAKE") && KMactual == 1){
		log_info(LOGCPU,"WAKE()");
		printf("WAKE()");
		WAKE();
		return 0; }else{
			if(0 == strcmp(instruccion,"WAKE")){
			log_error(LOGCPU, "PID: %d KM = %d, no tiene permiso para ejecutar WAKE()", PIDactual, KMactual);
			printf("no tiene permiso para ejecutar esta instruccion");
			return -12;}}
	return -12;

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
	case 'P':
		return &punteroInstruccionActual;
		break;
	case'X':
		return &baseStackActual;
		break;
	case 'S':
		return &cursorStackActual;
		break;
	}
	return 0;
}

void abortar(int codigoOperacion){
	actualizarTCB();
	devolverTCBactual(codigoOperacion);
	finalizarEjecucion = -1;
}



