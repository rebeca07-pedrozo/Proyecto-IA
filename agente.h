#ifndef AGENTE_H
#define AGENTE_H

#include "tablero.h"

// ─────────────────────────────────────────────────────────────────────────────
// ESTRUCTURA AGENTE
// Agrupa toda la informacion del raton en un solo lugar.
// No se usa POO (clases): es un struct con datos planos y funciones separadas.
//
// Contiene:
//   - Posicion actual (fila, col)
//   - Estado del juego
//   - Ultimo valor pisado (para detectar victoria/derrota)
//   - Turno actual
//   - Matriz de acciones: historial de movimientos realizados
//   - Matriz de sensaciones: historial de lo que percibio en cada turno
// ─────────────────────────────────────────────────────────────────────────────
struct Agente {
    int fila;                   // Fila actual del raton en el tablero
    int col;                    // Columna actual del raton en el tablero
    int ultimo_valor_destino;   // Valor que habia en la celda a la que se movio
    int turno;                  // Numero del turno actual
    int estado;                 // Estado del juego: EN_CURSO, VICTORIA, etc.

    // Matriz de acciones: arreglo de strings de hasta 30 caracteres cada uno
    // Cada posicion guarda la descripcion de una accion realizada
    char acciones[MAX_TURNOS][30];
    int  num_acciones;           // Cuantas acciones se han registrado

    // Matriz de sensaciones: una Sensacion por turno jugado
    // Registra lo que el raton percibio en cada turno antes de moverse
    Sensacion sensaciones[MAX_TURNOS];
    int       num_sensaciones;   // Cuantas sensaciones se han registrado
};

// ─── Declaraciones de funciones del agente ───────────────────────────────────

// Pone al agente en su posicion inicial y reinicia todos sus contadores
void inicializar_agente(Agente &ag, int col_inicial);

// Agrega una descripcion de accion al historial de acciones
void registrar_accion(Agente &ag, const char* descripcion);

// Evalua las 4 celdas adyacentes y mueve al raton segun prioridades
void mover_agente(Agente &ag,
                  int tablero[][MAX_COLUMNAS],
                  bool visitado[][MAX_COLUMNAS],
                  int conteo[][MAX_COLUMNAS],
                  int filas, int columnas);

// Revisa si el raton gano, perdio o se agoto el limite de turnos
void verificar_estado(Agente &ag, int limite_turnos);

// Imprime el mensaje final segun el estado del juego
void mostrar_resultado(int estado);

#endif