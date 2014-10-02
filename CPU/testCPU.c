/*
 * testCPU.c
 *
 *  Created on: 02/10/2014
 *      Author: utnso
 */

#include "CPU.c"
#include "CUnit/Basic.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void inicializar(){
	TCBactual -> registrosProgramacion[0] = 1;
	TCBactual -> registrosProgramacion[1] = 2;
	TCBactual -> registrosProgramacion[2] = 3;
	TCBactual -> registrosProgramacion[3] = 4;
	TCBactual -> registrosProgramacion[4] = 5;
	PIDactual = 123;
	TIDactual = 456;
	KMactual = 1;
	baseSegmentoCodigoActual = &A;
	tamanioSegmentoCodigoActual = 789;
	punteroInstruccionActual = &B;
	baseStackActual = &C;
	cursorStackActual = D;
}

void limpiar(){
	TCBactual -> registrosProgramacion[0] = 0;
	TCBactual -> registrosProgramacion[1] = 0;
	TCBactual -> registrosProgramacion[2] = 0;
	TCBactual -> registrosProgramacion[3] = 0;
	TCBactual -> registrosProgramacion[4] = 0;
	PIDactual = 0;
	TIDactual = 0;
	KMactual = 0;
	baseSegmentoCodigoActual = NULL;
	tamanioSegmentoCodigoActual = 0;
	punteroInstruccionActual = NULL;
	baseStackActual = 0;
	cursorStackActual = NULL;
}


int main() {
  CU_initialize_registry();

  CU_pSuite suiteCPU = CU_add_suite("Suite de suiteCPU", inicializar(), limpiar());
  CU_add_test(suiteCPU, "CargarArchivoConfiguracion", testCargarArchivoConfiguracion);
  CU_add_test(suiteCPU, "CargarRegistrosCPU", testCargarRegistrosCPU);

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();

  return CU_get_error();
}

void testCargarArchivoConfiguracion(){

	cargarArchivoConfiguracion();
	CU_ASSERT_PTR_EQUAL(IPKERNEL, "192.168.1.103");
	CU_ASSERT_EQUAL(PUERTOKERNEL, 12345);
	CU_ASSERT_PTR_EQUAL(IPMSP, "192.168.1.104");
	CU_ASSERT_EQUAL(PUERTOMSP, 54321);
	CU_ASSERT_EQUAL(RETARDO,800);
}

void testCargarRegistrosCPU(){
	inicializar();
	cargarRegistrosCPU();
	CU_ASSERT_PTR_EQUAL(A,1);
	CU_ASSERT_PTR_EQUAL(B,2);
	CU_ASSERT_PTR_EQUAL(C,3);
	CU_ASSERT_PTR_EQUAL(D,4);
	CU_ASSERT_PTR_EQUAL(E,5);
}


void testActualizarRegistrosTCB(){
	inicializar();
	actualizarRegistrosTCB();
	CU_ASSERT_PTR_EQUAL(TCBactual -> registrosProgramacion[0], *A);
	CU_ASSERT_PTR_EQUAL(TCBactual -> registrosProgramacion[1], *B);
	CU_ASSERT_PTR_EQUAL(TCBactual -> registrosProgramacion[2], *C);
	CU_ASSERT_PTR_EQUAL(TCBactual -> registrosProgramacion[3], *D);
	CU_ASSERT_PTR_EQUAL(TCBactual -> registrosProgramacion[4], *E);
}

/*

TCBactual -> registrosProgramacion[0] = 1;
TCBactual -> registrosProgramacion[1] = 2;
TCBactual -> registrosProgramacion[2] = 3;
TCBactual -> registrosProgramacion[3] = 4;
TCBactual -> registrosProgramacion[4] = 5;
PIDactual = 123;
TIDactual = 456;
KMactual = 1;
baseSegmentoCodigoActual = &A;
tamanioSegmentoCodigoActual = 789;
punteroInstruccionActual = &B;
baseStackActual = &C;
cursorStackActual = D;*/

/*

void actualizarRegistrosTCB(){
	TCBactual -> registrosProgramacion[0] = *A;
	TCBactual -> registrosProgramacion[1] = *B;
	TCBactual -> registrosProgramacion[2] = *C;
	TCBactual -> registrosProgramacion[3] = *D;
	TCBactual -> registrosProgramacion[4] = *E;
}

int cargarDatosTCB(){
	TIDactual = TCBactual -> TID;
	KMactual = TCBactual -> KM;
	baseSegmentoCodigoActual = TCBactual -> baseSegmentoCodigo;
	tamanioSegmentoCodigoActual = TCBactual -> tamanioSegmentoCodigo;
	punteroInstruccionActual = TCBactual -> punteroInstruccion;
	baseStackActual = TCBactual -> baseStack;
	cursorStackActual = TCBactual -> cursorStack;
	cargarRegistrosCPU();
	return 0;
}

int actualizarTCB(){
	TCBactual -> TID = TIDactual;
	TCBactual -> KM = KMactual;
	TCBactual -> baseSegmentoCodigo = baseSegmentoCodigoActual;
	TCBactual -> tamanioSegmentoCodigo = tamanioSegmentoCodigoActual;
	TCBactual -> punteroInstruccion = punteroInstruccionActual;
	TCBactual -> baseStack = baseStackActual;
	TCBactual -> cursorStack = cursorStackActual;
	actualizarRegistrosTCB();
	return 0;
}

*/
