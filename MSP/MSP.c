/*
 * main.c
 *
 *  Created on: 08/09/2014
 *      Author: utnso
 */


#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <math.h>
#include <MSP.h>

int tamanioMemoria;
int memoriaDisponible;
int puerto;
int cantidadSwap;
int sust_pags;
char* MSP_CONFIG;
int tamanioPag = 256;

t_list* procesos;
t_list* marcosVacios;
t_list* marcosLlenos;

pthread_t hiloConsola;

int main (void)
{
	inicializar();

	int hilo = pthread_create(&hiloConsola, NULL, inicializarConsola, NULL);
	if(hilo == 0){
		puts("La Consola de MSP se inicializó correctamente");
	}
	else puts("Ha ocurrido un error en la  inicialización de la Consola de MSP");

	pthread_join(hiloConsola, NULL);

	return 1; //todo Hacer las constantes globales exito o no

	/*t_config* configuracion = config_create(args[1]);
	logi = log_create(args[2], "UMV", 0, LOG_LEVEL_TRACE);
	int sizeMem = config_get_int_value(configuracion, "sizeMemoria");

	memPpal = malloc (sizeMem);	//El tamaño va por configuracion
	char* aux = (char*) memPpal;
	finMemPpal = memPpal + sizeMem; //El tamaño va por configuracion
	for(i=0; i<sizeMem; i++){aux[i] = 0;}

	rhConsola = pthread_create(&consola, NULL, mainConsola, NULL);
	rhEsperarConexiones = pthread_create(&esperarConexiones, NULL, mainEsperarConexiones, NULL);

	pthread_join(consola, NULL);
	pthread_join(esperarConexiones, NULL);

	printf("%d",rhConsola);
		printf("%d",rhEsperarConexiones);

		exit(0);*/
}

void inicializar(){
	cargarArchivoConfiguracion();
	crearMarcos();

	procesos = list_create();
}

void inicializarConsola(){
	char* comando[50];
	int seguimiento = 1;

	while(seguimiento){
		printf(">");
		fgets(comando,50,stdin);

		if((string_length(comando) > 0) && (comando[string_length(comando)]-1) == "\n"){
			comando[string_length(comando)] = '\O';
		}

		interpretarComando(comando);
		printf("\r\n");
	}

}

void interpretarComando (char* comando){
	char** palabras = string_split(comando,"");
	char** parametros;

	if (palabras[1] != NULL){
		parametros = string_split(palabras[1],",");
	}

	if (string_equals_ignore_casse(palabras[0],"Crear segmento")){
		printf("Creando segmento...");
		uint32_t baseSegmento = crearSegmento(atoi(parametros[0]), atoi (parametros[1]));
		printf("La dirección base del segmento creado es %d",baseSegmento);
	}

	else if (string_equals_ignore_casse(palabras[0],"Destruir Segmento")){
		printf("Destruyendo segmento...");
		int resultado = destruirSegmento(atoi(parametros[0]),atoi(parametros[1]));
		if(resultado == 1){
			printf ("El segmento ha sido detruido exitósamente");
		}
		else printf("El segmento no ha sido destruido, ha ocurrido un error de:");
		// todo switch case segun el error
	}

	else if (string_equals_ignore_casse(palabras[0],"Escribir Memoria")){

	}

	else if (string_equals_ignore_casse(palabras[0],"Leer Memoria")){

	}

	else if (string_equals_ignore_casse(palabras[0],"Tabla de Segmentos")){
		int resultado = tablaSegmentos();
	}

	else if (string_equals_ignore_casse(palabras[0],"Tabla de Páginas")){
		int resultado = tablaPaginas(atoi(parametros[0]));
	}

	else if (string_equals_ignore_casse(palabras[0],"Listar Marcos")){
		int resultado = tablaMarcos();
	}
}

void cargarArchivoConfiguracion(void){
	t_config* configuracion = config_create("config.txt");

	if(config_has_property(configuracion, "CANTIDAD_MEMORIA")){
		tamanioMemoria = config_get_int_value(configuracion, "CANTIDAD_MEMORIA") * pow(2,10);
		memoriaDisponible = tamanioMemoria;
		printf("Tamanio Memoria =  %d /n", tamanioMemoria);
	}

	if(config_has_property(configuracion,"PUERTO")){
		puerto= config_get_int_value(configuracion, "PUERTO");
		printf("Puerto =  %d /n", puerto);
	}

	if(config_has_property(configuracion,"CANTIDAD_SWAP")){
		cantidadSwap = config_get_int_value(configuracion, "CANTIDAD_SWAP");
		printf("Cantidad Swap =  %d /n", cantidadSwap);
	}

	if(config_has_property(configuracion,"SUST_PAGS")){
		sust_pags = config_get_int_value(configuracion, "SUST_PAGS");
		printf("Algoritmo de Sustitución de Páginas =  %d /n", sust_pags);
	}

}

void crearmarcos(){
	marcosLlenos = list_create();
	marcosVacios = list_create();

	int cantidadMarcos = tamanioMemoria / tamanioPag;
	int i;

	for(i=0;i < cantidadMarcos; i++) {
		list_add(marcosVacios, crearMarcoVacio(i));
	}
}

T_MARCO* crearMarcoVacio (int marcoId){
	T_MARCO marcoVacio = malloc(sizeof(T_MARCO));
	marcoVacio.marcoID = marcoId;
	return marcoVacio;
}

uint32_t* crearSegmento(int PID,int tamanio){

	if (tamanio > pow (2,20)){
	//todo validar que no pueda superar al mega de capacidad
	}

	if((memoriaDisponible < tamanio) || (cantidadSwap == 0) ){
		return ERROR_MEMORIA_LLENA;
	}

	bool procesoPorPid (T_PROCESO* proceso){
		return proceso->PID == PID;
	}

	T_PROCESO*  proceso = list_find(procesos, (void*) procesoPorPid);

	if (proceso == NULL){
		proceso->PID = PID;
		proceso->segmentos = list_create();

		list_add(procesos,proceso);
	}

	T_SEGMENTO seg = crearSegmentoVacio(proceso, tamanio);
	proceso->segmentos = list_add(seg);

	return seg.direccionVirtual;

}

T_SEGMENTO* crearSegmentoVacio (T_PROCESO proceso, int tamanio){

	T_SEGMENTO segVacio = malloc(sizeof(T_SEGMENTO));

	segVacio.SID = calcularProximoSID(proceso);
	segVacio.paginas = crearPagsPorTamanioSeg(tamanio);

	T_DIRECCION_LOG direccionLogica;
	direccionLogica.SID = segVacio->SID;
	direccionLogica.paginaId = 0;
	direccionLogica.desplazamiento = 0;

	segVacio.direccionVirtual = algoritmoParaConvertir(direccionLogica);
	// todo desarrollar el algoritmo para convertir la estructura T_DIRECCION_SEG
	// en la direccion uint32_t que es lo que reconoce el Kernal

	return segVacio;
}

int calcularProximoSID (T_PROCESO proceso){
	if (list_size(proceso->segmentos) == 0){
		return 0;
	}
	T_SEGMENTO* ultimoSegmento = list_get(proceso->segmentos, list_size(proceso->segmentos)-1);
	return  (ultimoSegmento->SID) + 1;
}

t_list* crearPagsPorTamanioSeg(int tamanio) {

	t_list* paginas = list_create();

	//todo necesario que la cantidadPaginas redondee para arriba
	int cantidadPaginas = (tamanio / tamanioPag);
	int i;

	for(i=0;  i < cantidadPaginas; i++){
		list_add(paginas,crearPaginaVacia(i));
	}

	return paginas;
}

T_PAGINA* crearPaginaVacia (int paginaID){
	T_PAGINA paginaVacia = malloc(sizeof(T_PAGINA));
	paginaVacia.paginaID = paginaID;

	int i;

	for(i=0;(tamanioPag -1) > i;i++){
		paginaVacia.data[i] = "";
	}

	paginaVacia.data[tamanioPag] = '\n';
	return paginaVacia;
}

int destruirSegmento (int PID, uint32_t* baseSegmento){

	bool procesoPorPid (T_PROCESO* proceso){
		return proceso->PID == PID;
	}

	bool segmentoPorBase (T_SEGMENTO* segmento){
		return segmento->direccionVirtual == baseSegmento;
	}

	//busco en la lista de procesos el proceso con ese PID
	T_PROCESO*  proceso = list_find(procesos,(void*) procesoPorPid);

	if(proceso != NULL){

		//busco en la lista de segmentos del proceso el segmento con esa base
		T_SEGMENTO seg = list_find(proceso->segmentos, (void*) segmentoPorBase);

		if (seg != NULL){

			//elimino las paginas del segmento
			list_clean_and_destroy_elements(seg->paginas, (void*) destruirPag);

			//elimino de la lista de segmentos del proceso, el segmento
			list_remove_by_condition(proceso->segmentos, (void*) segmentoPorBase);

			memoriaDisponible = memoriaDisponible + size(seg);
			free(seg);
		}

		else return ERROR_SEGMENTO_INEXISTENTE;
	}
	else return ERROR_PROCESO_INEXISTENTE;

	return OPERACION_EXITOSA;
}

static void destruirPag(T_PAGINA* pagina){
	free(pagina);
}

char* solicitarMemoria(int PID, uint32_t* direccionVirtual, int tamanio){

	//TODO if (direccionVirtual){
		//return ERROR_VIOLACION_DE_SEGMENTO_MEMORIA_INVALIDA;
	//}

	T_DIRECCION_LOG* direccionLogica = algoritmoParaConvertir(direccionVirtual);
		// todo desarrollar el algoritmo para convertir la estructura T_DIRECCION_SEG
		// en la direccion uint32_t que es lo que reconoce el Kernal

	bool procesoPorPid (T_PROCESO* proceso){
		return proceso->PID == PID;
	}
	bool segmentoPorSid (T_SEGMENTO* segmento){
		return segmento->SID == direccionLogica->SID;
	}
	bool paginaPorPagid (T_PAGINA* pagina){
		return pagina->paginaID == direccionLogica->paginaId;
	}
	bool marcoPorPagid (T_MARCO* marco){
		return marco->pagina->paginaID == direccionLogica->paginaId;
	}

	if ((direccionLogica->desplazamiento + tamanio) > tamanioPag ) {
		return ERROR_VIOLACION_DE_SEGMENTO_LIMITES_SEG_EXCEDIDOS;
	}

	T_PROCESO*  proceso = list_find(procesos, (void*) procesoPorPid);

	if(proceso != NULL){
		T_SEGMENTO* seg = list_find(proceso->segmentos, (void*) segmentoPorSid);

		if (seg != NULL){
			T_PAGINA* pag = list_find(seg->paginas, (void*) paginaPorPagid);

			if (pag != NULL){

				if (pag->marcoID == 0){

					if (marcosVacios->elements_count == 0){
						//todo swapping - ejecutar algoritmo de sustitucion de pags
					}
					else{
						//todo le asigno un marco a la pgina
					}
				}

				int inicio = direccionLogica->desplazamiento;
				int final = direccionLogica->desplazamiento + tamanio;
				int i;
				for(i = inicio; final > i  ; i++){
					return pag->data[i];
				}
			}
			else return ERROR_PAGINA_INEXISTENTE;
		}
		else return ERROR_SEGMENTO_INEXISTENTE;
	}
	else return ERROR_PROCESO_INEXISTENTE;

	return OPERACION_EXITOSA;
}

uint32_t* escribirMemoria(int PID, uint32_t* direccionLogica, int bytesAEscribir, int tamanio){
	return 0;
	//todo realizar operacion
}

int tablaMarcos(){
	printf("%s TABLA DE MARCOS %s",string_repeat("-",40));

	bool ordenarPorMenorId(T_MARCO* marco1, T_MARCO* marco2){
		return (marco1->marcoID < marco2->marcoID);
	}

	t_list* marcos = list_create();

	list_add_all(marcos,marcosLlenos);
	list_add_all(marcos,marcosVacios);

	list_sort(marcos,(void*) ordenarPorMenorId);

	int i;
	int cantidadMarcos = list_size(marcos);
	for(i=0;cantidadMarcos > i; i++){
		T_MARCO* marco = list_get(marcos,i);
		printf("Número de marco: %d",marco->marcoID);
		if (marco->empty){
			printf("Marco disponible");
		}
		else printf("Marco ocupado por el proceso: %d", marco->PID);
		printf("Información de los algoritmos de sustitución de páginas: %s", marco->alg_meta_data);
	}
	printf(string_repeat("-",100));
	return OPERACION_EXITOSA;
}

int tablaSegmentos(){
	printf("%s TABLA DE SEGMENTOS %s",string_repeat("-",40));

	int cantidadProcesos = list_size(procesos);
	int i;
	int j;

	for(i=0; cantidadProcesos > i; i++){
		T_PROCESO* proceso = list_get(procesos,i);
		int cantidadSegmentos = list_size(proceso->segmentos);

		printf("Para el proceso de ID: %d", proceso->PID);

		if(cantidadSegmentos == 0){
			printf("%c",'\n');
			printf("Este proceso no tiene segmentos");
		}

		for(j=0;cantidadSegmentos > j; j++){
			T_SEGMENTO* segmento = list_get(proceso->segmentos,j);
			printf("%c",'\n');
			printf("Número de segmento: %d", segmento->SID);
			printf("Tamaño: %d", segmento->tamanio);
			printf("Dirección virtual base: %d", segmento->direccionVirtual);
		}
	}
	printf(string_repeat("-",100));
	return OPERACION_EXITOSA;
}

int tablaPaginas(int PID){
	printf("%s TABLA DE PÁGINAS %s", string_repeat("-",40));

	bool procesoPorPid (T_PROCESO* proceso){
			return proceso->PID == PID;
		}

	T_PROCESO* proceso = list_find(procesos, (void*) procesoPorPid);

	if(proceso != NULL){
		int cantidadSegmentos = list_size(proceso->segmentos);
		int i;
		int j;

		for(i=0; cantidadSegmentos > i; i++){
			T_SEGMENTO* segmento = list_get(proceso->segmentos,i);
			printf("Para el segmento de ID: %d", segmento->SID);

			int cantidadPaginas = list_size(segmento->paginas);
			for(j=0; cantidadPaginas > j; j++){
				T_PAGINA* pagina = list_get(segmento->paginas, j);
				printf("%c", '\n');
				if(pagina->swapped){
					prtinf("La página se encuentra swappeada");
				}
				else if(pagina->marcoID != NULL){
					printf("La página se encuentra en memoria principal en el marco de ID: %d", pagina->marcoID);
				}
				else printf("La página no se encuentra en memoria principal");
			}
		}
	}
	else return ERROR_PROCESO_INEXISTENTE;

	printf("%c",'\n');
	printf(string_repeat("-",100));

	return OPERACION_EXITOSA;

}
