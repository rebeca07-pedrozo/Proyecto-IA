#ifndef AGENTE_H
#define AGENTE_H

#include "tablero.h"

// ─────────────────────────────────────────────────────────────────────────────
// ESTRUCTURA DEL AGENTE
// Agrupa toda la informacion del raton en un solo lugar.
// No se usa POO (clases), solo struct con datos planos y funciones separadas.
// ─────────────────────────────────────────────────────────────────────────────
struct Agente {
    int fila;                   // Fila actual del raton
    int col;                    // Columna actual del raton
    int ultimo_valor_destino;   // Que habia en la celda a la que se movio
    int turno;                  // Turno actual
    int estado;                 // Estado del juego (EN_CURSO, VICTORIA, etc.)

    // Matriz de acciones: cada fila es una accion realizada (texto de hasta 30 chars)
    char acciones[MAX_TURNOS][30];
    int  num_acciones;

    // Matriz de sensaciones: una por turno
    Sensacion sensaciones[MAX_TURNOS];
    int       num_sensaciones;
};

// ─── Funciones del agente ─────────────────────────────────────────────────────
void inicializar_agente(Agente &ag, int col_inicial);

void registrar_accion(Agente &ag, const char* descripcion);

void mover_agente(Agente &ag,
                  int tablero[][MAX_COLUMNAS],
                  bool visitado[][MAX_COLUMNAS],
                  int conteo[][MAX_COLUMNAS],
                  int filas, int columnas);

void verificar_estado(Agente &ag, int limite_turnos);

void mostrar_resultado(int estado);

#endif