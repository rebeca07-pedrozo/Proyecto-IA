#include "tablero.h"
#include <cstdlib>
#include <iostream>

// ─── Inicializar tablero y visitado en cero/false ────────────────────────────
void inicializar_tablero(int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
                         int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            tablero[i][j]  = VALOR_VACIO;
            visitado[i][j] = false;
        }
    }
}

// ─── Colocar N elementos de un tipo en posiciones aleatorias válidas ──────────
static void randomizar_actor(int tablero[][MAX_COLUMNAS],
                              int filas, int columnas,
                              int cantidad, int tipo,
                              int excluir_f, int excluir_c) {
    int colocados = 0;
    int intentos  = 0;
    const int MAX_INTENTOS = 10000;

    while (colocados < cantidad && intentos < MAX_INTENTOS) {
        int f = rand() % filas;
        int c = rand() % columnas;
        intentos++;

        // No colocar sobre la posición inicial del ratón ni sobre otra celda ocupada
        if ((f == excluir_f && c == excluir_c) || tablero[f][c] != VALOR_VACIO)
            continue;

        tablero[f][c] = tipo;
        colocados++;
    }
}

void colocar_actores(int tablero[][MAX_COLUMNAS],
                     int filas, int columnas) {
    // El ratón inicia en la esquina superior DERECHA (fila 0, columna última)
    int raton_f = 0;
    int raton_c = columnas - 1;

    randomizar_actor(tablero, filas, columnas, NUM_GATOS,   VALOR_GATO,   raton_f, raton_c);
    randomizar_actor(tablero, filas, columnas, NUM_TRAMPAS, VALOR_TRAMPA, raton_f, raton_c);
    randomizar_actor(tablero, filas, columnas, NUM_QUESOS,  VALOR_QUESO,  raton_f, raton_c);
}

// ─── Marcar celdas adyacentes (arriba, abajo, izq, der) como peligrosas ──────
static void marcar_adyacentes(int tablero[][MAX_COLUMNAS],
                               int filas, int columnas,
                               int f, int c, int tipo_peligro) {
    int df[] = {-1, 1,  0, 0};
    int dc[] = { 0, 0, -1, 1};

    for (int d = 0; d < 4; d++) {
        int nf = f + df[d];
        int nc = c + dc[d];
        if (nf >= 0 && nf < filas && nc >= 0 && nc < columnas) {
            // Solo marcar si la celda está vacía (no pisar actores)
            if (tablero[nf][nc] == VALOR_VACIO)
                tablero[nf][nc] = tipo_peligro;
        }
    }
}

void calcular_peligros(int tablero[][MAX_COLUMNAS],
                       int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            if (tablero[i][j] == VALOR_GATO)
                marcar_adyacentes(tablero, filas, columnas, i, j, VALOR_PELIGRO_GATO);
            else if (tablero[i][j] == VALOR_TRAMPA)
                marcar_adyacentes(tablero, filas, columnas, i, j, VALOR_PELIGRO_TRAMPA);
        }
    }
}

// ─── Imprimir el tablero con símbolos ─────────────────────────────────────────
void imprimir_tablero(int tablero[][MAX_COLUMNAS],
                      bool visitado[][MAX_COLUMNAS],
                      int filas, int columnas,
                      int turno,
                      int fila_raton, int col_raton) {
    system("clear");
    std::cout << "========================================\n";
    std::cout << "   RATON COME QUESO  |  Turno: " << turno << "\n";
    std::cout << "========================================\n\n";

    // Encabezado de columnas
    std::cout << "   ";
    for (int j = 0; j < columnas; j++) {
        std::cout << j % 10 << " ";
    }
    std::cout << "\n   ";
    for (int j = 0; j < columnas; j++) std::cout << "--";
    std::cout << "\n";

    for (int i = 0; i < filas; i++) {
        std::cout << i % 10 << " |";
        for (int j = 0; j < columnas; j++) {
            char c = '.';
            if (i == fila_raton && j == col_raton) {
                c = 'R';
            } else {
                switch (tablero[i][j]) {
                    case VALOR_VACIO:
                        c = visitado[i][j] ? '*' : '.'; break;
                    case VALOR_GATO:            c = 'G'; break;
                    case VALOR_TRAMPA:          c = 'T'; break;
                    case VALOR_QUESO:           c = 'Q'; break;
                    case VALOR_PELIGRO_GATO:    c = '!'; break;
                    case VALOR_PELIGRO_TRAMPA:  c = '~'; break;
                    default:                    c = '?'; break;
                }
            }
            std::cout << c << ' ';
        }
        std::cout << '\n';
    }

    std::cout << "\nConvenciones: R=Raton  G=Gato  T=Trampa  Q=Queso\n";
    std::cout << "             !=Peligro gato  ~=Peligro trampa  *=Visitado\n\n";
}

// ─── Tabla de percepciones del turno actual ───────────────────────────────────
void imprimir_tabla_percepciones(int tablero[][MAX_COLUMNAS],
                                 bool visitado[][MAX_COLUMNAS],
                                 int filas, int columnas,
                                 int fila_raton, int col_raton) {
    std::cout << "--------------------------------------------\n";
    std::cout << "        TABLA DE PERCEPCIONES\n";
    std::cout << "--------------------------------------------\n";
    std::cout << "Direccion  | Pos     | Contenido       | Visitado\n";
    std::cout << "--------------------------------------------\n";

    // Nombre, delta fila, delta columna
    const char* nombres[4] = {"Arriba   ", "Abajo    ", "Izquierda", "Derecha  "};
    int df[4] = {-1,  1,  0, 0};
    int dc[4] = { 0,  0, -1, 1};

    for (int d = 0; d < 4; d++) {
        int nf = fila_raton + df[d];
        int nc = col_raton  + dc[d];

        std::cout << nombres[d] << "  | ";

        if (nf < 0 || nf >= filas || nc < 0 || nc >= columnas) {
            std::cout << "  --   | Fuera del mapa   |   ---\n";
            continue;
        }

        std::cout << "(" << nf << "," << nc << ") | ";

        switch (tablero[nf][nc]) {
            case VALOR_VACIO:           std::cout << "Vacio          "; break;
            case VALOR_GATO:            std::cout << "GATO           "; break;
            case VALOR_TRAMPA:          std::cout << "TRAMPA         "; break;
            case VALOR_QUESO:           std::cout << "QUESO          "; break;
            case VALOR_PELIGRO_GATO:    std::cout << "Peligro gato   "; break;
            case VALOR_PELIGRO_TRAMPA:  std::cout << "Peligro trampa "; break;
            default:                    std::cout << "Desconocido    "; break;
        }

        std::cout << "  | " << (visitado[nf][nc] ? "Si" : "No") << "\n";
    }

    std::cout << "Posicion actual del raton: (" << fila_raton << "," << col_raton << ")\n";
    std::cout << "--------------------------------------------\n\n";
}

// ─── Tabla de acciones realizadas ─────────────────────────────────────────────
void imprimir_tabla_acciones(const char acciones[][30], int num_acciones) {
    std::cout << "--------------------------------------------\n";
    std::cout << "         TABLA DE ACCIONES\n";
    std::cout << "--------------------------------------------\n";
    std::cout << "#   | Accion\n";
    std::cout << "--------------------------------------------\n";

    for (int i = 0; i < num_acciones; i++) {
        std::cout << i + 1 << "   | " << acciones[i] << "\n";
    }

    std::cout << "Total de acciones: " << num_acciones << "\n";
    std::cout << "--------------------------------------------\n\n";
}