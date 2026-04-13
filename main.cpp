#include "tablero.h"
#include "agente.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h> // usleep: control de tiempo entre turnos

// ─── Configuracion general de la partida ─────────────────────────────────────
const int PAUSA_MS      = 600;  // Milisegundos de pausa entre turnos
                                 // Permite ver el tablero antes de que avance
const int LIMITE_TURNOS = 300;  // Maximo de turnos por partida
                                 // Evita que el juego corra infinitamente

// ─── Arreglos globales del juego ──────────────────────────────────────────────
// Se declaran con tamaño MAXIMO estatico para no usar memoria dinamica.
// Solo se usa la porcion [filas][columnas] definida en tiempo de ejecucion.
// Esto cumple la restriccion de no usar vector<> ni new/delete.
static int  tablero[MAX_FILAS][MAX_COLUMNAS]; // mapa del juego
static bool visitado[MAX_FILAS][MAX_COLUMNAS]; // celdas ya pisadas por el raton
static int  conteo[MAX_FILAS][MAX_COLUMNAS];   // cuantas veces visito cada celda

// ─────────────────────────────────────────────────────────────────────────────
// MOSTRAR MENU
// Imprime las opciones disponibles al inicio y entre partidas.
// Opcion 1: tablero con las dimensiones del enunciado (10 filas x 11 columnas)
// Opcion 2: el usuario elige las dimensiones en tiempo de ejecucion
// Opcion 3: salir del programa
// ─────────────────────────────────────────────────────────────────────────────
void mostrar_menu() {
    std::cout << "\n========================================\n";
    std::cout << "        RATON COME QUESO - MENU\n";
    std::cout << "========================================\n";
    std::cout << "  1. Jugar con tablero por defecto (10 filas x 11 columnas)\n";
    std::cout << "  2. Jugar con tablero personalizado\n";
    std::cout << "  3. Salir\n";
    std::cout << "Opcion: ";
}

// ─────────────────────────────────────────────────────────────────────────────
// LEER ENTERO
// Lee un entero del teclado y valida que este dentro del rango [minimo, maximo].
// Si el usuario ingresa un valor invalido, limpia el buffer y pide de nuevo.
// Evita que entradas invalidas (letras, simbolos) bloqueen el programa.
// ─────────────────────────────────────────────────────────────────────────────
int leer_entero(int minimo, int maximo) {
    int valor;
    while (true) {
        std::cin >> valor;
        if (std::cin.fail() || valor < minimo || valor > maximo) {
            std::cin.clear();                    // limpiar flag de error
            std::cin.ignore(1000, '\n');         // descartar entrada invalida
            std::cout << "Valor invalido. Ingrese entre "
                      << minimo << " y " << maximo << ": ";
        } else {
            return valor; // valor valido: retornar
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// EJECUTAR JUEGO
// Bucle principal de una partida completa. Recibe las dimensiones del tablero.
//
// Flujo de ejecucion:
//   INICIO:
//     1. Inicializar tablero, visitado y conteo en cero
//     2. Colocar actores aleatoriamente (gatos, trampas, queso)
//     3. Calcular celdas de peligro adyacentes
//     4. Colocar raton en esquina superior derecha
//
//   POR CADA TURNO (mientras estado == EN_CURSO):
//     1. Registrar sensacion: capturar lo que percibe el raton ANTES de moverse
//     2. Imprimir tablero visual
//     3. Imprimir matriz de sensaciones acumulada + estado actual
//     4. Imprimir matriz de acciones acumulada
//     5. Pausar (usleep) para que sea legible
//     6. Mover agente segun logica de prioridades
//     7. Verificar estado: victoria / derrota / limite
//     8. Incrementar turno
//
//   FIN:
//     Imprimir tablero y matrices finales + mensaje de resultado
// ─────────────────────────────────────────────────────────────────────────────
void ejecutar_juego(int filas, int columnas) {

    // Semilla aleatoria con tiempo actual: cada partida es diferente
    srand((unsigned int)time(0));

    // ── Preparar tablero ──────────────────────────────────────────────────────
    inicializar_tablero(tablero, visitado, conteo, filas, columnas);
    colocar_actores(tablero, filas, columnas);
    calcular_peligros(tablero, filas, columnas);

    // ── Preparar agente ───────────────────────────────────────────────────────
    Agente ag;
    inicializar_agente(ag, columnas - 1); // esquina superior derecha

    // Colocar raton en el tablero y marcar su celda inicial
    tablero[ag.fila][ag.col]  = VALOR_RATON;
    visitado[ag.fila][ag.col] = true;  // la celda inicial se considera visitada
    conteo[ag.fila][ag.col]   = 1;     // primera visita registrada

    // Registrar accion inicial en el historial
    registrar_accion(ag, "Inicio esquina sup-der");

    // ── Bucle principal ───────────────────────────────────────────────────────
    while (ag.estado == ESTADO_EN_CURSO) {

        // 1. Capturar sensacion del turno actual (antes de moverse)
        registrar_sensacion(ag.sensaciones, ag.num_sensaciones,
                            tablero, visitado,
                            filas, columnas,
                            ag.turno, ag.fila, ag.col);

        // 2. Mostrar tablero visual en consola
        imprimir_tablero(tablero, visitado,
                         filas, columnas,
                         ag.turno, ag.fila, ag.col);

        // 3. Mostrar matriz de sensaciones con estado actual
        imprimir_matriz_sensaciones(ag.sensaciones, ag.num_sensaciones,
                                    ag.estado, ag.turno);

        // 4. Mostrar matriz de acciones con conteo
        imprimir_tabla_acciones(ag.acciones, ag.num_acciones);

        // 5. Pausa para que el usuario pueda leer la informacion
        usleep(PAUSA_MS * 1000);

        // 6. Mover agente segun logica de prioridades
        mover_agente(ag, tablero, visitado, conteo, filas, columnas);

        // 7. Verificar si termino el juego
        verificar_estado(ag, LIMITE_TURNOS);

        // 8. Siguiente turno
        ag.turno++;
    }

    // ── Estado final ──────────────────────────────────────────────────────────
    // Registrar y mostrar la ultima sensacion (posicion final del raton)
    registrar_sensacion(ag.sensaciones, ag.num_sensaciones,
                        tablero, visitado,
                        filas, columnas,
                        ag.turno, ag.fila, ag.col);

    imprimir_tablero(tablero, visitado,
                     filas, columnas,
                     ag.turno, ag.fila, ag.col);

    imprimir_matriz_sensaciones(ag.sensaciones, ag.num_sensaciones,
                                ag.estado, ag.turno);

    imprimir_tabla_acciones(ag.acciones, ag.num_acciones);

    // Mensaje final de resultado
    mostrar_resultado(ag.estado);
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// Punto de entrada del programa.
// Muestra el menu y ejecuta partidas hasta que el usuario elija salir.
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    int opcion = 0;

    while (true) {
        mostrar_menu();
        opcion = leer_entero(1, 3);

        // Opcion 3: salir del programa
        if (opcion == 3) {
            std::cout << "\nHasta luego.\n";
            break;
        }

        // Dimensiones por defecto segun enunciado: 10 filas x 11 columnas
        int filas    = 10;
        int columnas = 11;

        // Opcion 2: el usuario elige las dimensiones
        if (opcion == 2) {
            std::cout << "Filas    (" << NUM_GATOS + NUM_TRAMPAS + 3
                      << " a " << MAX_FILAS << "): ";
            filas = leer_entero(NUM_GATOS + NUM_TRAMPAS + 3, MAX_FILAS);

            std::cout << "Columnas (" << NUM_GATOS + NUM_TRAMPAS + 3
                      << " a " << MAX_COLUMNAS << "): ";
            columnas = leer_entero(NUM_GATOS + NUM_TRAMPAS + 3, MAX_COLUMNAS);
        }

        // Ejecutar la partida con las dimensiones elegidas
        ejecutar_juego(filas, columnas);

        // Esperar ENTER para volver al menu
        std::cout << "\nPresione ENTER para volver al menu...";
        std::cin.ignore(1000, '\n');
        std::cin.get();
    }

    return 0;
}