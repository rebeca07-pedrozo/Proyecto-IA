#include "agente.h"
#include "tablero.h"
#include <iostream>
#include <cstdlib>

// ─────────────────────────────────────────────────────────────────────────────
// INICIALIZAR AGENTE
// Coloca al raton en la esquina superior derecha (fila 0, ultima columna).
// Reinicia todos los contadores y el estado a sus valores iniciales.
// Se llama una vez al inicio de cada partida.
// ─────────────────────────────────────────────────────────────────────────────
void inicializar_agente(Agente &ag, int col_inicial) {
    ag.fila                 = 0;            // fila 0: fila superior
    ag.col                  = col_inicial;  // ultima columna: lado derecho
    ag.ultimo_valor_destino = VALOR_VACIO;  // no ha pisado nada aun
    ag.turno                = 1;            // empieza en turno 1
    ag.estado               = ESTADO_EN_CURSO; // juego activo
    ag.num_acciones         = 0;            // sin acciones registradas
    ag.num_sensaciones      = 0;            // sin sensaciones registradas
}

// ─────────────────────────────────────────────────────────────────────────────
// COPIAR STRING (funcion interna)
// Copia un string manualmente sin usar <string> ni strcpy.
// Copia hasta max-1 caracteres y agrega el terminador '\0'.
// Requerido porque el enunciado prohibe ciertas librerias.
// ─────────────────────────────────────────────────────────────────────────────
static void copiar_str(char* dest, const char* src, int max) {
    int i = 0;
    while (src[i] != '\0' && i < max - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0'; // terminador obligatorio
}

// ─────────────────────────────────────────────────────────────────────────────
// REGISTRAR ACCION
// Guarda la descripcion de una accion en la siguiente posicion libre
// del arreglo de acciones. Protege contra desbordamiento con MAX_TURNOS.
// ─────────────────────────────────────────────────────────────────────────────
void registrar_accion(Agente &ag, const char* descripcion) {
    if (ag.num_acciones < MAX_TURNOS) {
        copiar_str(ag.acciones[ag.num_acciones], descripcion, 30);
        ag.num_acciones++;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// INT A STRING (funcion interna)
// Convierte un entero a su representacion en texto sin usar sprintf ni
// to_string, ya que el enunciado restringe el uso de ciertas librerias.
// Algoritmo: extrae digitos de derecha a izquierda, luego invierte.
// ─────────────────────────────────────────────────────────────────────────────
static void int_a_str(int n, char* buf) {
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }
    char tmp[12];
    int i   = 0;
    bool neg = n < 0;
    if (neg) n = -n;
    while (n > 0) { tmp[i++] = '0' + (n % 10); n /= 10; }
    if (neg) tmp[i++] = '-';
    int j = 0;
    for (int k = i - 1; k >= 0; k--) buf[j++] = tmp[k];
    buf[j] = '\0';
}

// ─────────────────────────────────────────────────────────────────────────────
// ELEGIR ALEATORIO (funcion interna)
// Dado un arreglo de indices candidatos y su tamaño, retorna uno al azar.
// Usa rand() % n para seleccionar uniformemente entre los candidatos.
// Retorna -1 si el arreglo esta vacio (sin candidatos disponibles).
// ─────────────────────────────────────────────────────────────────────────────
static int elegir_aleatorio(int arr[], int n) {
    if (n <= 0) return -1;
    return arr[rand() % n];
}

// ─────────────────────────────────────────────────────────────────────────────
// MOVER AGENTE
// Nucleo de la inteligencia del agente. Evalua las 4 celdas adyacentes y
// elige a cual moverse segun un sistema de prioridades:
//
//   PRIORIDAD 1 - Queso: si hay queso adyacente, ir ahi de inmediato
//   PRIORIDAD 2 - Seguro nuevo: celda VACIA nunca visitada (conteo == 0)
//   PRIORIDAD 3 - Peligro trampa nuevo: zona nueva aunque con alerta
//   PRIORIDAD 4 - Poco visitado: celda visitada menos de MAX_VISITAS_CELDA veces
//   PRIORIDAD 5 - Cualquier celda valida: ultimo recurso
//
// Por que este orden:
//   - El queso es el objetivo, nunca se ignora
//   - Explorar zonas nuevas evita ciclos
//   - El conteo de visitas penaliza celdas muy repetidas
//   - El limite de turnos corta el juego si el raton queda atrapado
//
// Gatos y trampas directos NUNCA se priorizan: solo entran como ultimo recurso.
// ─────────────────────────────────────────────────────────────────────────────
void mover_agente(Agente &ag,
                  int tablero[][MAX_COLUMNAS],
                  bool visitado[][MAX_COLUMNAS],
                  int conteo[][MAX_COLUMNAS],
                  int filas, int columnas) {

    // Vectores de direccion: arriba(-1,0), abajo(+1,0), izq(0,-1), der(0,+1)
    int df[4] = {-1,  1,  0,  0};
    int dc[4] = { 0,  0, -1,  1};
    const char* nombres_dir[4] = {"Arriba", "Abajo", "Izquierda", "Derecha"};

    // Arreglos de candidatos por categoria de prioridad
    // Cada arreglo guarda indices (0-3) que corresponden a direcciones
    int queso[4],      n_queso      = 0; // direcciones con queso
    int nuevo[4],      n_nuevo      = 0; // direcciones con celda nueva segura
    int peligro_t[4],  n_peligro_t  = 0; // direcciones con peligro trampa nuevo
    int poco_visto[4], n_poco_visto = 0; // direcciones con celda poco visitada
    int todos[4],      n_todos      = 0; // todas las direcciones validas

    // Clasificar cada direccion posible en su categoria
    for (int d = 0; d < 4; d++) {
        int nf = ag.fila + df[d]; // fila de la celda vecina
        int nc = ag.col  + dc[d]; // columna de la celda vecina

        // Validacion de limites: ignorar celdas fuera del tablero
        if (nf < 0 || nf >= filas || nc < 0 || nc >= columnas) continue;

        int val   = tablero[nf][nc]; // que hay en esa celda
        int veces = conteo[nf][nc];  // cuantas veces el raton ya estuvo ahi

        todos[n_todos++] = d; // toda direccion valida entra en 'todos'

        if (val == VALOR_QUESO) {
            // Prioridad 1: objetivo del juego, moverse de inmediato
            queso[n_queso++] = d;

        } else if (val == VALOR_VACIO && veces == 0) {
            // Prioridad 2: celda limpia nunca visitada, explorar
            nuevo[n_nuevo++] = d;

        } else if (val == VALOR_PELIGRO_TRAMPA && veces == 0) {
            // Prioridad 3: zona nueva con alerta de trampa, arriesgado pero avanza
            peligro_t[n_peligro_t++] = d;

        } else if ((val == VALOR_VACIO         ||
                    val == VALOR_PELIGRO_TRAMPA  ||
                    val == VALOR_PELIGRO_GATO)
                    && veces < MAX_VISITAS_CELDA) {
            // Prioridad 4: ya visitada pero pocas veces, aun candidata
            poco_visto[n_poco_visto++] = d;
        }
        // Celdas con veces >= MAX_VISITAS_CELDA: solo en 'todos' (ultimo recurso)
        // Gatos y trampas directos: solo en 'todos' (se evitan siempre que se pueda)
    }

    // Seleccionar direccion segun prioridad descendente
    // El primer grupo con candidatos gana
    int dir = -1;
    if (dir == -1 && n_queso > 0)      dir = elegir_aleatorio(queso,      n_queso);
    if (dir == -1 && n_nuevo > 0)      dir = elegir_aleatorio(nuevo,      n_nuevo);
    if (dir == -1 && n_peligro_t > 0)  dir = elegir_aleatorio(peligro_t,  n_peligro_t);
    if (dir == -1 && n_poco_visto > 0) dir = elegir_aleatorio(poco_visto, n_poco_visto);
    if (dir == -1)                      dir = elegir_aleatorio(todos,      n_todos);

    // Si no hay absolutamente ningun movimiento posible, no hacer nada
    if (dir == -1) return;

    // Calcular nueva posicion
    int nueva_f = ag.fila + df[dir];
    int nueva_c = ag.col  + dc[dir];

    // Guardar que habia en la celda destino ANTES de pisarla
    // verificar_estado usara este valor para determinar victoria o derrota
    ag.ultimo_valor_destino = tablero[nueva_f][nueva_c];

    // Actualizar tablero: limpiar celda anterior
    tablero[ag.fila][ag.col]  = VALOR_VACIO;
    visitado[ag.fila][ag.col] = true; // marcar como visitada

    // Colocar raton en nueva posicion
    tablero[nueva_f][nueva_c] = VALOR_RATON;

    // Incrementar conteo de visitas de la celda destino
    conteo[nueva_f][nueva_c]++;

    // ─── Construir descripcion de la accion ──────────────────────────────────
    // Formato: "Mover Arriba (2,5)"
    // Se construye manualmente sin sprintf para cumplir restricciones del enunciado
    char desc[30];
    int  i = 0;

    // Agregar "Mover "
    const char* prefijo = "Mover ";
    for (int x = 0; prefijo[x] && i < 28; x++) desc[i++] = prefijo[x];

    // Agregar nombre de la direccion
    const char* dn = nombres_dir[dir];
    for (int x = 0; dn[x] && i < 28; x++) desc[i++] = dn[x];

    // Agregar coordenadas "(fila,col)"
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
// Revisa que habia en la celda a la que se movio el raton y actualiza
// ag.estado segun el resultado:
//   - Queso  → VICTORIA
//   - Gato   → DERROTA por gato
//   - Trampa → DERROTA por trampa
//   - Turno >= limite → LIMITE (evita loop infinito)
//   - Cualquier otro → EN_CURSO (el juego continua)
// ─────────────────────────────────────────────────────────────────────────────
void verificar_estado(Agente &ag, int limite_turnos) {
    if (ag.ultimo_valor_destino == VALOR_QUESO)  { ag.estado = ESTADO_VICTORIA; return; }
    if (ag.ultimo_valor_destino == VALOR_GATO)   { ag.estado = ESTADO_GATO;     return; }
    if (ag.ultimo_valor_destino == VALOR_TRAMPA) { ag.estado = ESTADO_TRAMPA;   return; }
    if (ag.turno >= limite_turnos)               { ag.estado = ESTADO_LIMITE;   return; }
    ag.estado = ESTADO_EN_CURSO;
}

// ─────────────────────────────────────────────────────────────────────────────
// MOSTRAR RESULTADO
// Imprime el mensaje final del juego segun el estado en que termino.
// Se llama una sola vez al finalizar el bucle principal.
// ─────────────────────────────────────────────────────────────────────────────
void mostrar_resultado(int estado) {
    std::cout << "\n========================================\n";
    switch (estado) {
        case ESTADO_VICTORIA:
            std::cout << "  ¡EL RATON ENCONTRO EL QUESO! ¡Gano!\n"; break;
        case ESTADO_GATO:
            std::cout << "  ¡El raton fue atrapado por un gato!\n";  break;
        case ESTADO_TRAMPA:
            std::cout << "  ¡El raton cayo en una trampa!\n";        break;
        case ESTADO_LIMITE:
            std::cout << "  Limite de turnos alcanzado.\n";           break;
        default:
            std::cout << "  Juego terminado.\n";                      break;
    }
    std::cout << "========================================\n";
}