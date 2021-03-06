#include "../CPU.h"


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

	void* bytesAEnviar = malloc(parametrosSetm->num);
	memcpy(bytesAEnviar, devolverRegistro(parametrosSetm->reg2),parametrosSetm->num);
	int status = 0;
	if (KMactual == 1){
		status = MSP_EscribirEnMemoria(PIDkm,*devolverRegistro(parametrosSetm->reg1), bytesAEnviar,	parametrosSetm->num);
	}else{
		status = MSP_EscribirEnMemoria(PIDactual,*devolverRegistro(parametrosSetm->reg1), bytesAEnviar,	parametrosSetm->num);
	}
	if (status < 0) {
		finalizarEjecucion = -1;

	}
}


void GETM(tparam_getm* parametrosGetm){ //Obtiene el valor de memoria apuntado por el segundo registro. El valor obtenido lo asigna en el primer registro.
	t_datosAEnviar* respuesta;

	if(KMactual == 1){
		respuesta = MSP_SolicitarMemoria(PIDkm, *(devolverRegistro(parametrosGetm->reg2)),1, solicitarMemoria);
	}else{
		respuesta = MSP_SolicitarMemoria(PIDactual, *(devolverRegistro(parametrosGetm->reg2)),1, solicitarMemoria);
	}

	int status= procesarRespuesta(respuesta);

	int* valorMemoria = malloc(1);
	/**valorMemoria = *((int*) respuesta->datos);*/
	memcpy(valorMemoria, respuesta->datos,1 );

	if(status < 0){
		finalizarEjecucion = -1;
	}else{
		memcpy((devolverRegistro(parametrosGetm->reg1)), valorMemoria, 1);
	}
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


	if( *devolverRegistro(parametrosDivr->reg2) == 0){
		perror("division por cero");
		//TODO: en el caso que haya error de division por cero.. devuelvo el TCB? y espero a que me mande otro? o sigo con este?
		//devuelvo tcb
		devolverTCBactual(desconexion);
	}
	else {
		A = (*devolverRegistro(parametrosDivr->reg1)) % (*devolverRegistro(parametrosDivr->reg2));
	}
}

void INCR(tparam_incr* parametrosIncr){ //incrementar una unidad al registro
	 *devolverRegistro(parametrosIncr->reg1) += 1;
}

void DECR(tparam_decr* parametrosDecr){ //decrementar una unidad al registro
	*devolverRegistro(parametrosDecr->reg1) -= 1;
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
		punteroInstruccionActual = (parametrosJmpz->direccion); //+ baseSegmentoCodigoActual;  //TODO esta bien que sea mas la base?????????????????
		aumentoPuntero = -1;
	}
}

void JPNZ(tparam_jpnz* parametrosJpnz){
	// Altera el flujo de ejecución sólo si el valor del registro A no es cero, para ejecutar la instrucción apuntada por la Dirección.
	// El valor es el desplazamiento desde el inicio del programa.

	if(A != 0){
		punteroInstruccionActual = (parametrosJpnz->direccion); //+ baseSegmentoCodigoActual;
		aumentoPuntero = -1;
	}
}

void INTE(tparam_inte* parametrosInte){
	//abortar(interrupcion);

	KERNEL_ejecutarRutinaKernel(interrupcion ,parametrosInte->direccion);
	aumentoPuntero = -1;
	finalizarEjecucion = -3;


	/*
	cuando el proceso CPU notifique al Kernel que un hilo desea ejecutar una llamada al
	sistema que requiere su atención (INTE), el Kernel recibirá el TCB de este hilo cargado y la dirección en
	memoria de la llamada a ejecutar y lo encolará en el estado BLOCK. Luego agregará una entrada en la
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

		int* bytesAEscribir = malloc(parametrosPush->numero);
		memcpy(bytesAEscribir, devolverRegistro(parametrosPush->registro), parametrosPush->numero);


		printf("bytesAEscribir %d \n", *bytesAEscribir);
		printf("tamanio bytesAEnviar \n %d", parametrosPush->numero);

		int respuestaMSP;
		if (KMactual == 1) {
			respuestaMSP = MSP_EscribirEnMemoria(PIDkm, cursorStackActual, bytesAEscribir, parametrosPush->numero);
		} else {
			respuestaMSP = MSP_EscribirEnMemoria(PIDactual, cursorStackActual, bytesAEscribir,parametrosPush->numero );
		}

		if (respuestaMSP < 0) {
			finalizarEjecucion = -1;
		} else {
			cursorStackActual = cursorStackActual + parametrosPush->numero;
		}
	free(bytesAEscribir);
}

void TAKE(tparam_take* parametrosTake){
	//Desapila los primeros bytes, indicado por el número, del stack hacia el registro. Modifica el valor del registro de stack de forma acorde.


	t_datosAEnviar* paquete = malloc(sizeof(t_datosAEnviar));
	cursorStackActual -= parametrosTake->numero;
	if (KMactual == 1) {
		paquete = MSP_SolicitarMemoria(PIDkm, cursorStackActual,parametrosTake->numero, solicitarMemoria);
	} else {
		paquete = MSP_SolicitarMemoria(PIDactual,  cursorStackActual,parametrosTake->numero, solicitarMemoria);
	}

	int status = procesarRespuesta(paquete);
	if (status < 0) {
		finalizarEjecucion = -1;
	} else {
		memcpy(devolverRegistro(parametrosTake->registro), paquete->datos, parametrosTake->numero);
		//cursorStackActual = cursorStackActual + parametrosTake->numero;
	}
	free(paquete);
}


/*



	int* bytesAEscribir = malloc(parametrosTake->numero);
	memcpy(bytesAEscribir, devolverRegistro(parametrosTake->registro), parametrosTake->numero);


	printf("bytesAEscribir %d \n", *bytesAEscribir);
	printf("tamanio bytesAEnviar \n %d", parametrosTake->numero);

	cursorStackActual -= parametrosTake->numero;

	int status2;
	if(KMactual == 1){
		status2 = MSP_EscribirEnMemoria(PIDkm,cursorStackActual,bytesAEscribir,parametrosTake->numero);
	}else{
		status2 = MSP_EscribirEnMemoria(PIDactual,cursorStackActual,bytesAEscribir,parametrosTake->numero);
	}

	if(status2 < 0){
		finalizarEjecucion = -1;
	}else{
		cursorStackActual = cursorStackActual + parametrosTake->numero;
	}*/

void XXXX(){
	//Finaliza la ejecucion
	finalizarEjecucion = -2;

}

/*Instrucciones Protegidas*/
/*    Solo si KM == 1     */


void MALC(){
	//Reserva una cantidad de memoria especificada por el registro A. La direccion de esta se
	//almacena en el registro A. Crea en la MSP un nuevo segmento del tamaño especificado asociado
	//al programa en ejecución.
	int respuesta = MSP_CrearNuevoSegmento(PIDactual, A);

	if(respuesta < 0){
		finalizarEjecucion = -1;
	}else{
		A = respuesta;
		void * datos = malloc(3 * sizeof(int));
		memcpy(datos, &PIDactual, sizeof(int));
		memcpy(datos + sizeof(int), &TIDactual, sizeof(int));
		memcpy(datos + (2*sizeof(int)), &respuesta, sizeof(int));
		t_datosAEnviar * paquete = crear_paquete(33, datos, 3 * sizeof(int));
		enviar_datos(socketKernel, paquete);
	}
}

void FREE(){
	//Libera la memoria apuntada por el registro A. Solo se podrá liberar memoria alocada por la
	//instrucción de MALC. Destruye en la MSP el segmento indicado en el registro A.
	//segmento que no sea ninguno de los que crea el LOADER

	if((A =! baseSegmentoCodigoActual) && (A =! baseStackActual)){
		int respuesta = MSP_DestruirSegmento(PIDactual,  A);

		if(respuesta < 0){
				finalizarEjecucion = -1;
		}else{
			void * datos = malloc(3 * sizeof(int));
			memcpy(datos, &PIDactual, sizeof(int));
			memcpy(datos + sizeof(int), &TIDactual, sizeof(int));
			memcpy(datos + (2*sizeof(int)), &respuesta, sizeof(int));
			t_datosAEnviar * paquete = crear_paquete(34, datos, 3 * sizeof(int));
			enviar_datos(socketKernel, paquete);

		}
	}
}

void INNN(){
	// Pide por consola del programa que se ingrese un número, con signo entre –2.147.483.648 y
	// 2.147.483.647. El mismo será almacenado en el registro A. Invoca al servicio correspondiente en
	// el proceso Kernel.
	log_info(LOGCPU, "\n PEDIRLE AL KERNEL QUE INGRESE UN NUMERO\n");
	A = KERNEL_IngreseNumeroPorConsola(PIDactual);
	log_info(LOGCPU, "\n KERNEL INGRESO NUMERO\n");
}


void INNC(){
	//Pide por consola del programa que se ingrese una cadena no más larga de lo indicado por el
	//registro B. La misma será almacenada en la posición de memoria apuntada por el registro A.
	//Invoca al servicio correspondiente en el proceso Kernel.
	log_info(LOGCPU, "\n PEDIRLE AL KERNEL QUE INGRESE CADENA\n");
	t_datosAEnviar* respuesta = KERNEL_IngreseCadenaPorConsola(PIDactual, B);
	char* cadena = malloc(respuesta->tamanio);
	memcpy(cadena,respuesta->datos,respuesta->tamanio);

	printf("\n\n\nCADENAAAAAAAAAAAAAAA %s \n\n\n\n", cadena);
	noPIDkm = -1;
	int status = MSP_EscribirEnMemoria(PIDactual,A,cadena,respuesta->tamanio);    //Se le manda el PID original, no el KM
	if(status < 0){
		finalizarEjecucion = -1;
	}
	free(respuesta);
	free(cadena);
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
	noPIDkm = -1;
	t_datosAEnviar* respuesta = MSP_SolicitarMemoria(PIDactual, A, B, solicitarMemoria);     //Se le manda el PID original, no el KM
	int status= procesarRespuesta(respuesta);
		if(status < 0){
			finalizarEjecucion = -1;
		}else{
			char* cadena = malloc(respuesta->tamanio + 1);
			//cadena[respuesta->tamanio] = '\0';
			memcpy(cadena, respuesta->datos, respuesta->tamanio);
			char fin = '\0';
			memcpy(cadena + respuesta->tamanio, &fin, 1);
			log_info(LOGCPU, "cadena a mostrar: %s", cadena);
			KERNEL_MostrarCadenaPorConsola(PIDactual, cadena);
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
	que la diferencia entre cursor y base se mantenga igual) y luego invocar al servicio
	correspondiente en el proceso Kernel con el TCB recién generado.*/

	KERNEL_CrearHilo(TCBactual, B);
}

void JOIN(){
	//Bloquea el programa que ejecutó la llamada al sistema hasta que el hilo con el identificador
	//almacenado en el registro A haya finalizado. Invoca al servicio correspondiente en el proceso Kernel.
	KERNEL_JoinTCB(TCBactual, A);
}

void BLOK(){
	//Bloquea el programa que ejecutó la llamada al sistema hasta que el recurso apuntado por B se libere.
	//La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema WAIT pre-compilada.
	KERNEL_BloquearTCB(TCBactual, B);
}

void WAKE(){
	//Desbloquea al primer programa bloqueado por el recurso apuntado por B.
	//La evaluación y decisión de si el recurso está libre o no es hecha por la llamada al sistema SIGNAL pre-compilada.
	KERNEL_WakePrograma(B);
}
