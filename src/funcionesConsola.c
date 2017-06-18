#include "funcionesConsola.h"

int obtenerDatosScript (consola_kernel_pid_request* consolaKernelRequest){

	  FILE *archivo;
	  char path[80]= "scripts/";
	  char letra;
	  int cantidadDeLetras = 0;

	  //Se abre el archivo
	  archivo = fopen(strcat(path,consolaKernelRequest->nombrePrograma), "r");

	  //Si hubo algun error, loguea y retorna instantaneamente
	  if(archivo==NULL){
		  log_error(logger, "funcionesConsola->obtenerDatosScript: Error al abrir el archivo %s",consolaKernelRequest->nombrePrograma);
		  return 0;
	  }

	  //Recorro el archivo para obtener la cantidad de letras/caracteres que contiene
	  while (!feof(archivo)) {
	   letra = getc(archivo);
	   cantidadDeLetras++;
	  }

	  rewind(archivo);
	  cantidadDeLetras--;

	  char* mensajeAEnviar = malloc(cantidadDeLetras + 1);
	  char* script = malloc(cantidadDeLetras);

	  //Copio el contenido del archivo en variables locales
	  fread(script, cantidadDeLetras, 1, archivo);
	  memcpy(mensajeAEnviar, script, cantidadDeLetras);
	  mensajeAEnviar[cantidadDeLetras] = '\0';

	  //Se asigna a la estructura los datos del script obtenidos
	  consolaKernelRequest->script = mensajeAEnviar;
	  consolaKernelRequest->longitudScript = cantidadDeLetras;

	  //Se cierra el archivo
	  fclose(archivo);

	  //Si hay script retorna true, caso contrario, false
	  if(consolaKernelRequest->script > 0){
	  	return 1;
	  } else{
	  	return 0;
	  }
  
}

int finalizarPrograma(int socket_cliente){

	//Se pide ingresar el PID correspondiente al programa que se desea finalizar
	uint32_t pid;
	printf("Ingresar PID del programa a finalizar: ");
	fflush(stdout);
	scanf("%d",&pid);

	//Malloc de variable mensajeHeader de tipo t_mensaje_HEADER*
	t_mensaje_HEADER* mensajeHeader = malloc(sizeof(t_mensaje_HEADER));

	//Malloc de variable consolaKernelPIDReques de tipo consola_kernel_pid_request*
	consola_kernel_pid_request* consolaKernelPIDRequest = malloc(sizeof(consola_kernel_pid_request));

	mensajeHeader->codigoMensaje = KERNEL_FINALIZAR_PROGRAMA;
	mensajeHeader->tamanio =  sizeof(typeof(consolaKernelPIDRequest->pid))
							+ sizeof(typeof(consolaKernelPIDRequest->longitudNombrePrograma))
							+ (sizeof((char)*consolaKernelPIDRequest->nombrePrograma))
							+ sizeof(typeof(consolaKernelPIDRequest->longitudScript))
							+ (sizeof((char)*consolaKernelPIDRequest->script));

	//Se envia mensaje header a Kernel con cod mensaje KERNEL_FINALIZAR_PROGRAMA para que sepa proxima operacion, ademas de tamanio
	if(enviar_mensaje_header(mensajeHeader,socket_cliente)){
		log_trace(logger,"funcionesConsola->finalizarPrograma: Mensaje header KERNEL_FINALIZAR_PROGRAMA enviado correctamente.");
	}else{
		log_error(logger,"funcionesConsola->finalizarPrograma: Error al enviar mensaje header KERNEL_FINALIZAR_PROGRAMA.");
		free(mensajeHeader);
		free(consolaKernelPIDRequest);
		return 0;
	}

	//Se asigna en la estructura a enviar el PID ingresado por teclado
	consolaKernelPIDRequest->pid = pid;

	//Se envia el PID al Kernel para que finalice el programa que corresponda
	if(enviar_KernelPIDRequest(consolaKernelPIDRequest,socket_cliente)){
		log_trace(logger,"funcionesConsola->finalizarPrograma: PID enviado correctamente.");
	}else{
		log_error(logger,"funcionesConsola->finalizarPrograma: Error al enviar PID.");
		free(mensajeHeader);
		free(consolaKernelPIDRequest);
		return 0;
	}

	if(recibir_mensaje_header(mensajeHeader, socket_cliente)){
		log_trace(logger,"funcionesConsola->finalizarPrograma: Mensaje header recibido.");
	}else{
		log_error(logger,"funcionesConsola->finalizarPrograma: Error al recibir mensaje header.");
		return 0;
	}

	if(mensajeHeader->codigoMensaje == CONSOLA_PROGRAMA_TERMINADO){
		log_trace(logger,"funcionesConsola->finalizarPrograma: Codigo mensaje CONSOLA_PROGRAMA_TERMINADO recibido y validado");
	}else{
		log_error(logger,"funcionesConsola->finalizarPrograma: Codigo mensaje %s incorrecto, se esperaba CONSOLA_PROGRAMA_TERMINADO", mensajeHeader->codigoMensaje);
		return 0;
	}

	//Funcion que busca el programa a eliminar de la lista, luego de que Kernel lo elimine de su sistema
	bool buscarProgramaPorPID(void *data) {
		t_program * unPrograma = (t_program *) data;
		return (unPrograma->pid == consolaKernelPIDRequest->pid);
	}

	//Busco el programa por PID
	pthread_mutex_lock(&mutexListaDeProcesos);
	t_program* programaEncontrado = list_find(listaDeProcesos,(void*)buscarProgramaPorPID);
	pthread_mutex_unlock(&mutexListaDeProcesos);

	//Mato el hilo correspondiente a ese PID
	pthread_exit(&programaEncontrado->pthread_id);

	//Remuevo la estructura de la lista
	pthread_mutex_lock(&mutexListaDeProcesos);
	list_remove_by_condition(listaDeProcesos,(void*)(programaEncontrado->pid == consolaKernelPIDRequest->pid));
	pthread_mutex_unlock(&mutexListaDeProcesos);

	//Se libera memoria de las estructuras malloc-eadas
	free(mensajeHeader);
	free(consolaKernelPIDRequest);

	return 1;
}

int desconectarConsola(int socket_cliente){

	//Malloc de mensajeHeader de tipo t_mensaje_HEADER*
	t_mensaje_HEADER* mensajeHeader = malloc(sizeof(t_mensaje_HEADER));

	//Se le asigna al mensaje header el codigo y tamaño
	mensajeHeader->codigoMensaje = KERNEL_DESCONECTAR_CONSOLA;
	mensajeHeader->tamanio = 0; //todo: revisar si este tamaño hace falta

	//Se envia mensaje header KERNEL_DESCONECTAR_CONSOLA para avisar a Kernel que se abortaran todos los programas
	if(enviar_mensaje_header(mensajeHeader,socket_cliente)){
		log_trace(logger,"funcionesConsola->desconectarConsola: Mensaje header KERNEL_DESCONECTAR_CONSOLA enviado correctamente.");
	}else{
		log_error(logger,"funcionesConsola->desconectarConsola: Error al enviar mensaje header KERNEL_DESCONECTAR_CONSOLA.");
		free(mensajeHeader);;
		return 0;
	}

	//todo: iterar, recibiendo codigo mensaje de kernel por cada programa que el mismo elimina e ir eliminando de consola si se debe

	//Malloc de programa de variable t_program*
	t_program* programa = malloc(sizeof(t_program));

	//Recorro la lista obteniendo el pthread_id y mato los hilos en ejecucion de cada programa
	int i = 0;
	pthread_mutex_lock(&mutexListaDeProcesos);
	while(i < list_size(listaDeProcesos)){
			programa = list_get(listaDeProcesos,i);
			pthread_exit(&programa->pthread_id);
			i++;
	}
	pthread_mutex_unlock(&mutexListaDeProcesos);

	//Se limpia y destruyen los elementos de la lista
	pthread_mutex_lock(&mutexListaDeProcesos);
	list_clean_and_destroy_elements(listaDeProcesos, NULL);
	pthread_mutex_unlock(&mutexListaDeProcesos);

	//Loguea si vació bien o no la lista de program_t
	pthread_mutex_lock(&mutexListaDeProcesos);
	if(list_is_empty(listaDeProcesos)){
		log_trace(logger,"funcionesConsola->desconectarConsola: Lista de programas vaciada satisfactoriamente");
	}else{
		log_error(logger,"funcionesConsola->desconectarConsola: Error al limpiar y destruir los elementos de la lista programas");

	}
	pthread_mutex_unlock(&mutexListaDeProcesos);

	//Se libera memoria
	free(mensajeHeader);
	free(programa);
	free(listaDeProcesos);

	return 1;
}


char* elegirComando(){

	char* comando = malloc(30);

	printf("------------ Menu de comandos disponibles ------------\n");
	printf("IniciarPrograma\n");
	printf("FinalizarPrograma\n");
	printf("DesconectarConsola\n");
	printf("LimpiarMensajes\n");
	printf("-------------------------------------------------------\n");
	printf("Ingrese un comando: ");
	fflush(stdout);
	scanf("%s",comando);

	return comando;
}


menuConsola_t buscarComando(char *comando) {

	//Array que contiene las opciones validas del menu
	char* menuConsola_s[] = {"IniciarPrograma", "FinalizarPrograma", "DesconectarConsola", "LimpiarMensajes"};

	//Inicializo una opcion del menu
	menuConsola_t opcion = INICIAR_PROGRAMA;

	//Busqueda del comando ingresado en estructuras para validar si existe, y a cual perteneces
	int i = 0;
	for (i=0;i<4;i++,opcion++) {
		if (strcmp(comando, menuConsola_s[i]) == 0) {
			return opcion;
		}
	}
	return -1;

}

void* iniciarPrograma(void* args){

	//Guardo y casteo el parametro enviado desde el hilo
	t_estructura_prueba* unaEstructuta = (t_estructura_prueba*) args;


	//Malloc de variable fileName de tipo char*
//	char* fileName = malloc(50);
//
//	//Se ingresa el script a ejecutar
//	printf("Ingresar file name script a ejecutar: ");
//	fflush(stdout);
//	scanf("%s",fileName);

	//Malloc de variable consolaKernelRequest de tipo consola_kernel_pid_request*
	consola_kernel_pid_request* consolaKernelRequest = malloc(sizeof(consola_kernel_pid_request));

	//Guardo datos en la estructura que voy a enviar al Kernel
	consolaKernelRequest->nombrePrograma =malloc(strlen(unaEstructuta->fileName)+1);
	strcpy(consolaKernelRequest->nombrePrograma,unaEstructuta->fileName);
	consolaKernelRequest->longitudNombrePrograma = strlen(unaEstructuta->fileName);
	consolaKernelRequest->pid = 0;

	//Obtengo el script pedido, su longitud y la guardo en la estructura consolaKernelRequest
	if(obtenerDatosScript(consolaKernelRequest)){
		log_trace(logger,"funcionesConsola->iniciarPrograma: Script AnSISOP obtenido correctamente.");
	}else{
		log_error(logger,"funcionesConsola->iniciarPrograma: Error al obtener script AnSISOP.");
		free(unaEstructuta->fileName);
		free(consolaKernelRequest);
		return 0;
	}

	//Funcion que imprime estructura consolaKernelRequest completa
	//imprimirConsolaKernelRequest(consolaKernelRequest);

	//Malloc de la estructura mensajeHeader
	t_mensaje_HEADER* mensajeHeader = malloc(sizeof(t_mensaje_HEADER));

	//Asigno codigo mensaje KERNEL_SOLICITAR_PID y el tamaño de la estructura a enviar posteriormente
	mensajeHeader->codigoMensaje = KERNEL_SOLICITAR_PID;
	mensajeHeader->tamanio =  sizeof(typeof(consolaKernelRequest->pid))
							+ sizeof(typeof(consolaKernelRequest->longitudNombrePrograma))
							+ (sizeof((char)*consolaKernelRequest->nombrePrograma))
							+ sizeof(typeof(consolaKernelRequest->longitudScript))
							+ (sizeof((char)*consolaKernelRequest->script));

	//Se envia mensaje header al Kernel con codigo mensaje KERNEL_SOLICITAR_PID
//	if(enviar_mensaje_header(mensajeHeader, socket_cliente)){
//		log_trace(logger,"funcionesConsola->iniciarPrograma: Mensaje header con codigo mensaje KERNEL_SOLICITAR_PID enviado correctamente.");
//	}else{
//		log_error(logger,"funcionesConsola->iniciarPrograma: Error al enviar mensaje header con código mensaje KERNEL_SOLICITAR_PID.");
//		free(fileName);
//		free(consolaKernelRequest);
//		free(mensajeHeader);
//		return 0;
//	}

	//Se envia la estructura consolaKernelRequest con los datos del programa
//	if(enviar_KernelPIDRequest(consolaKernelRequest, socket_cliente)){
//		log_trace(logger,"funcionesConsola->iniciarPrograma: KernelPIDRequest con datos programa enviado correctamente.");
//	}else{
//		log_error(logger,"funcionesConsola->iniciarPrograma: Error al enviar KernelPIDRequest con datos programa.");
//		free(fileName);
//		free(consolaKernelRequest);
//		free(mensajeHeader);
//		return 0;
//	}

	//Se recibe un mensaje header desde el Kernel
//	if(recibir_mensaje_header(mensajeHeader,socket_cliente)){
//		log_trace(logger,"funcionesConsola->iniciarPrograma: Mensaje header recibido.");
//	}else{
//		log_error(logger,"funcionesConsola->iniciarPrograma: Error al recibir mensaje header.");
//		free(fileName);
//		free(consolaKernelRequest);
//		free(mensajeHeader);
//		return 0;
//	}

	//Se pregunta si el codigo mensaje enviado por Kernel es el correcto o no
//	if(mensajeHeader->codigoMensaje==KERNEL_SOLICITAR_PID){
//		log_trace(logger,"funcionesConsola->iniciarPrograma: Se recibio un codigo mensaje header correcto.");
//	}else{
//		log_error(logger,"funcionesConsola->iniciarPrograma: Se recibio un codigo mensaje header incorrecto.");
//		free(fileName);
//		free(consolaKernelRequest);
//		free(mensajeHeader);
//		return 0;
//	}

	//Se recibe la estructura enviada al Kernel anteriormente, pero con el PID del proceso agregado
//	if(recibir_KernelPIDRequest(consolaKernelRequest,socket_cliente,mensajeHeader->tamanio)){
//		log_trace(logger,"funcionesConsola->iniciarPrograma: PID recibido correctamente.");
//	}else{
//		log_error(logger,"funcionesConsola->iniciarPrograma: Error al recibir PID del proceso.");
//		free(fileName);
//		free(consolaKernelRequest);
//		free(mensajeHeader);
//		return 0;
//	}

	//Malloc de la variable nuevoPrograma de tipo t_program*
	t_program* nuevoPrograma = malloc(sizeof(t_program));

	//Se guarda en una estructura de tipo program_t los datos necesarios del programa creado para Consola
	pthread_mutex_lock(&mutexListaDeProcesos);
	nuevoPrograma->pid=i;
	pthread_mutex_unlock(&mutexListaDeProcesos);
	nuevoPrograma->nombrePrograma=malloc(sizeof(unaEstructuta->fileName));
	strcpy(nuevoPrograma->nombrePrograma,unaEstructuta->fileName);
	nuevoPrograma->socket_fd = unaEstructuta->socket;
	nuevoPrograma->pthread_id = pthread_self();

	//Se agrega la estructura programa en una lista
	pthread_mutex_lock(&mutexListaDeProcesos);
	list_add(listaDeProcesos,(void*)nuevoPrograma);
	pthread_mutex_unlock(&mutexListaDeProcesos);

	pthread_mutex_lock(&mutexListaDeProcesos);
	i++;
	pthread_mutex_unlock(&mutexListaDeProcesos);

	//Se recibe un mensaje header desde el Kernel
//	if(recibir_mensaje_header(mensajeHeader,socket_cliente)){
//		log_trace(logger,"funcionesConsola->iniciarPrograma: Mensaje header recibido.");
//	}else{
//		log_error(logger,"funcionesConsola->iniciarPrograma: Error al recibir mensaje header.");
//		free(fileName);
//		free(consolaKernelRequest);
//		free(mensajeHeader);
//		free(nuevoPrograma);
//		return 0;
//	}

	//Se pregunta si el codigo mensaje enviado por Kernel es el correcto o no
//	if(mensajeHeader->codigoMensaje==CONSOLA_RECIBIR_MENSAJE){
//		log_trace(logger,"funcionesConsola->iniciarPrograma: Mensaje header recibido.");
//	}else{
//		log_error(logger,"funcionesConsola->iniciarPrograma: Se recibio un codigo mensaje header incorrecto.");
//		free(fileName);
//		free(consolaKernelRequest);
//		free(mensajeHeader);
//		free(nuevoPrograma);
//		return 0;
//	}

	//Malloc de la variable mensajeImprimir de tipo KERNEL_CONSOLA_imprimir*
	KERNEL_CONSOLA_imprimir * mensajeImprimir = malloc(sizeof(KERNEL_CONSOLA_imprimir));
	mensajeImprimir->pid=nuevoPrograma->pid;
	mensajeImprimir->mensaje=malloc(strlen("hola")+1);
	strcpy(mensajeImprimir->mensaje,"hola");

	//Si no se aborto la operacion (el codigo recibido de Kernel es correcto), se recibirá de Kernel la estructura a imprimir
//	if(recibir_KERNEL_CONSOLA_imprimir(mensajeImprimir,socket_cliente,mensajeHeader->tamanio)){
//		log_trace(logger,"funcionesConsola->iniciarPrograma: Mensaje imprimir recibido.");
//	}else{
//		log_error(logger,"funcionesConsola->iniciarPrograma: Error al recibir mensaje para imprimir proveniente del Kernel.");
//		free(fileName);
//		free(consolaKernelRequest);
//		free(mensajeHeader);
//		free(nuevoPrograma);
//		free(mensajeImprimir);
//		return 0;
//	}

	//todo: Preguntar a Ale sobre el CONSOLA_PROGRAMA_TERMINADO

	//Imprime por pantalla el mensaje recibido de Kernel obtenido de la ejecucion del script AnSISOP
	pthread_mutex_lock(&mutexImprimirMensaje);
	if(flag==0){
		sleep(10);
		flag=1;

	}
	pthread_mutex_unlock(&mutexImprimirMensaje);
	pthread_mutex_lock(&mutexImprimirMensaje);
	imprimirMensajePrograma(mensajeImprimir);
	pthread_mutex_unlock(&mutexImprimirMensaje);

	//Busco el proceso a eliminar de la lista, luego de imprimir su mensaje
	bool buscarProgramaPorPID(void *data) {
		t_program * unPrograma = (t_program *) data;
		return (unPrograma->pid == mensajeImprimir->pid);
	}
	pthread_mutex_lock(&mutexListaDeProcesos);
	printf("el tamaño de la lista es:%d",list_size(listaDeProcesos));
	pthread_mutex_unlock(&mutexListaDeProcesos);

	//Remuevo la estructura de la lista
	pthread_mutex_lock(&mutexListaDeProcesos);
	list_remove_by_condition(listaDeProcesos,buscarProgramaPorPID);
	pthread_mutex_unlock(&mutexListaDeProcesos);

	pthread_mutex_lock(&mutexListaDeProcesos);
		printf("el tamaño de la lista es:%d",list_size(listaDeProcesos));
		pthread_mutex_unlock(&mutexListaDeProcesos);

	//Se libera memoria de las estructuras malloc-eadas
	free(unaEstructuta);
	free(nuevoPrograma);
	free(consolaKernelRequest);
	free(mensajeHeader);
	free(mensajeImprimir);

	return NULL;
}

void imprimirConsolaKernelRequest(consola_kernel_pid_request* consolaKernelRequest){

	printf("------ Estructura enviar kernel ------\n");
	printf("%s\n", consolaKernelRequest->nombrePrograma);
	printf("%d\n", consolaKernelRequest->longitudNombrePrograma);
	printf("%s\n", consolaKernelRequest->script);
	printf("%d\n", consolaKernelRequest->longitudScript);
	printf("------------------------------------\n");

}

void imprimirMensajePrograma(KERNEL_CONSOLA_imprimir* mensajeImprimir){

	//todo: Agregar mas detalles: fecha, hora, tiempo de ejecucion
	printf("-----PID: %d -----\n", mensajeImprimir->pid);
	printf("%s\n", mensajeImprimir->mensaje);
	printf("------------------\n");

}
