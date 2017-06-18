/*
 * funcionesConsola.h
 *
 *  Created on: 28/5/2017
 *      Author: utnso
 */

#ifndef SRC_FUNCIONESCONSOLA_H_
#define SRC_FUNCIONESCONSOLA_H_

#include "src/serialize-adm.h"

/*
 	 * menuConsola_t: estructura enum de comandos del menu de consola
*/
typedef enum t_menuConsola{
	INICIAR_PROGRAMA,
	FINALIZAR_PROGRAMA,
	DESCONECTAR_CONSOLA,
	LIMPIAR_MENSAJES
} menuConsola_t;

/*
 	 * t_program: estructura que almacena datos de cada programa que se inicia
 */
typedef struct program_t{
	uint32_t pid;
	char* nombrePrograma;
	uint32_t socket_fd;
	pthread_t pthread_id;
} t_program;


int i;
int flag;
/*
 	 * hiloPrograma: variable global hilo de programa nuevo
 */
pthread_t hiloPrograma;

/*
 	 * mutexImprimirMensaje: mutex para sincronizar impresion de mensaje por pantalla
 	 * mutexListaDeProcesos: mutex para sincronziar modificaciones en la lista de programas activos
 */
pthread_mutex_t mutexImprimirMensaje;
pthread_mutex_t mutexListaDeProcesos;

typedef struct estructura_prueba {
	uint32_t socket;
	char* fileName;
} t_estructura_prueba;

/*
 	 * listaDeProcesos: lista donde se almacenaran t_program (estructura de programas activos)
 */
t_list* listaDeProcesos;

/*
 	 * logger: variable global de log
 */
t_log *logger;

/*
	 * @NAME: obtenerDatosScript
	 * @DESC: Obtiene script AnSISOP y su longitud pasandole una estructura que contiene el nombre del programa
*/
int obtenerDatosScript(consola_kernel_pid_request*);


/*
	 * @NAME: elegirComando
	 * @DESC: Funcion que imprime y permite ingresar/elegir uno de los comandos a ejecutar por consola
*/
char* elegirComando();

/*
	 * @NAME: buscarComando
	 * @DESC: Recibe el comando elegido y lo busca para ver si es valido o no
*/
menuConsola_t buscarComando(char*);

/*
	 * @NAME: finalizarPrograma
	 * @DESC: Funcion que interactua con el Kernel para finalizar un programa con PID en especifico
*/
int finalizarPrograma(int);

/*
	 * @NAME: desconectarConsola
	 * @DESC: Funcion que avisa a Kernel y desconecta consola abruptamente
*/
int desconectarConsola(int);

/*
	 * @NAME: iniciarPrograma
	 * @DESC: Se llama a esta funcion cada vez que se crea un nuevo hilo. Inicia programa y envia/recibe datos con Kernel
*/
void* iniciarPrograma(void*);

/*
	 * @NAME: imprimirConsolaKernelRequest
	 * @DESC: Recibe una estructura de tipo consola_kernel_pid_request y la imprime por pantalla
*/
void imprimirConsolaKernelRequest(consola_kernel_pid_request*);

/*
	 * @NAME: imprimirMensajePrograma
	 * @DESC: Recibe una estructura de tipo KERNEL_CONSOLA_imprimir y la imprime por pantalla
*/
void imprimirMensajePrograma(KERNEL_CONSOLA_imprimir* mensajeImprimir);

#endif
