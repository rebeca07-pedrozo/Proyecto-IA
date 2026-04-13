#ifndef TABLERO_H
#define TABLERO_H

// ─── Constantes de valores del tablero ───────────────────────────────────────
const int VALOR_VACIO   = 0;
const int VALOR_RATON   = 1;
const int VALOR_GATO    = 2;
const int VALOR_TRAMPA  = 3;
const int VALOR_QUESO   = 4;
const int VALOR_PELIGRO_GATO    = 5;
const int VALOR_PELIGRO_TRAMPA  = 6;

// ─── Constantes de estado del juego ──────────────────────────────────────────
const int ESTADO_EN_CURSO = 0;
const int ESTADO_VICTORIA = 1;
const int ESTADO_GATO     = 2;
const int ESTADO_TRAMPA   = 3;
const int ESTADO_LIMITE   = 4; // Se agotaron los turnos

// ─── Límites del tablero (máximo posible) ────────────────────────────────────
const int MAX_FILAS    = 20;
const int MAX_COLUMNAS = 20;

// ─── Cantidad de actores ──────────────────────────────────────────────────────
const int NUM_GATOS   = 5;
const int NUM_TRAMPAS = 4;
const int NUM_QUESOS  = 1;

// ─── Funciones del tablero ────────────────────────────────────────────────────
void inicializar_tablero(int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
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

void imprimir_tabla_percepciones(int tablero[][MAX_COLUMNAS],
                                 bool visitado[][MAX_COLUMNAS],
                                 int filas, int columnas,
                                 int fila_raton, int col_raton);

void imprimir_tabla_acciones(const char acciones[][30], int num_acciones);

#endif