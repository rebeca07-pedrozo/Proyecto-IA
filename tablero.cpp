#include "tablero.h"
#include <cstdlib>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// INICIALIZAR TABLERO
// Recorre toda la porcion activa (filas x columnas) de las tres matrices y
// las deja en su estado inicial:
//   tablero  → VALOR_VACIO (0): ninguna celda tiene contenido
//   visitado → false: el raton no ha pisado ninguna celda
//   conteo   → 0: ninguna celda ha sido visitada ninguna vez
// Se usan los parametros filas/columnas (no MAX_FILAS/MAX_COLUMNAS) para
// operar solo sobre la porcion real del tablero elegida en tiempo de ejecucion.
// ─────────────────────────────────────────────────────────────────────────────
void inicializar_tablero(int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
                         int conteo[][MAX_COLUMNAS],
                         int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            tablero[i][j]  = VALOR_VACIO; // celda vacia
            visitado[i][j] = false;        // nunca visitada
            conteo[i][j]   = 0;            // cero visitas
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// RANDOMIZAR ACTOR (funcion interna, no visible fuera de este archivo)
// Coloca 'cantidad' elementos de tipo 'tipo' en posiciones aleatorias validas.
//
// Como funciona la aleatoriedad:
//   - rand() % filas genera un entero entre 0 y filas-1
//   - rand() % columnas genera un entero entre 0 y columnas-1
//   - La semilla se inicializa con srand(time(0)) en main, lo que garantiza
//     que cada ejecucion del programa genere posiciones distintas
//
// Validaciones antes de colocar:
//   - La celda no puede ser la posicion inicial del raton (excluir_f, excluir_c)
//   - La celda debe estar VACIA (no pisar otro actor ya colocado)
//   - Maximo 10000 intentos para evitar bucle infinito en tableros pequeños
// ─────────────────────────────────────────────────────────────────────────────
static void randomizar_actor(int tablero[][MAX_COLUMNAS],
                              int filas, int columnas,
                              int cantidad, int tipo,
                              int excluir_f, int excluir_c) {
    int colocados  = 0;
    int intentos   = 0;
    const int MAX_INTENTOS = 10000;

    while (colocados < cantidad && intentos < MAX_INTENTOS) {
        // Generar coordenadas aleatorias dentro del rango valido
        int f = rand() % filas;
        int c = rand() % columnas;
        intentos++;

        // Rechazar si es posicion del raton o la celda ya tiene algo
        if ((f == excluir_f && c == excluir_c) || tablero[f][c] != VALOR_VACIO)
            continue;

        // Celda valida: colocar el actor
        tablero[f][c] = tipo;
        colocados++;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// COLOCAR ACTORES
// Llama a randomizar_actor en orden: primero gatos, luego trampas, luego queso.
// El orden importa: al colocar gatos primero, las trampas y queso no pueden
// caer sobre ellos (validacion de celda ocupada).
// La posicion del raton (esquina superior derecha) se excluye siempre.
// ─────────────────────────────────────────────────────────────────────────────
void colocar_actores(int tablero[][MAX_COLUMNAS],
                     int filas, int columnas) {
    // Posicion inicial del raton: fila 0, ultima columna
    int raton_f = 0;
    int raton_c = columnas - 1;

    randomizar_actor(tablero, filas, columnas, NUM_GATOS,   VALOR_GATO,   raton_f, raton_c);
    randomizar_actor(tablero, filas, columnas, NUM_TRAMPAS, VALOR_TRAMPA, raton_f, raton_c);
    randomizar_actor(tablero, filas, columnas, NUM_QUESOS,  VALOR_QUESO,  raton_f, raton_c);
}

// ─────────────────────────────────────────────────────────────────────────────
// MARCAR ADYACENTES (funcion interna)
// Dado un actor peligroso en (f,c), marca sus 4 vecinos directos con
// el tipo de peligro correspondiente (VALOR_PELIGRO_GATO o VALOR_PELIGRO_TRAMPA).
// Solo marca si la celda vecina esta VACIA para no pisar otros actores.
// Valida que las coordenadas no salgan del tablero antes de marcar.
// ─────────────────────────────────────────────────────────────────────────────
static void marcar_adyacentes(int tablero[][MAX_COLUMNAS],
                               int filas, int columnas,
                               int f, int c, int tipo_peligro) {
    // Vectores de direccion: arriba, abajo, izquierda, derecha
    int df[] = {-1,  1,  0, 0};
    int dc[] = { 0,  0, -1, 1};

    for (int d = 0; d < 4; d++) {
        int nf = f + df[d];
        int nc = c + dc[d];

        // Validar que la celda vecina este dentro del tablero
        if (nf >= 0 && nf < filas && nc >= 0 && nc < columnas) {
            // Solo marcar si la celda esta vacia
            if (tablero[nf][nc] == VALOR_VACIO)
                tablero[nf][nc] = tipo_peligro;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CALCULAR PELIGROS
// Recorre todo el tablero buscando gatos y trampas.
// Por cada uno llama a marcar_adyacentes con el tipo de peligro correcto.
// Se llama UNA sola vez despues de colocar_actores, antes de iniciar el juego.
// ─────────────────────────────────────────────────────────────────────────────
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

// ─────────────────────────────────────────────────────────────────────────────
// NOMBRE VALOR
// Convierte el valor entero de una celda a su nombre en texto.
// Se usa en la matriz de sensaciones para mostrar contenido legible.
// ─────────────────────────────────────────────────────────────────────────────
const char* nombre_valor(int valor) {
    switch (valor) {
        case VALOR_VACIO:           return "Vacio         ";
        case VALOR_RATON:           return "Raton         ";
        case VALOR_GATO:            return "GATO          ";
        case VALOR_TRAMPA:          return "TRAMPA        ";
        case VALOR_QUESO:           return "QUESO         ";
        case VALOR_PELIGRO_GATO:    return "Peligro gato  ";
        case VALOR_PELIGRO_TRAMPA:  return "Peligro trampa";
        default:                    return "Desconocido   ";
    }
}

// Convierte bool a texto legible para la columna "Visitado" de la tabla
const char* bool_a_texto(bool valor) {
    return valor ? "Si " : "No ";
}

// ─────────────────────────────────────────────────────────────────────────────
// IMPRIMIR TABLERO
// Limpia la consola y dibuja el tablero completo con:
//   - Encabezado con numero de turno
//   - Indices de filas y columnas para orientacion
//   - Un simbolo por celda segun su contenido
//   - Leyenda de convenciones al final
//
// Simbolos usados:
//   R = Raton (posicion actual)      G = Gato
//   T = Trampa                       Q = Queso
//   ! = Peligro de gato adyacente    ~ = Peligro de trampa adyacente
//   * = Celda ya visitada por raton  . = Celda vacia sin visitar
// ─────────────────────────────────────────────────────────────────────────────
void imprimir_tablero(int tablero[][MAX_COLUMNAS],
                      bool visitado[][MAX_COLUMNAS],
                      int filas, int columnas,
                      int turno,
                      int fila_raton, int col_raton) {
    system("clear");

    std::cout << "========================================\n";
    std::cout << "   RATON COME QUESO  |  Turno: " << turno << "\n";
    std::cout << "========================================\n\n";

    // Encabezado de indices de columnas (ultimo digito para tableros grandes)
    std::cout << "   ";
    for (int j = 0; j < columnas; j++) std::cout << j % 10 << " ";
    std::cout << "\n   ";
    for (int j = 0; j < columnas; j++) std::cout << "--";
    std::cout << "\n";

    // Filas del tablero
    for (int i = 0; i < filas; i++) {
        std::cout << i % 10 << " |"; // indice de fila
        for (int j = 0; j < columnas; j++) {
            char c = '.';

            // El raton tiene prioridad visual sobre cualquier otro valor
            if (i == fila_raton && j == col_raton) {
                c = 'R';
            } else {
                switch (tablero[i][j]) {
                    case VALOR_VACIO:
                        // Si fue visitada se muestra *, si no fue visitada .
                        c = visitado[i][j] ? '*' : '.';
                        break;
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

    // Leyenda de simbolos
    std::cout << "\nConvenciones: R=Raton  G=Gato  T=Trampa  Q=Queso\n";
    std::cout << "             !=Peligro gato  ~=Peligro trampa  *=Visitado\n\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// REGISTRAR SENSACION
// Captura el estado perceptual del raton en el turno actual y lo guarda
// en el arreglo de sensaciones. Se llama ANTES de mover al agente.
//
// Para cada direccion (arriba, abajo, izq, der):
//   - Calcula las coordenadas de la celda vecina
//   - Si esta dentro del tablero: guarda el valor y si fue visitada
//   - Si esta fuera del tablero: guarda -1 (se muestra como "Desconocido")
// ─────────────────────────────────────────────────────────────────────────────
void registrar_sensacion(Sensacion sensaciones[], int &num_sensaciones,
                         int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
                         int filas, int columnas,
                         int turno, int fila_raton, int col_raton) {

    if (num_sensaciones >= MAX_TURNOS) return; // proteccion de desbordamiento

    Sensacion s;
    s.turno      = turno;
    s.fila_raton = fila_raton;
    s.col_raton  = col_raton;

    // ── Arriba (fila - 1) ────────────────────────────────────────────────────
    int nf = fila_raton - 1;
    int nc = col_raton;
    s.arriba       = (nf >= 0)       ? tablero[nf][nc]  : -1;
    s.visitado_arr = (nf >= 0)       ? visitado[nf][nc] : false;

    // ── Abajo (fila + 1) ─────────────────────────────────────────────────────
    nf = fila_raton + 1;
    nc = col_raton;
    s.abajo        = (nf < filas)    ? tablero[nf][nc]  : -1;
    s.visitado_aba = (nf < filas)    ? visitado[nf][nc] : false;

    // ── Izquierda (columna - 1) ──────────────────────────────────────────────
    nf = fila_raton;
    nc = col_raton - 1;
    s.izquierda    = (nc >= 0)       ? tablero[nf][nc]  : -1;
    s.visitado_izq = (nc >= 0)       ? visitado[nf][nc] : false;

    // ── Derecha (columna + 1) ────────────────────────────────────────────────
    nf = fila_raton;
    nc = col_raton + 1;
    s.derecha      = (nc < columnas) ? tablero[nf][nc]  : -1;
    s.visitado_der = (nc < columnas) ? visitado[nf][nc] : false;

    // Guardar sensacion en el arreglo e incrementar contador
    sensaciones[num_sensaciones] = s;
    num_sensaciones++;
}

// ─────────────────────────────────────────────────────────────────────────────
// IMPRIMIR MATRIZ DE SENSACIONES
// Muestra la tabla completa de percepciones acumuladas hasta el turno actual.
// Cada fila contiene: turno, posicion, contenido de 4 direcciones y si cada
// una fue visitada (Si/No).
// Al final muestra el estado actual del agente en ese turno.
// ─────────────────────────────────────────────────────────────────────────────
void imprimir_matriz_sensaciones(Sensacion sensaciones[], int num_sensaciones,
                                  int estado_actual, int turno_actual) {

    std::cout << "---------------------------------------------------------------------------------\n";
    std::cout << "                         MATRIZ DE SENSACIONES\n";
    std::cout << "---------------------------------------------------------------------------------\n";
    // Encabezado de columnas con la nueva columna de Visitado por direccion
    std::cout << "Turno | Pos    | Arriba         |Vis| Abajo          |Vis| Izq            |Vis| Der            |Vis\n";
    std::cout << "---------------------------------------------------------------------------------\n";

    for (int i = 0; i < num_sensaciones; i++) {
        Sensacion &s = sensaciones[i];

        // Numero de turno alineado
        std::cout << "  " << s.turno;
        if      (s.turno < 10)  std::cout << "   ";
        else if (s.turno < 100) std::cout << "  ";
        else                    std::cout << " ";

        // Posicion del raton
        std::cout << "| (" << s.fila_raton << "," << s.col_raton << ") | ";

        // Contenido y visitado de cada direccion
        std::cout << nombre_valor(s.arriba)    << " |" << bool_a_texto(s.visitado_arr) << "| ";
        std::cout << nombre_valor(s.abajo)     << " |" << bool_a_texto(s.visitado_aba) << "| ";
        std::cout << nombre_valor(s.izquierda) << " |" << bool_a_texto(s.visitado_izq) << "| ";
        std::cout << nombre_valor(s.derecha)   << " |" << bool_a_texto(s.visitado_der) << "\n";
    }

    std::cout << "Total sensaciones registradas: " << num_sensaciones << "\n";
    std::cout << "---------------------------------------------------------------------------------\n";

    // ─── Estado actual del agente ─────────────────────────────────────────────
    // Muestra en que estado se encuentra el agente en el turno actual
    std::cout << "\n--- ESTADO ACTUAL DEL AGENTE (Turno " << turno_actual << ") ---\n";
    std::cout << "Estado: ";
    switch (estado_actual) {
        case ESTADO_EN_CURSO: std::cout << "EN CURSO\n";                        break;
        case ESTADO_VICTORIA: std::cout << "VICTORIA - Encontro el queso\n";    break;
        case ESTADO_GATO:     std::cout << "DERROTA  - Atrapado por un gato\n"; break;
        case ESTADO_TRAMPA:   std::cout << "DERROTA  - Cayo en una trampa\n";   break;
        case ESTADO_LIMITE:   std::cout << "LIMITE   - Se agotaron los turnos\n"; break;
        default:              std::cout << "DESCONOCIDO\n";                      break;
    }
    std::cout << "---------------------------------------------------------------------------------\n\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// IMPRIMIR TABLA DE ACCIONES
// Muestra el historial completo de acciones realizadas por el agente.
// Cada fila tiene el numero de accion y su descripcion.
// Al final muestra el conteo total de acciones realizadas.
// ─────────────────────────────────────────────────────────────────────────────
void imprimir_tabla_acciones(const char acciones[][30], int num_acciones) {
    std::cout << "--------------------------------------------\n";
    std::cout << "           MATRIZ DE ACCIONES\n";
    std::cout << "--------------------------------------------\n";
    std::cout << "#     | Accion realizada\n";
    std::cout << "--------------------------------------------\n";

    for (int i = 0; i < num_acciones; i++) {
        // Numero de accion alineado
        std::cout << "  " << i + 1;
        if      (i + 1 < 10)  std::cout << "   ";
        else if (i + 1 < 100) std::cout << "  ";
        else                  std::cout << " ";
        std::cout << "| " << acciones[i] << "\n";
    }

    // Conteo total requerido por el enunciado
    std::cout << "Total de acciones: " << num_acciones << "\n";
    std::cout << "--------------------------------------------\n\n";
}