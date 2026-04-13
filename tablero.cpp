#include "tablero.h"
#include <cstdlib>
#include <iostream>

// ─────────────────────────────────────────────────────────────────────────────
// INICIALIZACION
// Recorre toda la matriz y pone cada celda en VACIO (0) y visitado en false.
// Se usan los parametros filas/columnas para no depender de constantes fijas,
// permitiendo tableros de tamaño variable en tiempo de ejecucion.
// ─────────────────────────────────────────────────────────────────────────────
void inicializar_tablero(int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
                         int conteo[][MAX_COLUMNAS],
                         int filas, int columnas) {
    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < columnas; j++) {
            tablero[i][j]  = VALOR_VACIO;
            visitado[i][j] = false;
            conteo[i][j]   = 0;  // cuantas veces paso el raton por aqui
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ALEATORIEDAD
// rand() % filas genera un numero entre 0 y filas-1.
// srand(time(0)) en main inicializa la semilla con el tiempo actual,
// garantizando que cada partida tenga posiciones distintas.
// Se valida que la celda destino este VACIA y no sea la posicion del raton
// antes de colocar cada actor. Si no cumple, se reintenta (maximo 10000 veces).
// ─────────────────────────────────────────────────────────────────────────────
static void randomizar_actor(int tablero[][MAX_COLUMNAS],
                              int filas, int columnas,
                              int cantidad, int tipo,
                              int excluir_f, int excluir_c) {
    int colocados = 0;
    int intentos  = 0;
    const int MAX_INTENTOS = 10000;

    while (colocados < cantidad && intentos < MAX_INTENTOS) {
        // Generar posicion aleatoria dentro del rango valido
        int f = rand() % filas;
        int c = rand() % columnas;
        intentos++;

        // Rechazar si es la posicion inicial del raton o si la celda ya tiene algo
        if ((f == excluir_f && c == excluir_c) || tablero[f][c] != VALOR_VACIO)
            continue;

        tablero[f][c] = tipo;
        colocados++;
    }
}

void colocar_actores(int tablero[][MAX_COLUMNAS],
                     int filas, int columnas) {
    // El raton siempre inicia en esquina superior DERECHA (fila 0, ultima columna)
    int raton_f = 0;
    int raton_c = columnas - 1;

    // Se colocan primero gatos, luego trampas, luego queso
    // para que la validacion de celda ocupada funcione correctamente
    randomizar_actor(tablero, filas, columnas, NUM_GATOS,   VALOR_GATO,   raton_f, raton_c);
    randomizar_actor(tablero, filas, columnas, NUM_TRAMPAS, VALOR_TRAMPA, raton_f, raton_c);
    randomizar_actor(tablero, filas, columnas, NUM_QUESOS,  VALOR_QUESO,  raton_f, raton_c);
}

// ─────────────────────────────────────────────────────────────────────────────
// CALCULO DE PELIGROS
// Despues de colocar actores, se recorre el tablero buscando gatos y trampas.
// Para cada uno se marcan sus 4 celdas adyacentes (arriba, abajo, izq, der)
// con el valor de peligro correspondiente, SOLO si la celda esta vacia.
// Esto evita pisar actores ya colocados con un valor de peligro.
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
        // Validar que no salga del tablero
        if (nf >= 0 && nf < filas && nc >= 0 && nc < columnas) {
            if (tablero[nf][nc] == VALOR_VACIO)
                tablero[nf][nc] = tipo_peligro;
        }
    }
}

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

// ─── Convierte valor entero a nombre legible (usado en tablas) ───────────────
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

// ─────────────────────────────────────────────────────────────────────────────
// IMPRESION DEL TABLERO
// Recorre la matriz fila por fila e imprime un simbolo por celda:
//   R = Raton (posicion actual)    G = Gato      T = Trampa
//   Q = Queso                      ! = Peligro gato
//   ~ = Peligro trampa             * = Celda visitada   . = Vacia sin visitar
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

    // Encabezado de indices de columna
    std::cout << "   ";
    for (int j = 0; j < columnas; j++) std::cout << j % 10 << " ";
    std::cout << "\n   ";
    for (int j = 0; j < columnas; j++) std::cout << "--";
    std::cout << "\n";

    for (int i = 0; i < filas; i++) {
        std::cout << i % 10 << " |";
        for (int j = 0; j < columnas; j++) {
            char c = '.';
            if (i == fila_raton && j == col_raton) {
                c = 'R';
            } else {
                switch (tablero[i][j]) {
                    case VALOR_VACIO:           c = visitado[i][j] ? '*' : '.'; break;
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

    std::cout << "\nConvenciones: R=Raton  G=Gato  T=Trampa  Q=Queso\n";
    std::cout << "             !=Peligro gato  ~=Peligro trampa  *=Visitado\n\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// MATRIZ DE SENSACIONES
// Cada turno se captura el entorno inmediato del raton (4 direcciones).
// Se guarda en un arreglo de structs Sensacion de tamaño MAX_TURNOS.
// Esta es la "memoria perceptual" del agente: lo que percibio en cada momento.
// ─────────────────────────────────────────────────────────────────────────────
void registrar_sensacion(Sensacion sensaciones[], int &num_sensaciones,
                         int tablero[][MAX_COLUMNAS],
                         bool visitado[][MAX_COLUMNAS],
                         int filas, int columnas,
                         int turno, int fila_raton, int col_raton) {

    if (num_sensaciones >= MAX_TURNOS) return;

    Sensacion s;
    s.turno      = turno;
    s.fila_raton = fila_raton;
    s.col_raton  = col_raton;

    // Arriba
    int nf = fila_raton - 1, nc = col_raton;
    s.arriba       = (nf >= 0)              ? tablero[nf][nc]  : -1;
    s.visitado_arr = (nf >= 0)              ? visitado[nf][nc] : false;

    // Abajo
    nf = fila_raton + 1; nc = col_raton;
    s.abajo        = (nf < filas)           ? tablero[nf][nc]  : -1;
    s.visitado_aba = (nf < filas)           ? visitado[nf][nc] : false;

    // Izquierda
    nf = fila_raton; nc = col_raton - 1;
    s.izquierda    = (nc >= 0)              ? tablero[nf][nc]  : -1;
    s.visitado_izq = (nc >= 0)              ? visitado[nf][nc] : false;

    // Derecha
    nf = fila_raton; nc = col_raton + 1;
    s.derecha      = (nc < columnas)        ? tablero[nf][nc]  : -1;
    s.visitado_der = (nc < columnas)        ? visitado[nf][nc] : false;

    sensaciones[num_sensaciones] = s;
    num_sensaciones++;
}

// Imprime la matriz de sensaciones completa acumulada hasta el turno actual
void imprimir_matriz_sensaciones(Sensacion sensaciones[], int num_sensaciones) {
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "                  MATRIZ DE SENSACIONES\n";
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "Turno | Pos    | Arriba         | Abajo          | Izq            | Der\n";
    std::cout << "----------------------------------------------------------------\n";

    for (int i = 0; i < num_sensaciones; i++) {
        Sensacion &s = sensaciones[i];
        std::cout << "  " << s.turno;
        if (s.turno < 10)  std::cout << "   ";
        else if (s.turno < 100) std::cout << "  ";
        else std::cout << " ";

        std::cout << "| (" << s.fila_raton << "," << s.col_raton << ") | ";
        std::cout << nombre_valor(s.arriba)    << " | ";
        std::cout << nombre_valor(s.abajo)     << " | ";
        std::cout << nombre_valor(s.izquierda) << " | ";
        std::cout << nombre_valor(s.derecha)   << "\n";
    }

    std::cout << "Total sensaciones registradas: " << num_sensaciones << "\n";
    std::cout << "----------------------------------------------------------------\n\n";
}

// ─── Tabla de acciones ────────────────────────────────────────────────────────
void imprimir_tabla_acciones(const char acciones[][30], int num_acciones) {
    std::cout << "--------------------------------------------\n";
    std::cout << "           MATRIZ DE ACCIONES\n";
    std::cout << "--------------------------------------------\n";
    std::cout << "#     | Accion realizada\n";
    std::cout << "--------------------------------------------\n";

    for (int i = 0; i < num_acciones; i++) {
        std::cout << "  " << i + 1;
        if (i + 1 < 10)  std::cout << "   ";
        else if (i + 1 < 100) std::cout << "  ";
        else std::cout << " ";
        std::cout << "| " << acciones[i] << "\n";
    }

    std::cout << "Total de acciones: " << num_acciones << "\n";
    std::cout << "--------------------------------------------\n\n";
}