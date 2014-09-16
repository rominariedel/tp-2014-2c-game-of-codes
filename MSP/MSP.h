/*
 * MSP.h
 *
 *  Created on: 16/09/2014
 *      Author: utnso
 */

#ifndef MSP_H_
#define MSP_H_

#include "stdint.h"


typedef struct T_PROCESO {
	int PID;
	t_list* segmentos;
} T_PROCESO;

typedef struct T_SEGMENTO {
	int SID;
	int tamanio;
	t_list* paginas;
} T_SEGMENTO;

typedef struct T_PAGINA {
	int pagainaID;
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

uint32_t* crearSegmento(int PID,int tamanio);

void destruirSegmento (int PID, uint32_t* baseSegmento);

uint32_t* solicitarMemoria(int PID, uint32_t* direccionLogica, int tamanio);

uint32_t* escribirMemoria(int PID, uint32_t* direccionLogica, int bytesAEscribir, int tamanio);



#endif /* MSP_H_ */
