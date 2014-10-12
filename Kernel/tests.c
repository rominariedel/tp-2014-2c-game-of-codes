/*
 * tests.c
 *
 *  Created on: 12/10/2014
 *      Author: utnso
 */

#include "auxiliares/auxiliares.h"

context(config){


	describe("archivo configuracion"){
		t_config * configuracion;

		before{
			configuracion = config_create("KERNEL_CONFIG");
		}end

		after{
			config_destroy(configuracion);
		}end

		it("configuracion contiene ip"){
			should_bool(config_has_property(configuracion, "IP")) be truthy;
		}end
	}end

}
