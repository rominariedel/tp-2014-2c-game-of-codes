/*
 * tests.c
 *
 *  Created on: 12/10/2014
 *      Author: utnso
 */

#include "auxiliares/auxiliares.h"
#include <cspecs/cspec.h>

context(config){


	describe("archivo configuracion"){
		t_config * configuracion;

		before{
			configuracion = config_create("KERNEL_CONFIG");
		}end

		after{
			//config_destroy(configuracion);
		}end

		it("configuracion contiene puerto"){
			should_bool(config_has_property(configuracion, "PUERTO")) be truthy;
		}end
	}end

	describe("colas de planificacion"){

		TCB_struct * tcb = malloc(sizeof(TCB_struct));
		reg_programacion * reg = malloc(sizeof(reg_programacion));
		tcb->registrosProgramacion = *reg;

		before{
			crear_colas();
			tcb->KM = 0;
			tcb->M = 0;
			tcb->P = 0;
			tcb->PID = obtener_PID();
			tcb->S = 0;
			tcb->TID = obtener_TID();
			tcb->X = 0;
			tcb->registrosProgramacion.A = 0;
			tcb->registrosProgramacion.B = 0;
			tcb->registrosProgramacion.C = 0;
			tcb->registrosProgramacion.D = 0;
			tcb->registrosProgramacion.E = 0;
		}end
		after{
			free_colas();
		}end
		it("exit no esta vacio"){
			planificar(*tcb);
			should_bool(queue_is_empty(EXIT)) be falsey;
		}end

		it("new quedo vacio"){
			planificar(*tcb);
			should_bool(queue_is_empty(NEW)) be truthy;
		}end

	}end

}
