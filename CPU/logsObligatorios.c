/*
 * logsObligatorios.c
 *
 *  Created on: 02/12/2014
 *      Author: utnso
 */

	#include "CPU.h"

void inicializar_panel(t_tipo_proceso tipo_proceso, char* path){
	char* tipo_proceso_str;

	if (tipo_proceso == KERNEL)
		tipo_proceso_str = "kernel";
	else if (tipo_proceso == CPU)
		tipo_proceso_str = "cpu";
	else
		tipo_proceso_str = "?";

	char* logFile = string_duplicate(path);
	string_append(&logFile, tipo_proceso_str);
	string_append(&logFile, ".log");

	remove(logFile);
	logger = log_create(logFile, tipo_proceso_str, true, LOG_LEVEL_INFO);

	log_info(logger, "Inicializando panel para %s, en \"%s\"", tipo_proceso_str, logFile);

	free(logFile);

	kernel_cpus_conectadas = list_create();
	kernel_consolas_conectadas = list_create();
}


void comienzo_ejecucion(t_hilo* hilo, uint32_t quantum) {
	char* mensaje = string_new();

	string_append_with_format(&mensaje, "Ejecuta hilo { PID: %d, TID: %d }", hilo->pid, hilo->tid);
	if (hilo->kernel_mode) string_append(&mensaje, " en modo kernel");

	log_info(logger, mensaje);
	free(mensaje);
}

void fin_ejecucion() {
	log_info(logger, "Empieza a estar iddle");
}

void ejecucion_instruccion(char* mnemonico, t_list* parametros) {
	char* mensaje = string_new();

	string_append_with_format(&mensaje, "Instrucción %s [", mnemonico);

	bool primero = true;
	void _imprimirParametro(char* parametro) {
		if (!primero) string_append(&mensaje, ", ");
		string_append_with_format(&mensaje, "%s", parametro);
		primero = false;
	}
	list_iterate(parametros, (void*) _imprimirParametro);

	string_append(&mensaje, "]");

	log_info(logger, mensaje);
	free(mensaje);
}

void cambio_registros(t_registros_cpu registros) {
	log_info(logger, "Registros: { A: %d, B: %d, C: %d, D: %d, E: %d, M: %d, P: %d, S: %d, K: %d, I: %d }",
		registros.registros_programacion[0],
		registros.registros_programacion[1],
		registros.registros_programacion[2],
		registros.registros_programacion[3],
		registros.registros_programacion[4],
		registros.M, registros.P, registros.S, registros.K, registros.I
	);
}

//-------------------------------------------------
//Retrocompatibilidad con el ejemplo del enunciado:
void ejecucion_hilo(t_hilo* hilo, uint32_t quantum) {
	comienzo_ejecucion(hilo, quantum);
}


char* ids_desde_lista(t_list* lista) {
	char* ids = string_new();

	void _escribirId(int* id) {
		string_append_with_format(&ids, "%d-", *id);
	}
	list_iterate(lista, (void*) _escribirId);

	if (!string_is_empty(ids)) ids[strlen(ids) - 1] = '\0';
	return ids;
}

char* hilos_desde_lista(t_list* lista) {
	char* resumen = string_new();

	void _imprimirTcb(t_hilo* hilo) {
		string_append_with_format(&resumen, "    { PID: %d, TID: %d, KM: %d, PC: %d, M: %d (%db) }\n", hilo->pid, hilo->tid, hilo->kernel_mode, hilo->puntero_instruccion, hilo->segmento_codigo, hilo->segmento_codigo_size);
		string_append_with_format(&resumen, "      { X: %d, S: %d, A: %d, B: %d, C: %d, D: %d, E: %d }", hilo->base_stack, hilo->cursor_stack, hilo->registros[0], hilo->registros[1], hilo->registros[2], hilo->registros[3], hilo->registros[4]);
	}
	list_iterate(lista, (void*) _imprimirTcb);

	return resumen;
}

void conexion_cpu(uint32_t id) {
	int* num = malloc(sizeof(uint32_t));
	memcpy(num, &id, sizeof(uint32_t));
	list_add(kernel_cpus_conectadas, num);

	char* ids = ids_desde_lista(kernel_cpus_conectadas);
	log_info(logger, "Nueva CPU (%d) => [%s]", id, ids);
	free(ids);
}
void desconexion_cpu(uint32_t id) {
	bool _esElId(int* _id) { return *_id == id; }
	list_remove_and_destroy_by_condition(kernel_cpus_conectadas, (void*) _esElId, free);

	char* ids = ids_desde_lista(kernel_cpus_conectadas);
	log_info(logger, "Desconexión CPU (%d) => [%s]", id, ids);
	free(ids);
}

void conexion_consola(uint32_t id) {
	int* num = malloc(sizeof(uint32_t));
	memcpy(num, &id, sizeof(uint32_t));
	list_add(kernel_consolas_conectadas, num);

	char* ids = ids_desde_lista(kernel_consolas_conectadas);
	log_info(logger, "Nueva Consola (%d) => [%s]", id, ids);
	free(ids);
}

void desconexion_consola(uint32_t id) {
	bool _esElId(int* _id) { return *_id == id; }
	list_remove_and_destroy_by_condition(kernel_consolas_conectadas, (void*) _esElId, free);

	char* ids = ids_desde_lista(kernel_consolas_conectadas);
	log_info(logger, "Desconexión Consola (%d) => [%s]", id, ids);
	free(ids);
}

void hilos(t_list* hilos) {
	char* resumen = string_new();

	string_append(&resumen, "Hilos ejecutando:\n");

	bool _esNew(t_hilo* hilo) { return hilo->cola == NEW; };
	bool _esReady(t_hilo* hilo) { return hilo->cola == READY; };
	bool _esExec(t_hilo* hilo) { return hilo->cola == EXEC; };
	bool _esBlock(t_hilo* hilo) { return hilo->cola == BLOCK; };
	bool _esExit(t_hilo* hilo) { return hilo->cola == EXIT; };

	void agregarResumen(char* nombre, bool (*filtro)(t_hilo*)) {
		string_append(&resumen, nombre);
		t_list* filtrada = list_filter(hilos, (void*) filtro);
		char* coso = hilos_desde_lista(filtrada);
		list_destroy(filtrada);
		string_append(&resumen, coso);
		free(coso);
	}

	agregarResumen("  NEW:\n", _esNew);
	agregarResumen("\n  READY:\n", _esReady);
	agregarResumen("\n  EXEC:\n", _esExec);
	agregarResumen("\n  BLOCK:\n", _esBlock);
	agregarResumen("\n  EXIT:\n", _esExit);

	log_info(logger, resumen);
	free(resumen);
}

void instruccion_protegida(char* mnemonico, t_hilo* hilo) {
	log_info(logger, "El hilo { PID: %d, TID: %d } ejecutó la instrucción %s", hilo->pid, hilo->tid, mnemonico);
}

