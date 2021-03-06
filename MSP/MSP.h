/*
 * MSP.h
 *
 *  Created on: 16/09/2014
 *      Author: utnso
 */

#ifndef MSP_H_
#define MSP_H_

#include "stdint.h"
#include <sockets.h>

enum mensajes {
	soy_CPU = 19,
	soy_kernel = 29,
	crear_segmento = 32,
	destruir_segmento = 30,
	solicitar_memoria = 31,
	escribir_memoria = 2,

	operacion_exitosa = 1,
	error_general = -1,
	error_segmentation_fault = -2,
	error_memoria_llena = -3,
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
	int SID;
	int PID;
	int swapped;
	int marcoID;
	int contadorLRU;
	int bitReferencia;
	char* data;
}T_PAGINA;

typedef struct T_MARCO {
	int marcoID;
	int empty;
	int PID;
	T_PAGINA* pagina;
}T_MARCO;

typedef struct T_DIRECCION_LOG {
	int SID;
	int paginaId;
	int desplazamiento;
}T_DIRECCION_LOG;

//Operaciones de interfaz
uint32_t 	crearSegmento		(int PID, int tamanio);
void		destruirSegmento 	(int PID, uint32_t baseSegmento);
char* 		solicitarMemoria	(int PID, uint32_t direccionLogica, int tamanio);
uint32_t 	escribirMemoria		(int PID, uint32_t direccionLogica, char * bytesAEscribir, int tamanio);

//Operaciones de consola
void 	inicializarConsola();
void 	interpretarComando (char* comando);
int 	tablaMarcos();
int 	tablaSegmentos();
int 	tablaPaginas(int PID);

//Funciones internas
void 		inicializar(char** args);
void 		cargarArchivoConfiguracion(char** args);
void 		crearMarcos();
T_SEGMENTO* crearSegmentoVacio(T_PROCESO* proceso, int tamanio);
int 		calcularProximoSID (T_PROCESO* proceso);
t_list* 	crearPaginasPorTamanioSegmento(int tamanio, int SID, int PID);
static void destruirPag(T_PAGINA* pagina);
int 		asignoMarcoAPagina(int PID, T_SEGMENTO* seg, T_PAGINA* pag);
void 		iniciarConexiones();
void		interpretarOperacion(int* socket);
void 		leoMemoria(T_PAGINA* pag);
void		escriboMemoria(T_PAGINA* pag, int inicio, int final, char* bytesAEscribir);
T_MARCO*	seleccionarMarcoVictima();
char*		obtenerFilePath(int PID, int SID, int paginaID);
T_PAGINA*	swapInPagina(int PID, T_SEGMENTO* seg, T_PAGINA* pag);
int			swapOutPagina(int PID, int SID, T_PAGINA* pag);
T_MARCO*	algoritmoLRU();
T_MARCO*	algoritmoClock();
void		cerrarMSP();

T_DIRECCION_LOG uint32ToDireccionLogica (uint32_t intDireccion);
uint32_t direccionLogicaToUint32 (T_DIRECCION_LOG direccionLogica);



#endif /* MSP_H_ */
