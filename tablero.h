#ifndef TABLERO_H
#define TABLERO_H

// ─── Constantes de valores del tablero ───────────────────────────────────────
// Cada celda del tablero guarda un entero que representa qué hay en ella
const int VALOR_VACIO          = 0; // Celda libre
const int VALOR_RATON          = 1; // Posicion actual del raton
const int VALOR_GATO           = 2; // Gato (peligro mortal)
const int VALOR_TRAMPA         = 3; // Trampa (peligro mortal)
const int VALOR_QUESO          = 4; // Objetivo del raton
const int VALOR_PELIGRO_GATO   = 5; // Celda adyacente a un gato   → simbolo '!'
const int VALOR_PELIGRO_TRAMPA = 6; // Celda adyacente a una trampa → simbolo '~'

// ─── Constantes de estado del juego ──────────────────────────────────────────
const int ESTADO_EN_CURSO = 0; // El juego sigue
const int ESTADO_VICTORIA = 1; // El raton llego al queso
const int ESTADO_GATO     = 2; // El raton choco con un gato
const int ESTADO_TRAMPA   = 3; // El raton cayo en una trampa
const int ESTADO_LIMITE   = 4; // Se agotaron los turnos maximos

// ─── Tamaño maximo del tablero ────────────────────────────────────────────────
// Se declara estaticamente con el maximo posible.
// El tamaño REAL usado en cada partida se pasa como parametro (filas, columnas)
const int MAX_FILAS    = 20;
const int MAX_COLUMNAS = 20;

// ─── Cantidad de actores en el juego ─────────────────────────────────────────
const int NUM_GATOS   = 5;
const int NUM_TRAMPAS = 4;
const int NUM_QUESOS  = 1;

// ─── Tamaño maximo del historial de sensaciones y acciones ───────────────────
const int MAX_TURNOS = 500;

// ─── Estructura de una sensacion por turno ───────────────────────────────────
// Cada turno se guarda una fila en la matriz de sensaciones
struct Sensacion {
    int  turno;          // Numero de turno
    int  fila_raton;     // Fila donde esta el raton
    int  col_raton;      // Columna donde esta el raton
    int  arriba;         // Valor de la celda de arriba
    int  abajo;          // Valor de la celda de abajo
    int  izquierda;      // Valor de la celda izquierda
    int  derecha;        // Valor de la celda derecha
    bool visitado_arr;   // Si la celda de arriba ya fue visitada
    bool visitado_aba;   // Si la celda de abajo ya fue visitada
    bool visitado_izq;   // Si la celda izquierda ya fue visitada
    bool visitado_der;   // Si la celda derecha ya fue visitada
};
// ─── Maximo de veces que el raton puede visitar una celda antes de penalizarla
const int MAX_VISITAS_CELDA = 3;
// ─── Funciones del tablero ────────────────────────────────────────────────────
void inicializar_tablero(int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
                         int conteo[][MAX_COLUMNAS],
                         int filas, int columnas);

void colocar_actores(int tablero[][MAX_COLUMNAS],
                     int filas, int columnas);

void calcular_peligros(int tablero[][MAX_COLUMNAS],
                       int filas, int columnas);

void imprimir_tablero(int tablero[][MAX_COLUMNAS],
                      bool visitado[][MAX_COLUMNAS],
                      int filas, int columnas,
                      int turno,
                      int fila_raton, int col_raton);

// Registra la sensacion del turno actual en la matriz de sensaciones
void registrar_sensacion(Sensacion sensaciones[], int &num_sensaciones,
                         int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
                         int filas, int columnas,
                         int turno, int fila_raton, int col_raton);

// Imprime toda la matriz de sensaciones acumulada
void imprimir_matriz_sensaciones(Sensacion sensaciones[], int num_sensaciones);

void imprimir_tabla_acciones(const char acciones[][30], int num_acciones);

// Convierte un valor entero del tablero a su nombre legible
const char* nombre_valor(int valor);

#endif