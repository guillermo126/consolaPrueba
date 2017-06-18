/*
 * config.c
 *
 *  Created on: 05/04/2017
 *      Author: Cristopher
 */

#include "config.h"

t_config* tConfig;

int cargarConfiguracionConsola(char* archivoRuta, consola_config* configConsola, t_log* logger) {

	tConfig = config_create(archivoRuta);

	//Se verifica que el archivo exista en la ruta indicada
	if (tConfig == NULL) {
		log_error(logger, "config->cargarConfiguracionConsola: No se encuentra el archivo de configuración en la ruta \t%s.", archivoRuta);
		return EXIT_FAILURE;
	}
	//Se verifica que el archivo tenga la cantidad de parametros correcta
	if (config_keys_amount(tConfig) == CANTIDAD_PARAM_CONFIG) {

		//Se verifica el parametro IP_KERNEL
		if (config_has_property(tConfig, "IP_KERNEL")) {
			configConsola->ip = config_get_string_value(tConfig, "IP_KERNEL");
		} else {
			log_error(logger, "config->cargarConfiguracionConsola: Falta el parametro: IP KERNEL.");
			config_destroy(tConfig);
			return EXIT_FAILURE;
		}

		//Se verifica el parametro PUERTO_KERNEL
		if (config_has_property(tConfig, "PUERTO_KERNEL")) {
			configConsola->puerto = config_get_string_value(tConfig, "PUERTO_KERNEL");
		} else {
			log_error(logger, "config->cargarConfiguracionConsola: Falta el parametro: PUERTO KERNEL.");
			config_destroy(tConfig);
			return EXIT_FAILURE;
		}

		//Se imprime el contenido del archivo de configuración
		printf("Datos archivo config de la consola:\t");
		printf("IP_KERNEL: %s", configConsola->ip);
		printf("\tPUERTO_KERNEL: %s\n", configConsola->puerto);
		return EXIT_SUCCESS;
	} else {
		log_error(logger, "config->cargarConfiguracionConsola: El archivo consola.cfg no tiene los %d campos que debería.", CANTIDAD_PARAM_CONFIG);
		config_destroy(tConfig);
		return EXIT_FAILURE;
	}

}

void consola_config_destroy(consola_config *config, t_log* logger){

	free(config->ip);
	free(config->puerto);
	free(config);

	log_trace(logger,"config->consola_config_destroy: Estructura config de consola liberada exitosamente.");

}


