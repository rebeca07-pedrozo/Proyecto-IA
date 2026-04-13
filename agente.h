#ifndef AGENTE_H
#define AGENTE_H

#include "tablero.h"

// ─── Estructura del agente ratón ──────────────────────────────────────────────
struct Agente {
    int fila;
    int col;
    int ultimo_valor_destino; // Qué había en la celda a la que se movió
    int turno;
    int estado;               // ESTADO_EN_CURSO, VICTORIA, etc.

    // Historial de acciones (máximo 500 acciones)
    char acciones[500][30];
    int  num_acciones;
};

// ─── Funciones del agente ─────────────────────────────────────────────────────
void inicializar_agente(Agente &ag, int col_inicial);

void registrar_accion(Agente &ag, const char* descripcion);

void mover_agente(Agente &ag,
                  int tablero[][MAX_COLUMNAS],
                  bool visitado[][MAX_COLUMNAS],
                  int filas, int columnas);

void verificar_estado(Agente &ag, int limite_turnos);

void mostrar_resultado(int estado);

#endif