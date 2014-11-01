/*
 * main.c
 *
 *  Created on: 08/09/2014
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <math.h>
#include "MSP.h"
#include <commons/error.h>
#include <commons/log.h>
#include <sockets.h>
#include <sys/select.h>

int tamanioMemoria;
int memoriaDisponible;
char* puerto;
int cantidadSwap;
char* sust_pags;
char* rutaLog;
char* MSP_CONFIG;
int tamanioPag = 256;
int socket_general;
int backlog;

int descriptor_mas_alto_cpu;
fd_set CPU_set;

t_list* procesos;
t_list* marcosVacios;
t_list* marcosLlenos;
t_list* cpu_list;

pthread_t hiloConsola;
t_log* logger;

int main(void) {
	inicializar();

	int hilo = pthread_create(&hiloConsola, NULL, (void*) inicializarConsola,
			NULL );
	if (hilo == 0) {
		log_info(logger,
				"La Consola de MSP se inicializó correctamente \n El tamaño de la memoria principal es: %d \n El tamaño del archivo de paginación: %d",
				tamanioMemoria, tamanioPag);
	} else
		log_error(logger,
				"Ha ocurrido un error en la  inicialización de la Consola de MSP");

	pthread_join(hiloConsola, NULL );

	return 1;

	/*t_config* configuracion = config_create(args[1]);
	 logi = log_create(args[2], "UMV", 0, LOG_LEVEL_TRACE);
	 int sizeMem = config_get_int_value(configuracion, "sizeMemoria");

	 memPpal = malloc (sizeMem);	//El tamaño va por configuracion
	 char* aux = (char*) memPpal;
	 finMemPpal = memPpal + sizeMem; //El tamaño va por configuracion
	 for(i=0; i<sizeMem; i++){aux[i] = 0;}

	 rhConsola = pthread_create(&consola, NULL, mainConsola, NULL);
	 rhEsperarConexiones = pthread_create(&esperarConexiones, NULL, mainEsperarConexiones, NULL);

	 pthread_join(consola, NULL);
	 pthread_join(esperarConexiones, NULL);

	 printf("%d",rhConsola);
	 printf("%d",rhEsperarConexiones);

	 exit(0);*/
}

void inicializar() {
	cargarArchivoConfiguracion();
	logger = log_create(rutaLog, "Log Programa", true, LOG_LEVEL_TRACE);
	crearMarcos();

	procesos = list_create();

	iniciarConexiones();
}

void inicializarConsola() {
	char* comando = malloc(50);
	int seguimiento = 1;

	while (seguimiento) {
		printf(">");
		scanf("%s", comando);

		interpretarComando(comando);
		printf("\r\n");
	}

}

void interpretarComando(char* comando) {
	char** palabras = string_n_split(comando, 2, " ");
	char** parametros = NULL;

	if (palabras[1] != NULL ) {
		parametros = string_split(palabras[1], " ");
	}

	if (string_equals_ignore_case(palabras[0], "Crear_Segmento")) {
		printf("Creando segmento...");
		crearSegmento(atoi(parametros[0]), atoi(parametros[1]));
	}

	else if (string_equals_ignore_case(palabras[0], "Destruir_Segmento")) {
		printf("Destruyendo segmento...");
		destruirSegmento(atoi(parametros[0]), (uint32_t) atoi(parametros[1]));
	}

	else if (string_equals_ignore_case(palabras[0], "Escribir_Memoria")) {
		printf("Iniciando proceso de escritura de memoria...");
		escribirMemoria(atoi(parametros[0]), (uint32_t) atoi(parametros[1]),
				parametros[2], atoi(parametros[3]));
	}

	else if (string_equals_ignore_case(palabras[0], "Leer_Memoria")) {
		printf("Solicitando memoria...");
		solicitarMemoria(atoi(parametros[0]), (uint32_t) atoi(parametros[1]),
				atoi(parametros[2]));
	}

	else if (string_equals_ignore_case(palabras[0], "Tabla_De_Paginas")) {
		tablaPaginas(atoi(parametros[0]));
	}

	else if (string_equals_ignore_case(palabras[0], "Tabla_De_Segmentos")) {
		tablaSegmentos();
	}

	else if (string_equals_ignore_case(palabras[0], "Listar_Marcos")) {
		tablaMarcos();
	}
}

void cargarArchivoConfiguracion(void) { //todo: args no harcodearlo
	t_config* configuracion = config_create("config.txt");

	if (config_has_property(configuracion, "CANTIDAD_MEMORIA")) {
		tamanioMemoria = config_get_int_value(configuracion, "CANTIDAD_MEMORIA")
				* pow(2, 10);
		memoriaDisponible = tamanioMemoria;
		printf("Tamanio Memoria =  %d \n", tamanioMemoria);
	}

	if (config_has_property(configuracion, "PUERTO")) {
		puerto = config_get_string_value(configuracion, "PUERTO");
		printf("Puerto =  %s \n", puerto);
	}

	if (config_has_property(configuracion, "CANTIDAD_SWAP")) {
		cantidadSwap = config_get_int_value(configuracion, "CANTIDAD_SWAP");
		printf("Cantidad Swap =  %d \n", cantidadSwap);
	}

	if (config_has_property(configuracion, "SUST_PAGS")) {
		sust_pags = config_get_string_value(configuracion, "SUST_PAGS");
		printf("Algoritmo de Sustitución de Páginas =  %s \n", sust_pags);
	}

	if (config_has_property(configuracion, "RUTA_LOG")) {
		rutaLog = config_get_string_value(configuracion, "RUTA_LOG");
		printf("Ruta del archivo logger =  %s \n", rutaLog);
	}

	if (config_has_property(configuracion, "BACKLOG")) {
		backlog = config_get_int_value(configuracion, "BACKLOG");
		printf("Backlog =  %d \n", backlog);
	}
}

void crearMarcos() {
	marcosLlenos = list_create();
	marcosVacios = list_create();

	int cantidadMarcos = tamanioMemoria / tamanioPag;
	int i;

	for (i = 0; i < cantidadMarcos; i++) {

		//creo un marco vacio
		T_MARCO * marcoVacio = malloc(sizeof(T_MARCO));
		marcoVacio->marcoID = i;
		marcoVacio->empty = true;

		//agrego el marcoVacio a la lista de marcosVacios
		list_add(marcosVacios, marcoVacio);
	}
}

//Crea un nuevo segmento para el programa PID del tamaño tamanio. Devuelve la direccion
//virtual base del segmento.
uint32_t crearSegmento(int PID, int tamanio) {

	if (tamanio > pow(2, 20)) {
		log_error(logger, "El tamanio supera al mega de capacidad");
		return -1;
	}

	if (memoriaDisponible < tamanio) {
		log_error(logger,
				"La memoria disponible no es suficiente para el tamanio del segmento");
		return -1;
	}

	bool procesoPorPid(T_PROCESO* proceso) {
		return proceso->PID == PID;
	}

	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso == NULL ) {

		T_PROCESO* proceso = malloc(sizeof(T_PROCESO));
		proceso->PID = PID;
		proceso->segmentos = list_create();

		list_add(procesos, proceso);
	}

	//creo un segmento vacio para luego añadirlo a la lista de segmentos del proceso
	T_SEGMENTO* segmentoVacio = malloc(sizeof(T_SEGMENTO));

	segmentoVacio->SID = calcularProximoSID(proceso);
	segmentoVacio->paginas = crearPaginasPorTamanioSegmento(tamanio);

	T_DIRECCION_LOG direccionLogica;
	direccionLogica.SID = segmentoVacio->SID;
	direccionLogica.paginaId = 0;
	direccionLogica.desplazamiento = 0;

	(segmentoVacio->baseSegmento) = DireccionLogicaToUint32(direccionLogica);

	list_add(proceso->segmentos, (void*) segmentoVacio);

	log_info(logger, "La dirección base del segmento creado es %d",
			segmentoVacio->baseSegmento);

	return segmentoVacio->baseSegmento;
}

int calcularProximoSID(T_PROCESO* proceso) {
	if (list_size(proceso->segmentos) == 0) {
		return 0;
	}
	T_SEGMENTO* ultimoSegmento = list_get(proceso->segmentos,
			list_size(proceso->segmentos) - 1);
	return ((ultimoSegmento->SID) + 1);
}

t_list* crearPaginasPorTamanioSegmento(int tamanio) {
	//instancio la lista de paginas
	t_list* paginas = list_create();

	//calculo la cantidad de páginas que va a tener el segmento
	//necesario que la cantidadPaginas redondee para arriba - me fijo por el resto, si es distinto de 0 le sumo uno
	div_t division = div(tamanio, tamanioPag);
	int cantidadPaginas = division.quot;

	if (division.rem != 0) {
		cantidadPaginas =+ 1;
	}

	int i;
	for (i = 0; i < cantidadPaginas; i++) {
		//creo una pagina vacia
		T_PAGINA * paginaVacia = malloc(sizeof(T_PAGINA));
		paginaVacia->paginaID = i;
		paginaVacia->swapped = 0;
		paginaVacia->marcoID = 0;

		memset(paginaVacia->data, tamanioPag - 1, ' ');

		//agrego la pagina a la lista de paginas
		list_add(paginas, paginaVacia);
	}

	return paginas;
}

//Destruye el segmento identificado por baseSegmento del programa PID y libera la
//memoria que ocupaba ese segmento.
uint32_t destruirSegmento(int PID, uint32_t baseSegmento) {

	bool procesoPorPid(T_PROCESO* proceso) {
		return proceso->PID == PID;
	}

	bool segmentoPorBase(T_SEGMENTO* segmento) {
		return segmento->baseSegmento == baseSegmento;
	}

	//busco en la lista de procesos el proceso con ese PID
	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso != NULL ) {

		//busco en la lista de segmentos del proceso el segmento con esa base
		T_SEGMENTO * seg = list_find(proceso->segmentos,
				(void*) segmentoPorBase);

		if (seg != NULL ) {

			//elimino las paginas del segmento
			list_clean_and_destroy_elements(seg->paginas, (void*) destruirPag);

			//elimino de la lista de segmentos del proceso, el segmento
			list_remove_by_condition(proceso->segmentos,
					(void*) segmentoPorBase);

			memoriaDisponible = memoriaDisponible + sizeof(seg->tamanio);
			free(seg);
		}

		else {
			log_error(logger,
					"El segmento no ha sido destruido porque es inexistente");
			return -1;
		}
	} else {
		log_error(logger,
				"El segmento no ha sido destruido porque el proceso con PID: %d no existe",
				PID);
		return -1;
	}

	log_info(logger, "El segmento ha sido detruido exitosamente");
	return 1;
}

static void destruirPag(T_PAGINA* pagina) {
	free(pagina);
}

//Para el espacio de direcciones del proceso PID, devuelve hasta tamanio bytes
//comenzando desde direccion.
char* solicitarMemoria(int PID, uint32_t direccion, int tamanio) {

	//TODO if (direccionVirtual){
	//log_error(logger,"Segmentation Fault: La memoria especificada es inválida");
	//return -1;
	//}

	T_DIRECCION_LOG direccionLogica = uint32ToDireccionLogica(direccion);
	char* memoriaSolicitada = malloc(tamanio);

	bool procesoPorPid(T_PROCESO* proceso) {
		return proceso->PID == PID;
	}
	bool segmentoPorSid(T_SEGMENTO* segmento) {
		return segmento->SID == direccionLogica.SID;
	}
	bool paginaPorPagid(T_PAGINA* pagina) {
		return pagina->paginaID == direccionLogica.paginaId;
	}
	bool marcoPorVacio(T_MARCO* marco) {
		return marco->empty == true;
	}

	if ((direccionLogica.desplazamiento + tamanio) > tamanioPag) {
		log_error(logger,
				"Segmentation Fault: Se excedieron los limites del segmento");
		return (char*) -1;
	}

	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso != NULL ) {
		T_SEGMENTO* seg = list_find(proceso->segmentos, (void*) segmentoPorSid);

		if (seg != NULL ) {
			T_PAGINA* pag = list_find(seg->paginas, (void*) paginaPorPagid);

			if (pag != NULL ) {

				if (pag->marcoID == 0) {

					if (list_is_empty(marcosVacios)) {
						//todo swapping - ejecutar algoritmo de sustitucion de pags
					} else {
						T_MARCO* marcoAsignado = list_remove(marcosVacios, 0); //todo revisar
						asignoMarcoAPagina(PID, marcoAsignado, pag);
					}
				}

				int inicio = direccionLogica.desplazamiento;
				int final = direccionLogica.desplazamiento + tamanio;
				int i;
				for (i = inicio; final > i; i++) {
					string_append((char**) &memoriaSolicitada, &(pag->data[i]));
				}
			} else {
				log_error(logger,
						"No se ha podido solicitar memoria ya que la página es inexistente");
				return (char*) -1;
			}
		} else {
			log_error(logger,
					"No se ha podido solicitar memoria ya que el segmento es inexistente");
			return (char*) -1;
		}
	} else {
		log_error(logger,
				"No se ha podido solicitar memoria ya que el proceso de PID: %d es inexistente",
				PID);
		return (char*) -1;
	}

	log_info(logger, "EL contenido de la página solicitada es: %s",
			memoriaSolicitada);
	return (char*) 1; //Este return es provisorio, capaz memoriaSolicitada haya que usarlo como variable global
	//return memoriaSolicitada; //Esto debería devolver un int, qué se hace con memoriaSolicitada?
}

//Para el espacio de direcciones del proceso PID, escribe hasta tamanio bytes del buffer bytesAEscribir
//comenzando en la direccion.
uint32_t escribirMemoria(int PID, uint32_t direccion, char* bytesAEscribir,
		int tamanio) {

	//TODO if (direccionVirtual){
	//log_error(logger,"Segmentation Fault: La memoria especificada es inválida");
	//return -1;
	//}

	T_DIRECCION_LOG direccionLogica = uint32ToDireccionLogica(direccion);

	bool procesoPorPid(T_PROCESO* proceso) {
		return proceso->PID == PID;
	}
	bool segmentoPorSid(T_SEGMENTO* segmento) {
		return segmento->SID == direccionLogica.SID;
	}
	bool paginaPorPagid(T_PAGINA* pagina) {
		return pagina->paginaID == direccionLogica.paginaId;
	}
	bool marcoPorVacio(T_MARCO* marco) {
		return marco->empty == true;
	}

	if (((direccionLogica.desplazamiento + tamanio) > tamanioPag)
			|| (string_length(bytesAEscribir) > tamanio)) {
		log_error(logger,
				"Segmentation Fault: Se excedieron los límites del segmento");
		return -1;
	}

	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso != NULL ) {
		T_SEGMENTO* seg = list_find(proceso->segmentos, (void*) segmentoPorSid);

		if (seg != NULL ) {
			T_PAGINA* pag = list_find(seg->paginas, (void*) paginaPorPagid);

			if (pag != NULL ) {

				if (pag->marcoID == 0) {

					if (list_is_empty(marcosVacios)) {
						//todo swapping - ejecutar algoritmo de sustitucion de pags
					} else {
						T_MARCO* marcoAsignado = list_remove(marcosVacios, 0); //todo= list_any_satisfy(marcosVacios, (void*) marcoPorVacio); //list_any_satisfy devuelve un BOOL
						asignoMarcoAPagina(PID, marcoAsignado, pag);
					}
				}

				int inicio = direccionLogica.desplazamiento;
				int final = direccionLogica.desplazamiento + tamanio;
				int i;
				for (i = inicio; final > i; i++) {
					pag->data[i] = *string_substring_until(bytesAEscribir, i);
				}
			} else {
				log_error(logger,
						"No se ha podido escribir en memoria porque la página es inexistente");
				return -1;
			}
		} else {
			log_error(logger,
					"No se ha podido escribir en memoria porque el segmento es inexistente");
			return -1;
		}
	} else {
		log_error(logger,
				"No se ha podido escribir en memoria porque el proceso de PID: %d es inexistente",
				PID);
		return -1;
	}

	log_info(logger, "Se ha escrito en memoria exitósamente");
	return 1;
}

void asignoMarcoAPagina(int PID, T_MARCO* marcoAsignado, T_PAGINA* pag) { //todo actualizarlo
	//Primero me fijo si la pagina no tiene un marco asignado
	//me fijo si tengo marcos libres y acá haría esto:
	pag->marcoID = marcoAsignado->marcoID;
	marcoAsignado->pagina = pag;
	marcoAsignado->PID = PID;
	marcoAsignado->empty = false;
	actualizarMarcos();

	//si no tiene marcos libres, entonces:
	//me fijo su tiene memoriaDisponibleParaSwapping
	//si la swappeo entonces borro el archivo segun su FilePath
	//Bajo a disco la pagina que contiene
	//Indico que ya no contiene ningun marco y que está swappeada
	//Bajo a disco el archivo y disminuyo la cantidad de memoriaDisponibleParaSwapping -- swapOut
	//si no tengo memoriaDisponibleParaSwapping
	//tengo que volver a sumar la capacidad del archivo que le hice swapOut
}

void actualizarMarcos() {
	t_list* marcos = list_create();
	list_add_all(marcos, marcosLlenos);
	list_add_all(marcos, marcosVacios);

	bool marcoPorVacio(T_MARCO* marco) {
		return marco->empty == true;
	}
	bool marcoPorLleno(T_MARCO* marco) {
		return marco->empty == false;
	}

	list_clean(marcosVacios);
	list_add_all(marcosVacios, list_filter(marcos, (void*) marcoPorVacio));

	list_clean(marcosLlenos);
	list_add_all(marcosLlenos, list_filter(marcos, (void*) marcoPorLleno));
}

int tablaMarcos() {
	printf("%s TABLA DE MARCOS %s", string_repeat('-', 40),
			string_repeat('-', 40));

	bool ordenarPorMenorId(T_MARCO* marco1, T_MARCO* marco2) {
		return (marco1->marcoID < marco2->marcoID);
	}

	t_list* marcos = list_create();

	list_add_all(marcos, marcosLlenos);
	list_add_all(marcos, marcosVacios);

	list_sort(marcos, (void*) ordenarPorMenorId);

	int i;
	int cantidadMarcos = list_size(marcos);
	for (i = 0; cantidadMarcos > i; i++) {
		T_MARCO* marco = list_get(marcos, i);
		printf("Número de marco: %d", marco->marcoID);
		if (marco->empty) {
			printf("Marco disponible");
		} else
			printf("Marco ocupado por el proceso: %d", marco->PID);
		printf("Información de los algoritmos de sustitución de páginas: %s",
				marco->alg_meta_data);
	}
	printf("%s\n", string_repeat('-', 100));
	return 1;
}

int tablaSegmentos() {
	printf("%s TABLA DE SEGMENTOS %s", string_repeat('-', 40),
			string_repeat('-', 40));

	int cantidadProcesos = list_size(procesos);
	int i;
	int j;

	for (i = 0; cantidadProcesos > i; i++) {
		T_PROCESO* proceso = list_get(procesos, i);
		int cantidadSegmentos = list_size(proceso->segmentos);

		printf("Para el proceso de ID: %d", proceso->PID);

		if (cantidadSegmentos == 0) {
			printf("%c", '\n');
			log_info(logger, "El proceso no tiene segmentos");
			return 1;
		}

		for (j = 0; cantidadSegmentos > j; j++) {
			T_SEGMENTO* segmento = list_get(proceso->segmentos, j);
			printf("%c", '\n');
			printf("Número de segmento: %d", (int) segmento->SID);
			printf("Tamaño: %d", segmento->tamanio);
			printf("Dirección virtual base: %d", (int) segmento->baseSegmento);
		}
	}
	printf("%s\n", string_repeat('-', 100));
	log_info(logger, "Tabla de segmentos mostrada satisfactoriamente");
	return 1;
}

int tablaPaginas(int PID) {
	printf("%s TABLA DE PÁGINAS %s", string_repeat('-', 40),
			string_repeat('-', 40));

	bool procesoPorPid(T_PROCESO* proceso) {
		return proceso->PID == PID;
	}

	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso != NULL ) {
		int cantidadSegmentos = list_size(proceso->segmentos);
		int i;
		int j;

		for (i = 0; cantidadSegmentos > i; i++) {
			T_SEGMENTO* segmento = list_get(proceso->segmentos, i);
			printf("Para el segmento de ID: %d", segmento->SID);

			int cantidadPaginas = list_size(segmento->paginas);
			for (j = 0; cantidadPaginas > j; j++) {
				T_PAGINA* pagina = list_get(segmento->paginas, j);
				printf("%c", '\n');
				if (pagina->swapped) {
					printf("La página se encuentra swappeada");
				} else if (&(pagina->marcoID) != NULL ) {
					log_info(logger,
							"La página se encuentra en memoria principal en el marco de ID: %d",
							pagina->marcoID);
				} else
					log_info(logger,
							"La página no se encuentra en memoria principal");
			}
		}
	} else {
		log_error(logger, "El proceso de PID: %d es inexistente", PID);
		return -1;
	}

	printf("%c", '\n');
	printf("%s", string_repeat('-', 100));

	log_info(logger, "Tabla de páginas mostrada satisfactoriamente");
	return 1;
}

T_DIRECCION_LOG uint32ToDireccionLogica(uint32_t intDireccion) {

	T_DIRECCION_LOG direccionLogica;
	direccionLogica.SID = intDireccion / pow(2, 20);
	direccionLogica.paginaId = (intDireccion % (int) pow(2, 20)) / pow(2, 8);
	direccionLogica.desplazamiento = intDireccion % (int) pow(2, 8);

	return direccionLogica;
}

uint32_t DireccionLogicaToUint32(T_DIRECCION_LOG direccionLogica) {

	uint32_t intDireccion = direccionLogica.SID * pow(2, 20);
	intDireccion += direccionLogica.paginaId * pow(2, 8);
	intDireccion += direccionLogica.desplazamiento;

	return intDireccion;
}

void iniciarConexiones() {
	socket_general = crear_servidor(puerto, backlog);
	if (socket_general < 0) {
		log_error(logger, "No se pudo crear el servidor");
		exit(-1);
	}
	log_info(logger, "Se ha creado el servidor exitósamente");

	printf("Esperando conexiones...\n");

	pthread_t hiloPlanificador;
	pthread_t hiloKernel;

	while (1) {

		int socket_conectado = recibir_conexion(socket_general);
		printf("Se recibio una conexion!\n");
		int modulo_conectado = -1;
		t_datosAEnviar* datos = recibir_datos(socket_conectado);
		modulo_conectado = datos->codigo_operacion;

		if (modulo_conectado == soy_CPU) {
			printf("Se conecto una CPU\n");

			pthread_create(&hiloPlanificador, NULL,
					(void*) interpretarOperacion, &socket_conectado);
		}

		else if (modulo_conectado == soy_kernel) {
			printf("Se conecto el Kernel\n");

			pthread_create(&hiloKernel, NULL, (void*) interpretarOperacion,
					&socket_conectado);
		}

		free(datos->datos);
		free(datos);

	}

}

void interpretarOperacion(int* socket) {

	while (1) {
		t_datosAEnviar* datos;
		datos = recibir_datos(*socket);
		int pid;
		int tamanioSegmento;
		uint32_t baseSegmento;
		uint32_t respuesta;
		t_datosAEnviar* paquete;

		switch (datos->codigo_operacion) {

		case crear_segmento:

			memcpy(&pid, datos->datos, sizeof(int));
			memcpy(&tamanioSegmento, datos->datos + sizeof(int), sizeof(int));

			free(datos);

			respuesta = crearSegmento(pid, tamanioSegmento);

			paquete = crear_paquete(0, (void*) respuesta,
						sizeof(uint32_t));

			enviar_datos(*socket, paquete);

			break;

		case destruir_segmento:
			memcpy(&pid, datos->datos, sizeof(int));
			memcpy(&baseSegmento, datos->datos + sizeof(int), sizeof(uint32_t));

			respuesta = destruirSegmento(pid,baseSegmento);

			paquete = crear_paquete(0, (void*) respuesta, sizeof(uint32_t));

			enviar_datos(*socket, paquete);

			free(datos);

			break;

		case solicitar_memoria:
			//todo: hacerlo

			break;

		case escribir_memoria:

			//todo: hacerlo

			break;
		}

		free(datos);
	}
}


T_PAGINA* swapInPagina(int PID, T_SEGMENTO* seg, T_PAGINA* pag) {
//char* filePath = hago un metodo que me obtenga el nombre a partir del pid, seg, pag->pagid
//armo un archivo de tipo FILE* con la funcion txt_open_for_append(filePath)

//si mi archivo es distinto de NULL
//primero instancio la data

//cierro el archivo

//hago un for donde a la data de la pagina le asigno lo que voy leyendo
//le indico a la pag que no está mas swappeada
//le sumo a memoriaDisponibleParaSwapping el tamanioPag

return pag; //puse esto provisorio para que no me de error
}

int swapOutPagina(int PID, int SID, T_PAGINA* pag) {
 //creo un filePath
 //creo el archivo con ese filePath

 //si el file es distinto de NULL
 //char* contenidoDelArchivo = metodo que rellene el archivo
 //uso la funcion txt_write_in_file(contenidoDelArchivo)
 //cierro el archivo

 //le resto a la memoriaDisponibleParaSwapping el tamanioPag

return 0;
}
