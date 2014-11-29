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
#include <commons/txt.h>
#include <semaphore.h>

//	Variables
int tamanioMemoria;
int memoriaDisponible;
char* puerto;
int cantidadSwap;
char* sust_pags;
char* rutaLog;
char* rutaSwap;
char* MSP_CONFIG;
int tamanioPag = 256;
int socket_general;
int backlog;
int contadorLRU;

t_list* procesos;
t_list* marcosVacios;
t_list* marcosLlenos;
t_list* paginasEnMemoria;

//	Hilos
pthread_t hiloConsola;
pthread_t hiloEsperarConexiones;

//	Logger
t_log* logger;

//	Semáforos
sem_t mutex_MemoriaDisponible;
sem_t mutex_cantSwap;
sem_t mutex_contadorLRU;
sem_t mutex_procesos;
sem_t mutex_marcosLlenos;
sem_t mutex_marcosVacios;
sem_t mutex_paginasEnMemoria;

int main(int cantArgs, char** args) {
	printf("\n%s MSP %s", string_repeat('-', 20), string_repeat('-', 20));
	printf("\n");
	printf("\nIniciando...\n");

	inicializar(args);
	logger = log_create(rutaLog, "Log Programa", true, LOG_LEVEL_DEBUG);
	log_info(logger, "Inicio de la MSP \n El tamaño de la memoria principal es: %d \n El tamaño del archivo de paginación: %d", tamanioMemoria, tamanioPag);

	int hilo_EsperarConexiones = pthread_create(&hiloEsperarConexiones, NULL, (void*) iniciarConexiones, NULL );

	if (hilo_EsperarConexiones == 0) {
		log_info(logger, "La espera de conexiones se incializó correctamente");
	} else {
		log_error(logger, "Ha ocurrido un error en la espera de conexioness");
	}

	int hilo_Consola = pthread_create(&hiloConsola, NULL, (void*) inicializarConsola, NULL );

	if (hilo_Consola == 0) {
		log_info(logger, "La Consola de MSP se inicializó correctamente \n");
	} else {
		log_error(logger, "Ha ocurrido un error en la inicialización de la Consola de MSP");
	}

	pthread_join(hiloEsperarConexiones, NULL );
	pthread_join(hiloConsola, NULL );

	sem_destroy(&mutex_MemoriaDisponible);
	sem_destroy(&mutex_cantSwap);
	sem_destroy(&mutex_contadorLRU);
	sem_destroy(&mutex_procesos);
	sem_destroy(&mutex_marcosLlenos);
	sem_destroy(&mutex_marcosVacios);
	sem_destroy(&mutex_paginasEnMemoria);

	exit(0);
}

void inicializar(char** args) {
	cargarArchivoConfiguracion(args);
	crearMarcos();
	procesos = list_create();
	paginasEnMemoria = list_create();

	sem_init(&mutex_MemoriaDisponible, 0, 1);
	sem_init(&mutex_cantSwap, 0, 1);
	sem_init(&mutex_contadorLRU, 0, 1);
	sem_init(&mutex_procesos, 0, 1);
	sem_init(&mutex_marcosLlenos, 0, 1);
	sem_init(&mutex_marcosVacios, 0, 1);
	sem_init(&mutex_paginasEnMemoria, 0, 1);
}

void inicializarConsola() {
	char* comando = malloc(sizeof(char) * 1500);
	int seguimiento = 1;

	while (seguimiento) {
		printf("\n>");
		fgets(comando, 1500, stdin); //todo
		//scanf("%s",comando);
		comando[string_length(comando) - 1] = '\0';

		if (string_equals_ignore_case(comando, "Cerrar")) {
			log_debug(logger, "Se pidió Cerrar Consola");
			free(comando);
			seguimiento = 0;
		} else {
			interpretarComando(comando);
		}
		//todo free??
		printf("\r\n");
	}
	log_debug(logger, "Se cerro la Consola");
}

void interpretarComando(char* comando) {
	char** operacion;
	operacion = string_n_split(comando,2, " ");
	log_debug(logger, "La operación a ejecutar es: %s", operacion[0]);
	char** parametros = NULL;

	log_debug(logger,"%s", operacion[1]);
	if (operacion[1] != NULL) {
		log_debug(logger,"tiene parametros");
		parametros = string_split(operacion[1], ",");
	}

	if (string_equals_ignore_case(operacion[0], "Crear_Segmento")) {
		log_info(logger, "Se solicito crear un segmento por Consola");
		log_info(logger, "Los parámetros para la operación son: %s, %s", parametros[0], parametros[1]);
		crearSegmento(atoi(parametros[0]), atoi(parametros[1]));

	}

	else if (string_equals_ignore_case(operacion[0], "Destruir_Segmento")) {
		log_info(logger, "Se solicito destruir un segmento por Consola");
		log_info(logger, "Los parámetros para la operación son: %s, %s", parametros[0], parametros[1]);
		destruirSegmento(atoi(parametros[0]), (uint32_t) atoi(parametros[1]));
	}

	else if (string_equals_ignore_case(operacion[0], "Escribir_Memoria")) {
		log_info(logger, "Se solicito escribir en memoria por Consola");
		log_info(logger, "Los parámetros para la operación son: %s, %s, %s, %s",parametros[0], parametros[1], parametros[2], parametros[3]);
		//log_debug(logger,"se va a intentar escribir: %s", parametros[2]);
		escribirMemoria(atoi(parametros[0]), (uint32_t) atoi(parametros[1]),
				parametros[2], atoi(parametros[3]));
	}

	else if (string_equals_ignore_case(operacion[0], "Leer_Memoria")) {
		log_info(logger, "Se solicito leer en memoria por Consola");
		log_info(logger, "Los parámetros para la operación son: %s, %s, %s", parametros[0], parametros[1], parametros[2]);
		char* respuesta = solicitarMemoria(atoi(parametros[0]), atoi(parametros[1]), atoi(parametros[2]));
		printf("%s", respuesta);
		free(respuesta);
	}

	else if (string_equals_ignore_case(operacion[0], "Tabla_De_Paginas")) {
		log_info(logger, "Se solicito mostrar la tabla de paginas del proceso: %s", parametros[0]);
		tablaPaginas(atoi(parametros[0]));
	}

	else if (string_equals_ignore_case(operacion[0], "Tabla_De_Segmentos")) {
		log_info(logger, "Se solicito mostrar la tabla de segmentos");
		tablaSegmentos();
	}

	else if (string_equals_ignore_case(operacion[0], "Listar_Marcos")) {
		log_info(logger, "Se solicito mostrar la tabla de marcos");
		tablaMarcos();
	}
	else if (string_equals_ignore_case(operacion[0], "Cambiar_algoritmo")) {
		log_info(logger, "Se solicito Cambiar el algoritmo de susticion de paginas a: %s", parametros[0]);
		sust_pags = parametros[0];
		printf("Algoritmo de Sustitución de Páginas =  %s \n", sust_pags);
	}
	else {
		log_debug(logger, "No existe esa operación");
	}
}

void cargarArchivoConfiguracion(char** args) {
	t_config* configuracion = config_create(args[1]);

	if (config_has_property(configuracion, "CANTIDAD_MEMORIA")) {
		tamanioMemoria = config_get_int_value(configuracion, "CANTIDAD_MEMORIA") * pow(2, 10);
		printf("\nTamanio Memoria =  %d \n", tamanioMemoria);
	}

	if (config_has_property(configuracion, "PUERTO")) {
		puerto = config_get_string_value(configuracion, "PUERTO");
		printf("Puerto =  %s \n", puerto);
	}

	if (config_has_property(configuracion, "CANTIDAD_SWAP")) {
		cantidadSwap = config_get_int_value(configuracion, "CANTIDAD_SWAP") * pow(2, 20);
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
	if (config_has_property(configuracion, "RUTA_SWAP")) {
		rutaSwap = config_get_string_value(configuracion, "RUTA_SWAP");
		printf("Ruta de swapping =  %s \n", rutaSwap);
	}
	if (config_has_property(configuracion, "BACKLOG")) {
		backlog = config_get_int_value(configuracion, "BACKLOG");
		printf("Backlog =  %d \n", backlog);
	}

	memoriaDisponible = tamanioMemoria + cantidadSwap;
}

void crearMarcos() {
	marcosLlenos = list_create();
	marcosVacios = list_create();

	float cantidadDeMarcos = (float) tamanioMemoria / (float) tamanioPag;
	int cantidadMarcos = (int) cantidadDeMarcos;
	int i;

	for (i = 0; i < cantidadMarcos; i++) {

		//creo un marco vacio
		T_MARCO * marcoVacio = malloc(sizeof(T_MARCO));
		marcoVacio->marcoID = i;
		marcoVacio->empty = true;
		marcoVacio->PID = -1;
		marcoVacio->pagina = NULL;

		//agrego el marcoVacio a la lista de marcosVacios
		list_add(marcosVacios, marcoVacio);
	}
}

//Crea un nuevo segmento para el programa PID del tamaño tamanio. Devuelve la direccion
//virtual base del segmento.
uint32_t crearSegmento(int PID, int tamanio) {
	printf("\nCreando segmento...\n");
	log_debug(logger, "Entra a la función crearSegmento");

	if (tamanio > pow(2, 20)) {
		log_error(logger, "El tamanio especificado para el segmento supera al mega de capacidad");
		return error_memoria_llena;
	}

	sem_wait(&mutex_MemoriaDisponible);
	if (memoriaDisponible < tamanio) {
		log_error(logger, "La memoria disponible (principal + swap) no es suficiente para el tamanio del segmento");
		sem_post(&mutex_MemoriaDisponible);
		return error_memoria_llena;
	}
	sem_post(&mutex_MemoriaDisponible);

	bool procesoPorPid(T_PROCESO* proceso) {
		return proceso->PID == PID;
	}

	sem_wait(&mutex_procesos);
	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso == NULL ) {

		log_info(logger, "No existe un proceso con ese PID, se creará uno nuevo");
		proceso = malloc(sizeof(T_PROCESO));
		proceso->PID = PID;
		log_debug(logger, "El PID para el proceso creado es: %d", proceso->PID);

		proceso->segmentos = list_create();
		log_debug(logger, "Se creó la lista de segmentos");

		list_add(procesos, proceso);
		log_debug(logger, "Se agregó el proceso a la lista de procesos");
	}

	log_debug(logger, "Salió del if");

	//creo un segmento vacio para luego añadirlo a la lista de segmentos del proceso
	T_SEGMENTO* segmentoVacio = crearSegmentoVacio(proceso, tamanio);

	list_add(proceso->segmentos, segmentoVacio);
	log_debug(logger, "Se agrega el segmento a la lista de segmentos del proceso, tamanio: %d", list_size(proceso->segmentos));

	log_info(logger,"Se creó exitósamente el segmento");
	log_info(logger, "La dirección base del segmento creado es %d", segmentoVacio->baseSegmento);

	sem_wait(&mutex_MemoriaDisponible);
	memoriaDisponible = memoriaDisponible - sizeof(segmentoVacio->tamanio);
	sem_post(&mutex_MemoriaDisponible);

	sem_post(&mutex_procesos);

	printf("La base del segmento creado es: %d \n", segmentoVacio->baseSegmento);
	log_info(logger, "La base del segmento creado es: %d \n", segmentoVacio->baseSegmento);
	return segmentoVacio->baseSegmento;
}

T_SEGMENTO* crearSegmentoVacio(T_PROCESO* proceso, int tamanio) {
	T_SEGMENTO* segmentoVacio = malloc(sizeof(T_SEGMENTO));

	segmentoVacio->SID = calcularProximoSID(proceso);
	log_debug(logger, "el SID del segmento creado es: %d", segmentoVacio->SID);

	segmentoVacio->paginas = crearPaginasPorTamanioSegmento(tamanio,
			segmentoVacio->SID, proceso->PID);
	segmentoVacio->tamanio = tamanio;

	T_DIRECCION_LOG direccionLogica;
	direccionLogica.SID = segmentoVacio->SID;
	direccionLogica.paginaId = 0;
	direccionLogica.desplazamiento = 0;

	(segmentoVacio->baseSegmento) = direccionLogicaToUint32(direccionLogica);

	return segmentoVacio;
}

int calcularProximoSID(T_PROCESO* proceso) {

	if (list_is_empty(proceso->segmentos)) {
		return 0;
	}

	T_SEGMENTO* ultimoSegmento = list_get(proceso->segmentos, list_size(proceso->segmentos) - 1);
	return ((ultimoSegmento->SID) + 1);
}

t_list* crearPaginasPorTamanioSegmento(int tamanio, int SID, int PID) {
	log_debug(logger, "Entro a la funcion crearPaginasPorTamanioSegmento");

	//instancio la lista de paginas
	t_list* paginas = list_create();
	log_debug(logger, "Creo la lista de paginas");

	//calculo la cantidad de páginas que va a tener el segmento
	//necesario que la cantidadPaginas redondee para arriba - me fijo por el resto, si es distinto de 0 le sumo uno
	div_t division = div(tamanio, tamanioPag);
	log_debug(logger, "El resto de la division es: %d", division.rem);
	int cantidadPaginas = division.quot;

	if (division.rem != 0) {
		cantidadPaginas += 1;
	}
	log_debug(logger, "La cantidad de paginas que va a tener el segmento son: %d", cantidadPaginas);

	int i;
	for (i = 0; i < cantidadPaginas; i++) {
		log_debug(logger, "Entro al for para crear cada pagina");

		//creo una pagina vacia
		T_PAGINA * paginaVacia = malloc(sizeof(T_PAGINA));
		paginaVacia->paginaID = i;
		log_debug(logger, "El id de la pag es: %d", paginaVacia->paginaID);
		paginaVacia->swapped = false;
		paginaVacia->marcoID = -1;
		log_debug(logger, "El id del marco para la pag es: %d", paginaVacia->marcoID);
		paginaVacia->SID = SID;
		paginaVacia->PID = PID;
		paginaVacia->contadorLRU = 0;
		paginaVacia->bitReferencia = 0;

		//paginaVacia->data = calloc(256, sizeof(char));
		paginaVacia->data = malloc(tamanioPag);
		int i;
		for(i=0; i < tamanioPag; i++) {
			paginaVacia->data[i] = '\0';
		}

		//agrego la pagina a la lista de paginas
		list_add(paginas, paginaVacia);
		log_debug(logger, "Agrego la pagina a la lista de paginas");
	}

	log_debug(logger, "El tamaño de la lista de paginas es: %d", list_size(paginas));
	return paginas;
}

//Destruye el segmento identificado por baseSegmento del programa PID y libera la
//memoria que ocupaba ese segmento.
void destruirSegmento(int PID, uint32_t baseSegmento) {
	printf("\nDestruyendo segmento...\n");
	log_debug(logger, "Entro a la función destruirSegmento");

	bool procesoPorPid(T_PROCESO* proceso) {
		return proceso->PID == PID;
	}

	bool segmentoPorBase(T_SEGMENTO* segmento) {
		return segmento->baseSegmento == baseSegmento;
	}

	sem_wait(&mutex_procesos);
	//busco en la lista de procesos el proceso con ese PID
	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso != NULL ) {
		log_debug(logger, "Encontró al proceso con el pid: %d", proceso->PID);

		//busco en la lista de segmentos del proceso el segmento con esa base
		T_SEGMENTO * seg = list_find(proceso->segmentos, (void*) segmentoPorBase);

		if (seg != NULL ) {

			log_debug(logger, "Encontró al proceso con la base: %d", seg->baseSegmento);

			//elimino las paginas del segmento
			log_info(logger, "Comienzan a destruirse las paginas del segmento");
			list_clean_and_destroy_elements(seg->paginas, (void*) destruirPag);
			log_debug(logger, "Se eliminaron las páginas del segmento, ahora el tamaño de la lista de paginas es: %d", list_size(seg->paginas));
			log_info(logger, "Se destruyeron todas las paginas correspondientes al segmento");

			//elimino de la lista de segmentos del proceso, el segmento
			log_debug(logger, "El proceso tiene %d segmentos", list_size(proceso->segmentos));
			list_remove_by_condition(proceso->segmentos, (void*) segmentoPorBase);
			log_debug(logger, "Se elimino el segmento de la lista de segmentos del proceso, ahora tiene %d segmentos", list_size(proceso->segmentos));

			sem_wait(&mutex_MemoriaDisponible);
			memoriaDisponible = memoriaDisponible + sizeof(seg->tamanio);
			sem_post(&mutex_MemoriaDisponible);

			free(seg);
			log_debug(logger, "Se libero el segmento\n");
			log_info(logger, "Se destruyo el segmento exitosamente");

			sem_post(&mutex_procesos);
			printf("Se destruyo el segmento\n");
		}
		else {
			sem_post(&mutex_procesos);
			log_error(logger, "El segmento no ha sido destruido porque es inexistente");
		}
	} else {
		sem_post(&mutex_procesos);
		log_error(logger, "El segmento no ha sido destruido porque el proceso con PID: %d no existe", PID);
	}
}

static void destruirPag(T_PAGINA* pagina) {

	bool marcoPorPagina(T_MARCO* marco) {
		return marco->pagina == pagina;
	}

	bool paginaPorId(T_PAGINA* pag) {
		return pag->paginaID == pagina->paginaID;
	}

	//busco en la lista de marcos el marco con esa pagina si esta asignado
	sem_wait(&mutex_marcosLlenos);
	T_MARCO* marco = list_find(marcosLlenos, (void*) marcoPorPagina);

	if (marco != NULL ) {

		marco->empty = true;
		marco->PID = -1;
		marco->pagina = NULL;

		log_info(logger, "Se desasigna el marco de id %d de la pagina de id %d para destruirla", marco->marcoID, pagina->paginaID);
		sem_wait(&mutex_paginasEnMemoria);
		list_remove_by_condition(paginasEnMemoria, (void*) paginaPorId);
		sem_post(&mutex_paginasEnMemoria);

		list_remove_by_condition(marcosLlenos, (void*) marcoPorPagina);

		sem_wait(&mutex_marcosVacios);
		list_add(marcosVacios, marco);
		sem_post(&mutex_marcosVacios);
	}
	sem_post(&mutex_marcosLlenos);

	if (pagina->swapped){
		char* filePath = obtenerFilePath(pagina->PID, pagina->SID, pagina->paginaID);

		if (remove(filePath) == 0) {

			sem_wait(&mutex_cantSwap);
			cantidadSwap += tamanioPag;
			sem_post(&mutex_cantSwap);

		} else {
			log_error(logger, "Se quiso destruir la pagina de id %d que estaba swappeada y no existe el archivo de esta", pagina->paginaID);
		}
	}

	free(pagina->data);
	free(pagina);
}

//Para el espacio de direcciones del proceso PID, devuelve hasta tamanio bytes
//comenzando desde direccion.
char* solicitarMemoria(int PID, uint32_t direccion, int tamanio) {
	printf("\nSolicitando memoria...\n");
	log_debug(logger, "Entro a la funcion solicitarMemoria");

	T_DIRECCION_LOG direccionLogica = uint32ToDireccionLogica(direccion);

	int contadorPagina;
	contadorPagina = direccionLogica.paginaId;

	char* memoriaSolicitada = malloc((sizeof(char) * tamanio)+1);
	int tamanioLeido;

	bool procesoPorPid(T_PROCESO* proceso) {
		return proceso->PID == PID;
	}
	bool segmentoPorSid(T_SEGMENTO* segmento) {
		return segmento->SID == direccionLogica.SID;
	}
	bool paginaPorPagid(T_PAGINA* pagina) {
		return pagina->paginaID == direccionLogica.paginaId;
	}
	bool paginaSiguiente(T_PAGINA* pagina) {
		return pagina->paginaID == (contadorPagina + 1);
	}
	bool marcoPorVacio(T_MARCO* marco) {
		return marco->empty == true;
	}

	sem_wait(&mutex_procesos);
	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso != NULL ) {
		log_debug(logger, "Encontró el proceso de PID %d", proceso->PID);
		T_SEGMENTO* seg = list_find(proceso->segmentos, (void*) segmentoPorSid);

		if (seg != NULL ) {
			log_debug(logger, "Encontró el segmento de SID %d", seg->SID);
			//aca me trae la pagina donde esta el desplazamiento pero con el tamanio me puedo pasar de pagina.
			T_PAGINA* pag = list_find(seg->paginas, (void*) paginaPorPagid);

			if (pag != NULL ) {
				log_debug(logger, "Encontró la pagina de ID %d y marcoId %d", pag->paginaID, pag->marcoID);

				if ((tamanioPag * (pag->paginaID) + direccionLogica.desplazamiento) > seg->tamanio) {
					log_error(logger, "Segmentation Fault: Direccion Invalida");
					sem_post(&mutex_procesos);
					return "error_segmentation_fault";
				}

				log_debug(logger, "tamanioPag * pag.pagID = %d",(tamanioPag * (pag->paginaID)));
				log_debug(logger, "Dire.desplazamiento = %d",(direccionLogica.desplazamiento));
				log_debug(logger, "tamanio que se suma al desplazamiento = %d",(tamanio));
				log_debug(logger, "Tamano de este segmento = %d",(seg->tamanio));

				if ((tamanioPag * (pag->paginaID) + direccionLogica.desplazamiento + tamanio) > seg->tamanio) {
					log_error(logger, "Segmentation Fault: Se excedieron los limites del segmento");
					sem_post(&mutex_procesos);
					return "error_segmentation_fault";
				}

				log_debug(logger, "La pagina tiene el marco asignado de id: %d", pag->marcoID);
				if (pag->marcoID == -1) {
					log_debug(logger, "Como la página no tiene marco asignado, le asigno");
					int resultado = asignoMarcoAPagina(PID, seg, pag);

					if (resultado < 0) {
						log_error(logger, "No se ha podido solicitar memoria ya que no se pudo asignar un marco a la página");
						sem_post(&mutex_procesos);
						return (char*) resultado;
					}
				}

				int inicio = direccionLogica.desplazamiento;
				int final = direccionLogica.desplazamiento + tamanio;

				log_debug(logger, "Comienza a leer desde: %d", inicio);
				log_debug(logger, "Termina a leer en: %d", final);

				if (final > tamanioPag) {
					log_debug(logger, "El final es mayor que el tamanioPag");
					log_debug(logger,"Va a copiar %d bytes", (tamanioPag - inicio));

					memcpy(memoriaSolicitada, (pag->data + inicio), (tamanioPag - inicio));

					log_debug(logger,"tamanio memoriaSolicitada: %d", string_length(memoriaSolicitada));

					leoMemoria(pag); //aviso que lei para que settea atributos de algoritmos a las paginas

					tamanio = tamanio - (tamanioPag - inicio);
					tamanioLeido = (tamanioPag - inicio);

					memoriaSolicitada[tamanioLeido]='\0';

					log_debug(logger, "La memoria leida es %s", memoriaSolicitada);

					pag = list_find(seg->paginas, (void*) paginaSiguiente);
					contadorPagina++;

					while (tamanio > tamanioPag) {

						if (pag->marcoID == -1) {
							int resultado = asignoMarcoAPagina(PID, seg, pag);

							if (resultado < 0) {
								log_error(logger, "No se ha podido solicitar memoria ya que no se pudo asignar un marco a la página");
								sem_post(&mutex_procesos);
								return "error_memoria_llena";
							}
						}

						memcpy(memoriaSolicitada + tamanioLeido, pag->data, tamanioPag);
						leoMemoria(pag);

						tamanioLeido += tamanioPag;
						tamanio = tamanio - tamanioPag;
						memoriaSolicitada[tamanioLeido]='\0';

						pag = list_find(seg->paginas, (void*) paginaSiguiente);
						contadorPagina++;

					}

					if (tamanio > 0) {
						log_debug(logger, "El tamanio es mayor a 0");
						if (pag->marcoID == -1) {
							int resultado = asignoMarcoAPagina(PID, seg, pag);

							if (resultado < 0) {
								log_error(logger, "No se ha podido solicitar memoria ya que no se pudo asignar un marco a la página");
								sem_post(&mutex_procesos);
								return "error_memoria_llena";
							}
						}
						memcpy(memoriaSolicitada + tamanioLeido, pag->data, tamanio);
						leoMemoria(pag);
						tamanioLeido = tamanioLeido + tamanio;
						memoriaSolicitada[tamanioLeido]='\0';
					}
				} else {
					log_debug(logger, "El tamanio es menor que el tamanioPag");
					log_debug(logger, "Lee memoria directamente");
					memcpy(memoriaSolicitada, pag->data + inicio, final - inicio);
					leoMemoria(pag);
					memoriaSolicitada[final-inicio]='\0';
				}

			} else {
				log_error(logger, "No se ha podido solicitar memoria ya que la página es inexistente");
				sem_post(&mutex_procesos);
				return "error_segmentation_fault";
			}
		} else {
			log_error(logger, "No se ha podido solicitar memoria ya que el segmento es inexistente");
			sem_post(&mutex_procesos);
			return "error_segmentation_fault";
		}
	} else {
		log_error(logger, "No se ha podido solicitar memoria ya que el proceso de PID: %d es inexistente", PID);
		sem_post(&mutex_procesos);
		return "error_general";
	}

	sem_post(&mutex_procesos);
	log_info(logger, "El contenido de la página solicitada es: %s", memoriaSolicitada);

	return memoriaSolicitada;
}

void leoMemoria(T_PAGINA* pag) {

	sem_wait(&mutex_contadorLRU);
	pag->bitReferencia = 1;
	pag->contadorLRU = contadorLRU;
	contadorLRU++;
	sem_post(&mutex_contadorLRU);
}

//Para el espacio de direcciones del proceso PID, escribe hasta tamanio bytes del buffer bytesAEscribir
//comenzando en la direccion.
uint32_t escribirMemoria(int PID, uint32_t direccion, char* bytesAEscribir,	int tamanio) {
	printf("\nIniciando proceso de escritura de memoria...\n");
	log_debug(logger, "Entro a la funcion escribirMemoria");
	log_debug(logger,"parametros %d %d %d", PID, direccion, tamanio);

	if(bytesAEscribir == NULL){
		log_debug(logger,"Es null los bytesAEscribir");
	}

	bytesAEscribir[tamanio] = '\0';

	//if (string_length(bytesAEscribir) != tamanio){
	//	log_error(logger, "Tamanio especificado distinto de los que se quieren escribir");
	//	return error_general;
	//}

	T_DIRECCION_LOG direccionLogica = uint32ToDireccionLogica(direccion);
	log_debug(logger, "Sid %d", direccionLogica.SID);
	log_debug(logger, "desplazamiento %d", direccionLogica.desplazamiento);
	log_debug(logger, "paginaId %d", direccionLogica.paginaId);

	int contadorPagina;
	contadorPagina = direccionLogica.paginaId;
	log_debug(logger, "El contador de página es: %d", contadorPagina);

	char* bytesQuedanPorEscribir = malloc(tamanio);

	bool procesoPorPid(T_PROCESO* proceso) {
		return proceso->PID == PID;
	}
	bool segmentoPorSid(T_SEGMENTO* segmento) {
		return segmento->SID == direccionLogica.SID;
	}
	bool paginaPorPagid(T_PAGINA* pagina) {
		return pagina->paginaID == direccionLogica.paginaId;
	}
	bool paginaSiguiente(T_PAGINA* pagina) {
		return pagina->paginaID == contadorPagina + 1;
	}
	bool marcoPorVacio(T_MARCO* marco) {
		return marco->empty == true;
	}

	sem_wait(&mutex_procesos);
	log_debug(logger, "procesos %d", list_size(procesos));
	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso != NULL ) {

		log_debug(logger, "Encontró al proceso de PID: %d", proceso->PID);

		T_SEGMENTO* seg = list_find(proceso->segmentos, (void*) segmentoPorSid);
		log_debug(logger,"El tamanio de la lista de segmentos del proceso es: %d", list_size(proceso->segmentos));

		if (seg != NULL ) {
			log_debug(logger, "Encontró al segmento de SID: %d", seg->SID);

			//aca me trae la pagina donde esta el desplazamiento pero con el tamanio me puedo pasar de pagina.
			T_PAGINA* pag = list_find(seg->paginas, (void*) paginaPorPagid);

			if (pag != NULL ) {
				log_debug(logger, "Encontró a la página de ID: %d", pag->paginaID);

				if ((tamanioPag * (pag->paginaID) + direccionLogica.desplazamiento) > seg->tamanio) {
					log_error(logger, "Segmentation Fault: Dirección Invalida");
					sem_post(&mutex_procesos);
					free(bytesQuedanPorEscribir);
					return error_segmentation_fault;
				}
				if ((tamanioPag * (pag->paginaID) + direccionLogica.desplazamiento + tamanio) > seg->tamanio) {
					log_error(logger, "Segmentation Fault: Se excedieron los limites del segmento");
					sem_post(&mutex_procesos);
					free(bytesQuedanPorEscribir);
					return error_segmentation_fault;
				}

				if (pag->marcoID == -1) {
					log_debug(logger, "Como la pag no tenía marco, le asigno");
					int resultado = asignoMarcoAPagina(PID, seg, pag);
					log_debug(logger,"El resultado de asignarMarco es: %d",resultado);

					if (resultado < 0) {
						log_error(logger, "No se ha podido escribir memoria ya que no se pudo asignar un marco a la página");
						sem_post(&mutex_procesos);
						free(bytesQuedanPorEscribir);
						return resultado;
					}
				}

				int inicio = direccionLogica.desplazamiento;
				int final = (direccionLogica.desplazamiento + tamanio);

				log_debug(logger,"El inicio para escribir es: %d", inicio);
				log_debug(logger,"El final para escribir es: %d",final);

				if (final > tamanioPag) {
					log_debug(logger,"El final es mayor que el tamanioPag => Hay que escribir mas de una pagina");
					escriboMemoria(pag, inicio, tamanioPag, bytesAEscribir);

					tamanio = tamanio - (tamanioPag - inicio);

					memcpy(bytesQuedanPorEscribir,bytesAEscribir + (tamanioPag-inicio), tamanio);
					memcpy(bytesAEscribir, bytesQuedanPorEscribir, tamanio);

					log_debug(logger, "tamanio escrito= tamanioPag - inicio: %d", (tamanioPag - inicio));
					log_debug(logger, "tamanio a escribir: %d", tamanio);

					pag = list_find(seg->paginas, (void*) paginaSiguiente);
					log_debug(logger,"Encontro la página siguiente de id %d", pag->paginaID);
					contadorPagina++;

					while (tamanio > tamanioPag) {
						log_debug(logger,"El tamanio que resta escribir es mayor a tamanioPag");

						if (pag->marcoID == -1) {
							int resultado = asignoMarcoAPagina(PID, seg, pag);

							if (resultado < 0) {
								log_error(logger, "No se ha podido escribir memoria ya que no se pudo asignar un marco a la página");
								sem_post(&mutex_procesos);
								free(bytesQuedanPorEscribir);
								return resultado;
							}
						}

						escriboMemoria(pag, 0, tamanioPag, bytesAEscribir);

						tamanio = tamanio - tamanioPag;
						memcpy(bytesQuedanPorEscribir,bytesAEscribir + (tamanioPag), tamanio);
						memcpy(bytesAEscribir, bytesQuedanPorEscribir, tamanio);

						log_debug(logger,"El tamanio que queda escribir es %d bytes", tamanio);

						pag = list_find(seg->paginas, (void*) paginaSiguiente);
						contadorPagina++;
					}

					if (tamanio > 0) {
						log_debug(logger, "tamanio de bytes que quedan escribir menor al de una pagina");

						if (pag->marcoID == -1) {
							int resultado = asignoMarcoAPagina(PID, seg, pag);
							log_debug(logger,"EL resultado de asignar marcoAPag es %d", resultado);

							if (resultado < 0) {
								log_error(logger, "No se ha podido escribir memoria ya que no se pudo asignar un marco a la página");
								sem_post(&mutex_procesos);
								free(bytesQuedanPorEscribir);
								return resultado;
							}
						}
						escriboMemoria(pag, 0, tamanio, bytesAEscribir);
					}
				} else {
					escriboMemoria(pag, inicio, final, bytesAEscribir);
				}

			} else {
				log_error(logger, "No se ha podido escribir en memoria porque la página es inexistente");
				sem_post(&mutex_procesos);
				free(bytesQuedanPorEscribir);
				return error_segmentation_fault;
			}
		} else {
			log_error(logger, "No se ha podido escribir en memoria porque el segmento es inexistente");
			sem_post(&mutex_procesos);
			free(bytesQuedanPorEscribir);
			return error_segmentation_fault;
		}
	} else {
		log_error(logger, "No se ha podido escribir en memoria porque el proceso de PID: %d es inexistente",PID);
		sem_post(&mutex_procesos);
		free(bytesQuedanPorEscribir);
		return error_general;
	}

	sem_post(&mutex_procesos);
	log_info(logger, "Se ha escrito en memoria exitósamente");
	printf("Se ha escrito en memoria exitósamente\n");
	free(bytesQuedanPorEscribir);

	return operacion_exitosa;
}

void escriboMemoria(T_PAGINA* pag, int inicio, int final, char* bytesAEscribir) {
	log_debug(logger,"Entre a escriboMemoria");

	int i;
	int j = 0;
	for(i = inicio; i<final;i++){
		pag->data[i] = bytesAEscribir[j];
		//log_debug(logger,"Escribo Caracter %c en Posicion %d", bytesAEscribir[j], j);
		j++;
	}

	//memcpy((pag->data + inicio),bytesAEscribir, (final - inicio)); //TODO
	log_debug(logger,"tamanio pag.data %d", string_length(pag->data));
	log_debug(logger,"Se escribio: %s", pag->data);

	sem_wait(&mutex_contadorLRU);
	pag->bitReferencia = 1;
	pag->contadorLRU = contadorLRU;
	contadorLRU++;
	sem_post(&mutex_contadorLRU);
}

int asignoMarcoAPagina(int PID, T_SEGMENTO* seg, T_PAGINA* pag) {
	log_debug(logger, "Entro a la funcion asignoMarcoAPagina");

	T_MARCO* marcoAsignado;

	if (pag->swapped) {
		log_info(logger, "La pagina a la que se le va a asignar un marco está swappeada, Swap in");
		pag = swapInPagina(PID, seg, pag);

		if (pag == NULL){
			log_error(logger, "No funciono el SwapIN, pagina en null");
			return error_general;
		}
	}

	sem_wait(&mutex_marcosVacios);

	if (list_is_empty(marcosVacios)) {
		log_debug(logger, "No hay marcos libres");

		sem_wait(&mutex_cantSwap);
		if (cantidadSwap > tamanioPag) {
			log_info(logger, "No hay marcos libres, buscando un marco victima");
			log_debug(logger, "Hay cantidad de swap disponible");

			marcoAsignado = seleccionarMarcoVictima();
			log_debug(logger, "El marco asignado tiene id: %d", marcoAsignado->marcoID);

			if (swapOutPagina(marcoAsignado->PID, marcoAsignado->pagina->SID, marcoAsignado->pagina) < 0) {
				log_debug(logger,"El swapOut de la pagina dio error");
				sem_post(&mutex_cantSwap);
				sem_post(&mutex_marcosVacios);
				return error_general;
			}

			log_info(logger, "Se desasigna la pagina de id %d del marco de id %d para poder asignarle la pagina a referenciar", marcoAsignado->pagina->paginaID, marcoAsignado->marcoID);

		} else {
			log_error(logger, "No hay marcos libres y no hay sufiente espacio de swapping");
			sem_post(&mutex_cantSwap);
			sem_post(&mutex_marcosVacios);
			return error_memoria_llena;
		}
		sem_post(&mutex_cantSwap);
	} else {
		log_debug(logger,"Hay marcos vacios entonces le asigno uno de esos");
		marcoAsignado = list_remove(marcosVacios, 0);
		sem_post(&mutex_marcosVacios);
	}

	pag->marcoID = marcoAsignado->marcoID;
	pag->bitReferencia = 1;

	marcoAsignado->pagina = pag;
	marcoAsignado->PID = PID;
	marcoAsignado->empty = false;

	sem_wait(&mutex_paginasEnMemoria);
	list_add(paginasEnMemoria, pag);
	log_debug(logger,"EL tamanio de pagsEnMemoria es: %d", list_size(paginasEnMemoria));
	sem_post(&mutex_paginasEnMemoria);

	sem_wait(&mutex_marcosLlenos);
	list_add(marcosLlenos, marcoAsignado);
	log_debug(logger,"EL tamanio de marcosLlenos es: %d", list_size(marcosLlenos));
	sem_post(&mutex_marcosLlenos);

	log_info(logger, "Se le ha asignado el marco de id %d a la pagina de id %d", pag->marcoID, pag->paginaID);
	return operacion_exitosa;
}

int tablaMarcos() {
	printf("\n%s TABLA DE MARCOS %s \n", string_repeat('-', 36), string_repeat('-', 36));
	log_info(logger,"%s TABLA DE MARCOS %s", string_repeat('-', 10), string_repeat('-', 10));

	bool ordenarPorMenorId(T_MARCO* marco1, T_MARCO* marco2) {
		return (marco1->marcoID < marco2->marcoID);
	}

	t_list* marcos = list_create();

	sem_wait(&mutex_marcosLlenos);
	sem_wait(&mutex_marcosVacios);

	list_add_all(marcos, marcosLlenos);
	list_add_all(marcos, marcosVacios);

	sem_post(&mutex_marcosLlenos);
	sem_post(&mutex_marcosVacios);

	list_sort(marcos, (void*) ordenarPorMenorId);

	int i;
	int cantidadMarcos = list_size(marcos);
	for (i = 0; cantidadMarcos > i; i++) {
		T_MARCO* marco = list_get(marcos, i);
		printf("\nNúmero de marco: %d     ", marco->marcoID);
		log_info(logger, "Número de marco: %d", marco->marcoID);

		if (marco->empty) {
			printf("Marco disponible \n");
			log_info(logger, "Marco disponible");
		} else {
			printf("Marco ocupado por el proceso: %-4d", marco->PID);
			log_info(logger,"Marco ocupado por el proceso: %d", marco->PID);
			printf("     ");
			printf("Segmento: %-4d", marco->pagina->SID);
			log_info(logger, "Segmento: %d", marco->pagina->SID);
			printf("     ");
			printf("Página: %-4d \n", marco->pagina->paginaID);
			log_info(logger, "Página: %d", marco->pagina->paginaID);
		}
	}
	printf("\n%s\n", string_repeat('-', 90));
	log_info(logger, "%s", string_repeat('-', 30));

	log_info(logger, "Tabla de marcos mostrada satisfactoriamente");
	return operacion_exitosa;
}

int tablaSegmentos() {
	log_debug(logger, "Entre a la funcion tablaSegmentos");
	printf("\n");
	printf("%s TABLA DE SEGMENTOS %s", string_repeat('-', 35), string_repeat('-', 35));
	log_info(logger, "%s TABLA DE SEGMENTOS %s", string_repeat('-', 10), string_repeat('-', 10));
	printf("\n");

	sem_wait(&mutex_procesos);
	log_debug(logger, "Cantidad de proceso: %d",list_size(procesos) );
	int cantidadProcesos = list_size(procesos);
	int i;
	int j;

	if(cantidadProcesos == 0){
		printf("\nNo hay procesos, por lo que no existen segmentos\n");
		log_info(logger,"No hay procesos, por lo que no existen segmentos");
	}

	for (i = 0; cantidadProcesos > i; i++) {
		T_PROCESO* proceso = list_get(procesos, i);
		int cantidadSegmentos = list_size(proceso->segmentos);

		printf("\nPara el proceso de ID: %-d \n", proceso->PID);
		log_info(logger, "Para el proceso de ID: %d", proceso->PID);

		if (cantidadSegmentos == 0) {
			printf("%c", '\n');
			log_info(logger, "El proceso no tiene segmentos");
			printf("El proceso no tiene segmentos");
			//sem_post(&mutex_procesos);
			//return operacion_exitosa;
		}

		for (j = 0; cantidadSegmentos > j; j++) {
			T_SEGMENTO* segmento = list_get(proceso->segmentos, j);
			printf("%c", '\n');
			printf("Número de segmento: %-d", (int) segmento->SID);
			log_info(logger, "Número de segmento: %d", (int) segmento->SID);
			printf("     ");
			printf("Tamaño: %-d", segmento->tamanio);
			log_info(logger,"Tamaño: %-d", segmento->tamanio);
			printf("     ");
			printf("Dirección virtual base: %-d \n", (int) segmento->baseSegmento);
			log_info(logger, "Dirección virtual base: %d \n", (int) segmento->baseSegmento);
		}

		printf("\n%s\n", string_repeat('*', 90));
		log_info(logger, "%s", string_repeat('*', 30));
	}

	sem_post(&mutex_procesos);

	printf("\n");
	printf("%s\n", string_repeat('-', 90));
	log_info(logger,"%s", string_repeat('-', 30));
	printf("\n");
	log_info(logger, "Tabla de segmentos mostrada satisfactoriamente");
	return operacion_exitosa;
}

int tablaPaginas(int PID) {
	log_debug(logger, "Entramos a tablaPaginas funcion");

	printf("\n");
	printf("%s TABLA DE PÁGINAS %s", string_repeat('-', 36), string_repeat('-', 36));
	log_info(logger, "%s TABLA DE PÁGINAS %s", string_repeat('-', 10), string_repeat('-', 10));
	printf("\n");

	sem_wait(&mutex_procesos);
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
			printf("\nPara el segmento de ID: %d \n", segmento->SID);
			log_info(logger, "Para el segmento de ID: %d", segmento->SID);

			int cantidadPaginas = list_size(segmento->paginas);
			for (j = 0; cantidadPaginas > j; j++) {
				T_PAGINA* pagina = list_get(segmento->paginas, j);
				printf("%c", '\n');
				printf("Página ID = %-d", pagina->paginaID);
				log_info(logger, "Página ID = %d", pagina->paginaID);

				if (pagina->swapped) {
					printf("     ");
					printf("La página se encuentra swappeada \n");
					log_info(logger, "La página se encuentra swappeada");
				} else if ((pagina->marcoID) != -1) {
					printf("     ");
					printf("Se encuentra en memoria principal en el marco de ID: %-d \n", pagina->marcoID);
					log_info(logger, "Se encuentra en memoria principal en el marco de ID: %-d", pagina->marcoID);
				} else {
					printf("     ");
					printf("No se encuentra en memoria principal \n");
					log_info(logger,"No se encuentra en memoria principal");
				}
			}
			printf("\n%s\n", string_repeat('*', 90));
			log_info(logger, "%s", string_repeat('*', 90));
		}
	} else {
		log_error(logger, "El proceso de PID: %-d es inexistente \n", PID);
		sem_post(&mutex_procesos);
		return error_general;
	}

	sem_post(&mutex_procesos);

	printf("%c", '\n');
	printf("%s", string_repeat('-', 90));
	log_info(logger,"%s", string_repeat('-', 30));
	printf("\n");

	log_info(logger, "Tabla de páginas mostrada satisfactoriamente");
	return operacion_exitosa;
}

T_DIRECCION_LOG uint32ToDireccionLogica(uint32_t intDireccion) {

	T_DIRECCION_LOG direccionLogica;
	direccionLogica.SID = intDireccion / pow(2, 20);
	direccionLogica.paginaId = (intDireccion % (int) pow(2, 20)) / pow(2, 8);
	direccionLogica.desplazamiento = intDireccion % (int) pow(2, 8);

	return direccionLogica;
}

uint32_t direccionLogicaToUint32(T_DIRECCION_LOG direccionLogica) {

	uint32_t intDireccion = direccionLogica.SID * pow(2, 20);
	intDireccion += direccionLogica.paginaId * pow(2, 8);
	intDireccion += direccionLogica.desplazamiento;

	return intDireccion;
}

void iniciarConexiones() {
	socket_general = crear_servidor(puerto, backlog);

	if (socket_general < 0) {
		log_error(logger, "No se pudo crear el servidor en iniciar conexiones");
		exit(-1);
	}

	log_info(logger, "Se ha creado el servidor exitósamente");

	printf("\nEsperando conexiones...\n");

	pthread_t hiloKernel;
	pthread_t hiloCPU;
	int * sock;

	while (1) {
		sock = malloc(sizeof(int));
		*sock = recibir_conexion(socket_general);
		printf("\n*** Se recibio una conexión! ***\n");
		log_info(logger, "Se recibio una conexión");

		int modulo_conectado = -1;
		t_datosAEnviar* datos = recibir_datos(*sock);
		modulo_conectado = datos->codigo_operacion;

		if (modulo_conectado == soy_CPU) {
			printf("\nSe conecto una CPU\n");
			log_info(logger,"Se conectó una CPU");
			pthread_create(&hiloCPU, NULL, (void*) interpretarOperacion, sock);
		}

		else if (modulo_conectado == soy_kernel) {
			printf("\nSe conecto el Kernel\n");
			log_info(logger,"Se concectó el Kernel");
			pthread_create(&hiloKernel, NULL, (void*) interpretarOperacion, sock);
		}

		free(datos->datos);
		free(datos);
	}

}

void interpretarOperacion(int* sock_conectado) {
	int seguimiento = 1;
	int * socket_conectado = malloc(sizeof(int));
	memcpy(socket_conectado, sock_conectado, sizeof(int));

	t_datosAEnviar* datos;
	int pid;
	int tamanio;
	char* bytesAEscribir;
	uint32_t baseSegmento;
	uint32_t direccion;
	uint32_t respuesta;
	t_datosAEnviar* paquete;

	while (seguimiento) {

		datos = recibir_datos(*socket_conectado);
		//log_debug(logger,"Se recibieron datos del socket : %d", *socket_conectado);

		if (datos == NULL){
			seguimiento = 0;
			close(*socket_conectado);
		}
		else {

			switch (datos->codigo_operacion) {

			case crear_segmento:
				log_info(logger,"Se solicitó crear un segmento");

				memcpy(&pid, datos->datos, sizeof(int));
				memcpy(&tamanio, datos->datos + sizeof(int), sizeof(int));

				log_info(logger,"Los parámetros que se recibieron son: %d, %d", pid, tamanio);

				respuesta = crearSegmento(pid, tamanio);
				log_debug(logger,"La respuesta es: %d", respuesta);

				paquete = crear_paquete(1, (void*) &respuesta, sizeof(uint32_t));

				int r = enviar_datos(*socket_conectado, paquete);

				log_debug(logger,"se enviaron los datos %d al socket %d", r, *socket_conectado);

				break;

			case destruir_segmento:
				log_info(logger,"Se solicitó destruir un segmento");

				memcpy(&pid, datos->datos, sizeof(int));
				memcpy(&baseSegmento, datos->datos + sizeof(int), sizeof(uint32_t));

				log_info(logger,"Los parámetros que se recibieron son: %d, %d", pid, baseSegmento);

				destruirSegmento(pid, (uint32_t) baseSegmento);

				break;

			case solicitar_memoria:
				log_info(logger,"Se solicitó leer memoria");
				int liberarRespuesta = 1;

				memcpy(&pid, datos->datos, sizeof(int));
				log_debug(logger,"del proceso: %d", pid);
				memcpy(&direccion, datos->datos + sizeof(int), sizeof(int));
				log_debug(logger,"direccion: %d", direccion);
				memcpy(&tamanio, datos->datos + datos->tamanio - sizeof(int), sizeof(int));
				log_debug(logger,"El tamanio a leer es: %d", tamanio);

				log_info(logger,"Los parámetros que se recibieron son: %d, %d, %d", pid, direccion, tamanio);

				char* resultado = solicitarMemoria(pid, direccion, tamanio);

				int codigo_operacion = 0;

				if(strcmp(resultado,"error_segmentation_fault") == 0){
					codigo_operacion = error_segmentation_fault;
					liberarRespuesta = 0;
				}
				if(strcmp(resultado,"error_general") == 0){
					codigo_operacion = error_general;
					liberarRespuesta = 0;
				}
				if(strcmp(resultado,"error_memoria_llena") == 0){
					codigo_operacion = error_memoria_llena;
					liberarRespuesta = 0;
				}

				log_debug(logger, "Codigo de Operacion a enviar: %d", codigo_operacion);
				paquete = crear_paquete(codigo_operacion, (void*) resultado, sizeof(int));

				enviar_datos(*socket_conectado, paquete);

				if (liberarRespuesta != 0){
					free(resultado);
				}

				break;

			case escribir_memoria:
				log_info(logger,"Se solicitó escribir en memoria");

				memcpy(&pid, datos->datos, sizeof(int));
				log_debug(logger,"El pid es: %d", pid);

				memcpy(&direccion, datos->datos + sizeof(int), sizeof(int));
				log_debug(logger,"La direccion es: %d", direccion);

				bytesAEscribir = malloc((datos->tamanio - sizeof(int)*3)+1);

				log_debug(logger,"tamanio - sizeof(int) %d",datos->tamanio - sizeof(int)*3 );

				memcpy(bytesAEscribir, datos->datos + sizeof(int) + sizeof(int), datos->tamanio - (3 * sizeof(int)));
				log_debug(logger,"bytes a escribir %s", bytesAEscribir);

				memcpy(&tamanio, datos->datos + datos->tamanio - sizeof(int), sizeof(int));
				log_debug(logger,"El tamanio es: %d", tamanio);
				bytesAEscribir[tamanio] = '\0';

				log_info(logger,"Los parámetros que se recibieron son: %d, %d, %s, %d", pid, direccion, bytesAEscribir, tamanio);

				respuesta = escribirMemoria(pid, direccion, bytesAEscribir, tamanio);

				paquete = crear_paquete(0, (void*) &respuesta, sizeof(uint32_t));

				enviar_datos(*socket_conectado, paquete);

				free(bytesAEscribir);
				break;
			}

			free(datos->datos);
			free(datos);

		}

	}
}

T_PAGINA* swapInPagina(int PID, T_SEGMENTO* seg, T_PAGINA* pag) {

	char* filePath = obtenerFilePath(PID, seg->SID, pag->paginaID);
	FILE* archivoPagina = fopen(filePath,"r+b");

	if (archivoPagina != NULL ) {
		log_debug(logger,"el archivo es distinto de NULL");

		unsigned char data[tamanioPag];
		fread(data, 1 , sizeof(data), archivoPagina);
		log_debug(logger, "contenido de data %s", data);
		memcpy(pag->data, data, strlen(data)+1);

		//int r = fread(pag->data,1,tamanioPag,archivo);
		log_debug(logger, "contenido de data %s", pag->data);
		//log_debug(logger, "resultado de fread %d", r);
		pag->swapped = false;

		fclose(archivoPagina);

		remove(filePath);

		sem_wait(&mutex_cantSwap);
		cantidadSwap += tamanioPag;
		sem_post(&mutex_cantSwap);

	} else {
		log_error(logger, "No existe el archivo de la pagina swappeada");
		return NULL;
	}

	log_info(logger, "SwapIn: Se realizo el intercambio de la pagina de id %d desde disco", pag->paginaID);
	return pag;
}

int swapOutPagina(int PID, int SID, T_PAGINA* pag) {
	log_debug(logger, "Entre a swapOut");

	bool pagPorId(T_PAGINA* pagina) {
			return pagina->paginaID == pag->paginaID;
	}

	char* filePath = obtenerFilePath(PID, SID, pag->paginaID);
	log_debug(logger, "El nombre del archivo es: %s", filePath);
	FILE* archivo = fopen(filePath,"w+b");

	if (archivo != NULL ) {
		log_debug(logger,"Abrio/Creo el archivo");
		fwrite(pag->data,1,tamanioPag,archivo);
		log_debug(logger,"Escribio en el archivo");
		fclose(archivo);
		log_debug(logger,"Cerro el archivo");

		pag->swapped = true;
		pag->marcoID = -1;

		int i;
		for(i=0; i < tamanioPag; i++) {
			pag->data[i] = '\0';
		}

		cantidadSwap -= tamanioPag;
		if (cantidadSwap == 0){
			log_info(logger, "Se lleno el espacio de intercambio");
		}
		log_debug(logger,"El espacio de intercambio ahora es: %d", cantidadSwap);

	} else {
		log_error(logger, "No se pudo crear el archivo de la pagina a swappear");
		return error_general; //todo chequear
	}

	list_remove_by_condition(paginasEnMemoria,(void*) pagPorId);
	log_info(logger, "SwapOut: se realizo el intercambio de la pagina de id %d hacia disco", pag->paginaID);
	return operacion_exitosa;
}

char* obtenerFilePath(int PID, int SID, int pagId) {
	log_debug(logger,"Entre a obtenerFilePath");
	char* filePath = string_new();

	string_append(&filePath, rutaSwap);
	string_append(&filePath, string_itoa(PID));
	string_append(&filePath, string_itoa(SID));
	string_append(&filePath, string_itoa(pagId));

	log_debug(logger,"El filepath es: %s", filePath);
	return filePath;
}

T_MARCO* seleccionarMarcoVictima() {
	log_debug(logger,"Entre a seleccionar marco victimia");
	T_MARCO* marcoVictima;

	if (strcmp(sust_pags, "CLOCK") == 0) {
		log_debug(logger,"El algoritmo a implementar es clock");
		marcoVictima = algoritmoClock();
	} else if (strcmp(sust_pags, "LRU") == 0) {
		log_debug(logger,"El algoritmo a implementar es lru");
		marcoVictima = algoritmoLRU();
	}

	log_debug(logger,"El marco seleccionado tiene id: %d", marcoVictima->marcoID);
	return marcoVictima;
}

T_MARCO* algoritmoLRU() {
	log_debug(logger,"Entre a la funcion LRU");
	T_MARCO* marcoVictima;

	int marcoId;
	int contador = 100000;

	bool marcoPorId(T_MARCO* marco) {
		return marco->marcoID == marcoId;
	}

	int i;
	sem_wait(&mutex_paginasEnMemoria);
	int cantidadPaginasEnMemoria = list_size(paginasEnMemoria);

	for (i = 0; i < cantidadPaginasEnMemoria; i++) {

		T_PAGINA* pagina = list_get(paginasEnMemoria, i);

		if (pagina->contadorLRU < contador) {
			contador = pagina->contadorLRU;
			marcoId = pagina->marcoID;
		}
	}
	sem_post(&mutex_paginasEnMemoria);
	sem_wait(&mutex_marcosLlenos);
	//marcoVictima = list_find(marcosLlenos, (void*) marcoPorId);
	marcoVictima = list_remove_by_condition(marcosLlenos, (void*) marcoPorId);
	sem_post(&mutex_marcosLlenos);
	return marcoVictima;
}

T_MARCO* algoritmoClock() {
	log_debug(logger,"Entre al algoritmoClock");
	T_MARCO* marcoVictima;

	int marcoId = -1;

	bool marcoPorId(T_MARCO* marco) {
		return marco->marcoID == marcoId;
	}

	int i;

	sem_wait(&mutex_paginasEnMemoria);

	int cantidadPaginasEnMemoria = list_size(paginasEnMemoria);
	log_debug(logger,"cantidad de pags en memoria: %d", cantidadPaginasEnMemoria);

	for (i = 0; i < cantidadPaginasEnMemoria; i++) {

		T_PAGINA* pagina = list_get(paginasEnMemoria, i);
		log_debug(logger,"encontre la pagina en memoria de id: %d", pagina->paginaID);
		if (pagina->bitReferencia == 0) {
			log_debug(logger,"el bit de referencia es 0");
			log_debug(logger,"el marco id de la pag es: %d", pagina->marcoID);
			marcoId = pagina->marcoID;
			i = cantidadPaginasEnMemoria + 1;
		} else if (pagina->bitReferencia == 1) {
			log_debug(logger,"el bit de referencia es 1");
			pagina->bitReferencia = 0;
		}
	}
	//si sale del for y no encontro ningun cero vuelve a recorrer todas las paginas.
	if (marcoId == -1) {
		log_debug(logger,"todavia no encontro ninguna pagina con 0");
		for (i = 0; i < cantidadPaginasEnMemoria; i++) {

			T_PAGINA* pagina = list_get(paginasEnMemoria, i);

			if (pagina->bitReferencia == 0) {
				marcoId = pagina->marcoID;
				i = cantidadPaginasEnMemoria + 1;
			}
		}
	}

	sem_post(&mutex_paginasEnMemoria);
	sem_wait(&mutex_marcosLlenos);
	log_debug(logger,"tamaño de marcos llenos: %d", list_size(marcosLlenos));
	//marcoVictima = list_find(marcosLlenos, (void*) marcoPorId);
	marcoVictima = list_remove_by_condition(marcosLlenos, (void*) marcoPorId);
	log_debug(logger,"marco seleccionado de id %d", marcoVictima->marcoID);
	log_debug(logger,"tamaño de marcos llenos: %d", list_size(marcosLlenos));

	sem_post(&mutex_marcosLlenos);
	return marcoVictima;
}
