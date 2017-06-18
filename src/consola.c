/*
 * consola.c
 *
 *  Created on: 05/04/2017
 *      Author: Cristopher
 */

#include "consola.h"

int main (int argc, char* argv[]){

	i=0;
	flag=0;
	//Malloc de estructuras a usar
	consola_config* configConsola = malloc(sizeof(consola_config));
	t_mensaje_HANDSHAKE * mensaje_handshake;
	t_estructura_prueba* unaEstructuta;

	//Inicializo mutex de imprimir mensaje y lista de procesos
	pthread_mutex_init(&mutexImprimirMensaje,NULL);
	pthread_mutex_init(&mutexListaDeProcesos,NULL);

	//Se crea una instancia del logger
	logger = log_create(CONSOLA_PATH_LOG, PROGRAM_NAME, false, LOG_LEVEL_TRACE);

	//Se crea la lista que va a contener a las estructuras de los diferentes programas al iniciarse
	listaDeProcesos = list_create();

	//Se obtienen las propiedades de configuración
	cargarConfiguracionConsola(CONSOLA_PATH_CONFIG, configConsola, logger);

	//Se crea el socket de tipo cliente
//	int socket_cliente = socket_create(configConsola->ip, configConsola->puerto, TIPO_CLIENTE, 0, logger);
int socket_cliente=1;
	//Se envía handshake.Si pasa las validaciones, se debe ingresar comando por teclado y esperar una respuesta
	if (socket_cliente != 0) {
		log_trace(logger, "Main: Socket cliente creado.");
		mensaje_handshake = malloc(sizeof(t_mensaje_HANDSHAKE));
		mensaje_handshake->codigoHandshake = HANDSHAKE_CONSOLA;
//		if (enviar_handshake(mensaje_handshake,socket_cliente)) {
			log_trace(logger,"Main: Se envio un mensaje de handshake correcto.");
	//		if (recibir_handshake(mensaje_handshake,socket_cliente)) {
//				if (mensaje_handshake->codigoHandshake == HANDSHAKE_CONSOLA) {
					log_trace(logger,"Main: Se recibio un mensaje de handshake correcto.");
					int iterar = 1;
					while(iterar == 1){
						char* comandoElegido = elegirComando();
						switch(buscarComando(comandoElegido)){
							case INICIAR_PROGRAMA:
								 unaEstructuta= malloc(sizeof(t_estructura_prueba));
								char* fileName = malloc(50);

								//Se ingresa el script a ejecutar
								printf("Ingresar file name script a ejecutar: ");
								fflush(stdout);
								scanf("%s",fileName);
								unaEstructuta->fileName=malloc(strlen(fileName)+1);
								strcpy(unaEstructuta->fileName,fileName);
								unaEstructuta->socket=socket_cliente;
								//Se crea un hilo nuevo por cada INICIAR_PROGRAMA
								if(pthread_create(&hiloPrograma,NULL,&iniciarPrograma,(void*)unaEstructuta) < 0){
									log_error(logger,"Main: Error al crear hilo nuevo programa.");
								}else{
									log_trace(logger,"Main: Hilo programa creado correctamente.");
								}
							break;

							case FINALIZAR_PROGRAMA:
								 if(finalizarPrograma(socket_cliente)){
									 log_trace(logger,"Main: Programa finalizado correctamente.");
								 }else{
									 log_error(logger,"Main: Error al finalizar programa.");
							     }
							break;

							case DESCONECTAR_CONSOLA:
								if(desconectarConsola(socket_cliente)){
									log_trace(logger,"Main: Consola desconectada correctamente.");
									iterar = 0;
								}else{
									log_error(logger,"Main: Error al desconectar consola.");
								}
								break;

							case LIMPIAR_MENSAJES:
								//Se limpia la pantalla
								system("clear");
								break;

							default:
								log_error(logger,"Main: No se ingreso un comando valido.");
								break;
							}
					}
//				} else {
//					log_error(logger,"Main: Se recibio un mensaje de handshake incorrecto.");
//				}
//			} else {
//				log_error(logger,"Main: Fallo recepcion de handshake de Kernel.");
//			}
//		} else {
//			log_error(logger,"Main: Fallo envio de handshake a Kernel, se cierra conexion.");
//		}
	} else {
		log_trace(logger,"Main: No se pudo crear el socket cliente.");
	}

	//Libero memoria y cierro socket
	socket_close(socket_cliente);
	free(mensaje_handshake);
	consola_config_destroy(configConsola, logger);
	log_destroy(logger);

	return 0;

}
