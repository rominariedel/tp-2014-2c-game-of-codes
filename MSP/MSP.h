/*
 * MSP.h
 *
 *  Created on: 16/09/2014
 *      Author: utnso
 */

#ifndef MSP_H_
#define MSP_H_

#include "stdint.h"

// Declaración de variables globales
#define OPERACION_EXITOSA 1
#define OPERACION_ERRONEA 0

#define ERROR_MEMORIA_LLENA 								10
#define ERROR_VIOLACION_DE_SEGMENTO_MEMORIA_INVALIDA 		11
#define ERROR_VIOLACION_DE_SEGMENTO_LIMITES_SEG_EXCEDIDOS 	12
#define ERROR_PROCESO_INEXISTENTE 							13
#define ERROR_SEGMENTO_INEXISTENTE 							14
#define ERROR_PAGINA_INEXISTENTE 							15



typedef struct T_PROCESO {
	int PID;
	t_list* segmentos;
} T_PROCESO;

typedef struct T_SEGMENTO {
	int SID;
	int tamanio;
	t_list* paginas;
	uint32_t* direccionVirtual;
} T_SEGMENTO;

typedef struct T_PAGINA {
	int paginaID;
	int swapped;
	int marcoID;
	char data [256];
}T_PAGINA;

typedef struct T_MARCO {
	int marcoID;
	int empty;
	int PID;
	T_PAGINA* pagina;
	char* alg_meta_data;
}T_MARCO;

typedef struct T_DIRECCION_LOG {
	int SID;
	int paginaId;
	int desplazamiento;
}T_DIRECCION_LOG;


//Operaciones de interfaz
uint32_t* 	crearSegmento		(int PID, int tamanio);
int 		destruirSegmento 	(int PID, uint32_t baseSegmento);
int 		solicitarMemoria	(int PID, uint32_t direccionLogica, int tamanio);
int 	escribirMemoria		(int PID, uint32_t direccionLogica, char * bytesAEscribir, int tamanio);

//Operaciones de consola
void 	inicializarConsola();
void 	interpretarComando (char* comando);
int 	tablaMarcos();
int 	tablaSegmentos();
int 	tablaPaginas(int PID);

//Funciones internas
void 		inicializar();
void 		cargarArchivoConfiguracion(void);
void 		crearMarcos();
T_MARCO* 	crearMarcoVacio (int marcoId);
T_SEGMENTO* crearSegmentoVacio (T_PROCESO proceso, int tamanio);
int 		calcularProximoSID (T_PROCESO proceso);
t_list* 	crearPagsPorTamanioSeg(int tamanio);
T_PAGINA* 	crearPaginaVacia (int paginaID);
static void destruirPag(T_PAGINA* pagina);
void 		asignoMarcoAPagina(int PID, T_MARCO* marcoAsignado, T_PAGINA* pag);
void 		actualizarMarcos();

T_DIRECCION_LOG uint32ToDireccionLogica (uint32_t intDireccion);
uint32_t DireccionLogicaToUint32 (T_DIRECCION_LOG direccionLogica);

#endif /* MSP_H_ */
