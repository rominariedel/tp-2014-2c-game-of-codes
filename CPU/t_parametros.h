/*
 * t_parametros.h
 *
 *  Created on: 01/11/2014
 *      Author: utnso
 */

#ifndef T_PARAMETROS_H_
#define T_PARAMETROS_H_

/*Tipos parametros instruccion*/

typedef struct{
	char reg1;
	int num;
}__attribute__ ((__packed__))  tparam_load;

typedef struct{
	int num;
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_setm;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_getm;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_movr;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_addr;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_subr;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_mulr;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_modr;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_divr;

typedef struct{
	char reg1;
}__attribute__ ((__packed__))  tparam_incr;

typedef struct{
	char reg1;
}__attribute__ ((__packed__))  tparam_decr;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_comp;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_cgeq;

typedef struct{
	char reg1;
	char reg2;
}__attribute__ ((__packed__))  tparam_cleq;

typedef struct{
	char reg1;
}__attribute__ ((__packed__))  tparam_goto;

typedef struct{
	int direccion;
}__attribute__ ((__packed__))  tparam_jmpz;

typedef struct{
	int direccion;
}__attribute__ ((__packed__))  tparam_jpnz;

typedef struct{
	int direccion;
}__attribute__ ((__packed__))  tparam_inte;

typedef struct{
	int numero;
	char registro;
}__attribute__ ((__packed__))  tparam_shif;

typedef struct{
	int numero;
	char registro;
}__attribute__ ((__packed__))  tparam_push;

typedef struct{
	int numero;
	char registro;
}__attribute__ ((__packed__))  tparam_take;

#endif /* T_PARAMETROS_H_ */
