/*
 * instruccionesCPU.h
 *
 *  Created on: 06/11/2014
 *      Author: utnso
 */

#ifndef INSTRUCCIONESCPU_H_
#define INSTRUCCIONESCPU_H_



/*Instrucciones*/
void LOAD(tparam_load*);
void SETM(tparam_setm*);
void GETM(tparam_getm*);
void MOVR(tparam_movr*);
void ADDR(tparam_addr*);
void SUBR(tparam_subr*);
void MULR(tparam_mulr*);
void MODR(tparam_modr*);
void DIVR(tparam_divr*);
void INCR(tparam_incr*);
void DECR(tparam_decr*);
void COMP(tparam_comp*);
void CGEQ(tparam_cgeq*);
void CLEQ(tparam_cleq*);
void GOTO(tparam_goto*);
void JMPZ(tparam_jmpz*);
void JPNZ(tparam_jpnz*);
void INTE(tparam_inte*);
void SHIF(tparam_shif*);
void NOPP();
void PUSH(tparam_push*);
void TAKE(tparam_take*);
void XXXX();

/*Instrucciones Protegidas*/
void MALC();
void FREE();
void INNN();
void INNC();
void OUTN();
void OUTC();
void CREA();
void JOIN();
void BLOK();
void WAKE();


#endif /* INSTRUCCIONESCPU_H_ */
