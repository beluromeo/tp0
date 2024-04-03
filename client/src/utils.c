#include "utils.h" // Incluye el archivo de cabecera utils.h donde se definen las funciones y estructuras necesarias

extern t_log* logger; // Declara una variable logger definida en otro archivo

// Función para serializar un paquete
void* serializar_paquete(t_paquete* paquete, int bytes) {
    void* magic = malloc(bytes); // Reserva memoria para almacenar los datos serializados
    int desplazamiento = 0; // Variable para rastrear la posición actual en el buffer

    // Copia el código de operación al buffer serializado
    memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
    desplazamiento += sizeof(int);

    // Copia el tamaño del buffer de datos al buffer serializado
    memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
    desplazamiento += sizeof(int);

    // Copia los datos del buffer al buffer serializado
    memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
    desplazamiento += paquete->buffer->size;

    return magic; // Retorna el buffer serializado
}

// Función para crear una conexión TCP con un servidor
int crear_conexion(char* ip, char* puerto) {
    struct addrinfo hints;
    struct addrinfo* server_info;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    // Obtener información del servidor
    int error_code = getaddrinfo(ip, puerto, &hints, &server_info);
    if(error_code) {
        log_error(logger, "Error al crear socket (func getaddrinfo): %s [%d]", gai_strerror(error_code), error_code);
        exit(1);
    }

    // Crear el socket
    int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    if(socket_cliente == -1) {
        log_error(logger, "Error al crear socket (func socket): %s [%d]", strerror(errno), errno);
        exit(1);
    }

    // Conectar al servidor
    int res = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
    if(res == -1) {
        log_error(logger, "Error al conectarse al server (func connect): %s [%d]", strerror(errno), errno);
        exit(1);
    }

    freeaddrinfo(server_info); // Liberar la estructura de información del servidor

    return socket_cliente; // Retorna el socket conectado
}

// Función para enviar un mensaje al servidor
void enviar_mensaje(char* mensaje, int socket_cliente) {
    t_paquete* paquete = malloc(sizeof(t_paquete)); // Reservar memoria para un nuevo paquete

    // Configurar el paquete con el código de operación MENSAJE y el mensaje proporcionado
    paquete->codigo_operacion = MENSAJE;
    paquete->buffer = malloc(sizeof(t_buffer));
    paquete->buffer->size = strlen(mensaje) + 1;
    paquete->buffer->stream = malloc(paquete->buffer->size);
    memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

    // Calcular el tamaño total del paquete
    int bytes = paquete->buffer->size + 2 * sizeof(int);

    // Serializar el paquete y enviarlo al servidor
    void* a_enviar = serializar_paquete(paquete, bytes);
    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar); // Liberar la memoria utilizada por el paquete serializado
    eliminar_paquete(paquete); // Liberar la memoria utilizada por el paquete
}

// Función para crear un nuevo paquete con el código de operación PAQUETE
t_paquete* crear_paquete(void) {
    t_paquete* paquete = malloc(sizeof(t_paquete)); // Reservar memoria para un nuevo paquete
    paquete->codigo_operacion = PAQUETE; // Establecer el código de operación del paquete
    crear_buffer(paquete); // Crear un nuevo buffer para el paquete
    return paquete; // Retorna el paquete creado
}

// Función para agregar datos al paquete
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio) {
    // Reasignar la memoria del stream del buffer para agregar nuevos datos
    paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

    // Copiar el tamaño del valor al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));

    // Copiar los datos al stream del buffer
    memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

    // Actualizar el tamaño total del buffer
    paquete->buffer->size += tamanio + sizeof(int);
}

// Función para enviar un paquete al servidor
void enviar_paquete(t_paquete* paquete, int socket_cliente) {
    int bytes = paquete->buffer->size + 2 * sizeof(int); // Calcular el tamaño total del paquete
    void* a_enviar = serializar_paquete(paquete, bytes); // Serializar el paquete

    // Enviar el paquete serializado al servidor
    send(socket_cliente, a_enviar, bytes, 0);

    free(a_enviar); // Liberar la memoria utilizada por el paquete serializado
}

// Función para eliminar un paquete y liberar la memoria utilizada
void eliminar_paquete(t_paquete* paquete) {
    free(paquete->buffer->stream); // Liberar la memoria utilizada por el stream del buffer
    free(paquete->buffer); // Liberar la memoria utilizada por el buffer
    free(paquete); // Liberar la memoria utilizada por el paquete
}

// Función para liberar la conexión con el servidor
void liberar_conexion(int socket_cliente) {
    close(socket_cliente); // Cerrar el socket
}

