#include "tablero.h"
#include "agente.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

// ─── Configuracion general ────────────────────────────────────────────────────
const int PAUSA_MS      = 600;  // Milisegundos entre turno y turno
const int LIMITE_TURNOS = 300;  // Maximo de turnos antes de terminar el juego

// ─── Arreglos globales del juego ──────────────────────────────────────────────
// Se declaran con tamaño maximo estatico (MAX_FILAS x MAX_COLUMNAS).
// Solo se usa la porcion filas x columnas definida en tiempo de ejecucion.
static int  tablero[MAX_FILAS][MAX_COLUMNAS];
static bool visitado[MAX_FILAS][MAX_COLUMNAS];
static int  conteo[MAX_FILAS][MAX_COLUMNAS];  // veces que el raton visito cada celda

// ─── Menu principal ───────────────────────────────────────────────────────────
void mostrar_menu() {
    std::cout << "\n========================================\n";
    std::cout << "        RATON COME QUESO - MENU\n";
    std::cout << "========================================\n";
    std::cout << "  1. Jugar con tablero por defecto (10x11)\n";
    std::cout << "  2. Jugar con tablero personalizado\n";
    std::cout << "  3. Salir\n";
    std::cout << "Opcion: ";
}

// ─── Leer entero con validacion de rango ─────────────────────────────────────
int leer_entero(int minimo, int maximo) {
    int valor;
    while (true) {
        std::cin >> valor;
        if (std::cin.fail() || valor < minimo || valor > maximo) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            std::cout << "Valor invalido. Ingrese entre "
                      << minimo << " y " << maximo << ": ";
        } else {
            return valor;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// BUCLE PRINCIPAL DEL JUEGO
// Orden de cada turno:
//   1. Registrar sensacion (lo que percibe el raton ANTES de moverse)
//   2. Imprimir tablero visual
//   3. Imprimir matriz de sensaciones acumulada
//   4. Imprimir matriz de acciones acumulada
//   5. Pausar
//   6. Mover agente
//   7. Verificar estado (victoria / derrota / limite)
//   8. Incrementar turno
// ─────────────────────────────────────────────────────────────────────────────
void ejecutar_juego(int filas, int columnas) {

    // Semilla aleatoria basada en tiempo → partidas distintas cada vez
    srand((unsigned int)time(0));

    // 1. Preparar tablero y conteo
    inicializar_tablero(tablero, visitado, conteo, filas, columnas);
    colocar_actores(tablero, filas, columnas);
    calcular_peligros(tablero, filas, columnas);

    // 2. Preparar agente en esquina superior derecha (fila 0, ultima columna)
    Agente ag;
    inicializar_agente(ag, columnas - 1);
    tablero[ag.fila][ag.col]  = VALOR_RATON;
    visitado[ag.fila][ag.col] = true;
    conteo[ag.fila][ag.col]   = 1;
    registrar_accion(ag, "Inicio esquina sup-der");

    // 3. Bucle principal
    while (ag.estado == ESTADO_EN_CURSO) {

        // Registrar sensacion antes de moverse
        registrar_sensacion(ag.sensaciones, ag.num_sensaciones,
                            tablero, visitado,
                            filas, columnas,
                            ag.turno, ag.fila, ag.col);

        // Mostrar tablero visual
        imprimir_tablero(tablero, visitado,
                         filas, columnas,
                         ag.turno, ag.fila, ag.col);

        // Mostrar matriz de sensaciones acumulada
        imprimir_matriz_sensaciones(ag.sensaciones, ag.num_sensaciones);

        // Mostrar matriz de acciones acumulada
        imprimir_tabla_acciones(ag.acciones, ag.num_acciones);

        // Pausa entre turnos
        usleep(PAUSA_MS * 1000);

        // Mover agente (ahora recibe conteo)
        mover_agente(ag, tablero, visitado, conteo, filas, columnas);

        // Verificar si gano, perdio o llego al limite
        verificar_estado(ag, LIMITE_TURNOS);

        ag.turno++;
    }

    // 4. Mostrar estado final
    registrar_sensacion(ag.sensaciones, ag.num_sensaciones,
                        tablero, visitado,
                        filas, columnas,
                        ag.turno, ag.fila, ag.col);

    imprimir_tablero(tablero, visitado,
                     filas, columnas,
                     ag.turno, ag.fila, ag.col);
    imprimir_matriz_sensaciones(ag.sensaciones, ag.num_sensaciones);
    imprimir_tabla_acciones(ag.acciones, ag.num_acciones);
    mostrar_resultado(ag.estado);
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main() {
    int opcion = 0;

    while (true) {
        mostrar_menu();
        opcion = leer_entero(1, 3);

        if (opcion == 3) {
            std::cout << "\nHasta luego.\n";
            break;
        }

        int filas    = 10;
        int columnas = 11;

        if (opcion == 2) {
            std::cout << "Filas    (" << NUM_GATOS + NUM_TRAMPAS + 3
                      << " a " << MAX_FILAS << "): ";
            filas = leer_entero(NUM_GATOS + NUM_TRAMPAS + 3, MAX_FILAS);

            std::cout << "Columnas (" << NUM_GATOS + NUM_TRAMPAS + 3
                      << " a " << MAX_COLUMNAS << "): ";
            columnas = leer_entero(NUM_GATOS + NUM_TRAMPAS + 3, MAX_COLUMNAS);
        }

        ejecutar_juego(filas, columnas);

        std::cout << "\nPresione ENTER para volver al menu...";
        std::cin.ignore(1000, '\n');
        std::cin.get();
    }

    return 0;
}