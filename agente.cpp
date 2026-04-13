#include "agente.h"
#include "tablero.h"
#include <iostream>
#include <cstdlib>

// ─────────────────────────────────────────────────────────────────────────────
// INICIALIZACION DEL AGENTE
// El raton siempre arranca en fila 0, columna = columnas-1 (esquina sup derecha)
// Todos los contadores y el estado se ponen en cero.
// ─────────────────────────────────────────────────────────────────────────────
void inicializar_agente(Agente &ag, int col_inicial) {
    ag.fila                 = 0;
    ag.col                  = col_inicial;
    ag.ultimo_valor_destino = VALOR_VACIO;
    ag.turno                = 1;
    ag.estado               = ESTADO_EN_CURSO;
    ag.num_acciones         = 0;
    ag.num_sensaciones      = 0;
}

// ─── Copia manual de string (sin usar <string> ni strcpy) ────────────────────
static void copiar_str(char* dest, const char* src, int max) {
    int i = 0;
    while (src[i] != '\0' && i < max - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

// ─── Registrar accion en la matriz de acciones ───────────────────────────────
void registrar_accion(Agente &ag, const char* descripcion) {
    if (ag.num_acciones < MAX_TURNOS) {
        copiar_str(ag.acciones[ag.num_acciones], descripcion, 30);
        ag.num_acciones++;
    }
}

// ─── Convierte entero a string manualmente (sin sprintf ni to_string) ────────
static void int_a_str(int n, char* buf) {
    if (n == 0) { buf[0]='0'; buf[1]='\0'; return; }
    char tmp[12];
    int i = 0;
    bool neg = n < 0;
    if (neg) n = -n;
    while (n > 0) { tmp[i++] = '0' + (n % 10); n /= 10; }
    if (neg) tmp[i++] = '-';
    int j = 0;
    for (int k = i-1; k >= 0; k--) buf[j++] = tmp[k];
    buf[j] = '\0';
}

// ─── Elegir indice aleatorio de un arreglo de candidatos ─────────────────────
static int elegir_aleatorio(int arr[], int n) {
    if (n <= 0) return -1;
    return arr[rand() % n];
}

// ─────────────────────────────────────────────────────────────────────────────
// LOGICA DE MOVIMIENTO MEJORADA CON CONTEO DE VISITAS
//
// El problema anterior: el raton ciclaba entre celdas visitadas porque
// todas tenian el mismo "peso". Ahora se usa un conteo de visitas por celda:
// mientras mas veces visito una celda, menos prioridad tiene.
//
// Prioridades:
//   1. Queso                          → objetivo, moverse de inmediato
//   2. Seguro completamente nuevo     → nunca visitado, explorar
//   3. Peligro trampa nuevo           → arriesgado pero zona nueva
//   4. Poco visitado (< MAX_VISITAS)  → ya estuvo pero pocas veces
//   5. Cualquier celda valida         → ultimo recurso
//
// El conteo evita que el raton quede oscilando entre 2 celdas.
// ─────────────────────────────────────────────────────────────────────────────
void mover_agente(Agente &ag,
                  int tablero[][MAX_COLUMNAS],
                  bool visitado[][MAX_COLUMNAS],
                  int conteo[][MAX_COLUMNAS],
                  int filas, int columnas) {

    // Vectores de direccion: arriba, abajo, izquierda, derecha
    int df[4] = {-1,  1,  0,  0};
    int dc[4] = { 0,  0, -1,  1};
    const char* nombres_dir[4] = {"Arriba", "Abajo", "Izquierda", "Derecha"};

    // Arreglos de candidatos clasificados por prioridad
    int queso[4],      n_queso      = 0;
    int nuevo[4],      n_nuevo      = 0;
    int peligro_t[4],  n_peligro_t  = 0;
    int poco_visto[4], n_poco_visto = 0;
    int todos[4],      n_todos      = 0;

    for (int d = 0; d < 4; d++) {
        int nf = ag.fila + df[d];
        int nc = ag.col  + dc[d];

        // Validar limites del tablero — no salir de la matriz
        if (nf < 0 || nf >= filas || nc < 0 || nc >= columnas) continue;

        int val   = tablero[nf][nc];
        int veces = conteo[nf][nc]; // cuantas veces el raton ya paso por aqui

        todos[n_todos++] = d; // todo movimiento valido entra aqui como ultimo recurso

        if (val == VALOR_QUESO) {
            // Prioridad maxima: es el objetivo del juego
            queso[n_queso++] = d;

        } else if (val == VALOR_VACIO && veces == 0) {
            // Celda limpia y nunca visitada: explorar territorio nuevo
            nuevo[n_nuevo++] = d;

        } else if (val == VALOR_PELIGRO_TRAMPA && veces == 0) {
            // Zona nueva aunque con alerta de trampa: arriesgado pero avanza
            peligro_t[n_peligro_t++] = d;

        } else if ((val == VALOR_VACIO        ||
                    val == VALOR_PELIGRO_TRAMPA ||
                    val == VALOR_PELIGRO_GATO)
                    && veces < MAX_VISITAS_CELDA) {
            // Ya estuvo pero pocas veces: aun es candidato razonable
            poco_visto[n_poco_visto++] = d;
        }
        // Celdas con conteo >= MAX_VISITAS_CELDA quedan solo en 'todos'
        // Gatos y trampas directos nunca se priorizan, solo en 'todos'
    }

    // Seleccionar direccion segun prioridad descendente
    int dir = -1;
    if (dir == -1 && n_queso > 0)      dir = elegir_aleatorio(queso,      n_queso);
    if (dir == -1 && n_nuevo > 0)      dir = elegir_aleatorio(nuevo,      n_nuevo);
    if (dir == -1 && n_peligro_t > 0)  dir = elegir_aleatorio(peligro_t,  n_peligro_t);
    if (dir == -1 && n_poco_visto > 0) dir = elegir_aleatorio(poco_visto, n_poco_visto);
    if (dir == -1)                      dir = elegir_aleatorio(todos,      n_todos);

    if (dir == -1) return; // Sin ningun movimiento posible (caso extremo)

    int nueva_f = ag.fila + df[dir];
    int nueva_c = ag.col  + dc[dir];

    // Guardar que habia antes en la celda destino (para verificar_estado)
    ag.ultimo_valor_destino = tablero[nueva_f][nueva_c];

    // Limpiar celda anterior y marcarla como visitada
    tablero[ag.fila][ag.col]  = VALOR_VACIO;
    visitado[ag.fila][ag.col] = true;

    // Colocar raton en nueva posicion
    tablero[nueva_f][nueva_c] = VALOR_RATON;

    // Incrementar conteo de visitas de la celda destino
    conteo[nueva_f][nueva_c]++;

    // ─── Construir descripcion de la accion para la matriz ───────────────────
    // Formato final: "Mover Arriba (2,5)"
    char desc[30];
    int  i = 0;
    const char* prefijo = "Mover ";
    for (int x = 0; prefijo[x] && i < 28; x++) desc[i++] = prefijo[x];
    const char* dn = nombres_dir[dir];
    for (int x = 0; dn[x] && i < 28; x++) desc[i++] = dn[x];
    desc[i++] = ' '; desc[i++] = '(';
    char buf[6];
    int_a_str(nueva_f, buf);
    for (int x = 0; buf[x] && i < 28; x++) desc[i++] = buf[x];
    desc[i++] = ',';
    int_a_str(nueva_c, buf);
    for (int x = 0; buf[x] && i < 28; x++) desc[i++] = buf[x];
    desc[i++] = ')'; desc[i] = '\0';

    registrar_accion(ag, desc);

    // Actualizar posicion del agente
    ag.fila = nueva_f;
    ag.col  = nueva_c;
}

// ─────────────────────────────────────────────────────────────────────────────
// VERIFICAR ESTADO
// Se revisa que habia en la celda a la que se movio el raton.
// Si supera el limite de turnos se declara ESTADO_LIMITE (evita loop infinito).
// ─────────────────────────────────────────────────────────────────────────────
void verificar_estado(Agente &ag, int limite_turnos) {
    if (ag.ultimo_valor_destino == VALOR_QUESO)  { ag.estado = ESTADO_VICTORIA; return; }
    if (ag.ultimo_valor_destino == VALOR_GATO)   { ag.estado = ESTADO_GATO;     return; }
    if (ag.ultimo_valor_destino == VALOR_TRAMPA) { ag.estado = ESTADO_TRAMPA;   return; }
    if (ag.turno >= limite_turnos)               { ag.estado = ESTADO_LIMITE;   return; }
    ag.estado = ESTADO_EN_CURSO;
}

// ─── Mostrar resultado final ──────────────────────────────────────────────────
void mostrar_resultado(int estado) {
    std::cout << "\n========================================\n";
    switch (estado) {
        case ESTADO_VICTORIA: std::cout << "  ¡EL RATON ENCONTRO EL QUESO! ¡Gano!\n"; break;
        case ESTADO_GATO:     std::cout << "  ¡El raton fue atrapado por un gato!\n"; break;
        case ESTADO_TRAMPA:   std::cout << "  ¡El raton cayo en una trampa!\n";        break;
        case ESTADO_LIMITE:   std::cout << "  Limite de turnos alcanzado.\n";           break;
        default:              std::cout << "  Juego terminado.\n";                      break;
    }
    std::cout << "========================================\n";
}