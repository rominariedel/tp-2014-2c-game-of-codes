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


int main() {
  CU_initialize_registry();

  CU_pSuite suiteCPU = CU_add_suite("Suite de suiteCPU", NULL, NULL);
  CU_add_test(suiteCPU, "CargarRegistrosCPU", testCargarRegistrosCPU);
  CU_add_test(suiteCPU, "CargarArchivoConfiguracion", testCargarArchivoConfiguracion);


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
//hola
}

