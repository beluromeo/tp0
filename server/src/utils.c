#include "utils.h"

t_log* logger; // Declaración global del logger

// Función para iniciar el servidor
int iniciar_servidor(t_log* logger)
{
    // Creación de los parámetros de conexión
    struct addrinfo hints;
    struct addrinfo* server_info;

    // Configuración de los parámetros de conexión
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;         // Protocolo IPv4
    hints.ai_socktype = SOCK_STREAM;    // Protocolo TCP
    hints.ai_flags = AI_PASSIVE;        // Escuchar en cualquier dirección

    // Obtención de la información del servidor
    int error_code = getaddrinfo(NULL, "4444", &hints, &server_info);
    if (error_code) {
        fprintf(stderr, "Error al crear socket (func getaddrinfo): %s [%d]\n", gai_strerror(error_code), error_code);
        exit(1);
    }

    // Creación del socket del servidor
    int socket_servidor = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if (socket_servidor == -1) {
        perror("Error al crear socket");
        exit(1);
    }

    // Asociación del socket con una dirección IP y un puerto
    if (bind(socket_servidor, server_info->ai_addr, server_info->ai_addrlen) == -1) {
        perror("Error al hacer bind");
        exit(1);
    }

    // Establecimiento del socket en modo escucha
    if (listen(socket_servidor, SOMAXCONN) == -1) {
        perror("Error al poner el socket en modo escucha");
        exit(1);
    }

    freeaddrinfo(server_info); // Liberación de la memoria de la estructura de información del servidor

    return socket_servidor; // Retorno del socket del servidor
}

// Función para esperar la conexión de un cliente
int esperar_cliente(int socket_servidor, t_log* logger)
{
    // Espera a que un cliente se conecte
    int socket_cliente = accept(socket_servidor, NULL, NULL);
    if (socket_cliente == -1) {
        perror("Error al aceptar conexión de cliente");
        exit(1);
    }

    log_info(logger, "Se conectó un cliente!");

    return socket_cliente; // Retorno del socket del cliente
}

// Función para recibir la operación enviada por el cliente
int recibir_operacion(int socket_cliente)
{
    int cod_op;
    if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
        return cod_op; // Retorno del código de operación recibido
    else {
        close(socket_cliente);
        return -1; // Retorno de -1 en caso de error
    }
}

// Función para recibir un buffer del cliente
void* recibir_buffer(int* size, int socket_cliente)
{
    void* buffer;

    // Recepción del tamaño del buffer
    recv(socket_cliente, size, sizeof(int), MSG_WAITALL);

    // Asignación de memoria para el buffer
    buffer = malloc(*size);

    // Recepción del buffer
    recv(socket_cliente, buffer, *size, MSG_WAITALL);

    return buffer; // Retorno del buffer recibido
}

// Función para recibir un mensaje del cliente
void recibir_mensaje(int socket_cliente)
{
    int size;
    char* buffer = recibir_buffer(&size, socket_cliente);
    printf(buffer); // Impresión del mensaje recibido en consola
    //log_info(logger, "Me llegó el mensaje %s", buffer); // Log del mensaje recibido
    free(buffer); // Liberación de la memoria del buffer
}

// Función para recibir un paquete del cliente
t_list* recibir_paquete(int socket_cliente)
{
    int size;
    int desplazamiento = 0;
    void* buffer;
    t_list* valores = list_create(); // Creación de la lista para almacenar los valores del paquete
    int tamanio;

    // Recepción del buffer
    buffer = recibir_buffer(&size, socket_cliente);

    // Procesamiento del buffer recibido
    while (desplazamiento < size) {
        // Lectura del tamaño del valor
        memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
        desplazamiento += sizeof(int);
        
        // Lectura del valor y almacenamiento en la lista
        char* valor = malloc(tamanio);
        memcpy(valor, buffer + desplazamiento, tamanio);
        desplazamiento += tamanio;
        list_add(valores, valor);
    }

    free(buffer); // Liberación de la memoria del buffer

    return valores; // Retorno de la lista con los valores del paquete
}
