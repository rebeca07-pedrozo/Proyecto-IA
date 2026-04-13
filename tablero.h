#ifndef TABLERO_H
#define TABLERO_H

// ─── Constantes de valores del tablero ───────────────────────────────────────
// Cada celda del tablero guarda un entero que representa qué hay en ella
const int VALOR_VACIO          = 0; // Celda libre sin contenido
const int VALOR_RATON          = 1; // Posicion actual del raton en el tablero
const int VALOR_GATO           = 2; // Gato: si el raton llega aqui pierde
const int VALOR_TRAMPA         = 3; // Trampa: si el raton llega aqui pierde
const int VALOR_QUESO          = 4; // Queso: objetivo del raton, ganar al llegar
const int VALOR_PELIGRO_GATO   = 5; // Celda adyacente a un gato   → simbolo '!'
const int VALOR_PELIGRO_TRAMPA = 6; // Celda adyacente a una trampa → simbolo '~'

// ─── Constantes de estado del juego ──────────────────────────────────────────
// Controlan el flujo del bucle principal en main
const int ESTADO_EN_CURSO = 0; // El juego continua normalmente
const int ESTADO_VICTORIA = 1; // El raton llego al queso: gana
const int ESTADO_GATO     = 2; // El raton choco con un gato: pierde
const int ESTADO_TRAMPA   = 3; // El raton cayo en una trampa: pierde
const int ESTADO_LIMITE   = 4; // Se agotaron los turnos maximos: empate

// ─── Tamaño maximo del tablero ────────────────────────────────────────────────
// El tablero se declara estaticamente con estas dimensiones maximas.
// El tamaño REAL de cada partida se pasa como parametro (filas, columnas),
// lo que permite cambiar el tamaño en tiempo de ejecucion sin usar memoria
// dinamica (new/delete) ni la libreria vector de C++.
const int MAX_FILAS    = 20;
const int MAX_COLUMNAS = 20;

// ─── Cantidad de actores en el juego ─────────────────────────────────────────
// Definidos como constantes para facil modificacion y referencia en el codigo
const int NUM_GATOS   = 5; // Numero de gatos en el tablero
const int NUM_TRAMPAS = 4; // Numero de trampas en el tablero
const int NUM_QUESOS  = 1; // Siempre un solo queso (objetivo unico)

// ─── Control de ciclos del agente ────────────────────────────────────────────
// Maximo de veces que el raton puede visitar una celda antes de que
// esa celda pierda prioridad en la logica de movimiento.
// Valor 2: si visita una celda 2 veces, ya no es candidata prioritaria.
// Esto evita que el raton oscile entre 2 celdas indefinidamente.
const int MAX_VISITAS_CELDA = 2;

// ─── Tamaño maximo del historial ─────────────────────────────────────────────
// Maximo de turnos/acciones/sensaciones que se pueden registrar por partida
const int MAX_TURNOS = 500;

// ─────────────────────────────────────────────────────────────────────────────
// ESTRUCTURA SENSACION
// Representa lo que el agente percibe en UN turno especifico.
// Se acumula en un arreglo dentro del Agente formando la matriz de sensaciones.
// Cada campo corresponde a una direccion adyacente al raton.
// ─────────────────────────────────────────────────────────────────────────────
struct Sensacion {
    int  turno;          // Numero de turno en que se registro esta sensacion
    int  fila_raton;     // Fila donde estaba el raton al registrar
    int  col_raton;      // Columna donde estaba el raton al registrar

    // Valor de la celda en cada direccion (-1 = fuera del tablero)
    int  arriba;         // Contenido de la celda de arriba
    int  abajo;          // Contenido de la celda de abajo
    int  izquierda;      // Contenido de la celda izquierda
    int  derecha;        // Contenido de la celda derecha

    // Si esa direccion ya fue visitada antes por el raton
    bool visitado_arr;   // true si la celda de arriba ya fue pisada
    bool visitado_aba;   // true si la celda de abajo ya fue pisada
    bool visitado_izq;   // true si la celda izquierda ya fue pisada
    bool visitado_der;   // true si la celda derecha ya fue pisada
};

// ─── Declaraciones de funciones del tablero ──────────────────────────────────

// Pone todas las celdas en VACIO, visitado en false y conteo en 0
void inicializar_tablero(int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
                         int conteo[][MAX_COLUMNAS],
                         int filas, int columnas);

// Coloca aleatoriamente gatos, trampas y queso en el tablero
void colocar_actores(int tablero[][MAX_COLUMNAS],
                     int filas, int columnas);

// Marca las celdas adyacentes a gatos y trampas como peligrosas
void calcular_peligros(int tablero[][MAX_COLUMNAS],
                       int filas, int columnas);

// Dibuja el tablero en consola con simbolos y encabezado de turno
void imprimir_tablero(int tablero[][MAX_COLUMNAS],
                      bool visitado[][MAX_COLUMNAS],
                      int filas, int columnas,
                      int turno,
                      int fila_raton, int col_raton);

// Captura y guarda lo que percibe el raton en el turno actual
void registrar_sensacion(Sensacion sensaciones[], int &num_sensaciones,
                         int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
                         int filas, int columnas,
                         int turno, int fila_raton, int col_raton);

// Imprime la tabla completa de sensaciones acumuladas + estado actual
void imprimir_matriz_sensaciones(Sensacion sensaciones[], int num_sensaciones,
                                  int estado_actual, int turno_actual);

// Imprime la tabla completa de acciones realizadas con conteo
void imprimir_tabla_acciones(const char acciones[][30], int num_acciones);

// Convierte un valor entero del tablero a su nombre legible en texto
const char* nombre_valor(int valor);

// Convierte bool a "Si" o "No" para mostrar en tablas
const char* bool_a_texto(bool valor);

#endif