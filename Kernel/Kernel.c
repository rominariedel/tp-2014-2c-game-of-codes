/*
 * main.c
 *
 *  Created on: 09/09/2014
 *      Author: utnso
 */

#include "auxiliares/auxiliares.h"

#define pid_KM_boot 0

/*VARIABLES GLOBALES*/

enum bit_de_estado{
	libre,
	ocupado,
};
enum mensajes{

	//Mensajes enviados

	reservar_segmento = 1,
	escribir_en_memoria = 2,
	ejecucion_abortada = 3,
	imprimir_en_pantalla = 4,
	ingresar_cadena = 5,
	ejecutar = 6,
	//Mensajes recibidos

	finaliza_quantum = 10,
	finaliza_ejecucion = 11,
	ejecucion_erronea = 12,
	desconexion = 13,
	interrupcion = 14,
	creacion_hilo = 15,
	error_memoriaLlena = 16,
	error_segmentationFault = 17,
	soy_consola = 18,
	soy_CPU = 19,
	entrada_estandar = 20,
	salida_estandar = 21,
	join = 22,
	bloquear = 23,
	despertar = 24,
};


/*FUNCIONES*/

void boot();
void obtenerDatosConfig(char**);
void agregar_hilo(t_queue* , TCB_struct);
int solicitar_segmento(int mensaje[2]);
int tamanio_syscalls(void*);
void escribir_memoria(int, int, int, void*);
void ejecutarSysCall(int dirSyscall);
int es_CPU(int socket);
void finalizo_quantum(TCB_struct*);
void sacar_de_ejecucion(TCB_struct *);
void finalizo_ejecucion(TCB_struct*);
void abortar(TCB_struct*);
void interrumpir(TCB_struct*, int);
void crear_hilo(TCB_struct);
void lodear(int PID);

int main(int argc, char ** argv){

	crear_colas();
	obtenerDatosConfig(argv);
	TID = 0;
	PID = 0;
	boot();



	return 0;
}

void obtenerDatosConfig(char ** argv){
	t_config * configuracion = config_create(argv[1]);
	PUERTO = config_get_string_value(configuracion, "PUERTO");
	IP_MSP = config_get_string_value(configuracion, "IP_MSP");
	PUERTO_MSP = config_get_string_value(configuracion, "PUERTO_MSP");
	QUANTUM = config_get_int_value(configuracion, "QUANTUM");
	SYSCALLS = config_get_string_value(configuracion, "SYSCALLS");
	TAMANIO_STACK = config_get_int_value(configuracion, "TAMANIO_STACK");
}

void boot(){

	lodear(3);
	void * syscalls = extraer_syscalls();
	socket_MSP = crear_cliente(IP_MSP, PUERTO_MSP);
	int mensaje_codigo[2];
	mensaje_codigo[0] = pid_KM_boot;
	mensaje_codigo[1] = tamanio_syscalls(syscalls);

	int base_segmento_codigo = solicitar_segmento(mensaje_codigo);
	//TODO: validar la base del segmento
	escribir_memoria(pid_KM_boot, base_segmento_codigo, mensaje_codigo[1], syscalls);
	free(syscalls);

	int mensaje_stack[2];
	mensaje_stack[0] = pid_KM_boot;
	mensaje_stack[1] = TAMANIO_STACK;
	int base_segmento_stack = solicitar_segmento(mensaje_stack);
	//TODO: validar la base del stack
	tcb_km.KM = 1;
	tcb_km.M = base_segmento_codigo;
	tcb_km.tamanioSegmentoCodigo = mensaje_codigo[1];
	tcb_km.P = 0;
	tcb_km.PID = pid_KM_boot;
	tcb_km.S = base_segmento_stack;
	tcb_km.TID = 0;
	tcb_km.X = base_segmento_stack;
	//TODO: inicializar los registros de programacion

	queue_push(BLOCK.prioridad_0, (void *) &tcb_km);

	int socket_gral = crear_servidor(PUERTO,backlog);
	//TODO: validar socket

	FD_ZERO(&clientes_set);
	FD_SET(socket_gral, &clientes_set);

	fd_set copia_set;
	int descriptor_mas_alto = socket_gral;

	while(1){
		copia_set = clientes_set;

		int i = select(descriptor_mas_alto + 1, &copia_set, NULL, NULL, NULL);
		if (i == -1) {
					//ERROR
					break;
		}

		int n_descriptor = 0;

		while (n_descriptor <= descriptor_mas_alto) {

			if (FD_ISSET(n_descriptor,&copia_set)) {

				if (n_descriptor == socket_gral) {
					int socket_conectado = recibir_conexion(socket_gral);
					FD_SET(socket_conectado, &clientes_set);
					int modulo_conectado = -1;
					t_datosAEnviar * datos = recibir_datos(socket_conectado);
					modulo_conectado = datos->codigo_operacion;

					if(modulo_conectado == soy_consola){
						struct_consola * consola_conectada = malloc(sizeof(struct_consola));
						int pid = obtener_PID();
						consola_conectada->PID = pid;
						consola_conectada->socket_consola = socket_conectado;
						list_add(consola_list, consola_conectada);
						//loader(consola_conectada); El PID ya esta asignado y es el que tiene que usar el
						//loader para crear el TCB


					}else if(modulo_conectado == soy_CPU){
						struct_CPU* cpu_conectada = malloc(sizeof(struct_CPU));
						cpu_conectada->PID = -1;
						cpu_conectada->bit_estado = libre;
						cpu_conectada->socket_CPU = socket_conectado;
						list_add(CPU_list, cpu_conectada);
					}
					free(datos->datos);
					free(datos);


				}
				else{
					t_datosAEnviar * datos;
					datos = recibir_datos(n_descriptor);
					int codigo_operacion = datos->codigo_operacion;
					if(es_CPU(n_descriptor)){
						TCB_struct* tcb;
						int * dirSysCall;
						switch(codigo_operacion){

						case finaliza_quantum:
							finalizo_quantum((TCB_struct*)datos->datos);
							break;
						case finaliza_ejecucion:
							finalizo_ejecucion((TCB_struct*)datos->datos);
							break;
						case ejecucion_erronea:
							abortar((TCB_struct*) datos->datos);
							break;
						case desconexion:
							abortar((TCB_struct*) datos->datos);
							break;
						case interrupcion:
							tcb = malloc(sizeof(TCB_struct));
							dirSysCall = malloc(sizeof(int));
							memcpy(tcb, datos->datos, sizeof(TCB_struct));
							memcpy(dirSysCall, datos->datos + sizeof(TCB_struct),sizeof(int));
							interrumpir(tcb, *dirSysCall);
							break;
						case creacion_hilo:
							tcb = malloc(sizeof(TCB_struct));
							memcpy(tcb, datos->datos, sizeof(TCB_struct));
							crear_hilo(*tcb);
							break;
						case entrada_estandar:
							//pedir_entrada(datos->datos);
							break;
						case salida_estandar:

							break;
						case join:

							break;
						case bloquear:

							break;
						case despertar:

							break;

						}
					}else{
						switch(codigo_operacion){
						case desconexion:
							break;
						}
					}
					free(datos->datos);
					free(datos);

				}


			}
			n_descriptor++;
		}


	}
}

void interrumpir(TCB_struct * tcb, int dirSyscall){
	list_add(BLOCK.prioridad_1, tcb);
	agregar_hilo(SYS_CALL, *tcb);
	//Esperar a que haya alguna cpu libre
	ejecutarSysCall(dirSyscall);
}

/* la SYS_CALL se ejecuta siempre que hay una CPU disponible y haya algun elemento en la cola de
 * SYS_CALL*/

void ejecutarSysCall(int dirSyscall){

	TCB_struct * tcb_user = queue_peek(SYS_CALL);
	tcb_km.registrosProgramacion = tcb_user->registrosProgramacion;
	tcb_km.PID = tcb_user->PID;

	tcb_km.P = dirSyscall;

	//TODO: se envia a ejecutar el tcb a alguna cpu libre
	agregar_hilo(BLOCK.prioridad_0, tcb_km);

	tcb_user->registrosProgramacion = tcb_km.registrosProgramacion;
	//queue_pop(BLOCK.prioridad_1);
	//TODO: re-planificar el tcb_user
}

void crear_hilo(TCB_struct tcb){

	TCB_struct nuevoTCB;
	int mensaje[2];
	mensaje[0] = tcb.PID;
	mensaje[1] = TAMANIO_STACK;

	int base_stack = solicitar_segmento(mensaje);


	nuevoTCB.PID = tcb.PID;
	int tid = obtener_TID(tcb.PID);
	nuevoTCB.X = base_stack;
	nuevoTCB.S = base_stack;
	nuevoTCB.KM = 0;
	nuevoTCB.PID = tcb.PID;
	nuevoTCB.TID = tid;
	nuevoTCB.M = tcb.M;
	nuevoTCB.tamanioSegmentoCodigo = tcb.tamanioSegmentoCodigo;
	nuevoTCB.P = tcb.P;
	reg_programacion nuevosRegistros = tcb.registrosProgramacion;
	nuevoTCB.registrosProgramacion = nuevosRegistros;
	agregar_hilo(READY.prioridad_1, nuevoTCB);
}

void agregar_hilo(t_queue * COLA, TCB_struct tcb){

	queue_push(COLA, (void*)&tcb);

}

void planificador(){
	//recibe conexiones de diferentes CPUs
}

/*Esta operacion le solicita a la MSP un segmento, retorna la direccion base del
 * segmento reservado*/
int solicitar_segmento(int mensaje[2]){

	char * datos = malloc(2 * sizeof(int));
	memcpy(datos, &mensaje[0], sizeof(int));
	memcpy(datos + sizeof(int), &mensaje[1], sizeof(int));

	t_datosAEnviar * paquete = crear_paquete(reservar_segmento, (void*) datos, 2*sizeof(int));

	enviar_datos(socket_MSP, paquete);
	free(datos);
	t_datosAEnviar * respuesta = recibir_datos(socket_MSP);

	int dir_base =(int) malloc(sizeof(int));
	memcpy(&dir_base, respuesta->datos, sizeof(int));
	//TODO: validar si no hay violacion de segmento

	return dir_base;
}


void escribir_memoria(int pid, int dir_logica, int tamanio, void * bytes){

	char * datos = malloc((3 * sizeof(int)) + tamanio);

	memcpy(datos, &pid, sizeof(int));
	memcpy(datos + sizeof(int), &dir_logica, sizeof(int));
	memcpy(datos + (2*sizeof(int)), &tamanio, sizeof(int));
	memcpy(datos + (3*sizeof(int)), bytes, tamanio);

	t_datosAEnviar * paquete = crear_paquete(escribir_en_memoria, datos, (3*sizeof(int)) + sizeof(void*));
	enviar_datos(socket_MSP, paquete);
	free(datos);

	t_datosAEnviar * respuesta = recibir_datos(socket_MSP);

	int rta =(int) malloc(sizeof(int));
	memcpy(&rta, respuesta->datos, sizeof(int));


	//TODO:validar si hay segmentation fault

}



int es_CPU(int socket){

	bool tiene_mismo_socket(struct_CPU estructura){
		return estructura.socket_CPU == socket;
	}

	return list_any_satisfy(CPU_list, (void*)tiene_mismo_socket);
}


void finalizo_quantum(TCB_struct* tcb){
	sacar_de_ejecucion(tcb);
	agregar_hilo(READY.prioridad_1, *tcb);

}

void sacar_de_ejecucion(TCB_struct* tcb){
	int PID = tcb->PID;
	int TID = tcb->TID;
	bool es_TCB(TCB_struct tcb_comparar){
		return (tcb_comparar.PID == PID) && (tcb_comparar.TID == TID);
	}
	TCB_struct * tcb_exec = list_remove_by_condition(EXEC, (void*)es_TCB);
	free(tcb_exec);


}

void finalizo_ejecucion(TCB_struct *tcb){
	sacar_de_ejecucion(tcb);
	agregar_hilo(EXIT, *tcb);
}

void abortar(TCB_struct* tcb){
	sacar_de_ejecucion(tcb);
	agregar_hilo(EXIT, *tcb);
	//LOGUEAR que tuvo que abortar el hilo
}

void enviar_a_ejecucion(TCB_struct * tcb){
	struct_CPU* cpu = list_find(CPU_list, (void*) CPU_esta_libre);
	int op = ejecutar;
	void * mensaje = malloc(sizeof(TCB_struct) + sizeof(int));
	memcpy(mensaje, &op, sizeof(int));
	memcpy(mensaje + sizeof(int), tcb, sizeof(TCB_struct));
	enviar_datos(cpu->socket_CPU, mensaje);
}

void lodear(int PID){
	TCB_struct * tcb = malloc(sizeof(TCB_struct));
	tcb->KM = 0;
	tcb->M = 0;
	tcb->P = 0;
	tcb->PID = PID;
	tcb->S = 0;
	tcb->TID = obtener_TID();
	tcb->X = 0;

	queue_push(NEW, tcb);
	queue_pop(NEW);
	queue_push(EXIT, tcb);
	free(tcb);
}
