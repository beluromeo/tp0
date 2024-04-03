#include "client.h" // Incluye el archivo de cabecera client.h donde se definen las funciones necesarias

// Declaración de variables globales para el logger y la configuración
t_log* logger;
t_config* config;

// Función principal del programa
int main(void) {
    int conexion;
    char* ip;
    char* puerto;
    char* valor;

    // INICIO LOGGER
    logger = iniciar_logger(); // Inicializa el logger y asigna el resultado a la variable global 'logger'

    // LEO CONFIGURACION
    config = iniciar_config(); // Inicializa la configuración y asigna el resultado a la variable global 'config'
   
    // Se obtienen los valores de la configuración (IP, PUERTO, CLAVE) y se verifica su existencia
    valor = config_get_string_value(config, "CLAVE");
    if(valor == NULL) {
        log_error(logger, "No se encontro 'CLAVE' en tp0.config");
        exit(1);
    }
    ip = config_get_string_value(config, "IP");
    if(ip == NULL) {
        log_error(logger, "No se encontro 'IP' en tp0.config");
        exit(1);
    }
    puerto = config_get_string_value(config, "PUERTO");
    if(puerto == NULL) {
        log_error(logger, "No se encontro 'PUERTO' en tp0.config");
        exit(1);
    }

    // Se imprime la configuración leída
    char msg_log[200];
    snprintf(msg_log, 200, "Config leida\n\tClave: %s\n\tIp: %s\n\tPuerto: %s\n", valor, ip, puerto);
    log_info(logger, msg_log);

    // ME CONECTO AL SERVER
    conexion = crear_conexion(ip, puerto); // Se establece la conexión con el servidor utilizando la IP y puerto obtenidos de la configuración

    // Enviamos al servidor el valor de CLAVE como mensaje
    enviar_mensaje(valor, conexion); // Se envía al servidor el valor de la clave obtenido de la configuración

    // LEO DE LA CONSOLA
    paquete(conexion); // Se lee desde la consola, se arma un paquete y se envía al servidor

    // Cierro conexion, destruyo config y logger y termino el programa
    terminar_programa(conexion); // Se liberan los recursos y se termina el programa

    /* PARTE 5 */
    // Proximamente
}

// Función para inicializar el logger
t_log* iniciar_logger() {
    t_log* logger = log_create("tp0.log", "client", true, LOG_LEVEL_INFO); // Se crea un nuevo logger
    if(logger == NULL) {
        fprintf(stderr, "No se pudo escribir el archivo de log tp0.log\n");
        exit(1);
    }
    return logger; // Se retorna el logger inicializado
}

// Función para inicializar la configuración
t_config* iniciar_config() {
    t_config* config = config_create("tp0.config"); // Se crea una nueva configuración
    if(config == NULL) {
        log_error(logger, "No se pudo leer el archivo de configuración tp0.config");
        exit(1);
    }
    return config; // Se retorna la configuración inicializada
}

// Función para leer texto desde la consola
void leer_consola(t_paquete* paquete) {
    while(true) {
        char* leido = readline("> "); // Se lee una línea de texto desde la consola
        if(strcmp(leido, "") == 0) { // Si se ingresa una línea vacía, se termina la lectura
            free(leido);
            return;
        }
        log_info(logger, leido); // Se registra la línea leída en el logger
        agregar_a_paquete(paquete, leido, strlen(leido) + 1); // Se agrega la línea al paquete
        free(leido);
    }
}

// Función para manejar el paquete
void paquete(int conexion) {
    t_paquete* paquete = crear_paquete(); // Se crea un nuevo paquete
    leer_consola(paquete); // Se lee texto desde la consola y se agrega al paquete
    enviar_paquete(paquete, conexion); // Se envía el paquete al servidor
    eliminar_paquete(paquete); // Se elimina el paquete después de enviarlo
}

// Función para terminar el programa
void terminar_programa(int conexion) {
    liberar_conexion(conexion); // Se libera la conexión con el servidor
    log_destroy(logger); // Se destruye el logger
    config_destroy(config); // Se destruye la configuración
    exit(0); // Se termina el programa
}


