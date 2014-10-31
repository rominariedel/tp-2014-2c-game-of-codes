/*
 * MSP.h
 *
 *  Created on: 16/09/2014
 *      Author: utnso
 */

#ifndef MSP_H_
#define MSP_H_

#include "stdint.h"

enum mensajes {
	soy_CPU = 19,
	soy_kernel = 18,
	crear_segmento = 1,
	destruir_segmento =2,
	solicitar_memoria = 3,
	escribir_memoria = 4,
};

typedef struct T_PROCESO {
	int PID;
	t_list* segmentos;
} T_PROCESO;

typedef struct T_SEGMENTO {
	int SID;
	int tamanio;
	t_list* paginas;
	uint32_t baseSegmento;
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
uint32_t 	crearSegmento		(int PID, int tamanio);
uint32_t	destruirSegmento 	(int PID, uint32_t baseSegmento);
char* 		solicitarMemoria	(int PID, uint32_t direccionLogica, int tamanio);
uint32_t 	escribirMemoria		(int PID, uint32_t direccionLogica, char * bytesAEscribir, int tamanio);

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
T_SEGMENTO* crearSegmentoVacio (T_PROCESO proceso, int tamanio);
int 		calcularProximoSID (T_PROCESO* proceso);
t_list* 	crearPaginasPorTamanioSegmento(int tamanio);
static void destruirPag(T_PAGINA* pagina);
void 		asignoMarcoAPagina(int PID, T_MARCO* marcoAsignado, T_PAGINA* pag);
void 		actualizarMarcos();

T_DIRECCION_LOG uint32ToDireccionLogica (uint32_t intDireccion);
uint32_t DireccionLogicaToUint32 (T_DIRECCION_LOG direccionLogica);

#endif /* MSP_H_ */
