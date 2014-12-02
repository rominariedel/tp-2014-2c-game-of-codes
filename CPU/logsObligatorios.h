/*
 * logsObligatorios.h
 *
 *  Created on: 02/12/2014
 *      Author: utnso
 */

#ifndef LOGSOBLIGATORIOS_H_
#define LOGSOBLIGATORIOS_H_


typedef enum { KERNEL, CPU } t_tipo_proceso;
typedef enum { NEW, READY, EXEC, BLOCK, EXIT } t_cola;

typedef struct {
	uint32_t pid;
	uint32_t tid;
	bool kernel_mode;
	uint32_t segmento_codigo;
	uint32_t segmento_codigo_size;
	uint32_t puntero_instruccion;
	uint32_t base_stack;
	uint32_t cursor_stack;
	int32_t registros[5];
	t_cola cola;
} t_hilo;


	t_log* logger;

	/*
	 * Las funciones declaradas en los headers panel.h, cpu.h,
	 * y kernel.h deben ser invocadas a modo de notificación de
	 * los eventos que representan.
	 */

	/**
	 * Debe invocarse luego de inicializar el proceso en cuestión (CPU o Kernel).
	 * Por ejemplo: inicializar_panel(CPU, "/home/utnso/panel");
	 *
	 * @param  tipo_proceso Indica si es un CPU o un Kernel.
	 * @param  path  Es la ruta absoluta a un directorio donde el panel pueda
	 *               generar archivos de log libremente. El directorio debe existir.
	 **/
	void inicializar_panel(t_tipo_proceso tipo_proceso, char* path);


	typedef struct {
		int32_t registros_programacion[5]; //A, B, C, D y E
		uint32_t M; //Base de segmento de código
		uint32_t P; //Puntero de instrucción
		uint32_t X; //Base del segmento de Stack
		uint32_t S; //Cursor de stack
		uint32_t K; //Kernel Mode
		uint32_t I; //PID
	} t_registros_cpu;

	/**
	 * Debe invocarse cada vez que un hilo ingrese a la CPU.
	 *
	 * @param  hilo  Estructura conteniendo todos los campos del TCB del hilo.
	 * @param  quantum  Tamaño del Quantum.
	 */
	void comienzo_ejecucion(t_hilo* hilo, uint32_t quantum);

	/**
	 * Debe invocarse cada vez que un hilo salga de la CPU.
	 */
	void fin_ejecucion();

	/**
	 * Debe invocarse cada vez se vaya a ejecutar una instrucción.
	 * Por ejemplo: ejecucion_instruccion("ABCD", "soy", 1, "parametro");
	 *
	 * @param  mnemonico  Nombre de la instrucción a ejecutar.
	 * @param  parametros  Parametros de la instrucción. a ejecutar.
	 */
	void ejecucion_instruccion(char* mnemonico, t_list* parametros);

	/**
	 * Debe invocarse cada vez que ocura algún cambio en alguno de los
	 * registros de la CPU (una vez por instruccion a ejecutar, luego de
	 * llamar a ejecucion_instruccion()).
	 *
	 * @param  registros  Estructura conteniendo cada uno de los registros de la CPU.
	 */
	void cambio_registros(t_registros_cpu registros);

	//-------------------------------------------------
	//Retrocompatibilidad con el ejemplo del enunciado:
	void ejecucion_hilo(t_hilo* hilo, uint32_t quantum);

	t_list* kernel_cpus_conectadas;
		t_list* kernel_consolas_conectadas;

		/**
		* Debe invocarse tras conectarse una CPU.
		*
		* @param  id  ID de la CPU.
		*/
		void conexion_cpu(uint32_t id);

		/**
		* Debe invocarse tras desconectarse una CPU.
		*
		* @param  id  ID de la CPU.
		*/
		void desconexion_cpu(uint32_t id);

		/**
		* Debe invocarse tras conectarse una consola de un programa.
		*
		* @param  id  ID de la consola.
		*/
		void conexion_consola(uint32_t id);

		/**
		* Debe invocarse tras desconectarse una consola de un programa.
		*
		* @param  id  ID de la CPU.
		*/
		void desconexion_consola(uint32_t id);

		/**
		* Debe invocarse cada vez que ocurra algún cambio en algun campo
		* del TCB de un hilo.
		*
		*/
		void hilos(t_list* hilos);

		/**
		* Debe invocarse cada vez que la CPU invoque una funcionalidad definida
		* en la sección del enunciado "Servicios expuestos a la CPU".
		*
		* @param  mnemonico  Nombre de la funcionalidad.
		* @param  hilo Estructura TCB del hilo que invocó la funcionalidad.
		*/
		void instruccion_protegida(char* mnemonico, t_hilo* hilo);




#endif /* LOGSOBLIGATORIOS_H_ */
