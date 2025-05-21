#include "tdas/extra.h"
#include "tdas/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

Escenario* buscarEscenarioPorId(int id) {
    Escenario* actual = grafo.escenarios;
    while (actual != NULL) {
        if (actual->id == id) return actual;
        actual = actual->siguiente;
    }
    return NULL;
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


void agregarConexion(Escenario* origen, Escenario* destino, Direccion dir) {
    Conexion* nueva = malloc(sizeof(Conexion));
    nueva->direccion = dir;
    nueva->destino = destino;
    nueva->siguiente = origen->conexiones;
    origen->conexiones = nueva;
}

void agregarItem(Escenario* esc, const char* nombre, int valor, int peso) {
    Item* nuevo = malloc(sizeof(Item));
    strncpy(nuevo->nombre, nombre, 49);
    nuevo->nombre[49] = '\0';
    nuevo->valor = valor;
    nuevo->peso = peso;
    nuevo->siguiente = esc->items;
    esc->items = nuevo;
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
    // Leer encabezado (y descartar)
    campos = leer_linea_csv(archivo, ',');
    if (campos) {
        list_clean(campos);
        free(campos);
    }

    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        int id = atoi(campos[0]);
        char* nombre = campos[1];
        char* descripcion = campos[2];

        // Crear escenario o buscar si ya existe
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

        // Guardar conexiones pendientes en una estructura temporal
        int arriba = atoi(campos[4]);
        int abajo = atoi(campos[5]);
        int izquierda = atoi(campos[6]);
        int derecha = atoi(campos[7]);

        // Para las conexiones, por ahora guardamos los IDs y las conectaremos al final
        // Guardamos en campos extra o estructura temporal
        // Mejor guardamos las conexiones pendientes en un array estático o dinámico (depende)

        // Marcar inicio y salida
        int is_final = atoi(campos[8]);
        if (is_final == 1) {
            grafo.salida = esc;
        }
        if (strcmp(nombre, "Entrada principal") == 0) {
            grafo.inicio = esc;
        }

        list_clean(campos);
        free(campos);
    }
    fclose(archivo);

    // Ahora conectamos los escenarios entre sí (tras crear todos)
    // Esta parte la debes implementar fuera de esta función, usando un segundo recorrido del archivo
}


int main() {
  leer_escenarios();
    char opcion;
    do {
        puts("========================================");
        puts("             Menu del juego             ");
        puts("========================================");
        pust("Estado Actual");
        
        puts("1)Recoger item(s)");
        puts("2)Descartar Ítem(s)");
        puts("3)Avanzar en una Dirección");
        puts("4)Reiniciar Partida"); 
        puts("5)Salir");
    
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
        presioneTeclaParaContinuar();
        limpiarPantalla();

  } while (opcion != '4');

  return 0;
}
