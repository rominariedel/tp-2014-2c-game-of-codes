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

int tamanioMemoria;
int puerto;
int cantidadSwap;
int sust_pags;
char* MSP_CONFIG;
int tamanioPag = 256;

t_list* procesos;
t_list* marcosVacios;
t_list* marcosLlenos;

int main (void)
{
	cargarArchivoConfiguracion();
	crearMarcos();


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

void cargarArchivoConfiguracion(void){
	t_config* configuracion = config_create("config.txt");

	tamanioMemoria = config_get_int_value(configuracion, "CANTIDAD_MEMORIA") * pow(2,10);
	printf("Tamanio Memoria =  %d /n", tamanioMemoria);

	puerto= config_get_int_value(configuracion, "PUERTO");
	printf("Puerto =  %d /n", puerto);

	cantidadSwap = config_get_int_value(configuracion, "CANTIDAD_SWAP");
	printf("Cantidad Swap =  %d /n", cantidadSwap);

	sust_pags = config_get_int_value(configuracion, "SUST_PAGS");
	printf("Algoritmo de Sustitución de Páginas =  %d /n", sust_pags);
}

void crearmarcos(void){

	int cantidadMarcos = tamanioMemoria / tamanioPag;

	for(int i=0;i < cantidadMarcos; i++) {
		list_add(marcosVacios, crearMarcoVacio(i));
	}
}

T_MARCO* crearMarcoVacio (int marcoId){
	T_MARCO marcoVacio = malloc(sizeof(T_MARCO));
	marcoVacio.marcoID = marcoId;
	return marcoVacio;
}

uint32_t* crearSegmento(int PID,int tamanio){

	if (tamanio > pow (2,20)){
	//validar que no pueda superar al mega de capacidad
	}

	//validar que la MSP tenga espacio disponible o en espacio swapping - Error de memoria llena


	bool procesoPorPid (T_PROCESO* proceso){
		return proceso->PID == PID;
	}

	T_PROCESO*  proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso == NULL){
		proceso->PID = PID;
		proceso->segmentos = list_create();

		list_add(procesos,proceso);
	}

	T_SEGMENTO seg = crearSegmentoVacio(proceso, tamanio);
	proceso->segmentos = list_add(seg);

	return seg.direccionVirtual;

}

T_SEGMENTO* crearSegmentoVacio (T_PROCESO proceso, int tamanio){

	T_SEGMENTO segVacio = malloc(sizeof(T_SEGMENTO));

	segVacio.SID = calcularSID(proceso);
	segVacio.paginas = crearPagsPorTamanioSeg(tamanio);

	T_DIRECCION_LOG direccionLogica;
	direccionLogica.SID = segVacio->SID;
	direccionLogica.paginaId = 0;
	direccionLogica.desplazamiento = 0;

	segVacio.direccionVirtual = algoritmoParaConvertir(direccionLogica);
	// desarrollar el algoritmo para convertir la estructura T_DIRECCION_SEG
	// en la direccion uint32_t que es lo que reconoce el Kernal

	return segVacio;
}

int calcularSID (T_PROCESO proceso){

	//Teniendo en cuenta la lista de segmentos del proceso, busco el ultimo y le sumo 1
	// y devuelvo eso

	return  0;
}

t_list* crearPagsPorTamanioSeg(int tamanio) {

	t_list* paginas = list_create();

	//necesario que la cantidadPaginas redondee para arriba
	int cantidadPaginas = (tamanio / tamanioPag);

	for(int i=0;  i < cantidadPaginas; i++){
		list_add(paginas,crearPagVacia(i));
	}

	return paginas;
}

T_PAGINA* crearPagVacia (int paginaID){
	T_PAGINA paginaVacia = malloc(sizeof(T_PAGINA));
	paginaVacia.paginaID = paginaID;
	return paginaVacia;
}

void destruirSegmento (int PID, uint32_t* baseSegmento){

	bool procesoPorPid (T_PROCESO* proceso){
		return proceso->PID == PID;
	}

	bool segmentoPorBase (T_SEGMENTO* segmento){
		return segmento->direccionVirtual == baseSegmento;
	}

	//busco en la lista de procesos el proceso con ese PID
	T_PROCESO*  proceso = list_find(procesos,(void*) procesoPorPid);

	if(proceso != NULL){

		//busco en la lista de segmentos del proceso el segmento con esa base
		T_SEGMENTO seg = list_find(proceso->segmentos, (void*) segmentoPorBase);

		if (seg != NULL){

			//elimino las paginas del segmento
			list_clean_and_destroy_elements(seg->paginas, (void*) destruirPag);

			//elimino de la lista de segmentos del proceso, el segmento
			list_remove_by_condition(proceso->segmentos, (void*) segmentoPorBase);

			free(seg);
		}
	}

}

static void destruirPag(T_PAGINA* pagina){
	free(pagina);
}

uint32_t* solicitarMemoria(int PID, uint32_t* direccionVirtual, int tamanio){

	//validar posicion de memoria invalida o que exceda los limites del segmento

	T_DIRECCION_LOG* direccionLogica = algoritmoParaConvertir(direccionVirtual);
		// desarrollar el algoritmo para convertir la estructura T_DIRECCION_SEG
		// en la direccion uint32_t que es lo que reconoce el Kernal

	bool procesoPorPid (T_PROCESO* proceso){
		return proceso->PID == PID;
	}
	bool segmentoPorSid (T_SEGMENTO* segmento){
		return segmento->SID == direccionLogica->SID;
	}
	bool paginaPorPagid (T_PAGINA* pagina){
		return pagina->paginaID == direccionLogica->paginaId;
	}
	bool marcoPorPagid (T_MARCO* marco){
		return marco->pagina->paginaID == direccionLogica->paginaId;
	}

	if ((direccionLogica->desplazamiento + tamanio) > tamanioPag ) {
		//error segmentation fault (o pagina???????)
	}

	T_PROCESO*  proceso = list_find(procesos, (void*) procesoPorPid);

	if(proceso != NULL){
		T_SEGMENTO* seg = list_find(proceso->segmentos, (void*) segmentoPorSid);

		if (seg != NULL){
			T_PAGINA* pag = list_find(seg->paginas, (void*) paginaPorPagid);

			if (pag != NULL){
				T_MARCO* marco = list_find(marcosLlenos, (void*) marcoPorPagid);

				if (marco == NULL){

					if (marcosVacios->elements_count == 0){
						///swapping
					}
					else{
						//le asigno un marco a la pgina
					}
				}

				//return desde desplazamiento hasta despĺazamiento mas tamaño
				return 0;
			}
		}
	}


	return 0;
}
uint32_t* escribirMemoria(int PID, uint32_t* direccionLogica, int bytesAEscribir, int tamanio){
	return 0;
}


