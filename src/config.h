/*
 * config.h
 *
 *  Created on: 05/04/2017
 *      Author: Cristopher
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/log.h>

typedef struct {
	char* ip;
	char* puerto;
} consola_config;

/*
 	 * CANTIDAD_PARAM_CONFIG: Cantidad de parametros en el archivo de configuracion
 */
#define CANTIDAD_PARAM_CONFIG 2

/*
	 * @NAME: cargarConfiguracionConsola
	 * @DESC: Obtiene y retorna desde un archivo de config la configuracion para iniciar la consola
*/
int cargarConfiguracionConsola(char*, consola_config*, t_log*);

/*
	 * @NAME: consola_config_destroy
	 * @DESC: Recibe una estructura tipo consola_config, libera memoria y la destruye
*/
void consola_config_destroy(consola_config *, t_log*);

#endif
