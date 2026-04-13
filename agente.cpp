#include "agente.h"
#include "tablero.h"
#include <iostream>
#include <cstdlib>
#include <cstring>

// ─── Inicializar agente en esquina superior derecha ───────────────────────────
void inicializar_agente(Agente &ag, int col_inicial) {
    ag.fila                 = 0;
    ag.col                  = col_inicial; // columnas - 1
    ag.ultimo_valor_destino = VALOR_VACIO;
    ag.turno                = 1;
    ag.estado               = ESTADO_EN_CURSO;
    ag.num_acciones         = 0;
}

// ─── Copiar string manualmente (sin <string>) ─────────────────────────────────
static void copiar_str(char* dest, const char* src, int max) {
    int i = 0;
    while (src[i] != '\0' && i < max - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// ─── Registrar una acción en el historial ─────────────────────────────────────
void registrar_accion(Agente &ag, const char* descripcion) {
    if (ag.num_acciones < 500) {
        copiar_str(ag.acciones[ag.num_acciones], descripcion, 30);
        ag.num_acciones++;
    }
}

// ─── Función auxiliar: copiar entero a string simple ─────────────────────────
static void int_a_str(int n, char* buf) {
    if (n == 0) { buf[0]='0'; buf[1]='\0'; return; }
    char tmp[12];
    int i = 0;
    bool neg = n < 0;
    if (neg) n = -n;
    while (n > 0) { tmp[i++] = '0' + (n % 10); n /= 10; }
    if (neg) tmp[i++] = '-';
    int j = 0;
    for (int k = i - 1; k >= 0; k--) buf[j++] = tmp[k];
    buf[j] = '\0';
}

// ─── Elegir índice aleatorio de un arreglo de candidatos ─────────────────────
static int elegir_aleatorio(int arr[], int n) {
    if (n <= 0) return -1;
    return arr[rand() % n];
}

// ─────────────────────────────────────────────────────────────────────────────
// LÓGICA DE MOVIMIENTO MEJORADA
// Prioridades:
//   1. Queso (objetivo inmediato)
//   2. Celdas seguras no visitadas
//   3. Celdas con peligro de trampa no visitadas (arriesgado pero avanza)
//   4. Celdas seguras ya visitadas (retroceder)
//   5. Cualquier celda válida (último recurso)
//   La trampa y el gato directos se evitan salvo que sea lo único disponible.
// ─────────────────────────────────────────────────────────────────────────────
void mover_agente(Agente &ag,
                  int tablero[][MAX_COLUMNAS],
                  bool visitado[][MAX_COLUMNAS],
                  int filas, int columnas) {

    int df[4] = {-1,  1,  0, 0};
    int dc[4] = { 0,  0, -1, 1};
    const char* nombres_dir[4] = {"Arriba", "Abajo", "Izquierda", "Derecha"};

    // Candidatos por categoría
    int queso[4],          n_queso          = 0;
    int seguro_nuevo[4],   n_seguro_nuevo   = 0;
    int peligro_trampa[4], n_peligro_trampa = 0;
    int seguro_viejo[4],   n_seguro_viejo   = 0;
    int todos[4],          n_todos          = 0;

    for (int d = 0; d < 4; d++) {
        int nf = ag.fila + df[d];
        int nc = ag.col  + dc[d];

        // Fuera del tablero: ignorar
        if (nf < 0 || nf >= filas || nc < 0 || nc >= columnas) continue;

        int val = tablero[nf][nc];
        todos[n_todos++] = d;

        if (val == VALOR_QUESO) {
            queso[n_queso++] = d;
        } else if (val == VALOR_VACIO && !visitado[nf][nc]) {
            seguro_nuevo[n_seguro_nuevo++] = d;
        } else if (val == VALOR_PELIGRO_TRAMPA && !visitado[nf][nc]) {
            peligro_trampa[n_peligro_trampa++] = d;
        } else if ((val == VALOR_VACIO || val == VALOR_PELIGRO_GATO ||
                    val == VALOR_PELIGRO_TRAMPA) && visitado[nf][nc]) {
            seguro_viejo[n_seguro_viejo++] = d;
        }
        // Gato y trampa directos quedan solo en 'todos' como último recurso
    }

    // Elegir según prioridad
    int dir = -1;

    if (dir == -1 && n_queso > 0)
        dir = elegir_aleatorio(queso, n_queso);
    if (dir == -1 && n_seguro_nuevo > 0)
        dir = elegir_aleatorio(seguro_nuevo, n_seguro_nuevo);
    if (dir == -1 && n_peligro_trampa > 0)
        dir = elegir_aleatorio(peligro_trampa, n_peligro_trampa);
    if (dir == -1 && n_seguro_viejo > 0)
        dir = elegir_aleatorio(seguro_viejo, n_seguro_viejo);
    if (dir == -1)
        dir = elegir_aleatorio(todos, n_todos);

    if (dir == -1) return; // Sin movimientos posibles

    int nueva_f = ag.fila + df[dir];
    int nueva_c = ag.col  + dc[dir];

    // Guardar qué había en la celda destino antes de pisarla
    ag.ultimo_valor_destino = tablero[nueva_f][nueva_c];

    // Limpiar celda anterior y marcar como visitada
    tablero[ag.fila][ag.col] = VALOR_VACIO;
    visitado[ag.fila][ag.col] = true;

    // Colocar ratón en nueva posición
    tablero[nueva_f][nueva_c] = VALOR_RATON;

    // Registrar acción en historial
    char desc[30] = "Mover ";
    int i = 6;
    const char* dir_name = nombres_dir[dir];
    int k = 0;
    while (dir_name[k] != '\0' && i < 25) { desc[i++] = dir_name[k++]; }
    desc[i++] = ' ';
    desc[i++] = '(';
    char buf[6];
    int_a_str(nueva_f, buf);
    for (int x = 0; buf[x] != '\0' && i < 27; x++) desc[i++] = buf[x];
    desc[i++] = ',';
    int_a_str(nueva_c, buf);
    for (int x = 0; buf[x] != '\0' && i < 29; x++) desc[i++] = buf[x];
    desc[i++] = ')';
    desc[i]   = '\0';

    registrar_accion(ag, desc);

    // Actualizar posición
    ag.fila = nueva_f;
    ag.col  = nueva_c;
}

// ─── Verificar estado del juego ───────────────────────────────────────────────
void verificar_estado(Agente &ag, int limite_turnos) {
    if (ag.ultimo_valor_destino == VALOR_QUESO) {
        ag.estado = ESTADO_VICTORIA;
        return;
    }
    if (ag.ultimo_valor_destino == VALOR_GATO) {
        ag.estado = ESTADO_GATO;
        return;
    }
    if (ag.ultimo_valor_destino == VALOR_TRAMPA) {
        ag.estado = ESTADO_TRAMPA;
        return;
    }
    if (ag.turno >= limite_turnos) {
        ag.estado = ESTADO_LIMITE;
        return;
    }
    ag.estado = ESTADO_EN_CURSO;
}

// ─── Mostrar resultado final ──────────────────────────────────────────────────
void mostrar_resultado(int estado) {
    std::cout << "\n========================================\n";
    switch (estado) {
        case ESTADO_VICTORIA:
            std::cout << "  ¡EL RATON ENCONTRO EL QUESO! ¡Gano!\n"; break;
        case ESTADO_GATO:
            std::cout << "  ¡El raton fue atrapado por un gato!\n"; break;
        case ESTADO_TRAMPA:
            std::cout << "  ¡El raton cayo en una trampa!\n"; break;
        case ESTADO_LIMITE:
            std::cout << "  Se agoto el limite de turnos. Sin salida.\n"; break;
        default:
            std::cout << "  Juego terminado.\n"; break;
    }
    std::cout << "========================================\n";
}