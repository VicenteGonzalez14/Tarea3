#include "tdas/extra.h"
#include "tdas/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Constantes para direcciones
typedef enum { ARRIBA, ABAJO, IZQUIERDA, DERECHA, NINGUNA } Direccion;

// Conversión dirección <-> string (opcional para imprimir)
const char* dir_to_str(Direccion d) {
    switch(d) {
        case ARRIBA: return "Arriba";
        case ABAJO: return "Abajo";
        case IZQUIERDA: return "Izquierda";
        case DERECHA: return "Derecha";
        default: return "Ninguna";
    }
}

// Ítem
typedef struct Item {
    char nombre[50];
    int peso;
    int valor;
    struct Item* siguiente;
} Item;

// arco
typedef struct Conexion {
    Direccion direccion;
    struct Escenario* destino;
    struct Conexion* siguiente;
} Conexion;



// nodo
typedef struct Escenario {
    int id; // identificador único
    char descripcion[256];
    Item* items;               // lista de ítems en este escenario
    Conexion* conexiones;      // lista de conexiones a otros escenarios
    struct Escenario* siguiente;  // para mantener lista de escenarios en el grafo
} Escenario;

// Grafo
typedef struct {
    Escenario* escenarios; // lista de todos los escenarios
    Escenario* inicio;     // escenario inicial
    Escenario* salida;     // escenario final
} Grafo;
Grafo grafo = { NULL, NULL, NULL };

typedef struct ConexionTemporal {
    int origen;
    int arriba;
    int abajo;
    int izquierda;
    int derecha;
    struct ConexionTemporal* siguiente;
} ConexionTemporal;
ConexionTemporal* conexionesTemp = NULL;


typedef struct {
    Item* inventario;    // Lista enlazada de ítems recogidos
    int peso_total;      // Peso acumulado del inventario
    int puntaje;         // Puntaje acumulado por ítems
    int tiempo_restante; // Tiempo que queda para jugar
    Escenario* escenario_actual; // Escenario donde está el jugador
} Jugador;

Jugador jugador;

Escenario* buscarEscenarioPorId(int id) {
    Escenario* actual = grafo.escenarios;
    while (actual != NULL) {
        if (actual->id == id) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

void agregarConexionTemporal(int origen, int arriba, int abajo, int izquierda, int derecha) {
    ConexionTemporal* c = malloc(sizeof(ConexionTemporal));
    c->origen = origen;
    c->arriba = arriba;
    c->abajo = abajo;
    c->izquierda = izquierda;
    c->derecha = derecha;
    c->siguiente = conexionesTemp;
    conexionesTemp = c;
}

Escenario* crearEscenario(int id, const char* descripcion) {
    Escenario* esc = malloc(sizeof(Escenario));
    esc->id = id;
    strncpy(esc->descripcion, descripcion, 255);
    esc->descripcion[255] = '\0';
    esc->items = NULL;
    esc->conexiones = NULL;
    esc->siguiente = NULL;

    // Insertar al inicio de la lista de escenarios del grafo
    esc->siguiente = grafo.escenarios;
    grafo.escenarios = esc;

    return esc;
}

int esUltimaStage(Escenario* actual) {
    if (actual == NULL || grafo.salida == NULL) return 0;
    return actual->id == grafo.salida->id;
}



void agregarConexion(Escenario* origen, Escenario* destino, Direccion dir) {
    Conexion* nueva = malloc(sizeof(Conexion));
    nueva->direccion = dir;
    nueva->destino = destino;
    nueva->siguiente = origen->conexiones;
    origen->conexiones = nueva;
}


void mostrarEstadoActual() {
    Escenario* esc = jugador.escenario_actual;
    if (!esc) {
        printf("Error: escenario actual no definido.\n");
        return;
    }

    printf("\n--- Escenario actual: %s ---\n", esc->descripcion);

    printf("Ítems disponibles:\n");
    if (esc->items == NULL) {
        printf("  (No hay ítems en este escenario)\n");
    } else {
        int idx = 1;
        for (Item* it = esc->items; it != NULL; it = it->siguiente, idx++) {
            printf("  %d) %s (Peso: %d, Valor: %d)\n", idx, it->nombre, it->peso, it->valor);
        }
    }

    printf("\nInventario (%d kg, %d pts):\n", jugador.peso_total, jugador.puntaje);
    if (jugador.inventario == NULL) {
        printf("  (Inventario vacío)\n");
    } else {
        int idx = 1;
        for (Item* it = jugador.inventario; it != NULL; it = it->siguiente, idx++) {
            printf("  %d) %s (Peso: %d, Valor: %d)\n", idx, it->nombre, it->peso, it->valor);
        }
    }

    printf("\nTiempo restante: %d\n", jugador.tiempo_restante);

    printf("Direcciones disponibles:\n");
    if (esc->conexiones == NULL) {
        printf("  (No hay direcciones para avanzar)\n");
    } else {
        for (Conexion* c = esc->conexiones; c != NULL; c = c->siguiente) {
            printf("  - %s\n", dir_to_str(c->direccion));
        }
    }
}

void inicializarJugador() {
    jugador.inventario = NULL;
    jugador.peso_total = 0;
    jugador.puntaje = 0;
    jugador.tiempo_restante = 10;  // o el tiempo que desees
    jugador.escenario_actual = grafo.inicio;
}


void liberarInventario() {
    Item* it = jugador.inventario;
    while (it) {
        Item* siguiente = it->siguiente;
        free(it);
        it = siguiente;
    }
    jugador.inventario = NULL;
}

void reiniciarPartida() {
    liberarInventario();
    inicializarJugador();
    printf("Partida reiniciada.\n");
}

void conectarEscenarios() {
    ConexionTemporal* c = conexionesTemp;
    while (c != NULL) {
        Escenario* origen = buscarEscenarioPorId(c->origen);
        if (!origen) {
            c = c->siguiente;
            continue;
        }

        if (c->arriba != -1) {
            Escenario* destino = buscarEscenarioPorId(c->arriba);
            if (destino) agregarConexion(origen, destino, ARRIBA);
        }
        if (c->abajo != -1) {
            Escenario* destino = buscarEscenarioPorId(c->abajo);
            if (destino) agregarConexion(origen, destino, ABAJO);
        }
        if (c->izquierda != -1) {
            Escenario* destino = buscarEscenarioPorId(c->izquierda);
            if (destino) agregarConexion(origen, destino, IZQUIERDA);
        }
        if (c->derecha != -1) {
            Escenario* destino = buscarEscenarioPorId(c->derecha);
            if (destino) agregarConexion(origen, destino, DERECHA);
        }

        c = c->siguiente;
    }
}

/**
 * Carga canciones desde un archivo CSV
 */
void leer_escenarios() {
    FILE *archivo = fopen("data/graphquest.csv", "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    char **campos;
    campos = leer_linea_csv(archivo, ','); // Leer encabezados y descartar
    if (campos) { list_clean(campos); free(campos); }

    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        int id = atoi(campos[0]);
        char* descripcion = campos[2];

        // Crear escenario si no existe
        Escenario* esc = buscarEscenarioPorId(id);
        if (!esc) {
            esc = crearEscenario(id, descripcion);
        }

        // Cargar ítems
        List* items = split_string(campos[3], ";");
        for (char* item = list_first(items); item != NULL; item = list_next(items)) {
            List* valores = split_string(item, ",");
            char* item_name = list_first(valores);
            int item_valor = atoi(list_next(valores));
            int item_peso = atoi(list_next(valores));
            agregarItem(esc, item_name, item_valor, item_peso);
            list_clean(valores);
            free(valores);
        }
        list_clean(items);
        free(items);

        int arriba = atoi(campos[4]);
        int abajo = atoi(campos[5]);
        int izquierda = atoi(campos[6]);
        int derecha = atoi(campos[7]);

        agregarConexionTemporal(id, arriba, abajo, izquierda, derecha);

        int is_final = atoi(campos[8]);
        if (is_final) grafo.salida = esc;
        if (strcmp(campos[1], "Entrada principal") == 0) grafo.inicio = esc;

        list_clean(campos);
        free(campos);
    }
    fclose(archivo);
}


int main() {
    char opcion;
    leer_escenarios();
    conectarEscenarios();
    inicializarJugador(); 
    do {
        puts("========================================");
        puts("             Menu del juego             ");
        puts("========================================");
        
        mostrarEstadoActual();

        puts("1) Recoger ítem(s)");
        puts("2) Descartar ítem(s)");
        puts("3) Avanzar en una Dirección");
        puts("4) Reiniciar Partida");
        puts("5) Salir");

        printf("Ingrese su opción: ");
        scanf(" %c", &opcion);

        switch (opcion) {
            case '1':
                recogerItems();
                break;
            case '2':
                descartarItems();
                break;
            case '3':
                avanzarDireccion();
                break;
            case '4':
                reiniciarPartida();
                break;
            case '5':
                printf("Saliendo del juego...\n");
                break;
            default:
                printf("Opción inválida, intenta de nuevo.\n");
        }

        printf("\nPresiona Enter para continuar...");
        getchar(); getchar();  // Pausa para que el usuario vea el resultado

    } while (opcion != '5');

    return 0;
}

