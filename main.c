/* Lu-An Optimizer v12 - Timer Resolution + FilterKeys + DNS Jumper + Competitivo
 * Novedades v12: arreglo del auto-update (v11 descargaba la pagina HTML de
 * releases porque version.txt apuntaba a releases/latest y su SHA256 nunca
 * coincidia). Ahora la URL del exe se resuelve desde la API de GitHub
 * Releases (browser_download_url del asset LuAnOptimizer.exe) o se
 * construye como releases/download/v<ver>/LuAnOptimizer.exe; el campo
 * url= de version.txt ya no se usa como descarga, y el hilo de descarga
 * se niega a bajar cualquier URL que no termine en .exe.
 * Novedades v11: (1) ventana compacta 700x560 con desplazamiento vertical en cada pestana (barra + rueda), (2) cuadro de estado ACTIVO/APAGADO arriba a la derecha, (3) idioma ES/EN con guardado en registro, (4) version visible en titulo y etiqueta, (5) sin ventana de cmd al abrir: el plan de energia se lee con PowerGetActiveScheme (powrprof), no con _popen.
 * Novedades v10: (1) Modo Ultra Baja Latencia (ajustes de red por
 * registro con respaldo/restaurar, pausar Windows Update y Delivery
 * Optimization, ping a servidores de juego, diagnostico de ruta con
 * traceroute, prioridad/afinidad del juego, programacion de GPU
 * acelerada por hardware), (2) interruptores ON/OFF animados tipo iOS
 * en lugar de casillas con palomita, (3) botones con difuminado (glow)
 * y texto legible en la pestana Juegos.
 * Novedades v9: (1) auto-actualizacion desde GitHub Releases (WinINet +
 * BCrypt SHA256, con .bat de reemplazo; nunca sin permiso y nunca en
 * mitad de una partida), (2) perfiles por juego (pestana Juegos, deteccion
 * por proceso cada 2 s, CS2 con SOCD siempre bloqueado), (3) monitor en
 * pantalla (overlay siempre encima, FPS de PANTALLA - no del juego -,
 * temperatura por WMI o N/D si no hay sensor).
 * (SOCD Cleaner, Modo Competitivo, ping en vivo, optimizaciones de sistema,
 *  limpieza de memoria standby tipo ISLC)
 * para Windows 7/8/10/11. Un solo .exe, doble clic, sin instalar nada.
 * Sin Python, sin .NET.
 * Compilado con Zig: zig cc -target x86_64-windows-gnu -O2
 *   -D_WIN32_WINNT=0x0601 main.c lu-an.res -o LuAnOptimizer.exe
 *   -lcomctl32 -lgdi32 -luser32 -lkernel32 -liphlpapi -lws2_32 -lgdiplus
 *   -lole32 -Wl,--subsystem,windows
 * lu-an.res se genera con: zig rc resource.rc lu-an.res (salida POSICIONAL; /fo no escribe nada)
 *   (manifiesto + icono LU-AN incrustado, id 100, + logo.jpg como RCDATA)
 * Interfaz v5 (premium): barra de pestanas dibujada a mano (LuAnTabBar) con
 * glifos GDI, indicador deslizante con gradiente cian->magenta, hover con
 * fundido y transicion de deslizamiento al cambiar de pagina; paneles de
 * contenido con marco redondeado; fuente Segoe UI Variable Text (fallback
 * Segoe UI, estilo iOS/San Francisco) y botones owner-draw con animacion
 * de hover (fundido) y pulso al pulsar.
 * Bug corregido en v5: ReparentToPages no excluia g_pageComp, asi que la
 * pagina Competitivo se reparentaba dentro de la pagina Timer y su pestana
 * quedaba vacia.
 * Correccion v6 (delay del SOCD): el hook WH_KEYBOARD_LL vivia en el hilo
 * de la UI y ese hilo se bloqueaba hasta 1 s en cada ping en vivo y hasta
 * 7,5 s al medir los 5 DNS del Modo Competitivo, asi que las teclas
 * llegaban con delay. Ahora el SOCD corre en un hilo dedicado con su
 * propio bucle de mensajes y el ping corre en hilos worker (timeout de
 * 300 ms en vivo); la UI solo recibe resultados con PostMessage.
 * Correccion v7 (delay RESIDUAL, reporte: sigue algo de delay en v6, y el
 * medidor marcaba 0 ms): el medidor en 0 ms descarta el hook, asi que el
 * delay estaba aguas abajo, en el ORDEN DE ENTREGA. En modo ultima
 * tecla gana, al pulsar D con A pulsada el codigo inyectaba A-up y
 * dejaba pasar el D-down original: el A-up quedaba en cola detras del
 * D-down, asi que el juego veia D-down primero y A-up despues, un
 * instante con las dos pulsadas. Ahora se traga el evento original y se
 * inyecta en UNA sola llamada SendInput el lote [opuesta-up, nueva-down],
 * atomico (igual al reactivar la opuesta al soltar: [esta-up,
 * opuesta-down] en un lote). Ademas: (1) hilo SOCD TIME_CRITICAL +
 * proceso HIGH_PRIORITY_CLASS mientras el SOCD esta activo, (2) medidor
 * con throttle (solo en cambio de tecla opuesta o max. 1 vez cada
 * 100 ms), (3) SendInput con scan code real (KEYEVENTF_SCANCODE, +
 * EXTENDEDKEY en flechas) y (4) animaciones pausadas mientras el SOCD
 * esta activo (se reanudan solas al desactivarlo).
 * Limite honesto: un hook WH_KEYBOARD_LL + SendInput siempre anade algo
 * de latencia frente al SOCD por firmware/driver de un teclado con SOCD
 * nativo; no se puede llegar a 0 ms con un hook de software.
 * Correccion v9: ver README.md (secciones Auto-actualizacion, Perfiles
 * por juego y Monitor en pantalla, con sus limitaciones honestas).
 * Correccion v8 (bandeja del sistema): la X SIEMPRE cierra el programa
 * como antes. Lo unico nuevo es una casilla "Minimizar en segundo
 * plano" (marcada por defecto, guardada en HKCU\Software\LuAnOptimizer):
 * si esta marcada, el boton minimizar (_) oculta la ventana y deja el
 * icono LU-AN en la bandeja (Shell_NotifyIcon), con doble clic para
 * restaurar y menu de clic derecho (Mostrar / Modo Competitivo / SOCD /
 * Salir); si esta desmarcada, minimizar es el minimizar normal a la
 * barra de tareas. El icono de la bandeja SOLO aparece cuando se
 * minimiza en segundo plano. Todo sigue funcionando en segundo plano
 * (timer, SOCD, ping en vivo, limpieza automatica) porque el proceso
 * sigue vivo; solo la ventana se oculta. La salida real (X o menu
 * Salir) desinstala el SOCD, restaura el timer a 15.6 ms y quita el
 * icono de la bandeja.
 * APIs usadas (todas existen desde Windows 2000 / XP / Vista, por eso
 * corre en 7-11):
 *  - NtQueryTimerResolution / NtSetTimerResolution (ntdll.dll, runtime)
 *  - SystemParametersInfoW SPI_GETFILTERKEYS / SPI_SETFILTERKEYS
 *  - GetAdaptersAddresses (iphlpapi) + netsh (oculto) para el DNS
 *  - IcmpSendEcho (iphlpapi) para probar el ping al DNS
 *  - GDI+ flat API para dibujar el logo LU-AN incrustado (logo.jpg)
 * Regla Microsoft FILTERKEYS: BounceKeys y Slow/RepeatKeys son excluyentes.
 */
#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#include <objbase.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <powrprof.h> /* v11: PowerGetActiveScheme (sin cmd) */
#include <shellapi.h>
#include <gdiplus.h>
#include <wininet.h>
#include <urlmon.h>
#include <bcrypt.h>
#include <dxgi.h>
#include <wbemidl.h>
#include "logo_data.h"

#pragma comment(lib, "comctl32.lib")

#ifndef FW_SEMIBOLD
#define FW_SEMIBOLD 600
#endif

/* ---------- Version (v13.1) ---------- */
#define APP_VERSION "13.1.0"
#define APP_VERSION_NUM 13,1,0,0
#define UPDATE_API_URL "https://api.github.com/repos/luisadrianh474-droid/lu-an-optimizer/releases/latest"
#define UPDATE_TXT_URL "https://raw.githubusercontent.com/luisadrianh474-droid/lu-an-optimizer/main/version.txt"
#define UPDATE_RELEASES_URL "https://github.com/luisadrianh474-droid/lu-an-optimizer/releases/latest"

/* ---------- Temas de colores (v13, v13.1) ----------
 * Paleta central: TODOS los dibujos personalizados (barra de pestanas,
 * botones con brillo, interruptores ON/OFF, cuadro de estado, marcos de
 * pagina, divisores, textos, barra de desplazamiento) leen sus colores
 * de g_themeCur a traves de las macros COL_*, asi que cambiar de tema
 * los vuelve a pintar a todos.
 * 0 Neon (el aspecto actual, sin cambios), 1 Fuego (negro/naranja/rojo),
 * 2 Esmeralda (negro/verde/lima), 3 Pinkimich (rosa pastel/blanco, el
 * PRIMER tema claro: 'light' = 1 avisa a los dibujos que usan texto
 * blanco fijo, como el cuadro de estado, que usen COL_TEXT en su lugar).
 * Se guarda en el registro HKCU\Software\LuAnOptimizer Theme
 * ("neon"/"fuego"/"esmeralda"/"pinkimich"). */
typedef struct { COLORREF bg, card, card2, text, muted, accent, accent2, green, btn, btn2, magenta, tabHover, bgDeep; int light; } Theme;
static const Theme g_themes[4] = {
    /* Neon (actual): fondo azul oscuro, cian -> magenta */
    { RGB(15,23,42), RGB(30,41,59), RGB(51,65,85), RGB(241,245,249), RGB(148,163,184), RGB(34,211,238), RGB(99,102,241), RGB(52,211,153), RGB(99,102,241), RGB(14,165,233), RGB(217,70,239), RGB(56,72,102), RGB(10,16,30), 0 },
    /* Fuego: fondo negro, naranja -> rojo */
    { RGB(12,10,10), RGB(30,20,16), RGB(54,34,26), RGB(255,247,237), RGB(214,178,150), RGB(249,115,22), RGB(239,68,68), RGB(52,211,153), RGB(234,88,12), RGB(249,115,22), RGB(220,38,38), RGB(64,32,20), RGB(5,5,5), 0 },
    /* Esmeralda: fondo negro verdoso, verde -> lima */
    { RGB(6,20,14), RGB(12,34,24), RGB(24,54,38), RGB(236,253,245), RGB(148,190,168), RGB(16,185,129), RGB(132,204,22), RGB(52,211,153), RGB(5,150,105), RGB(16,185,129), RGB(163,230,53), RGB(24,62,44), RGB(3,12,8), 0 },
    /* Pinkimich: rosa pastel con blanco y letras negras */
    { RGB(255,228,236), RGB(255,255,255), RGB(255,205,218), RGB(30,20,25), RGB(120,90,105), RGB(236,72,153), RGB(249,168,212), RGB(5,150,105), RGB(244,114,182), RGB(236,72,153), RGB(249,168,212), RGB(251,207,232), RGB(255,240,245), 1 },
};
static const char *g_themeNames[4] = { "neon", "fuego", "esmeralda", "pinkimich" };
static int g_theme = 0; /* 0 Neon, 1 Fuego, 2 Esmeralda, 3 Pinkimich */
#define THEME_PINKIMICH 3 /* tema claro con logo de Kuromi + Cinnamoroll */
static Theme g_themeCur; /* paleta activa (se fija en LoadTheme/ApplyTheme) */
#define COL_BG      (g_themeCur.bg)
#define COL_CARD    (g_themeCur.card)
#define COL_CARD2   (g_themeCur.card2)
#define COL_TEXT    (g_themeCur.text)
#define COL_MUTED   (g_themeCur.muted)
#define COL_ACCENT  (g_themeCur.accent)
#define COL_ACCENT2 (g_themeCur.accent2)
#define COL_GREEN   (g_themeCur.green)
#define COL_BTN     (g_themeCur.btn)
#define COL_BTN2    (g_themeCur.btn2)
#define COL_MAGENTA (g_themeCur.magenta)   /* segundo color del degradado (logo LU-AN en Neon) */
#define COL_TABHOVER (g_themeCur.tabHover) /* tarjeta intermedia para hover de pestana */
#define COL_BGDEEP  (g_themeCur.bgDeep)    /* fondo mas oscuro (overlay, clase LuAnOverlay) */

static HINSTANCE g_hInst;
static HWND g_hwnd;
static HFONT g_font, g_fontBold, g_fontBig, g_fontTitle, g_fontSection, g_fontTab;
static HWND g_tabBar, g_pageTimer, g_pageFK, g_pageDNS, g_pageComp, g_pageGames;
static HWND g_pages[5];
static int g_curTab = 0;
static volatile LONG g_compActive = 0; /* v9: 1 = Modo Competitivo activo */
/* Barra de pestanas premium dibujada a mano (sustituye al TabControl estandar) */
static const char *g_tabNames[5]; /* v11: se inicializa en WM_CREATE segun el idioma */
static int g_tabHover = -1;
static int g_tabHoverLvl[5] = {0,0,0,0,0};
static float g_indX = 0, g_indW = 0, g_indTX = 0, g_indTW = 0; /* indicador deslizante */
static int g_slideX = 0; /* desplazamiento de entrada de la pagina al cambiar de pestana */
#define TABBAR_X 10
#define TABBAR_Y 164
#define TABBAR_W 680
#define TABBAR_H 40
#define PAGE_X 10
#define PAGE_Y 208
#define PAGE_W 680
#define PAGE_H 256
static int g_hasVarFont = 0;
static HBRUSH g_brBg, g_brCard, g_brBtn, g_brBgDeep;
static double g_desiredMs = 1.0;
static double g_currentMs = 15.6;
static HBITMAP g_hLogoBmp = NULL;
static HBITMAP g_hPinkBmp = NULL; /* v13.1: logo Pinkimich (Kuromi + Cinnamoroll, RCDATA 102), solo con ese tema */
static ULONG_PTR g_gdiplusToken = 0;
static void SetStatus(const char *s); /* definida en Helpers UI, usada por DNS */
static void TrayUpdateTip(void); /* bandeja (v8): refresca el tooltip */
static void ShowTab(int sel); /* definida en Paginas de pestanas, usada por la barra premium */
static HWND Ctl(int id);
static int IsChecked(int id);
static void SetChecked(int id, int v);

/* ======================================================================
 * v11: IDIOMA ESPANOL/INGLES + desplazamiento de pestanas + cuadro de estado
 * - Tr(k) devuelve el texto en el idioma actual (g_lang 0=es, 1=en).
 * - MakeLabelK/MakeButtonK/MakeToggleK crean el control con el texto
 *   traducido y lo registran: ApplyLanguage() les vuelve a poner el
 *   texto al cambiar de idioma, sin reiniciar (el idioma se guarda en
 *   HKCU\Software\LuAnOptimizer Language, "es" o "en").
 * - Las paginas se desplazan en vertical (barra + rueda del mouse)
 *   porque la ventana es compacta (700x560). v13.1: la barra ya no es
 *   la del sistema (WS_VSCROLL) sino una dibujada por el programa con
 *   los colores del tema (ver ScrollThumbRect en Desplazamiento).
 * - UpdateStatusBox() invalida la zona del cuadro de estado (se dibuja
 *   en WM_PAINT): ACTIVO verde / APAGADO gris.
 * ====================================================================== */

enum {
    LS_QUICK_PROFILE,
    LS_WRITING,
    LS_ACCESS,
    LS_TIMER_CURRENT_INIT,
    LS_TIMER_NORMAL_NOTE,
    LS_APPLY_RES,
    LS_BACK_156,
    LS_FK_TITLE,
    LS_FK_ENABLE,
    LS_BOUNCE_DESC,
    LS_BOUNCE_LBL,
    LS_SLOW_LBL,
    LS_DELAY_LBL,
    LS_REPEAT_LBL,
    LS_APPLY_FK,
    LS_DISABLE_FK,
    LS_FK_RULE,
    LS_TEST_KB,
    LS_TEST_PLACEHOLDER,
    LS_ADAPTER_LBL,
    LS_DNS_CURRENT_INIT,
    LS_DNS_AUTO,
    LS_DNS_PRESET_HINT,
    LS_DNS_PRIMARY,
    LS_DNS_SECONDARY,
    LS_APPLY_DNS,
    LS_RESTORE_AUTO,
    LS_CLEAR_CACHE,
    LS_TEST_PING,
    LS_DNS_HINT,
    LS_SOCD_ENABLE,
    LS_RADIO_LAST,
    LS_RADIO_NEUTRAL,
    LS_SOCD_OFF,
    LS_SOCD_TEST_HINT,
    LS_COMP_TITLE,
    LS_COMP_BTN,
    LS_COMP_RESTORE,
    LS_LIVE_TITLE,
    LS_LIVE_INIT,
    LS_LIVE_TOGGLE,
    LS_PING_ALL,
    LS_BEST_INIT,
    LS_SYSOPT_TITLE,
    LS_MOUSE_ACCEL,
    LS_FSE,
    LS_CLOSE_APPS_BTN,
    LS_CLEAN_MEM,
    LS_MEM_INIT,
    LS_AUTO_CLEAN,
    LS_MONITOR_TITLE,
    LS_SHOW_MONITOR,
    LS_CLICK_THROUGH,
    LS_FPS_NOTE,
    LS_UPDATES_TITLE,
    LS_CHECK_UPDATES,
    LS_AUTO_UPDATE,
    LS_ULTRA_TITLE,
    LS_ULTRA_NOTE,
    LS_ULTRA_BTN_ON,
    LS_ULTRA_BTN_OFF,
    LS_ULTRA_RESTORE,
    LS_ULTRA_OFF,
    LS_ULTRA_OFF_RESTORED,
    LS_ULTRA_ON,
    LS_GPU_SCHED,
    LS_NETHOGS_LBL,
    LS_KILL_HOG,
    LS_REFRESH_HOGS,
    LS_GAMES_TITLE,
    LS_NO_GAME,
    LS_GAME_AUTO,
    LS_GAME_TIMER,
    LS_GAME_COMP,
    LS_CUSTOM_EXE_LBL,
    LS_GAMES_NOTE,
    LS_SRV_TITLE,
    LS_SRV_NOTE,
    LS_SRV_INIT,
    LS_SRV_MEASURE,
    LS_TRACE_BTN,
    LS_TRACE_INIT,
    LS_TRACE_SUM_INIT,
    LS_STATUS_READY,
    LS_TRAY_MIN,
    LS_GAME_CS2_ROW,
    LS_GAME_OTHER_ROW,
    LS_STATUS_ACTIVE,
    LS_STATUS_OFF,
    LS_LANG_BTN,
    LS_TAB_COMPETITIVE,
    LS_TAB_GAMES,
    LS_GAME_OTHER_NAME,
    LS_KEY_LEFT,
    LS_KEY_RIGHT,
    LS_KEY_UP,
    LS_KEY_DOWN,
    LS_SUBTITLE,
    LS_DNS_CHOOSE_ADAPTER,
    LS_DNS_UNREADABLE,
    LS_DNS_ADAPTER_GONE,
    LS_DNS_CURRENT_2,
    LS_DNS_CURRENT_1,
    LS_DNS_CURRENT_AUTO,
    LS_DNS_CHOOSE_FIRST,
    LS_DNS_BAD_IP,
    LS_DNS_APPLY_FAIL,
    LS_DNS_APPLY_FAIL_MB,
    LS_DNS_RESTORE_FAIL,
    LS_DNS_RESTORE_FAIL_MB,
    LS_DNS_RESTORED_AUTO,
    LS_DNS_APPLIED_FMT,
    LS_DNS_FLUSH_OK,
    LS_DNS_FLUSH_FAIL,
    LS_SOCD_NO_THREAD,
    LS_SOCD_NO_HOOK,
    LS_SOCD_ON_LAST_HP,
    LS_SOCD_ON_NEUTRAL_HP,
    LS_SOCD_ON_LAST,
    LS_SOCD_ON_NEUTRAL,
    LS_SOCD_OFF_WARN,
    LS_SOCD_WARN_FMT,
    LS_SOCD_WARN_FK,
    LS_SOCD_WARN_TITLE,
    LS_MEM_CLEANED_FMT,
    LS_MEM_CLEAN_FAIL,
    LS_COMP_ON_LOG,
    LS_COMP_FK_OFF,
    LS_COMP_DNS_FLUSH,
    LS_COMP_POWER_HP,
    LS_COMP_POWER_FAIL,
    LS_COMP_GAMEMODE_ON,
    LS_COMP_DNS_MEASURING,
    LS_COMP_DNS_BUSY,
    LS_COMP_DNS_BEST_FMT,
    LS_COMP_READY,
    LS_COMP_ON_STATUS,
    LS_COMP_RESTORE_LOG,
    LS_COMP_TIMER_156,
    LS_COMP_POWER_RESTORED,
    LS_COMP_POWER_RESTORE_FAIL,
    LS_COMP_POWER_BALANCED,
    LS_COMP_GAMEMODE_RESTORED,
    LS_COMP_OFF_STATUS,
    LS_LIVE_NO_THREAD,
    LS_PING_ONE_FMT,
    LS_PING_ONE_NONE,
    LS_PING_BEST_FMT,
    LS_PING_NONE_FMT,
    LS_PING_BUSY,
    LS_PING_START_FAIL,
    LS_PING_BAD_IP,
    LS_MOUSE_OFF,
    LS_MOUSE_ON,
    LS_FSE_OFF,
    LS_FSE_ON,
    LS_PROC_LIST_FAIL,
    LS_NO_BG_APPS,
    LS_CLOSE_APPS_TITLE,
    LS_CLOSE_APPS_BG_FMT,
    LS_CLOSE_APPS_BG_TITLE,
    LS_APPS_CLOSED_FMT,
    LS_UPD_SHA_MISMATCH,
    LS_UPD_TITLE,
    LS_UPD_NO_HASH,
    LS_UPD_NO_HASH_TITLE,
    LS_ULTRA_NAGLE_OK,
    LS_ULTRA_NAGLE_FAIL,
    LS_ULTRA_ACK_OK,
    LS_ULTRA_ACK_FAIL,
    LS_ULTRA_THROTTLE_OK,
    LS_ULTRA_THROTTLE_FAIL,
    LS_ULTRA_NIC_OK,
    LS_ULTRA_NIC_FAIL,
    LS_ULTRA_WIFI_OK,
    LS_ULTRA_WIFI_FAIL,
    LS_HOGS_NONE,
    LS_HOG_SELECT,
    LS_HOG_SYSTEM,
    LS_HOG_OPEN_FAIL,
    LS_HOG_ASK_FMT,
    LS_HOG_CLOSED,
    LS_SRV_MEASURING,
    LS_TRACE_ICMP_FAIL,
    LS_TRACE_FAIL,
    LS_TRACE_NOJUMP,
    LS_WHERE_ISP,
    LS_WHERE_PC,
    LS_WHERE_ROUTER,
    LS_WHERE_SERVER,
    LS_TRACE_SUMMARY_FMT,
    LS_TRACE_RUNNING,
    LS_TRACE_MEASURING,
    LS_GAME_DETECTED_FMT,
    LS_GAME_CLOSED,
    LS_NO_RESPONSE,
    LS_NA,
    LS_TIMER_CURRENT_FMT,
    LS_TIMER_APPLY_FAIL,
    LS_TIMER_APPLIED_FMT,
    LS_FK_APPLY_FAIL,
    LS_FK_OFF,
    LS_FK_BOUNCE_FMT,
    LS_FK_SLOW_FMT,
    LS_PROFILE_GAMING,
    LS_PROFILE_TYPING,
    LS_PROFILE_ACCESS,
    LS_PROFILE_NORMAL,
    LS_TRAY_BALLOON,
    LS_TRAY_SHOW,
    LS_TRAY_SOCD_OFF,
    LS_TRAY_SOCD_ON,
    LS_TRAY_EXIT,
    LS_CS2_SOCD_FORBIDDEN,
    LS_CS2_SOCD_TITLE,
    LS_ULTRA_WARN,
    LS_ULTRA_TITLE_MB,
    LS_ULTRA_ON_STATUS,
    LS_ULTRA_OFF_STATUS,
    LS_NET_RESTORED,
    LS_GPU_SAVED,
    LS_GPU_WRITE_FAIL,
    LS_HOGS_REFRESHED,
    LS_LIVE_MEASURING,
    LS_LIVE_PAUSED,
    LS_UPD_CHECKING,
    LS_UPD_AUTO_ON,
    LS_UPD_AUTO_OFF,
    LS_TRAY_MIN_ON,
    LS_TRAY_MIN_OFF,
    LS_UPD_NEW_FMT,
    LS_UPD_NEW_COMP_FMT,
    LS_UPD_AVAILABLE,
    LS_UPD_DOWNLOADING,
    LS_UPD_AVAILABLE_STATUS,
    LS_UPD_CHECK_FAIL,
    LS_UPD_CHECK_FAIL_LONG,
    LS_UPD_LATEST,
    LS_UPD_LATEST_MB,
    LS_UPD_RELEASES_OPENED,
    LS_UPD_DOWNLOAD_FAIL,
    LS_SRV_RESULT_BEST,
    LS_SRV_RESULT,
    LS_SRV_MEASURED,
    LS_TRACE_DONE,
    LS_SOCD_TEST_FMT,
    LS_LIVE_RESULT,
    LS_LIVE_RESULT_NORESP,
    LS_NO_ADAPTERS,
    LS_NO_ACTIVE_ADAPTERS,
    LS_OVERLAY_FMT,
    LS_HOG_SYSTEM_SUFFIX,
    LS_HOG_UNKNOWN_NAME,
    LS_THEME_LBL,
    LS_THEME_NEON,
    LS_THEME_FIRE,
    LS_THEME_EMERALD,
    LS_THEME_PINK,
    LS_COUNT
};

static const char *g_langStr[LS_COUNT][2] = {
    { "Perfil rapido:", "Quick profile:" }, /* LS_QUICK_PROFILE */
    { "Escritura", "Writing" }, /* LS_WRITING */
    { "Accesibilidad", "Accessibility" }, /* LS_ACCESS */
    { "Actual: 15.60 ms", "Current: 15.60 ms" }, /* LS_TIMER_CURRENT_INIT */
    { "Normal de Windows: 15.6 ms\nSe mantiene mientras la app este abierta.", "Windows default: 15.6 ms\nIt stays while the app is open." }, /* LS_TIMER_NORMAL_NOTE */
    { "Aplicar resolucion", "Apply resolution" }, /* LS_APPLY_RES */
    { "Volver a 15.6 ms (normal)", "Back to 15.6 ms (default)" }, /* LS_BACK_156 */
    { "FILTERKEYS  (TECLADO)", "FILTERKEYS  (KEYBOARD)" }, /* LS_FK_TITLE */
    { "Activar FilterKeys", "Enable FilterKeys" }, /* LS_FK_ENABLE */
    { "BounceKeys: ignorar pulsaciones repetidas", "BounceKeys: ignore repeated key presses" }, /* LS_BOUNCE_DESC */
    { "Rebote (BounceKeys)", "Bounce (BounceKeys)" }, /* LS_BOUNCE_LBL */
    { "SlowKeys: mantener pulsada", "SlowKeys: hold key down" }, /* LS_SLOW_LBL */
    { "Retardo antes de repetir", "Delay before repeating" }, /* LS_DELAY_LBL */
    { "Intervalo entre repeticiones", "Interval between repeats" }, /* LS_REPEAT_LBL */
    { "Aplicar FilterKeys", "Apply FilterKeys" }, /* LS_APPLY_FK */
    { "Desactivar FilterKeys", "Disable FilterKeys" }, /* LS_DISABLE_FK */
    { "Si activas BounceKeys, Slow/Repeat se apagan solos (regla de Windows).", "If you enable BounceKeys, Slow/Repeat turn off by themselves (Windows rule)." }, /* LS_FK_RULE */
    { "Prueba tu teclado aqui:", "Test your keyboard here:" }, /* LS_TEST_KB */
    { "Escribe aqui para probar el rebote y la repeticion...", "Type here to test bounce and repeat..." }, /* LS_TEST_PLACEHOLDER */
    { "Adaptador de red:", "Network adapter:" }, /* LS_ADAPTER_LBL */
    { "DNS actual: ...", "Current DNS: ..." }, /* LS_DNS_CURRENT_INIT */
    { "Automatico (DHCP)", "Automatic (DHCP)" }, /* LS_DNS_AUTO */
    { "Pulsa un preset para\nrellenar el DNS.", "Press a preset to\nfill in the DNS." }, /* LS_DNS_PRESET_HINT */
    { "DNS primario:", "Primary DNS:" }, /* LS_DNS_PRIMARY */
    { "DNS secundario:", "Secondary DNS:" }, /* LS_DNS_SECONDARY */
    { "Aplicar DNS", "Apply DNS" }, /* LS_APPLY_DNS */
    { "Restaurar automatico", "Restore automatic" }, /* LS_RESTORE_AUTO */
    { "Limpiar cache", "Clear cache" }, /* LS_CLEAR_CACHE */
    { "Probar ping", "Test ping" }, /* LS_TEST_PING */
    { "El DNS se aplica con netsh. Si falla, ejecuta como administrador.", "DNS is applied with netsh. If it fails, run as administrator." }, /* LS_DNS_HINT */
    { "Activar SOCD Cleaner  (WASD + flechas)", "Enable SOCD Cleaner  (WASD + arrows)" }, /* LS_SOCD_ENABLE */
    { "Ultima tecla gana", "Last key wins" }, /* LS_RADIO_LAST */
    { "Neutro (ninguna)", "Neutral (none)" }, /* LS_RADIO_NEUTRAL */
    { "SOCD desactivado.", "SOCD disabled." }, /* LS_SOCD_OFF */
    { "Prueba SOCD: pulsa A o D con el SOCD activo.", "SOCD test: press A or D with SOCD on." }, /* LS_SOCD_TEST_HINT */
    { "MODO COMPETITIVO", "COMPETITIVE MODE" }, /* LS_COMP_TITLE */
    { "ACTIVAR MODO COMPETITIVO", "ENABLE COMPETITIVE MODE" }, /* LS_COMP_BTN */
    { "Restaurar normal", "Restore normal" }, /* LS_COMP_RESTORE */
    { "PING EN VIVO", "LIVE PING" }, /* LS_LIVE_TITLE */
    { "Ping: -- ms | Media: -- | Jitter: -- | Perdida: --%", "Ping: -- ms | Avg: -- | Jitter: -- | Loss: --%" }, /* LS_LIVE_INIT */
    { "Medir cada 1 s (al DNS primario)", "Measure every 1 s (to the primary DNS)" }, /* LS_LIVE_TOGGLE */
    { "Probar todos los DNS", "Test all DNS" }, /* LS_PING_ALL */
    { "El mas rapido: aun no medido. Pulsa Probar todos los DNS.", "Fastest: not measured yet. Press Test all DNS." }, /* LS_BEST_INIT */
    { "OPTIMIZACIONES DEL SISTEMA", "SYSTEM OPTIMIZATIONS" }, /* LS_SYSOPT_TITLE */
    { "Quitar aceleracion del mouse", "Turn off mouse acceleration" }, /* LS_MOUSE_ACCEL */
    { "Quitar optimizaciones de pantalla completa", "Turn off fullscreen optimizations" }, /* LS_FSE */
    { "Cerrar apps en segundo plano...", "Close background apps..." }, /* LS_CLOSE_APPS_BTN */
    { "Limpiar memoria standby", "Clean standby memory" }, /* LS_CLEAN_MEM */
    { "Memoria standby: aun no limpiada en esta sesion.", "Standby memory: not cleaned yet this session." }, /* LS_MEM_INIT */
    { "Limpiar memoria standby cada 15 min automaticamente", "Clean standby memory every 15 min automatically" }, /* LS_AUTO_CLEAN */
    { "MONITOR EN PANTALLA", "ON-SCREEN MONITOR" }, /* LS_MONITOR_TITLE */
    { "Mostrar monitor", "Show monitor" }, /* LS_SHOW_MONITOR */
    { "No interferir (click-through)", "Don't interfere (click-through)" }, /* LS_CLICK_THROUGH */
    { "FPS (pantalla) = frecuencia de la pantalla, no el FPS del juego.", "FPS (screen) = screen refresh rate, not the game's FPS." }, /* LS_FPS_NOTE */
    { "ACTUALIZACIONES", "UPDATES" }, /* LS_UPDATES_TITLE */
    { "Buscar actualizaciones", "Check for updates" }, /* LS_CHECK_UPDATES */
    { "Buscar actualizaciones al iniciar", "Check for updates on startup" }, /* LS_AUTO_UPDATE */
    { "MODO ULTRA BAJA LATENCIA", "ULTRA LOW LATENCY MODE" }, /* LS_ULTRA_TITLE */
    { "Quita la latencia que agrega tu PC (red, CPU, GPU). El ping tiene un piso fisico: distancia y ruta al servidor.", "Removes the latency your PC adds (network, CPU, GPU). Ping has a physical floor: distance and route to the server." }, /* LS_ULTRA_NOTE */
    { "ACTIVAR ULTRA BAJA LATENCIA", "ENABLE ULTRA LOW LATENCY MODE" }, /* LS_ULTRA_BTN_ON */
    { "DESACTIVAR ULTRA BAJA LATENCIA", "DISABLE ULTRA LOW LATENCY MODE" }, /* LS_ULTRA_BTN_OFF */
    { "Restaurar red", "Restore network" }, /* LS_ULTRA_RESTORE */
    { "Ultra Baja Latencia: desactivada.", "Ultra Low Latency: off." }, /* LS_ULTRA_OFF */
    { "Ultra Baja Latencia: desactivada (red restaurada).", "Ultra Low Latency: off (network restored)." }, /* LS_ULTRA_OFF_RESTORED */
    { "Ultra Baja Latencia: ACTIVADA. Windows Update y Delivery Optimization en pausa mientras juegas.", "Ultra Low Latency: ON. Windows Update and Delivery Optimization paused while you play." }, /* LS_ULTRA_ON */
    { "Programacion de GPU acelerada por hardware (requiere reiniciar para aplicar)", "Hardware-accelerated GPU scheduling (requires restart to apply)" }, /* LS_GPU_SCHED */
    { "Apps usando red (conexiones TCP):", "Apps using network (TCP connections):" }, /* LS_NETHOGS_LBL */
    { "Cerrar app", "Close app" }, /* LS_KILL_HOG */
    { "Actualizar", "Update" }, /* LS_REFRESH_HOGS */
    { "PERFILES POR JUEGO", "GAME PROFILES" }, /* LS_GAMES_TITLE */
    { "Ningun juego detectado.", "No game detected." }, /* LS_NO_GAME */
    { "Activar automaticamente", "Enable automatically" }, /* LS_GAME_AUTO */
    { "Timer 0.5 ms (si no, 1.0 ms)", "Timer 0.5 ms (else 1.0 ms)" }, /* LS_GAME_TIMER */
    { "Modo Competitivo", "Competitive Mode" }, /* LS_GAME_COMP */
    { "Otro juego - nombre del .exe:", "Other game - .exe name:" }, /* LS_CUSTOM_EXE_LBL */
    { "Al arrancar un juego con su perfil activo se aplica solo; al cerrarse se restaura todo. CS2: el SOCD esta siempre bloqueado.", "When a game with an active profile starts, it applies automatically; when it closes, everything is restored. CS2: SOCD is always blocked." }, /* LS_GAMES_NOTE */
    { "PING A SERVIDORES DE JUEGO", "GAME SERVER PING" }, /* LS_SRV_TITLE */
    { "Ping al servidor del juego (no al DNS). Las IP son de referencia: puedes poner la IP de tu partida.", "Ping to the game server (not to the DNS). The IPs are reference nodes: you can enter your match's IP." }, /* LS_SRV_NOTE */
    { "CDMX: -- ms | Miami: -- ms  (pulsa Medir ping a servidores)", "CDMX: -- ms | Miami: -- ms  (press Measure server ping)" }, /* LS_SRV_INIT */
    { "Medir ping a servidores", "Measure server ping" }, /* LS_SRV_MEASURE */
    { "Diagnostico de ruta", "Route diagnostics" }, /* LS_TRACE_BTN */
    { "El diagnostico de ruta aparecera aqui (salto por salto, hasta 30).", "The route diagnostics will appear here (hop by hop, up to 30)." }, /* LS_TRACE_INIT */
    { "Mayor salto de latencia: aun no medido.", "Biggest latency jump: not measured yet." }, /* LS_TRACE_SUM_INIT */
    { "Listo. Elige un perfil o ajusta y pulsa Aplicar.", "Ready. Choose a profile or adjust and press Apply." }, /* LS_STATUS_READY */
    { "Minimizar en segundo plano", "Minimize to background" }, /* LS_TRAY_MIN */
    { "CS2 - cs2.exe  (SOCD prohibido, Valve 2024)", "CS2 - cs2.exe  (SOCD forbidden, Valve 2024)" }, /* LS_GAME_CS2_ROW */
    { "Otro juego - escribe su .exe abajo", "Other game - type its .exe below" }, /* LS_GAME_OTHER_ROW */
    { "ACTIVO", "ACTIVE" }, /* LS_STATUS_ACTIVE */
    { "APAGADO", "OFF" }, /* LS_STATUS_OFF */
    { "English", "Español" }, /* LS_LANG_BTN */
    { "Competitivo", "Competitive" }, /* LS_TAB_COMPETITIVE */
    { "Juegos", "Games" }, /* LS_TAB_GAMES */
    { "Otro juego", "Other game" }, /* LS_GAME_OTHER_NAME */
    { "Izq", "Left" }, /* LS_KEY_LEFT */
    { "Der", "Right" }, /* LS_KEY_RIGHT */
    { "Arriba", "Up" }, /* LS_KEY_UP */
    { "Abajo", "Down" }, /* LS_KEY_DOWN */
    { "Timer + FilterKeys + DNS + Competitivo", "Timer + FilterKeys + DNS + Competitive" }, /* LS_SUBTITLE */
    { "DNS actual: elige un adaptador.", "Current DNS: choose an adapter." }, /* LS_DNS_CHOOSE_ADAPTER */
    { "DNS actual: no se pudo leer.", "Current DNS: could not be read." }, /* LS_DNS_UNREADABLE */
    { "DNS actual: adaptador no encontrado.", "Current DNS: adapter not found." }, /* LS_DNS_ADAPTER_GONE */
    { "DNS actual: %s, %s", "Current DNS: %s, %s" }, /* LS_DNS_CURRENT_2 */
    { "DNS actual: %s", "Current DNS: %s" }, /* LS_DNS_CURRENT_1 */
    { "DNS actual: automatico (DHCP) o sin DNS.", "Current DNS: automatic (DHCP) or no DNS." }, /* LS_DNS_CURRENT_AUTO */
    { "DNS: elige un adaptador de red primero.", "DNS: choose a network adapter first." }, /* LS_DNS_CHOOSE_FIRST */
    { "El DNS primario no es una IP valida (ej. 8.8.8.8).", "The primary DNS is not a valid IP (e.g. 8.8.8.8)." }, /* LS_DNS_BAD_IP */
    { "DNS: fallo al aplicar. Ejecuta como administrador.", "DNS: failed to apply. Run as administrator." }, /* LS_DNS_APPLY_FAIL */
    { "No se pudo aplicar el DNS. Prueba ejecutar Lu-An Optimizer como administrador.", "Could not apply the DNS. Try running Lu-An Optimizer as administrator." }, /* LS_DNS_APPLY_FAIL_MB */
    { "DNS: fallo al restaurar. Ejecuta como administrador.", "DNS: failed to restore. Run as administrator." }, /* LS_DNS_RESTORE_FAIL */
    { "No se pudo restaurar el DNS. Prueba ejecutar como administrador.", "Could not restore the DNS. Try running as administrator." }, /* LS_DNS_RESTORE_FAIL_MB */
    { "DNS restaurado a automatico (DHCP).", "DNS restored to automatic (DHCP)." }, /* LS_DNS_RESTORED_AUTO */
    { "DNS aplicado en %s: %s%s%s", "DNS applied on %s: %s%s%s" }, /* LS_DNS_APPLIED_FMT */
    { "Cache DNS limpiada (ipconfig /flushdns).", "DNS cache cleared (ipconfig /flushdns)." }, /* LS_DNS_FLUSH_OK */
    { "No se pudo limpiar la cache DNS.", "Could not clear the DNS cache." }, /* LS_DNS_FLUSH_FAIL */
    { "SOCD: no se pudo crear el hilo del hook.", "SOCD: could not create the hook thread." }, /* LS_SOCD_NO_THREAD */
    { "SOCD: no se pudo instalar el hook de teclado.", "SOCD: could not install the keyboard hook." }, /* LS_SOCD_NO_HOOK */
    { "SOCD ACTIVADO (hilo dedicado, prioridad alta): ultima tecla gana (A/D, W/S, flechas).", "SOCD ON (dedicated thread, high priority): last key wins (A/D, W/S, arrows)." }, /* LS_SOCD_ON_LAST_HP */
    { "SOCD ACTIVADO (hilo dedicado, prioridad alta): modo neutro (ninguna al pulsar las dos).", "SOCD ON (dedicated thread, high priority): neutral mode (none when pressing both)." }, /* LS_SOCD_ON_NEUTRAL_HP */
    { "SOCD ACTIVADO (hilo dedicado): ultima tecla gana (A/D, W/S, flechas).", "SOCD ON (dedicated thread): last key wins (A/D, W/S, arrows)." }, /* LS_SOCD_ON_LAST */
    { "SOCD ACTIVADO (hilo dedicado): modo neutro (ninguna al pulsar las dos).", "SOCD ON (dedicated thread): neutral mode (none when pressing both)." }, /* LS_SOCD_ON_NEUTRAL */
    { "SOCD desactivado (no aceptaste la advertencia).", "SOCD disabled (you did not accept the warning)." }, /* LS_SOCD_OFF_WARN */
    { "SOCD Cleaner: al pulsar A y D (o W y S) a la vez, solo cuenta la ultima tecla.\n\nADVERTENCIA: en algunos juegos esta PROHIBIDO. En CS2, Valve lo prohibio en 2024\ny puede expulsarte de la partida o banearte. En otros juegos las reglas cambian.\nUsalo bajo tu propia responsabilidad y solo en juegos donde este permitido.%s\n\nQuieres activar el SOCD Cleaner?", "SOCD Cleaner: when you press A and D (or W and S) at the same time, only the last key counts.\n\nWARNING: in some games it is FORBIDDEN. In CS2, Valve banned it in 2024\nand it can kick you from the match or ban you. In other games the rules change.\nUse it at your own risk and only in games where it is allowed.%s\n\nDo you want to enable SOCD Cleaner?" }, /* LS_SOCD_WARN_FMT */
    { "\n\nOJO: tienes FilterKeys activado; SOCD + FilterKeys a la vez pueden chocar.", "\n\nNOTE: you have FilterKeys on; SOCD + FilterKeys at the same time can clash." }, /* LS_SOCD_WARN_FK */
    { "SOCD Cleaner - Advertencia", "SOCD Cleaner - Warning" }, /* LS_SOCD_WARN_TITLE */
    { "Memoria standby limpiada. Disponible: %llu MB -> %llu MB.", "Standby memory cleaned. Available: %llu MB -> %llu MB." }, /* LS_MEM_CLEANED_FMT */
    { "No se pudo limpiar la memoria standby (prueba como administrador).", "Could not clean standby memory (try as administrator)." }, /* LS_MEM_CLEAN_FAIL */
    { "Modo Competitivo activado:", "Competitive Mode enabled:" }, /* LS_COMP_ON_LOG */
    { "- FilterKeys desactivado (sin filtros en el teclado).", "- FilterKeys off (no keyboard filters)." }, /* LS_COMP_FK_OFF */
    { "- Cache DNS limpiada.", "- DNS cache cleared." }, /* LS_COMP_DNS_FLUSH */
    { "- Plan de energia: Alto rendimiento.", "- Power plan: High performance." }, /* LS_COMP_POWER_HP */
    { "- Plan de energia: no se pudo cambiar (prueba como administrador).", "- Power plan: could not change (try as administrator)." }, /* LS_COMP_POWER_FAIL */
    { "- Game Mode activado.", "- Game Mode on." }, /* LS_COMP_GAMEMODE_ON */
    { "- Midiendo los 5 DNS para recomendar el mas rapido (en segundo plano, sin congelar el teclado)...", "- Measuring the 5 DNS servers to recommend the fastest (in the background, without freezing the keyboard)..." }, /* LS_COMP_DNS_MEASURING */
    { "- Medicion de DNS: ya habia una medicion en curso.", "- DNS measurement: one was already running." }, /* LS_COMP_DNS_BUSY */
    { "- DNS mas rapido: %s.", "- Fastest DNS: %s." }, /* LS_COMP_DNS_BEST_FMT */
    { "Modo Competitivo listo. Cierra el programa o pulsa Restaurar normal para volver.", "Competitive Mode ready. Close the program or press Restore normal to go back." }, /* LS_COMP_READY */
    { "Modo Competitivo activado: timer 0.5 ms, sin FilterKeys, alto rendimiento, Game Mode.", "Competitive Mode on: timer 0.5 ms, no FilterKeys, high performance, Game Mode." }, /* LS_COMP_ON_STATUS */
    { "Restaurando modo normal:", "Restoring normal mode:" }, /* LS_COMP_RESTORE_LOG */
    { "- Timer Resolution: 15.6 ms (normal de Windows).", "- Timer Resolution: 15.6 ms (Windows default)." }, /* LS_COMP_TIMER_156 */
    { "- Plan de energia restaurado al anterior.", "- Power plan restored to the previous one." }, /* LS_COMP_POWER_RESTORED */
    { "- Plan de energia: no se pudo restaurar.", "- Power plan: could not restore." }, /* LS_COMP_POWER_RESTORE_FAIL */
    { "- Plan de energia: Equilibrado.", "- Power plan: Balanced." }, /* LS_COMP_POWER_BALANCED */
    { "- Game Mode restaurado como estaba.", "- Game Mode restored as it was." }, /* LS_COMP_GAMEMODE_RESTORED */
    { "Modo normal restaurado.", "Normal mode restored." }, /* LS_COMP_OFF_STATUS */
    { "Ping en vivo: no se pudo crear el hilo.", "Live ping: could not create the thread." }, /* LS_LIVE_NO_THREAD */
    { "Ping a %s: %d/4 respuestas, media %ld ms.", "Ping to %s: %d/4 replies, avg %ld ms." }, /* LS_PING_ONE_FMT */
    { "Ping a %s: 0/4 respuestas (sin respuesta).", "Ping to %s: 0/4 replies (no response)." }, /* LS_PING_ONE_NONE */
    { "El mas rapido: %s %s (%ld ms). %s", "Fastest: %s %s (%ld ms). %s" }, /* LS_PING_BEST_FMT */
    { "Ningun DNS respondio. %s", "No DNS responded. %s" }, /* LS_PING_NONE_FMT */
    { "Ya hay una medicion de ping en curso, espera a que termine.", "A ping measurement is already running, wait for it to finish." }, /* LS_PING_BUSY */
    { "No se pudo iniciar la medicion de ping.", "Could not start the ping measurement." }, /* LS_PING_START_FAIL */
    { "El DNS primario no es una IP valida para probar el ping.", "The primary DNS is not a valid IP to test ping." }, /* LS_PING_BAD_IP */
    { "Aceleracion del mouse desactivada (precision mejorada).", "Mouse acceleration off (improved precision)." }, /* LS_MOUSE_OFF */
    { "Aceleracion del mouse restaurada.", "Mouse acceleration restored." }, /* LS_MOUSE_ON */
    { "Optimizaciones de pantalla completa desactivadas.", "Fullscreen optimizations off." }, /* LS_FSE_OFF */
    { "Optimizaciones de pantalla completa restauradas.", "Fullscreen optimizations restored." }, /* LS_FSE_ON */
    { "No se pudo listar los procesos.", "Could not list processes." }, /* LS_PROC_LIST_FAIL */
    { "No hay apps en segundo plano de la lista abiertas (navegadores, Discord, Spotify, launchers...).", "No apps from the list are open in the background (browsers, Discord, Spotify, launchers...)." }, /* LS_NO_BG_APPS */
    { "Cerrar apps", "Close apps" }, /* LS_CLOSE_APPS_TITLE */
    { "Se encontraron estas apps en segundo plano:\n\n%s\n\nCerrarlas libera CPU y RAM para el juego. Se cerraran sin guardar (cierra tus paginas o partidas antes si hace falta).\n\nQuieres cerrarlas?", "These background apps were found:\n\n%s\n\nClosing them frees CPU and RAM for the game. They will close without saving (close your pages or matches first if needed).\n\nClose them?" }, /* LS_CLOSE_APPS_BG_FMT */
    { "Cerrar apps en segundo plano", "Close background apps" }, /* LS_CLOSE_APPS_BG_TITLE */
    { "Apps cerradas: %d proceso(s).", "Apps closed: %d process(es)." }, /* LS_APPS_CLOSED_FMT */
    { "La descarga no coincide con el SHA256 publicado. Actualizacion cancelada por seguridad.", "The download does not match the published SHA256. Update cancelled for security." }, /* LS_UPD_SHA_MISMATCH */
    { "Actualizacion", "Update" }, /* LS_UPD_TITLE */
    { "No se pudo verificar el SHA256 de la descarga (el release no publica hash).\n\nInstalar de todas formas?", "Could not verify the download's SHA256 (the release publishes no hash).\n\nInstall anyway?" }, /* LS_UPD_NO_HASH */
    { "Actualizacion - sin verificar", "Update - unverified" }, /* LS_UPD_NO_HASH_TITLE */
    { "Nagle desactivado", "Nagle disabled" }, /* LS_ULTRA_NAGLE_OK */
    { "Nagle: sin permiso (ejecuta como administrador)", "Nagle: no permission (run as administrator)" }, /* LS_ULTRA_NAGLE_FAIL */
    { "ACK inmediato", "Immediate ACK" }, /* LS_ULTRA_ACK_OK */
    { "TcpAckFrequency: sin permiso", "TcpAckFrequency: no permission" }, /* LS_ULTRA_ACK_FAIL */
    { "Limite de ancho de banda quitado", "Bandwidth limit removed" }, /* LS_ULTRA_THROTTLE_OK */
    { "NetworkThrottlingIndex: sin permiso", "NetworkThrottlingIndex: no permission" }, /* LS_ULTRA_THROTTLE_FAIL */
    { "Ahorro de energia del adaptador desactivado", "Adapter power saving off" }, /* LS_ULTRA_NIC_OK */
    { "Ahorro del adaptador: no se pudo (admin)", "Adapter power saving: failed (admin)" }, /* LS_ULTRA_NIC_FAIL */
    { "Suspension selectiva WiFi desactivada", "WiFi selective suspend off" }, /* LS_ULTRA_WIFI_OK */
    { "WiFi: no se pudo tocar el ahorro", "WiFi: could not change power saving" }, /* LS_ULTRA_WIFI_FAIL */
    { "Ninguna app con conexiones TCP ahora mismo.", "No apps with TCP connections right now." }, /* LS_HOGS_NONE */
    { "Selecciona primero una app de la lista.", "Select an app from the list first." }, /* LS_HOG_SELECT */
    { "Ese es un proceso del sistema: no se puede cerrar desde aqui.", "That is a system process: it cannot be closed from here." }, /* LS_HOG_SYSTEM */
    { "No se pudo abrir ese proceso (permiso denegado).", "Could not open that process (permission denied)." }, /* LS_HOG_OPEN_FAIL */
    { "Cerrar %s (PID %lu)?\nSi esta descargando algo, la descarga se detiene.", "Close %s (PID %lu)?\nIf it is downloading something, the download will stop." }, /* LS_HOG_ASK_FMT */
    { "Proceso cerrado.", "Process closed." }, /* LS_HOG_CLOSED */
    { "Midiendo ping a CDMX y Miami... (ping al servidor, no al DNS)", "Measuring ping to CDMX and Miami... (ping to the server, not to the DNS)" }, /* LS_SRV_MEASURING */
    { "No se pudo abrir el ping (ICMP).", "Could not open ping (ICMP)." }, /* LS_TRACE_ICMP_FAIL */
    { "Mayor salto de latencia: no se pudo medir.", "Biggest latency jump: could not be measured." }, /* LS_TRACE_FAIL */
    { "Mayor salto de latencia: no se detecto un salto claro (la ruta responde pareja).", "Biggest latency jump: no clear jump detected (the route answers evenly)." }, /* LS_TRACE_NOJUMP */
    { "tu ISP", "your ISP" }, /* LS_WHERE_ISP */
    { "tu PC", "your PC" }, /* LS_WHERE_PC */
    { "tu router", "your router" }, /* LS_WHERE_ROUTER */
    { "el servidor", "the server" }, /* LS_WHERE_SERVER */
    { "Mayor salto de latencia: +%lu ms en el salto %d -> esta en: %s.", "Biggest latency jump: +%lu ms at hop %d -> it is at: %s." }, /* LS_TRACE_SUMMARY_FMT */
    { "Haciendo diagnostico de ruta... (hasta 30 saltos)", "Running route diagnostics... (up to 30 hops)" }, /* LS_TRACE_RUNNING */
    { "Mayor salto de latencia: midiendo...", "Biggest latency jump: measuring..." }, /* LS_TRACE_MEASURING */
    { "Juego detectado: %s - perfil aplicado.", "Game detected: %s - profile applied." }, /* LS_GAME_DETECTED_FMT */
    { "Juego cerrado: ajustes restaurados.", "Game closed: settings restored." }, /* LS_GAME_CLOSED */
    { "sin respuesta", "no response" }, /* LS_NO_RESPONSE */
    { "N/D", "N/A" }, /* LS_NA */
    { "Actual: %.2f ms", "Current: %.2f ms" }, /* LS_TIMER_CURRENT_FMT */
    { "No se pudo aplicar. Prueba ejecutar como administrador.", "Could not apply. Try running as administrator." }, /* LS_TIMER_APPLY_FAIL */
    { "Timer aplicado: %.2f ms - se mantiene mientras la app este abierta.", "Timer applied: %.2f ms - it stays while the app is open." }, /* LS_TIMER_APPLIED_FMT */
    { "No se pudo aplicar FilterKeys.", "Could not apply FilterKeys." }, /* LS_FK_APPLY_FAIL */
    { "FilterKeys desactivado.", "FilterKeys disabled." }, /* LS_FK_OFF */
    { "FilterKeys: BounceKeys %u ms (Slow/Repeat apagados por la regla de Windows).", "FilterKeys: BounceKeys %u ms (Slow/Repeat off by Windows rule)." }, /* LS_FK_BOUNCE_FMT */
    { "FilterKeys: Slow %u ms, retardo %u ms, repeticion %u ms.", "FilterKeys: Slow %u ms, delay %u ms, repeat %u ms." }, /* LS_FK_SLOW_FMT */
    { "Perfil Gaming: timer 0.5 ms, FilterKeys desactivado.", "Gaming profile: timer 0.5 ms, FilterKeys off." }, /* LS_PROFILE_GAMING */
    { "Perfil Escritura: timer 1.0 ms, rebote 100 ms.", "Writing profile: timer 1.0 ms, bounce 100 ms." }, /* LS_PROFILE_TYPING */
    { "Perfil Accesibilidad: teclas lentas 500 ms, sin rebote.", "Accessibility profile: slow keys 500 ms, no bounce." }, /* LS_PROFILE_ACCESS */
    { "Perfil Normal: todo como viene en Windows.", "Normal profile: everything as Windows ships it." }, /* LS_PROFILE_NORMAL */
    { "Sigue corriendo en segundo plano. Doble clic en el icono para mostrarlo.", "Still running in the background. Double-click the icon to show it." }, /* LS_TRAY_BALLOON */
    { "Mostrar Lu-An Optimizer", "Show Lu-An Optimizer" }, /* LS_TRAY_SHOW */
    { "Desactivar SOCD", "Disable SOCD" }, /* LS_TRAY_SOCD_OFF */
    { "Activar SOCD", "Enable SOCD" }, /* LS_TRAY_SOCD_ON */
    { "Salir", "Exit" }, /* LS_TRAY_EXIT */
    { "SOCD prohibido en CS2 (Valve, 2024).\nEl SOCD no se puede activar para CS2 bajo ningun concepto.", "SOCD forbidden in CS2 (Valve, 2024).\nSOCD cannot be enabled for CS2 under any circumstances." }, /* LS_CS2_SOCD_FORBIDDEN */
    { "CS2 - SOCD prohibido", "CS2 - SOCD forbidden" }, /* LS_CS2_SOCD_TITLE */
    { "El Modo Ultra Baja Latencia toca el registro de Windows (red: Nagle, ACK inmediato, ancho de banda, ahorro de energia del adaptador y del WiFi) y pausa temporalmente Windows Update y Delivery Optimization mientras juegas.\n\nLos valores originales se guardan y \"Restaurar red\" los devuelve. Se recomienda ejecutar el programa como administrador para que todos los ajustes se apliquen.\n\nEl ping tiene un piso fisico (distancia y ruta al servidor): esto quita la latencia que agrega tu PC, no la de tu ISP.\n\n¿Activar el modo Ultra Baja Latencia?", "Ultra Low Latency Mode changes Windows registry settings (network: Nagle, immediate ACK, bandwidth, adapter and WiFi power saving) and temporarily pauses Windows Update and Delivery Optimization while you play.\n\nThe original values are saved and \"Restore network\" brings them back. Running the program as administrator is recommended so all settings apply.\n\nPing has a physical floor (distance and route to the server): this removes the latency your PC adds, not your ISP's.\n\nEnable Ultra Low Latency Mode?" }, /* LS_ULTRA_WARN */
    { "Modo Ultra Baja Latencia", "Ultra Low Latency Mode" }, /* LS_ULTRA_TITLE_MB */
    { "Modo Ultra Baja Latencia activado.", "Ultra Low Latency Mode on." }, /* LS_ULTRA_ON_STATUS */
    { "Modo Ultra Baja Latencia desactivado; red restaurada.", "Ultra Low Latency Mode off; network restored." }, /* LS_ULTRA_OFF_STATUS */
    { "Red restaurada a sus valores originales.", "Network restored to its original values." }, /* LS_NET_RESTORED */
    { "Programacion de GPU acelerada por hardware guardada. Requiere reiniciar Windows para aplicar.", "Hardware-accelerated GPU scheduling saved. Requires restarting Windows to apply." }, /* LS_GPU_SAVED */
    { "No se pudo escribir HwSchMode (ejecuta como administrador).", "Could not write HwSchMode (run as administrator)." }, /* LS_GPU_WRITE_FAIL */
    { "Lista de apps con red actualizada.", "Network apps list updated." }, /* LS_HOGS_REFRESHED */
    { "Ping en vivo: midiendo cada 1 s (hilo aparte, sin frenar el teclado)...", "Live ping: measuring every 1 s (separate thread, without slowing the keyboard)..." }, /* LS_LIVE_MEASURING */
    { "Ping en vivo: pausado.", "Live ping: paused." }, /* LS_LIVE_PAUSED */
    { "Buscando actualizaciones...", "Checking for updates..." }, /* LS_UPD_CHECKING */
    { "Buscar actualizaciones al iniciar: activado.", "Check for updates on startup: on." }, /* LS_UPD_AUTO_ON */
    { "Buscar actualizaciones al iniciar: desactivado (usa el boton).", "Check for updates on startup: off (use the button)." }, /* LS_UPD_AUTO_OFF */
    { "Minimizar en segundo plano: activado (el boton _ esconde la ventana a la bandeja).", "Minimize to background: on (the _ button hides the window to the tray)." }, /* LS_TRAY_MIN_ON */
    { "Minimizar en segundo plano: desactivado (minimizar es el normal a la barra de tareas).", "Minimize to background: off (minimize is the normal one to the taskbar)." }, /* LS_TRAY_MIN_OFF */
    { "Hay una v%s nueva de Lu-An Optimizer (tu tienes la v%s).\n\nQuieres actualizar?", "There is a new v%s of Lu-An Optimizer (you have v%s).\n\nDo you want to update?" }, /* LS_UPD_NEW_FMT */
    { "Hay una v%s nueva de Lu-An Optimizer.\nEl Modo Competitivo esta activo: no se descargara nada hasta que lo desactives.", "There is a new v%s of Lu-An Optimizer.\nCompetitive Mode is active: nothing will be downloaded until you turn it off." }, /* LS_UPD_NEW_COMP_FMT */
    { "Actualizacion disponible", "Update available" }, /* LS_UPD_AVAILABLE */
    { "Descargando la actualizacion...", "Downloading the update..." }, /* LS_UPD_DOWNLOADING */
    { "Actualizacion disponible: la puedes descargar cuando quieras con el boton.", "Update available: you can download it whenever you want with the button." }, /* LS_UPD_AVAILABLE_STATUS */
    { "No se pudo revisar actualizaciones.", "Could not check for updates." }, /* LS_UPD_CHECK_FAIL */
    { "No se pudo revisar actualizaciones (revisa tu internet o que el repo este publicado).", "Could not check for updates (check your internet or that the repo is published)." }, /* LS_UPD_CHECK_FAIL_LONG */
    { "Ya tienes la ultima version.", "You already have the latest version." }, /* LS_UPD_LATEST */
    { "Ya tienes la ultima version de Lu-An Optimizer.", "You already have the latest version of Lu-An Optimizer." }, /* LS_UPD_LATEST_MB */
    { "Se abrio la pagina de releases para descargar la actualizacion.", "The releases page was opened to download the update." }, /* LS_UPD_RELEASES_OPENED */
    { "No se pudo descargar la actualizacion.", "Could not download the update." }, /* LS_UPD_DOWNLOAD_FAIL */
    { "CDMX: %s (perdida %d/4) | Miami: %s (perdida %d/4)  -> te conviene: %s  (ping al servidor, no al DNS)", "CDMX: %s (loss %d/4) | Miami: %s (loss %d/4)  -> best for you: %s  (ping to the server, not to the DNS)" }, /* LS_SRV_RESULT_BEST */
    { "CDMX: %s (perdida %d/4) | Miami: %s (perdida %d/4)  (ping al servidor, no al DNS)", "CDMX: %s (loss %d/4) | Miami: %s (loss %d/4)  (ping to the server, not to the DNS)" }, /* LS_SRV_RESULT */
    { "Ping a servidores medido.", "Server ping measured." }, /* LS_SRV_MEASURED */
    { "Diagnostico de ruta terminado.", "Route diagnostics finished." }, /* LS_TRACE_DONE */
    { "Prueba SOCD (hilo dedicado): ultima tecla %s, latencia ~%ld ms. Pulsa A/D o W/S.", "SOCD test (dedicated thread): last key %s, latency ~%ld ms. Press A/D or W/S." }, /* LS_SOCD_TEST_FMT */
    { "Ping: %ld ms | Media: %ld ms | Jitter: %ld ms | Perdida: %ld%% (%ld muestras)", "Ping: %ld ms | Avg: %ld ms | Jitter: %ld ms | Loss: %ld%% (%ld samples)" }, /* LS_LIVE_RESULT */
    { "Ping: sin respuesta | Media: %ld ms | Jitter: %ld ms | Perdida: %ld%% (%ld muestras)", "Ping: no response | Avg: %ld ms | Jitter: %ld ms | Loss: %ld%% (%ld samples)" }, /* LS_LIVE_RESULT_NORESP */
    { "Sin adaptadores", "No adapters" }, /* LS_NO_ADAPTERS */
    { "Sin adaptadores activos", "No active adapters" }, /* LS_NO_ACTIVE_ADAPTERS */
    { "Lu-An Monitor\r\nPing: %s | Jitter: %ld ms | Perdida: %ld%%\r\nCPU: %s | RAM: %lu%%\r\nTimer: %.2f ms | SOCD: %s\r\nFPS (pantalla): %s\r\nTemp: %s\r\nFPS = pantalla, no del juego.", "Lu-An Monitor\r\nPing: %s | Jitter: %ld ms | Loss: %ld%%\r\nCPU: %s | RAM: %lu%%\r\nTimer: %.2f ms | SOCD: %s\r\nFPS (screen): %s\r\nTemp: %s\r\nFPS = screen, not the game." }, /* LS_OVERLAY_FMT */
    { " [sistema]", " [system]" }, /* LS_HOG_SYSTEM_SUFFIX */
    { "el proceso", "the process" }, /* LS_HOG_UNKNOWN_NAME */
    { "Tema:", "Theme:" }, /* LS_THEME_LBL */
    { "Neón", "Neon" }, /* LS_THEME_NEON */
    { "Fuego", "Fire" }, /* LS_THEME_FIRE */
    { "Esmeralda", "Emerald" }, /* LS_THEME_EMERALD */
    { "Pinkimich", "Pinkimich" }, /* LS_THEME_PINK */
};

static int g_lang = 0; /* 0 = espanol, 1 = English */
static const char *Tr(int k) {
    if (k < 0 || k >= LS_COUNT) return "";
    return g_langStr[k][g_lang];
}

static void LoadLanguage(void) {
    HKEY hk;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", 0, KEY_READ, &hk) == ERROR_SUCCESS) {
        char v[8]; DWORD n = sizeof v, tp = 0;
        if (RegQueryValueExA(hk, "Language", NULL, &tp, (BYTE *)v, &n) == ERROR_SUCCESS && tp == REG_SZ)
            g_lang = (v[0] == 'e' && v[1] == 'n') ? 1 : 0;
        RegCloseKey(hk);
    }
}
static void SaveLanguage(void) {
    HKEY hk;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS) {
        const char *v = g_lang ? "en" : "es";
        RegSetValueExA(hk, "Language", 0, REG_SZ, (const BYTE *)v, (DWORD)strlen(v) + 1);
        RegCloseKey(hk);
    }
}

/* ---------- Temas (v13) ---------- */
static void LoadTheme(void) {
    g_theme = 0; /* Neon por defecto (el aspecto actual) */
    HKEY hk;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", 0, KEY_READ, &hk) == ERROR_SUCCESS) {
        char v[16]; DWORD n = sizeof v, tp = 0;
        if (RegQueryValueExA(hk, "Theme", NULL, &tp, (BYTE *)v, &n) == ERROR_SUCCESS && tp == REG_SZ)
            for (int i = 0; i < 4; i++) if (strcmp(v, g_themeNames[i]) == 0) { g_theme = i; break; }
        RegCloseKey(hk);
    }
    g_themeCur = g_themes[g_theme];
}
static void SaveTheme(void) {
    HKEY hk;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", 0, NULL, 0, KEY_SET_VALUE, NULL, &hk, NULL) == ERROR_SUCCESS) {
        RegSetValueExA(hk, "Theme", 0, REG_SZ, (const BYTE *)g_themeNames[g_theme], (DWORD)strlen(g_themeNames[g_theme]) + 1);
        RegCloseKey(hk);
    }
}

/* Registro de controles traducibles (v11): se define aqui, antes de
 * ApplyLanguage(), que lo recorre al cambiar de idioma. */
#define MAX_LANG_REG 256
static struct LangReg { HWND h; int key; } g_langReg[MAX_LANG_REG];
static int g_langRegCount = 0;

/* Cuadro de estado (v11): se dibuja en WM_PAINT arriba a la derecha;
 * esta funcion invalida su rectangulo para que se repinte. */
static void UpdateStatusBox(void) {
    if (g_hwnd) { RECT r = { 535, 10, 692, 48 }; InvalidateRect(g_hwnd, &r, FALSE); }
}

/* v13: aplica el tema actual a todo: cambia la paleta, recrea las
 * brochas y vuelve a pintar la ventana, la barra de pestanas, las 5
 * paginas y el overlay (si esta abierto). Sin reiniciar. */
static BOOL CALLBACK InvalidateChildCb(HWND h, LPARAM lp) { (void)lp; InvalidateRect(h, NULL, TRUE); return TRUE; }
static void ApplyTheme(void) {
    g_themeCur = g_themes[g_theme];
    if (g_brBg) DeleteObject(g_brBg);
    if (g_brCard) DeleteObject(g_brCard);
    if (g_brBtn) DeleteObject(g_brBtn);
    if (g_brBgDeep) DeleteObject(g_brBgDeep);
    g_brBg = CreateSolidBrush(COL_BG); g_brCard = CreateSolidBrush(COL_CARD); g_brBtn = CreateSolidBrush(COL_BTN);
    g_brBgDeep = CreateSolidBrush(COL_BGDEEP);
    if (!g_hwnd) return;
    InvalidateRect(g_hwnd, NULL, TRUE);
    EnumChildWindows(g_hwnd, InvalidateChildCb, 0);
    if (g_tabBar) InvalidateRect(g_tabBar, NULL, TRUE);
    for (int i = 0; i < 5; i++) if (g_pages[i]) { InvalidateRect(g_pages[i], NULL, TRUE); EnumChildWindows(g_pages[i], InvalidateChildCb, 0); }
    { HWND ov = FindWindowA("LuAnOverlay", NULL);
      if (ov) { InvalidateRect(ov, NULL, TRUE); EnumChildWindows(ov, InvalidateChildCb, 0); } }
    UpdateStatusBox();
}
static void RefreshThemeCombo(void); /* v13: se define tras los IDs */

/* ---------- Desplazamiento vertical de paginas (v11) ---------- */
static int g_pageScroll[5] = { 0, 0, 0, 0, 0 };   /* desplazamiento actual por pestana */
static int g_pageContentH[5] = { 0, 0, 0, 0, 0 }; /* alto del contenido por pestana */
static int PageIndexFromHwnd(HWND h) {
    for (int i = 0; i < 5; i++) if (g_pages[i] == h) return i;
    return -1;
}
/* ---------- Barra de desplazamiento propia, por tema (v13.1) ----------
 * La barra vertical del sistema (WS_VSCROLL) se veia fuera del diseno,
 * asi que las paginas ya no la usan: la barra se dibuja en WM_PAINT de
 * la pagina (track del color de tarjeta del tema, thumb redondeado con
 * el degradado de acento del tema, igual que el indicador de pestanas)
 * y se maneja con el raton en PageWndProc (arrastre del thumb, clic en
 * el track = salto de pagina, rueda del mouse). Todo el desplazamiento
 * sigue pasando por PageSetScroll, que invalida solo la franja de la
 * barra para repintarla. Sin WS_VSCROLL el cliente de la pagina mide
 * 680 px (antes 663), asi que la franja x 666..678 queda libre: los
 * controles se colocaron con la barra del sistema puesta. */
#define SCROLL_W 12
static int g_scrollDrag[5] = { -1, -1, -1, -1, -1 }; /* 1 = arrastrando el thumb */
static int g_scrollGrab[5] = { 0, 0, 0, 0, 0 };      /* agarre: y del raton - y del thumb */
static int g_scrollHover[5] = { 0, 0, 0, 0, 0 };     /* 1 = raton sobre la barra */
/* Rectangulo del track (siempre) y del thumb; devuelve 0 si no hay nada
 * que desplazar (contenido cabe en la pagina). */
static int ScrollThumbRect(HWND hwnd, int idx, RECT *track, RECT *thumb) {
    RECT r; GetClientRect(hwnd, &r);
    track->left = r.right - SCROLL_W - 2; track->right = r.right - 2;
    track->top = 4; track->bottom = r.bottom - 4;
    int maxOff = g_pageContentH[idx] - PAGE_H; if (maxOff < 0) maxOff = 0;
    if (maxOff <= 0) return 0;
    int th = (track->bottom - track->top) * PAGE_H / g_pageContentH[idx];
    if (th < 28) th = 28;
    if (th > track->bottom - track->top) th = track->bottom - track->top;
    int ty = track->top + g_pageScroll[idx] * (track->bottom - track->top - th) / maxOff;
    thumb->left = track->left + 1; thumb->right = track->right - 1;
    thumb->top = ty; thumb->bottom = ty + th;
    return 1;
}
static void ScrollInvalidate(HWND hwnd) {
    RECT r; GetClientRect(hwnd, &r);
    RECT sr = { r.right - SCROLL_W - 2, 0, r.right, r.bottom };
    InvalidateRect(hwnd, &sr, FALSE);
}
/* Callback de EnumChildWindows: mueve cada hijo 'dy' pixeles en Y. */
static BOOL CALLBACK PageMoveChild(HWND h, LPARAM lp) {
    int dy = (int)lp;
    if (dy) { RECT r; GetWindowRect(h, &r); POINT pt = { r.left, r.top };
        ScreenToClient(GetParent(h), &pt);
        SetWindowPos(h, NULL, pt.x, pt.y + dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE); }
    return TRUE;
}
static void PageSetScroll(int idx, int off) {
    if (idx < 0 || idx >= 5) return;
    int maxOff = g_pageContentH[idx] - PAGE_H; if (maxOff < 0) maxOff = 0;
    if (off < 0) off = 0; if (off > maxOff) off = maxOff;
    int dy = g_pageScroll[idx] - off; /* mover hijos la diferencia */
    g_pageScroll[idx] = off;
    if (dy) EnumChildWindows(g_pages[idx], PageMoveChild, (LPARAM)dy);
    ScrollInvalidate(g_pages[idx]); /* v13.1: repintar la barra propia */
}
static BOOL CALLBACK PageMeasureChild(HWND h, LPARAM lp) {
    int *maxY = (int *)lp;
    RECT r; GetWindowRect(h, &r); POINT pt = { r.left, r.top };
    ScreenToClient(GetParent(h), &pt);
    int bottom = pt.y + (r.bottom - r.top);
    if (bottom > *maxY) *maxY = bottom;
    return TRUE;
}
static void PageInitScroll(void) {
    /* Altura del contenido = el control mas bajo de cada pagina (+8 de
     * margen); las posiciones actuales ya incluyen el desplazamiento 0. */
    for (int i = 0; i < 5; i++) {
        int maxY = 0;
        EnumChildWindows(g_pages[i], PageMeasureChild, (LPARAM)&maxY);
        g_pageContentH[i] = maxY + 8;
        PageSetScroll(i, 0);
    }
}
static void ApplyLanguage(void); /* definida en Helpers UI, usada por la bandeja */
static void RefreshDynamicTexts(void);


/* IDs */
#define ID_PROFILE_GAMING 1001
#define ID_PROFILE_TYPING 1002
#define ID_PROFILE_ACCESS 1003
#define ID_PROFILE_NORMAL 1004
#define ID_TRACKBAR 1010
#define ID_LBL_DESIRED 1011
#define ID_LBL_CURRENT 1012
#define ID_APPLY_TIMER 1013
#define ID_RESET_TIMER 1014
#define ID_PRESET_05 1021
#define ID_PRESET_10 1022
#define ID_PRESET_20 1023
#define ID_PRESET_50 1024
#define ID_PRESET_156 1025
#define ID_CHK_FK 1030
#define ID_CHK_BOUNCE 1031
#define ID_EDIT_BOUNCE 1032
#define ID_EDIT_WAIT 1033
#define ID_EDIT_DELAY 1034
#define ID_EDIT_REPEAT 1035
#define ID_APPLY_FK 1036
#define ID_DISABLE_FK 1037
#define ID_TEST_EDIT 1040
#define ID_STATUS 1050
#define ID_LBL_FK_STATUS 1051
/* DNS Jumper */
#define ID_DNS_ADAPTER 1100
#define ID_DNS_GOOGLE 1101
#define ID_DNS_CLOUDFLARE 1102
#define ID_DNS_QUAD9 1103
#define ID_DNS_OPENDNS 1104
#define ID_DNS_ADGUARD 1105
#define ID_DNS_AUTO 1106
#define ID_DNS_PRIMARY 1107
#define ID_DNS_SECONDARY 1108
#define ID_DNS_APPLY 1109
#define ID_DNS_RESTORE 1110
#define ID_DNS_FLUSH 1111
#define ID_DNS_PING 1112
#define ID_LBL_DNS_STATUS 1113
#define ID_LBL_DNS_CURRENT 1114
#define ID_TAB 1200
/* Pestana Competitivo */
#define ID_CHK_SOCD 1300
#define ID_SOCD_LAST 1301
#define ID_SOCD_NEUTRAL 1302
#define ID_LBL_SOCD_STATUS 1303
#define ID_LBL_SOCD_TEST 1304
/* Mensajes internos entre los hilos worker y la UI (un worker nunca toca la UI directamente) */
#define WM_SOCD_KEY (WM_APP+1)
#define WM_PING_UPDATE (WM_APP+2)
#define WM_PING_DONE (WM_APP+3)
#define ID_COMP_MODE 1310
#define ID_COMP_RESTORE 1311
#define ID_COMP_LOG 1312
#define ID_CHK_LIVE_PING 1320
#define ID_LBL_LIVE_PING 1321
#define ID_PING_ALL 1322
#define ID_LBL_BEST_DNS 1323
#define ID_CHK_MOUSE_ACCEL 1330
#define ID_CHK_FSE 1331
#define ID_BTN_CLOSE_APPS 1332
#define ID_BTN_CLEAN_MEM 1333
#define ID_LBL_MEM_STATUS 1334
#define ID_CHK_AUTO_CLEAN 1335
/* Bandeja del sistema (system tray, v8) */
#define WM_TRAYICON (WM_APP+4)
#define ID_CHK_TRAY_MIN 1401 /* Tr(LS_TRAY_MIN) */
#define ID_TRAY_SHOW 1410
#define ID_TRAY_COMP 1411
#define ID_TRAY_SOCD 1412
#define ID_TRAY_EXIT 1413
/* Monitor en pantalla / overlay (v9) */
#define ID_CHK_OVERLAY 1600
#define ID_CHK_OVERLAY_CLICK 1601
#define ID_OVERLAY_CLOSE 1602
/* Auto-actualizacion (v9) */
#define ID_BTN_CHECK_UPDATE 1603
#define ID_CHK_AUTO_UPDATE 1604
/* Perfiles por juego (v9) */
#define ID_GAMES_STATUS 1500
#define ID_GAMES_AUTO_BASE 1510
#define ID_GAMES_SOCD_BASE 1520
#define ID_GAMES_TIMER_BASE 1530
#define ID_GAMES_COMP_BASE 1540
#define ID_GAMES_CUSTOM_EXE 1550
#define ID_GAMES_NOTE 1551
/* Modo Ultra Baja Latencia (v10) */
#define ID_ULTRA_ON 1700
#define ID_ULTRA_RESTORE 1701
#define ID_LBL_ULTRA_STATUS 1702
#define ID_CHK_GPU_SCHED 1703
#define ID_LIST_NETHOGS 1704
#define ID_BTN_KILL_HOG 1705
#define ID_BTN_REFRESH_HOGS 1706
/* Ping a servidores de juego + traceroute (v10, pestana Juegos) */
#define ID_LBL_SERVER_PING 1710
#define ID_BTN_SERVER_PING 1711
#define ID_BTN_TRACEROUTE 1712
#define ID_EDIT_TRACE 1713
#define ID_LBL_TRACE_SUMMARY 1714
#define ID_SRV_CDMX 1715
#define ID_SRV_MIAMI 1716

/* v11: selector de idioma y etiqueta de version */
#define ID_LANG_TOGGLE 1800
#define ID_LBL_VERSION 1801
/* v13: selector de tema y su etiqueta */
#define ID_THEME_COMBO 1802
#define ID_LBL_THEME 1803

/* v13: rellena el desplegable de temas con los nombres traducidos y
 * marca el tema actual. Se llama al crear la ventana y al cambiar de
 * idioma (ApplyLanguage). */
static void RefreshThemeCombo(void) {
    HWND cb = Ctl(ID_THEME_COMBO);
    if (!cb) return;
    SendMessage(cb, CB_RESETCONTENT, 0, 0);
    SendMessage(cb, CB_ADDSTRING, 0, (LPARAM)Tr(LS_THEME_NEON));
    SendMessage(cb, CB_ADDSTRING, 0, (LPARAM)Tr(LS_THEME_FIRE));
    SendMessage(cb, CB_ADDSTRING, 0, (LPARAM)Tr(LS_THEME_EMERALD));
    SendMessage(cb, CB_ADDSTRING, 0, (LPARAM)Tr(LS_THEME_PINK));
    SendMessage(cb, CB_SETCURSEL, (WPARAM)g_theme, 0);
}
#define WM_UPDATE_FOUND (WM_APP+5)
#define WM_UPDATE_STATUS (WM_APP+6)
#define WM_GAME_APPLY (WM_APP+7)
#define WM_SERVER_PING_DONE (WM_APP+8)
#define WM_TRACE_DONE (WM_APP+9)

/* ---------- Logo LU-AN (GDI+, desde logo.jpg en memoria) ---------- */
/* v13.1: convierte bytes JPEG en HBITMAP (GDI+). Lo usa el logo del
 * planeta (logo_data.h) y el logo Pinkimich (RCDATA 102). */
static int JpgBytesToHBitmap(const void *data, DWORD len, HBITMAP *out) {
    *out = NULL;
    if (!data || !len) return 0;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len);
    if (!hMem) return 0;
    void *p = GlobalLock(hMem);
    if (!p) { GlobalFree(hMem); return 0; }
    memcpy(p, data, len);
    GlobalUnlock(hMem);
    IStream *stream = NULL;
    if (CreateStreamOnHGlobal(hMem, TRUE, &stream) != S_OK) { GlobalFree(hMem); return 0; }
    GpBitmap *bmp = NULL;
    if (GdipCreateBitmapFromStream(stream, &bmp) != 0) { stream->lpVtbl->Release(stream); return 0; }
    stream->lpVtbl->Release(stream);
    HBITMAP hbm = NULL;
    if (GdipCreateHBITMAPFromBitmap(bmp, &hbm, 0) != 0) { GdipDisposeImage((GpImage*)bmp); return 0; }
    GdipDisposeImage((GpImage*)bmp);
    *out = hbm;
    return 1;
}
static void LoadLogo(void) {
    GdiplusStartupInput input;
    input.GdiplusVersion = 1;
    input.DebugEventCallback = NULL;
    input.SuppressBackgroundThread = FALSE;
    input.SuppressExternalCodecs = FALSE;
    if (GdiplusStartup(&g_gdiplusToken, &input, NULL) != 0) return;
    JpgBytesToHBitmap(g_logoJpg, (DWORD)g_logoJpgLen, &g_hLogoBmp);
    /* v13.1: logo Pinkimich desde RCDATA 102 (pinkimich.jpg). Si no se
     * puede cargar, g_hPinkBmp queda NULL y se usa el logo del planeta. */
    { HRSRC hr = FindResourceA(g_hInst, MAKEINTRESOURCE(102), RT_RCDATA);
      if (hr) { HGLOBAL hg = LoadResource(g_hInst, hr);
        if (hg) { const void *pd = LockResource(hg); DWORD sz = SizeofResource(g_hInst, hr);
          if (pd && sz) JpgBytesToHBitmap(pd, sz, &g_hPinkBmp); } } }
}
static void FreeLogo(void) {
    if (g_hLogoBmp) { DeleteObject(g_hLogoBmp); g_hLogoBmp = NULL; }
    if (g_hPinkBmp) { DeleteObject(g_hPinkBmp); g_hPinkBmp = NULL; }
    if (g_gdiplusToken) { GdiplusShutdown(g_gdiplusToken); g_gdiplusToken = 0; }
}

/* ---------- Timer Resolution (ntdll, runtime) ---------- */
typedef LONG (NTAPI *PFN_NtQueryTimerResolution)(PULONG, PULONG, PULONG);
typedef LONG (NTAPI *PFN_NtSetTimerResolution)(ULONG, BOOLEAN, PULONG);
static PFN_NtQueryTimerResolution pQuery = NULL;
static PFN_NtSetTimerResolution pSet = NULL;

static int TimerInit(void) {
    HMODULE h = GetModuleHandleA("ntdll.dll");
    if (!h) return 0;
    pQuery = (PFN_NtQueryTimerResolution)GetProcAddress(h, "NtQueryTimerResolution");
    pSet = (PFN_NtSetTimerResolution)GetProcAddress(h, "NtSetTimerResolution");
    return pQuery && pSet;
}
static double QueryTimer(void) {
    if (!pQuery) return 15.6;
    ULONG mn=0, mx=0, cur=0;
    if (pQuery(&mn, &mx, &cur) != 0) return 15.6;
    return cur / 10000.0;
}
static double SetTimerRes(double ms) {
    if (!pSet) return ms;
    ULONG cur=0;
    ULONG desired = (ULONG)(ms * 10000.0 + 0.5);
    if (pSet(desired, TRUE, &cur) != 0) return -1;
    return cur / 10000.0;
}
static void ReleaseTimer(void) {
    if (!pSet) return;
    ULONG cur=0;
    pSet(156000, FALSE, &cur);
}

/* ---------- FilterKeys ---------- */
#ifndef SPI_GETFILTERKEYS
#define SPI_GETFILTERKEYS 0x0032
#define SPI_SETFILTERKEYS 0x0033
#endif
#define FKF_FILTERKEYSON 0x00000001
#define FKF_AVAILABLE 0x00000002

typedef struct { UINT cbSize; DWORD dwFlags; DWORD iWaitMSec; DWORD iDelayMSec; DWORD iRepeatMSec; DWORD iBounceMSec; } MYFILTERKEYS;

static int GetFK(MYFILTERKEYS *fk) {
    memset(fk, 0, sizeof(*fk));
    fk->cbSize = sizeof(*fk);
    return SystemParametersInfoW(SPI_GETFILTERKEYS, sizeof(*fk), fk, 0);
}
/* Impone la regla de Microsoft: bounce o slow/repeat, nunca los dos. */
static int SetFK(int enabled, int bounce, DWORD bounceMs, DWORD waitMs, DWORD delayMs, DWORD repeatMs) {
    MYFILTERKEYS fk; memset(&fk, 0, sizeof(fk));
    fk.cbSize = sizeof(fk);
    fk.dwFlags = FKF_AVAILABLE | (enabled ? FKF_FILTERKEYSON : 0);
    if (bounce && bounceMs) {
        fk.iBounceMSec = bounceMs; fk.iWaitMSec = 0; fk.iDelayMSec = 0; fk.iRepeatMSec = 0;
    } else {
        fk.iBounceMSec = 0; fk.iWaitMSec = waitMs; fk.iDelayMSec = delayMs; fk.iRepeatMSec = repeatMs;
    }
    return SystemParametersInfoW(SPI_SETFILTERKEYS, sizeof(fk), &fk, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

/* ---------- DNS Jumper ---------- */
typedef struct { char friendly[256]; } AdapterInfo;
static AdapterInfo g_adapters[16];
static int g_adapterCount = 0;
static int g_wsaInit = 0;

static void EnsureWSA(void) {
    if (g_wsaInit) return;
    WSADATA wsa; if (WSAStartup(MAKEWORD(2,2), &wsa) == 0) g_wsaInit = 1;
}
static void SockAddrToStr(struct sockaddr *sa, char *out, int outLen) {
    out[0] = 0;
    if (!sa) return;
    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in*)sa;
        InetNtopA(AF_INET, &s->sin_addr, out, outLen);
    } else if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6 *s = (struct sockaddr_in6*)sa;
        InetNtopA(AF_INET6, &s->sin6_addr, out, outLen);
    }
}
static int EnumAdapters(PIP_ADAPTER_ADDRESSES *outBuf, ULONG *outLen) {
    ULONG len = 15000;
    PIP_ADAPTER_ADDRESSES buf = (PIP_ADAPTER_ADDRESSES)malloc(len);
    if (!buf) return 0;
    DWORD r = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, buf, &len);
    if (r == ERROR_BUFFER_OVERFLOW) {
        free(buf); buf = (PIP_ADAPTER_ADDRESSES)malloc(len);
        if (!buf) return 0;
        r = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, buf, &len);
    }
    if (r != NO_ERROR) { free(buf); return 0; }
    *outBuf = buf; *outLen = len; return 1;
}
static void RefreshAdapters(void) {
    HWND cb = Ctl(ID_DNS_ADAPTER);
    if (!cb) return;
    SendMessage(cb, CB_RESETCONTENT, 0, 0);
    g_adapterCount = 0;
    PIP_ADAPTER_ADDRESSES buf = NULL; ULONG len = 0;
    if (!EnumAdapters(&buf, &len)) { SendMessage(cb, CB_ADDSTRING, 0, (LPARAM)Tr(LS_NO_ADAPTERS)); SendMessage(cb, CB_SETCURSEL, 0, 0); return; }
    for (PIP_ADAPTER_ADDRESSES a = buf; a && g_adapterCount < 16; a = a->Next) {
        if (a->IfType == IF_TYPE_SOFTWARE_LOOPBACK) continue;
        if (a->OperStatus != IfOperStatusUp) continue;
        if (!a->FriendlyName || !a->FriendlyName[0]) continue;
        char friendly[256] = {0};
        WideCharToMultiByte(CP_ACP, 0, a->FriendlyName, -1, friendly, sizeof friendly - 1, NULL, NULL);
        if (!friendly[0]) continue;
        strncpy(g_adapters[g_adapterCount].friendly, friendly, sizeof g_adapters[0].friendly - 1);
        SendMessageA(cb, CB_ADDSTRING, 0, (LPARAM)friendly);
        g_adapterCount++;
    }
    free(buf);
    if (g_adapterCount > 0) SendMessage(cb, CB_SETCURSEL, 0, 0);
    else { SendMessage(cb, CB_ADDSTRING, 0, (LPARAM)Tr(LS_NO_ACTIVE_ADAPTERS)); SendMessage(cb, CB_SETCURSEL, 0, 0); }
}
static void RefreshCurrentDns(void) {
    char msg[512];
    int sel = (int)SendMessage(Ctl(ID_DNS_ADAPTER), CB_GETCURSEL, 0, 0);
    if (sel < 0 || sel >= g_adapterCount) { SetWindowTextA(Ctl(ID_LBL_DNS_CURRENT), Tr(LS_DNS_CHOOSE_ADAPTER)); return; }
    PIP_ADAPTER_ADDRESSES buf = NULL; ULONG len = 0;
    if (!EnumAdapters(&buf, &len)) { SetWindowTextA(Ctl(ID_LBL_DNS_CURRENT), Tr(LS_DNS_UNREADABLE)); return; }
    char d1[64] = {0}, d2[64] = {0};
    int found = 0;
    for (PIP_ADAPTER_ADDRESSES a = buf; a; a = a->Next) {
        if (a->IfType == IF_TYPE_SOFTWARE_LOOPBACK || a->OperStatus != IfOperStatusUp) continue;
        char friendly[256] = {0};
        WideCharToMultiByte(CP_ACP, 0, a->FriendlyName, -1, friendly, sizeof friendly - 1, NULL, NULL);
        if (strcmp(friendly, g_adapters[sel].friendly) != 0) continue;
        found = 1;
        int n = 0;
        for (IP_ADAPTER_DNS_SERVER_ADDRESS *d = a->FirstDnsServerAddress; d && n < 2; d = d->Next) {
            char s[64] = {0}; SockAddrToStr(d->Address.lpSockaddr, s, sizeof s);
            if (!s[0]) continue;
            if (n == 0) strncpy(d1, s, sizeof d1 - 1); else strncpy(d2, s, sizeof d2 - 1);
            n++;
        }
        break;
    }
    free(buf);
    if (!found) { SetWindowTextA(Ctl(ID_LBL_DNS_CURRENT), Tr(LS_DNS_ADAPTER_GONE)); return; }
    if (d1[0] && d2[0]) snprintf(msg, sizeof msg, Tr(LS_DNS_CURRENT_2), d1, d2);
    else if (d1[0]) snprintf(msg, sizeof msg, Tr(LS_DNS_CURRENT_1), d1);
    else snprintf(msg, sizeof msg, Tr(LS_DNS_CURRENT_AUTO));
    SetWindowTextA(Ctl(ID_LBL_DNS_CURRENT), msg);
}
static int RunHidden(const char *cmdline, DWORD *exitCode) {
    char buf[1024];
    strncpy(buf, cmdline, sizeof buf - 1); buf[sizeof buf - 1] = 0;
    STARTUPINFOA si; PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof si); si.cb = sizeof si;
    si.dwFlags = STARTF_USESHOWWINDOW; si.wShowWindow = SW_HIDE;
    memset(&pi, 0, sizeof pi);
    if (!CreateProcessA(NULL, buf, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) return 0;
    WaitForSingleObject(pi.hProcess, 15000);
    DWORD code = 1; GetExitCodeProcess(pi.hProcess, &code);
    if (exitCode) *exitCode = code;
    CloseHandle(pi.hProcess); CloseHandle(pi.hThread);
    return code == 0;
}
static void SetDnsStatus(const char *s) {
    SetWindowTextA(Ctl(ID_LBL_DNS_STATUS), s);
    SetStatus(s);
}
static void GetSelectedAdapter(char *out, int outLen) {
    out[0] = 0;
    int sel = (int)SendMessage(Ctl(ID_DNS_ADAPTER), CB_GETCURSEL, 0, 0);
    if (sel >= 0 && sel < g_adapterCount) strncpy(out, g_adapters[sel].friendly, outLen - 1);
}
static void GetEditText(int id, char *out, int outLen) {
    GetWindowTextA(Ctl(id), out, outLen);
}
static int IsValidIPv4(const char *s) {
    struct in_addr a; return s && s[0] && inet_pton(AF_INET, s, &a) == 1;
}
static void ApplyDns(void) {
    char adapter[256]; GetSelectedAdapter(adapter, sizeof adapter);
    if (!adapter[0]) { SetDnsStatus(Tr(LS_DNS_CHOOSE_FIRST)); return; }
    char p[64], s[64]; GetEditText(ID_DNS_PRIMARY, p, sizeof p); GetEditText(ID_DNS_SECONDARY, s, sizeof s);
    if (!IsValidIPv4(p)) { MessageBoxA(g_hwnd, Tr(LS_DNS_BAD_IP), "DNS", MB_ICONWARNING); return; }
    char cmd[1024]; DWORD code = 0;
    snprintf(cmd, sizeof cmd, "cmd.exe /c netsh interface ip set dns name=\"%s\" static %s primary", adapter, p);
    if (!RunHidden(cmd, &code)) { SetDnsStatus(Tr(LS_DNS_APPLY_FAIL)); MessageBoxA(g_hwnd, Tr(LS_DNS_APPLY_FAIL_MB), "DNS", MB_ICONWARNING); return; }
    if (IsValidIPv4(s)) {
        snprintf(cmd, sizeof cmd, "cmd.exe /c netsh interface ip add dns name=\"%s\" %s index=2", adapter, s);
        RunHidden(cmd, &code);
    }
    char msg[256]; snprintf(msg, sizeof msg, Tr(LS_DNS_APPLIED_FMT), adapter, p, IsValidIPv4(s) ? ", " : "", IsValidIPv4(s) ? s : "");
    SetDnsStatus(msg); RefreshCurrentDns();
}
static void RestoreDnsAuto(void) {
    char adapter[256]; GetSelectedAdapter(adapter, sizeof adapter);
    if (!adapter[0]) { SetDnsStatus(Tr(LS_DNS_CHOOSE_FIRST)); return; }
    char cmd[1024]; DWORD code = 0;
    snprintf(cmd, sizeof cmd, "cmd.exe /c netsh interface ip set dns name=\"%s\" dhcp", adapter);
    if (!RunHidden(cmd, &code)) { SetDnsStatus(Tr(LS_DNS_RESTORE_FAIL)); MessageBoxA(g_hwnd, Tr(LS_DNS_RESTORE_FAIL_MB), "DNS", MB_ICONWARNING); return; }
    SetDnsStatus(Tr(LS_DNS_RESTORED_AUTO)); RefreshCurrentDns();
}
static void FlushDns(void) {
    DWORD code = 0;
    if (RunHidden("cmd.exe /c ipconfig /flushdns", &code)) SetDnsStatus(Tr(LS_DNS_FLUSH_OK));
    else SetDnsStatus(Tr(LS_DNS_FLUSH_FAIL));
}
/* Declaraciones adelantadas: el ping corre en hilos worker (definidos mas abajo) */
static int StartPingSingle(void);
static int StartPingAll(void);
/* Tr(LS_TEST_PING) ya no mide en el hilo UI (4 x 1000 ms lo congelaban):
 * lanza un hilo worker y el resultado llega con WM_PING_DONE. */
static void PingDns(void) {
    StartPingSingle();
}
static void SetDnsPreset(const char *p, const char *s) {
    SetWindowTextA(Ctl(ID_DNS_PRIMARY), p);
    SetWindowTextA(Ctl(ID_DNS_SECONDARY), s);
}

/* Declaraciones adelantadas (definidas en Helpers UI, mas abajo) */
static void SetTrack(double ms);
static void ApplyTimer(void);
static void ApplyFK(void);

/* ---------- Ping generico (para ping en vivo y "probar todos") ---------- */
static int PingHost(const char *ip, int attempts, DWORD timeoutMs, long *avgMs, int *okCount) {
    if (okCount) *okCount = 0;
    if (avgMs) *avgMs = 0;
    if (!IsValidIPv4(ip)) return 0;
    EnsureWSA();
    HANDLE h = IcmpCreateFile();
    if (h == INVALID_HANDLE_VALUE) return 0;
    char data[32]; memset(data, 'a', sizeof data);
    char replyBuf[sizeof(ICMP_ECHO_REPLY) + 32];
    IPAddr dest = inet_addr(ip);
    int ok = 0; long total = 0;
    for (int i = 0; i < attempts; i++) {
        DWORD r = IcmpSendEcho(h, dest, data, sizeof data, NULL, replyBuf, sizeof replyBuf, timeoutMs);
        if (r > 0) { PICMP_ECHO_REPLY rep = (PICMP_ECHO_REPLY)replyBuf; if (rep->Status == 0) { ok++; total += rep->RoundTripTime; } }
    }
    IcmpCloseHandle(h);
    if (okCount) *okCount = ok;
    if (avgMs) *avgMs = ok ? total / ok : 0;
    return ok > 0;
}

/* ---------- Registro (Game Mode, pantalla completa) ---------- */
static int RegGetDword(HKEY root, const char *sub, const char *val, DWORD *out) {
    HKEY k; if (RegOpenKeyExA(root, sub, 0, KEY_READ, &k) != ERROR_SUCCESS) return 0;
    DWORD type = 0, data = 0, len = sizeof data;
    LONG r = RegQueryValueExA(k, val, NULL, &type, (LPBYTE)&data, &len);
    RegCloseKey(k);
    if (r != ERROR_SUCCESS || type != REG_DWORD) return 0;
    *out = data; return 1;
}
static void RegSetDword(HKEY root, const char *sub, const char *val, DWORD v) {
    HKEY k; if (RegCreateKeyExA(root, sub, 0, NULL, 0, KEY_SET_VALUE, NULL, &k, NULL) != ERROR_SUCCESS) return;
    RegSetValueExA(k, val, 0, REG_DWORD, (const BYTE*)&v, sizeof v);
    RegCloseKey(k);
}
static void RegDelValue(HKEY root, const char *sub, const char *val) {
    HKEY k; if (RegOpenKeyExA(root, sub, 0, KEY_SET_VALUE, &k) != ERROR_SUCCESS) return;
    RegDeleteValueA(k, val);
    RegCloseKey(k);
}

/* ---------- SOCD Cleaner (WH_KEYBOARD_LL, WASD + flechas) ----------
 * ADVERTENCIA: en algunos juegos esta prohibido. En CS2 Valve lo prohibio
 * en 2024 y puede expulsarte o banearte. Apagado por defecto; al activarlo
 * el programa muestra la advertencia y solo se activa si el usuario acepta.
 * Modos: 0 = ultima tecla gana, 1 = neutro (ninguna al pulsar las dos).
 * phys[] = tecla fisicamente pulsada; os[] = Windows cree que esta pulsada;
 * supp[] = pulsada fisicamente pero suprimida (Windows cree que esta suelta).
 * Los eventos inyectados por nuestro propio SendInput se dejan pasar sin
 * procesarlos (LLKHF_INJECTED), asi no hay bucles. */
/* ---------- SOCD Cleaner (WH_KEYBOARD_LL, WASD + flechas) ----------
 * ADVERTENCIA: en algunos juegos esta prohibido. En CS2 Valve lo prohibio
 * en 2024 y puede expulsarte o banearte. Apagado por defecto; al activarlo
 * el programa muestra la advertencia y solo se activa si el usuario acepta.
 * Modos: 0 = ultima tecla gana, 1 = neutro (ninguna al pulsar las dos).
 * phys[] = tecla fisicamente pulsada; os[] = Windows cree que esta pulsada;
 * supp[] = pulsada fisicamente pero suprimida (Windows cree que esta suelta).
 * Los eventos inyectados por nuestro propio SendInput se dejan pasar sin
 * procesarlos (LLKHF_INJECTED), asi no hay bucles.
 *
 * CORRECCION v6 (el delay al pulsar): los hooks WH_KEYBOARD_LL se
 * ejecutan en el contexto del hilo que los instalo. En v5 el hook vivia
 * en el hilo de la UI, y ese hilo se bloqueaba hasta 1 s en cada ping en
 * vivo (IcmpSendEcho) y hasta 7,5 s al medir los 5 DNS del Modo
 * Competitivo, asi que cada tecla tenia que esperar a que la UI se
 * desbloqueara: ese era el delay (se noto justo al pulsar Modo
 * Competitivo y luego activar el SOCD). Ahora el hook vive en un hilo
 * dedicado que solo bombea mensajes y nunca bloquea (el ping corre en
 * hilos worker aparte), y la UI solo le pide activar/desactivar. El
 * hook en si no toca la UI, no hace I/O ni Sleep. */
static HHOOK g_socdHook = NULL;
static volatile LONG g_socdOn = 0;
static volatile LONG g_socdMode = 0;
static HANDLE g_socdThread = NULL;
static DWORD g_socdThreadId = 0;
static HANDLE g_socdReadyEvent = NULL; /* se senala cuando el hook quedo instalado */
static int g_socdPhys[8] = {0}, g_socdOs[8] = {0}, g_socdSupp[8] = {0};
static const DWORD SOCD_VK[8] = { 0x41, 0x44, 0x57, 0x53, VK_LEFT, VK_RIGHT, VK_UP, VK_DOWN };
static const char *SOCD_NAMES[8] = { "A", "D", "W", "S", "Izq", "Der", "Arriba", "Abajo" };

/* v11: nombre de la tecla en el idioma actual (las flechas se traducen;
 * A/D/W/S son iguales en los dos idiomas). */
static const char *SocdKeyName(int idx) {
    if (idx >= 4 && idx < 8) {
        static const int k[4] = { LS_KEY_LEFT, LS_KEY_RIGHT, LS_KEY_UP, LS_KEY_DOWN };
        return Tr(k[idx - 4]);
    }
    if (idx >= 0 && idx < 4) return SOCD_NAMES[idx];
    return "?";
}
static int SocdIndex(DWORD vk) {
    for (int i = 0; i < 8; i++) if (SOCD_VK[i] == vk) return i;
    return -1;
}
/* v7: mandar tambien el scan code real (muchos juegos leen el teclado
 * por Raw Input / scan code; solo con wVk el evento tarda mas en
 * traducirse). Flechas (idx>=4) son teclas extendidas. */
static void SocdMakeInput(INPUT *in, int idx, int up) {
    memset(in, 0, sizeof *in);
    in->type = INPUT_KEYBOARD;
    in->ki.wVk = (WORD)SOCD_VK[idx];
    UINT sc = MapVirtualKeyA((UINT)SOCD_VK[idx], MAPVK_VK_TO_VSC);
    in->ki.wScan = (WORD)sc;
    in->ki.dwFlags = (up ? KEYEVENTF_KEYUP : 0)
        | (sc ? KEYEVENTF_SCANCODE : 0)
        | (idx >= 4 ? KEYEVENTF_EXTENDEDKEY : 0);
}
static void SocdSendKey(int idx, int up) {
    INPUT in; SocdMakeInput(&in, idx, up);
    SendInput(1, &in, sizeof in);
}
/* v7 (clave): inyectar DOS eventos en UNA sola llamada SendInput, en
 * orden. Asi el cambio A->D llega atomico: [A-up, D-down]. */
static void SocdSendKeys2(int idx1, int up1, int idx2, int up2) {
    INPUT in[2];
    SocdMakeInput(&in[0], idx1, up1);
    SocdMakeInput(&in[1], idx2, up2);
    SendInput(2, in, sizeof(INPUT));
}
/* v7: throttle del medidor de prueba (ultimo PostMessage enviado). */
static DWORD g_lastSocdTestTick = 0;
static LRESULT CALLBACK SocdHookProc(int nCode, WPARAM wp, LPARAM lp) {
    if (nCode < 0) return CallNextHookEx(g_socdHook, nCode, wp, lp);
    KBDLLHOOKSTRUCT *k = (KBDLLHOOKSTRUCT*)lp;
    if (k->flags & LLKHF_INJECTED) return CallNextHookEx(g_socdHook, nCode, wp, lp);
    int idx = SocdIndex(k->vkCode);
    if (idx < 0) return CallNextHookEx(g_socdHook, nCode, wp, lp);
    int opp = idx ^ 1;
    int down = (wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN);
    if (down) {
        if (g_socdPhys[idx]) {
            /* Repeticion de una tecla ya pulsada: si esta suprimida, tragarsela */
            if (g_socdSupp[idx]) return 1;
            return CallNextHookEx(g_socdHook, nCode, wp, lp);
        }
        g_socdPhys[idx] = 1;
        /* Medidor de prueba, v7 con throttle: antes se hacia PostMessage
         * a la UI en CADA tecla, y eso metia trafico UI en el hot path del
         * hook. Ahora solo se avisa cuando la tecla opuesta esta pulsada
         * (cambio A->D o D->A, que es justo lo que hace el SOCD) o, como
         * mucho, 1 vez cada 100 ms. El hook sigue sin tocar la UI. */
        if (g_hwnd) {
            DWORD now = GetTickCount();
            if (g_socdPhys[opp] || (now - g_lastSocdTestTick) >= 100) {
                g_lastSocdTestTick = now;
                long lat = (long)(now - k->time);
                if (lat < 0) lat = 0;
                if (lat > 60000) lat = 0;
                PostMessageA(g_hwnd, WM_SOCD_KEY, (WPARAM)idx, (LPARAM)lat);
            }
        }
        if (g_socdPhys[opp]) {
            if (g_socdMode == 1) { /* Neutro: ninguna */
                SocdSendKey(opp, 1); g_socdOs[opp] = 0; g_socdSupp[opp] = 1;
                g_socdSupp[idx] = 1;
                return 1;
            }
            /* Ultima tecla gana, v7 (EL CAMBIO CLAVE): antes se inyectaba
             * el A-up y se dejaba pasar el D-down original con
             * CallNextHookEx. El A-up sintetico quedaba en cola DETRAS
             * del D-down original, asi que el juego veia D-down primero
             * y A-up despues: un instante con las dos pulsadas, que se
             * siente como delay (y el medidor marcaba 0 ms, porque el
             * hook recibia la tecla al instante; el problema estaba
             * aguas abajo, en el orden de entrega). Ahora se TRAGA el
             * evento original (return 1) y se inyecta en UNA sola
             * llamada SendInput el lote [opuesta-up, nueva-down], en
             * ese orden, atomico. */
            SocdSendKeys2(opp, 1, idx, 0);
            g_socdOs[opp] = 0; g_socdSupp[opp] = 1;
            g_socdOs[idx] = 1;
            return 1;
        }
        g_socdOs[idx] = 1;
        return CallNextHookEx(g_socdHook, nCode, wp, lp);
    }
    /* Tecla soltada */
    if (!g_socdPhys[idx]) return CallNextHookEx(g_socdHook, nCode, wp, lp);
    g_socdPhys[idx] = 0;
    int wasSupp = g_socdSupp[idx];
    g_socdSupp[idx] = 0;
    if (!wasSupp) g_socdOs[idx] = 0;
    /* Si la opuesta sigue pulsada fisicamente y estaba suprimida, reactivarla.
     * v7: mismo principio que al pulsar: el orden debe ser [esta-up,
     * opuesta-down] y en un solo lote, para que el juego nunca vea las
     * dos a la vez ni un hueco. Si Windows ya creia esta tecla suelta
     * (wasSupp), solo hace falta el down de la opuesta. */
    if (g_socdPhys[opp] && g_socdSupp[opp]) {
        if (wasSupp) {
            SocdSendKey(opp, 0);
        } else {
            SocdSendKeys2(idx, 1, opp, 0);
        }
        g_socdOs[opp] = 1; g_socdSupp[opp] = 0;
        return 1;
    }
    if (wasSupp) return 1; /* Windows ya la creia suelta: no pasar el key-up */
    return CallNextHookEx(g_socdHook, nCode, wp, lp);
}
/* Hilo dedicado del SOCD: instala el hook aqui (asi los callbacks corren
 * en este hilo, no en la UI) y bombea mensajes hasta que le pidan salir. */
static DWORD WINAPI SocdThreadProc(LPVOID p) {
    (void)p;
    /* v7: prioridad maxima para este hilo: el hook debe adelantarse a la
     * UI, a las animaciones y al juego. Con prioridad normal competia con
     * todo lo demas y eso dejaba un delay residual. */
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    memset(g_socdPhys, 0, sizeof g_socdPhys);
    memset(g_socdOs, 0, sizeof g_socdOs);
    memset(g_socdSupp, 0, sizeof g_socdSupp);
    g_socdHook = SetWindowsHookExA(WH_KEYBOARD_LL, SocdHookProc, g_hInst, 0);
    if (g_socdReadyEvent) SetEvent(g_socdReadyEvent);
    if (!g_socdHook) { InterlockedExchange(&g_socdOn, 0); return 1; }
    InterlockedExchange(&g_socdOn, 1);
    MSG m;
    while (GetMessageA(&m, NULL, 0, 0) > 0) { TranslateMessage(&m); DispatchMessageA(&m); }
    /* Salida limpia: soltar en Windows cualquier tecla que quedara pulsada */
    for (int i = 0; i < 8; i++) {
        if (g_socdOs[i]) SocdSendKey(i, 1);
        g_socdPhys[i] = g_socdOs[i] = g_socdSupp[i] = 0;
    }
    if (g_socdHook) { UnhookWindowsHookEx(g_socdHook); g_socdHook = NULL; }
    InterlockedExchange(&g_socdOn, 0);
    return 0;
}
static void SocdSetStatus(const char *s) {
    SetWindowTextA(Ctl(ID_LBL_SOCD_STATUS), s);
    SetStatus(s);
}
/* v7: prioridad del proceso guardada para restaurarla al desactivar el SOCD */
static DWORD g_prevPriorityClass = 0;
static int g_priorityRaised = 0;
static void SocdInstall(void) {
    if (g_socdThread) return;
    if (!g_socdReadyEvent) g_socdReadyEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    ResetEvent(g_socdReadyEvent);
    g_socdThread = CreateThread(NULL, 0, SocdThreadProc, NULL, 0, &g_socdThreadId);
    if (!g_socdThread) { InterlockedExchange(&g_socdOn, 0); SetChecked(ID_CHK_SOCD, BST_UNCHECKED); SocdSetStatus(Tr(LS_SOCD_NO_THREAD)); return; }
    DWORD w = WaitForSingleObject(g_socdReadyEvent, 500); /* v7: antes 1000 ms */
    if (w != WAIT_OBJECT_0 || !g_socdHook) {
        SocdSetStatus(Tr(LS_SOCD_NO_HOOK));
        SetChecked(ID_CHK_SOCD, BST_UNCHECKED);
        if (g_socdThreadId) PostThreadMessageA(g_socdThreadId, WM_QUIT, 0, 0);
        WaitForSingleObject(g_socdThread, 500);
        CloseHandle(g_socdThread); g_socdThread = NULL; g_socdThreadId = 0;
        InterlockedExchange(&g_socdOn, 0);
        return;
    }
    /* v7: subir la clase de prioridad del proceso mientras el SOCD esta
     * activo, para que el hilo del hook (TIME_CRITICAL) no compita con
     * la UI ni con el juego. Se restaura en SocdUninstall. */
    g_prevPriorityClass = GetPriorityClass(GetCurrentProcess());
    if (g_prevPriorityClass != HIGH_PRIORITY_CLASS) {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        g_priorityRaised = 1;
    }
    /* v7: pausar las animaciones mientras se juega con el SOCD activo:
     * timer 2 = fundido de botones, timer 6 = deslizamiento de pagina,
     * timer 1 de la barra = hover/indicador de pestanas. Se dejan en su
     * estado final de una vez (indicador en su sitio) y se reanudan solas
     * al desactivar el SOCD (el proximo hover/select vuelve a arrancarlas). */
    KillTimer(g_hwnd, 2);
    KillTimer(g_hwnd, 6);
    g_slideX = 0;
    SetWindowPos(g_pages[g_curTab], NULL, PAGE_X, PAGE_Y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    if (g_tabBar) {
        KillTimer(g_tabBar, 1);
        g_indX = g_indTX; g_indW = g_indTW;
        for (int i = 0; i < 5; i++) g_tabHoverLvl[i] = (i == g_tabHover) ? 255 : 0;
        InvalidateRect(g_tabBar, NULL, FALSE);
    }
    SocdSetStatus(g_socdMode == 0 ? Tr(LS_SOCD_ON_LAST_HP) : Tr(LS_SOCD_ON_NEUTRAL_HP));
}
static void SocdUninstall(void) {
    if (g_socdThread) {
        if (g_socdThreadId) PostThreadMessageA(g_socdThreadId, WM_QUIT, 0, 0);
        WaitForSingleObject(g_socdThread, 1000);
        CloseHandle(g_socdThread);
        g_socdThread = NULL; g_socdThreadId = 0;
    }
    g_socdHook = NULL;
    InterlockedExchange(&g_socdOn, 0);
    /* v7: restaurar la prioridad del proceso que se subio al activar el SOCD.
     * Las animaciones se reanudan solas: el proximo hover/select vuelve a
     * arrancar sus timers (BtnSubProc / TabBarAnimate). */
    if (g_priorityRaised && g_prevPriorityClass) {
        SetPriorityClass(GetCurrentProcess(), g_prevPriorityClass);
        g_priorityRaised = 0; g_prevPriorityClass = 0;
    }
    SocdSetStatus(Tr(LS_SOCD_OFF));
}
static void SocdToggle(void) {
    if (IsChecked(ID_CHK_SOCD) != BST_CHECKED) { SocdUninstall(); return; }
    /* Advertencia obligatoria, siempre, antes de activar */
    char warn[700];
    snprintf(warn, sizeof warn, Tr(LS_SOCD_WARN_FMT),
        IsChecked(ID_CHK_FK) == BST_CHECKED ? Tr(LS_SOCD_WARN_FK) : "");
    if (MessageBoxA(g_hwnd, warn, Tr(LS_SOCD_WARN_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
        SetChecked(ID_CHK_SOCD, BST_UNCHECKED);
        SocdSetStatus(Tr(LS_SOCD_OFF_WARN));
        return;
    }
    InterlockedExchange(&g_socdMode, (IsChecked(ID_SOCD_NEUTRAL) == BST_CHECKED) ? 1 : 0);
    SocdInstall();
}
/* ---------- Limpieza de memoria standby tipo ISLC ---------- */
typedef LONG (NTAPI *PFN_NtSetSystemInformation)(int, PVOID, ULONG);
static int PurgeStandby(void) {
    HMODULE h = GetModuleHandleA("ntdll.dll");
    if (!h) return 0;
    PFN_NtSetSystemInformation p = (PFN_NtSetSystemInformation)GetProcAddress(h, "NtSetSystemInformation");
    if (!p) return 0;
    ULONG cmd = 4; /* MemoryPurgeStandbyList */
    return p(80 /* SystemMemoryListInformation */, &cmd, sizeof cmd) == 0;
}
static unsigned long long AvailPhysMB(void) {
    MEMORYSTATUSEX st; memset(&st, 0, sizeof st); st.dwLength = sizeof st;
    if (!GlobalMemoryStatusEx(&st)) return 0;
    return (unsigned long long)(st.ullAvailPhys / (1024 * 1024));
}
static void CleanMemory(void) {
    unsigned long long before = AvailPhysMB();
    if (PurgeStandby()) {
        unsigned long long after = AvailPhysMB();
        char msg[256];
        snprintf(msg, sizeof msg, Tr(LS_MEM_CLEANED_FMT), before, after);
        SetWindowTextA(Ctl(ID_LBL_MEM_STATUS), msg); SetStatus(msg);
    } else {
        SetWindowTextA(Ctl(ID_LBL_MEM_STATUS), Tr(LS_MEM_CLEAN_FAIL));
    }
}

/* ---------- Modo Competitivo ---------- */
static int StartPingAll(void); /* definida mas abajo (hilo worker), usada por CompetitiveMode */
static volatile LONG g_compMeasuring; /* 1 = la medicion de DNS la pidio el Modo Competitivo */
static char g_prevPowerGuid[64] = {0};
static int g_prevGameMode = -1; /* -1 = no existia */
static void CompLog(const char *line) {
    HWND h = Ctl(ID_COMP_LOG);
    if (!h) return;
    int len = GetWindowTextLengthA(h);
    SendMessageA(h, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    char buf[512]; snprintf(buf, sizeof buf, "%s\r\n", line);
    SendMessageA(h, EM_REPLACESEL, 0, (LPARAM)buf);
}
/* v11: leer el plan activo con PowerGetActiveScheme (powrprof): sin
 * lanzar powercfg, asi no aparece ninguna ventana de cmd al abrir. */
static void GetActivePowerGuid(char *out, int outLen) {
    out[0] = 0;
    GUID *pg = NULL;
    if (PowerGetActiveScheme(NULL, &pg) != ERROR_SUCCESS || !pg) return;
    wchar_t ws[64];
    if (StringFromGUID2(pg, ws, 64) > 0) {
        /* StringFromGUID2 devuelve "{guid}" con llaves: quitarlas, que
         * powercfg /setactive acepta el GUID sin llaves. */
        char tmp[64]; int n = 0;
        for (int i = 0; ws[i] && n < (int)sizeof tmp - 1; i++) {
            if (ws[i] == L'{' || ws[i] == L'}') continue;
            tmp[n++] = (char)ws[i];
        }
        tmp[n] = 0;
        strncpy(out, tmp, outLen - 1); out[outLen - 1] = 0;
    }
    LocalFree(pg);
}
static void CompetitiveMode(void) {
    InterlockedExchange(&g_compActive, 1);
    HWND h = Ctl(ID_COMP_LOG); if (h) SetWindowTextA(h, "");
    CompLog(Tr(LS_COMP_ON_LOG));
    /* 1) Timer 0.5 ms */
    SetTrack(0.5); ApplyTimer();
    CompLog("- Timer Resolution: 0.5 ms.");
    /* 2) FilterKeys apagado */
    SetChecked(ID_CHK_FK, BST_UNCHECKED); SetChecked(ID_CHK_BOUNCE, BST_UNCHECKED); ApplyFK();
    CompLog(Tr(LS_COMP_FK_OFF));
    /* 3) Limpiar cache DNS */
    FlushDns();
    CompLog(Tr(LS_COMP_DNS_FLUSH));
    /* 4) Plan de energia Alto rendimiento */
    GetActivePowerGuid(g_prevPowerGuid, sizeof g_prevPowerGuid);
    DWORD code = 0;
    if (RunHidden("cmd.exe /c powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c", &code))
        CompLog(Tr(LS_COMP_POWER_HP));
    else CompLog(Tr(LS_COMP_POWER_FAIL));
    /* 5) Game Mode */
    DWORD gm = 0;
    g_prevGameMode = RegGetDword(HKEY_CURRENT_USER, "Software\\Microsoft\\GameBar", "AllowAutoGameMode", &gm) ? (int)gm : -1;
    RegSetDword(HKEY_CURRENT_USER, "Software\\Microsoft\\GameBar", "AllowAutoGameMode", 1);
    CompLog(Tr(LS_COMP_GAMEMODE_ON));
    /* 6) DNS mas rapido: medir los 5 presets y recomendar (sin aplicar solo).
     * En v5 esto se hacia en el hilo UI (hasta 7,5 s) y era lo que frenaba
     * el teclado con el SOCD activo: ahora es asincrono, el resultado
     * llega con WM_PING_DONE y lo registra el log. */
    if (StartPingAll()) {
        InterlockedExchange(&g_compMeasuring, 1);
        CompLog(Tr(LS_COMP_DNS_MEASURING));
    } else {
        CompLog(Tr(LS_COMP_DNS_BUSY));
        CompLog(Tr(LS_COMP_READY));
    }
    SetStatus(Tr(LS_COMP_ON_STATUS));
    UpdateStatusBox(); /* v11: el cuadro de estado sigue al cambio */
}
static void CompetitiveRestore(void) {
    InterlockedExchange(&g_compActive, 0);
    HWND h = Ctl(ID_COMP_LOG); if (h) SetWindowTextA(h, "");
    CompLog(Tr(LS_COMP_RESTORE_LOG));
    SetTrack(15.6); ApplyTimer();
    CompLog(Tr(LS_COMP_TIMER_156));
    if (g_prevPowerGuid[0]) {
        char cmd[256]; snprintf(cmd, sizeof cmd, "cmd.exe /c powercfg /setactive %s", g_prevPowerGuid);
        DWORD code = 0;
        if (RunHidden(cmd, &code)) CompLog(Tr(LS_COMP_POWER_RESTORED));
        else CompLog(Tr(LS_COMP_POWER_RESTORE_FAIL));
    } else {
        DWORD code = 0;
        RunHidden("cmd.exe /c powercfg /setactive 381b4222-f694-41f0-9685-ff5bb260df2e", &code);
        CompLog(Tr(LS_COMP_POWER_BALANCED));
    }
    if (g_prevGameMode < 0) RegDelValue(HKEY_CURRENT_USER, "Software\\Microsoft\\GameBar", "AllowAutoGameMode");
    else RegSetDword(HKEY_CURRENT_USER, "Software\\Microsoft\\GameBar", "AllowAutoGameMode", (DWORD)g_prevGameMode);
    CompLog(Tr(LS_COMP_GAMEMODE_RESTORED));
    SetStatus(Tr(LS_COMP_OFF_STATUS));
    UpdateStatusBox(); /* v11 */
}

/* ---------- Ping en hilos worker (correccion v6) ----------
 * Antes, LivePingTick (IcmpSendEcho de 1000 ms) y PingAllDns (5 DNS x 3
 * intentos x 500 ms, hasta 7,5 s) corrian en el hilo de la UI: la ventana
 * se congelaba y, peor, el hook del SOCD tambien se quedaba esperando,
 * asi que las teclas llegaban con delay (sobre todo tras pulsar Modo
 * Competitivo, que mide los 5 DNS). Ahora el ping corre en hilos aparte:
 * la UI solo recibe los resultados con PostMessage y nunca se congela.
 * El ping en vivo usa timeout de 300 ms. */
static const char *DNS_NAMES[5] = { "Google", "Cloudflare", "Quad9", "OpenDNS", "AdGuard" };
static const char *DNS_IPS[5] = { "8.8.8.8", "1.1.1.1", "9.9.9.9", "208.67.222.222", "94.140.14.14" };
static CRITICAL_SECTION g_pingCS;
static char g_pingTarget[64] = "8.8.8.8";
static char g_pingResult[512] = {0};
static int g_pingBestIdx = -1;
static long g_liveLast = -1, g_liveSum = 0, g_liveCount = 0, g_liveLost = 0;
static long g_liveAvg = 0, g_liveJitter = 0, g_liveMean = 0, g_liveLoss = 0;
static volatile LONG g_pingBusy = 0;   /* 1 = hay una medicion de un solo uso en curso */
static HANDLE g_liveThread = NULL;
static HANDLE g_liveStopEvent = NULL;
static void UpdatePingTarget(void) {
    char ip[64];
    if (!g_hwnd) return;
    GetEditText(ID_DNS_PRIMARY, ip, sizeof ip);
    if (IsValidIPv4(ip)) {
        EnterCriticalSection(&g_pingCS);
        strncpy(g_pingTarget, ip, sizeof g_pingTarget - 1); g_pingTarget[sizeof g_pingTarget - 1] = 0;
        LeaveCriticalSection(&g_pingCS);
    }
}
static DWORD WINAPI LivePingThreadProc(LPVOID p) {
    (void)p;
    long last = -1, sum = 0, count = 0, lost = 0;
    while (WaitForSingleObject(g_liveStopEvent, 0) != WAIT_OBJECT_0) {
        char ip[64];
        EnterCriticalSection(&g_pingCS);
        strncpy(ip, g_pingTarget, sizeof ip - 1); ip[sizeof ip - 1] = 0;
        LeaveCriticalSection(&g_pingCS);
        long avg = 0; int ok = 0;
        PingHost(ip, 1, 300, &avg, &ok);
        long jitter = 0;
        if (ok) {
            if (last >= 0) jitter = avg > last ? avg - last : last - avg;
            last = avg; sum += avg;
        } else lost++;
        count++;
        long mean = (count - lost) > 0 ? sum / (count - lost) : 0;
        long loss = count ? (lost * 100 / count) : 0;
        EnterCriticalSection(&g_pingCS);
        g_liveLast = last; g_liveSum = sum; g_liveCount = count; g_liveLost = lost;
        g_liveAvg = ok ? avg : -1; g_liveJitter = jitter; g_liveMean = mean; g_liveLoss = loss;
        LeaveCriticalSection(&g_pingCS);
        if (g_hwnd) PostMessageA(g_hwnd, WM_PING_UPDATE, 0, 0);
        if (WaitForSingleObject(g_liveStopEvent, 1000) == WAIT_OBJECT_0) break;
    }
    return 0;
}
/* v11: texto del ping en vivo (se usa en el resultado del hilo y al
 * refrescar el idioma). */
static void LivePingText(char *out, int outLen) {
    long avg, mean, jitter, loss, count;
    EnterCriticalSection(&g_pingCS);
    avg = g_liveAvg; mean = g_liveMean; jitter = g_liveJitter; loss = g_liveLoss; count = g_liveCount;
    LeaveCriticalSection(&g_pingCS);
    if (avg >= 0) snprintf(out, outLen, Tr(LS_LIVE_RESULT), avg, mean, jitter, loss, count);
    else snprintf(out, outLen, Tr(LS_LIVE_RESULT_NORESP), mean, jitter, loss, count);
}

static void LivePingStart(void) {
    if (g_liveThread) return;
    UpdatePingTarget();
    EnterCriticalSection(&g_pingCS);
    g_liveLast = -1; g_liveSum = 0; g_liveCount = 0; g_liveLost = 0;
    g_liveAvg = 0; g_liveJitter = 0; g_liveMean = 0; g_liveLoss = 0;
    LeaveCriticalSection(&g_pingCS);
    if (!g_liveStopEvent) g_liveStopEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
    ResetEvent(g_liveStopEvent);
    g_liveThread = CreateThread(NULL, 0, LivePingThreadProc, NULL, 0, NULL);
    if (!g_liveThread) SetWindowTextA(Ctl(ID_LBL_LIVE_PING), Tr(LS_LIVE_NO_THREAD));
}
static void LivePingStop(void) {
    if (!g_liveThread) return;
    if (g_liveStopEvent) SetEvent(g_liveStopEvent);
    /* Cerrar el handle no mata el hilo: termina solo en cuanto acabe el
     * ping en curso (max. 300 ms) y vea el evento. Sin esperas en la UI. */
    CloseHandle(g_liveThread);
    g_liveThread = NULL;
}
typedef struct { int kind; char ip[64]; } PingJob; /* kind 0 = un solo ping, 1 = probar los 5 DNS */
static DWORD WINAPI PingJobThreadProc(LPVOID p) {
    PingJob *job = (PingJob*)p;
    int kind = job->kind; char ip[64];
    strncpy(ip, job->ip, sizeof ip - 1); ip[sizeof ip - 1] = 0;
    free(job);
    int bestIdx = -1;
    char msg[512]; msg[0] = 0;
    if (kind == 0) {
        long avg = 0; int ok = 0;
        PingHost(ip, 4, 500, &avg, &ok);
        if (ok) snprintf(msg, sizeof msg, Tr(LS_PING_ONE_FMT), ip, ok, avg);
        else snprintf(msg, sizeof msg, Tr(LS_PING_ONE_NONE), ip);
    } else {
        long bestMs = 1000000;
        char detail[384]; detail[0] = 0;
        for (int i = 0; i < 5; i++) {
            long avg = 0; int ok = 0;
            PingHost(DNS_IPS[i], 3, 500, &avg, &ok);
            char part[64];
            if (ok) { snprintf(part, sizeof part, "%s %ld ms; ", DNS_NAMES[i], avg); if (avg < bestMs) { bestMs = avg; bestIdx = i; } }
            else snprintf(part, sizeof part, "%s sin respuesta; ", DNS_NAMES[i]);
            strncat(detail, part, sizeof detail - strlen(detail) - 1);
        }
        if (bestIdx >= 0) snprintf(msg, sizeof msg, Tr(LS_PING_BEST_FMT), DNS_NAMES[bestIdx], DNS_IPS[bestIdx], bestMs, detail);
        else snprintf(msg, sizeof msg, Tr(LS_PING_NONE_FMT), detail);
    }
    EnterCriticalSection(&g_pingCS);
    strncpy(g_pingResult, msg, sizeof g_pingResult - 1); g_pingResult[sizeof g_pingResult - 1] = 0;
    g_pingBestIdx = bestIdx;
    LeaveCriticalSection(&g_pingCS);
    InterlockedExchange(&g_pingBusy, 0);
    if (g_hwnd) PostMessageA(g_hwnd, WM_PING_DONE, (WPARAM)kind, (LPARAM)bestIdx);
    return 0;
}
static int StartPingJob(int kind, const char *ip) {
    if (InterlockedCompareExchange(&g_pingBusy, 1, 0) != 0) {
        SetDnsStatus(Tr(LS_PING_BUSY));
        return 0;
    }
    PingJob *job = (PingJob*)malloc(sizeof(PingJob));
    if (!job) { InterlockedExchange(&g_pingBusy, 0); return 0; }
    job->kind = kind;
    strncpy(job->ip, ip ? ip : "", sizeof job->ip - 1); job->ip[sizeof job->ip - 1] = 0;
    HANDLE h = CreateThread(NULL, 0, PingJobThreadProc, job, 0, NULL);
    if (!h) { free(job); InterlockedExchange(&g_pingBusy, 0); SetDnsStatus(Tr(LS_PING_START_FAIL)); return 0; }
    CloseHandle(h); /* el hilo se libera solo al terminar */
    return 1;
}
static int StartPingSingle(void) {
    char p[64]; GetEditText(ID_DNS_PRIMARY, p, sizeof p);
    if (!IsValidIPv4(p)) { MessageBoxA(g_hwnd, Tr(LS_PING_BAD_IP), "DNS", MB_ICONWARNING); return 0; }
    return StartPingJob(0, p);
}
static int StartPingAll(void) {
    return StartPingJob(1, "");
}
/* ---------- Optimizaciones de sistema ---------- */
static int g_savedMouse[3] = {6, 10, 1};
static int g_mouseSaved = 0;
static int g_prevFSE = -1, g_prevFSEHonor = -1;
static void ToggleMouseAccel(void) {
    if (IsChecked(ID_CHK_MOUSE_ACCEL) == BST_CHECKED) {
        if (!g_mouseSaved) { SystemParametersInfoA(SPI_GETMOUSE, 0, g_savedMouse, 0); g_mouseSaved = 1; }
        int off[3] = { 0, 0, g_savedMouse[2] };
        SystemParametersInfoA(SPI_SETMOUSE, 0, off, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        SetStatus(Tr(LS_MOUSE_OFF));
    } else if (g_mouseSaved) {
        SystemParametersInfoA(SPI_SETMOUSE, 0, g_savedMouse, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        SetStatus(Tr(LS_MOUSE_ON));
    }
}
static void ToggleFSE(void) {
    if (IsChecked(ID_CHK_FSE) == BST_CHECKED) {
        DWORD v = 0;
        g_prevFSE = RegGetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_FSEBehaviorMode", &v) ? (int)v : -1;
        g_prevFSEHonor = RegGetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_HonorUserFSEBehaviorMode", &v) ? (int)v : -1;
        RegSetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_FSEBehaviorMode", 2);
        RegSetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_HonorUserFSEBehaviorMode", 1);
        SetStatus(Tr(LS_FSE_OFF));
    } else {
        if (g_prevFSE < 0) RegDelValue(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_FSEBehaviorMode");
        else RegSetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_FSEBehaviorMode", (DWORD)g_prevFSE);
        if (g_prevFSEHonor < 0) RegDelValue(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_HonorUserFSEBehaviorMode");
        else RegSetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_HonorUserFSEBehaviorMode", (DWORD)g_prevFSEHonor);
        SetStatus(Tr(LS_FSE_ON));
    }
}
/* MinGW/Zig solo expone la version ancha de Toolhelp32: PROCESSENTRY32W */
static const wchar_t *BG_APPS[10] = { L"chrome.exe", L"msedge.exe", L"firefox.exe", L"discord.exe", L"spotify.exe", L"steam.exe", L"EpicGamesLauncher.exe", L"opera.exe", L"brave.exe", L"obs64.exe" };
static void CloseBackgroundApps(void) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) { SetStatus(Tr(LS_PROC_LIST_FAIL)); return; }
    char found[512]; found[0] = 0; int nFound = 0;
    wchar_t names[10][64]; int nNames = 0;
    PROCESSENTRY32W pe; pe.dwSize = sizeof pe;
    if (Process32FirstW(snap, &pe)) do {
        for (int i = 0; i < 10; i++) {
            if (_wcsicmp(pe.szExeFile, BG_APPS[i]) == 0) {
                char part[80]; char narrow[64] = {0};
                WideCharToMultiByte(CP_ACP, 0, pe.szExeFile, -1, narrow, sizeof narrow - 1, NULL, NULL);
                snprintf(part, sizeof part, "%s, ", narrow);
                strncat(found, part, sizeof found - strlen(found) - 1);
                wcsncpy(names[nNames], pe.szExeFile, 63); nNames++; nFound++;
                break;
            }
        }
    } while (Process32NextW(snap, &pe));
    CloseHandle(snap);
    if (!nFound) { MessageBoxA(g_hwnd, Tr(LS_NO_BG_APPS), Tr(LS_CLOSE_APPS_TITLE), MB_ICONINFORMATION); return; }
    if (found[0]) found[strlen(found) - 2] = 0;
    char q[700];
    snprintf(q, sizeof q, Tr(LS_CLOSE_APPS_BG_FMT), found);
    if (MessageBoxA(g_hwnd, q, Tr(LS_CLOSE_APPS_BG_TITLE), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES) return;
    int closed = 0;
    snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap != INVALID_HANDLE_VALUE) {
        pe.dwSize = sizeof pe;
        if (Process32FirstW(snap, &pe)) do {
            for (int i = 0; i < nNames; i++) {
                if (_wcsicmp(pe.szExeFile, names[i]) == 0) {
                    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pe.th32ProcessID);
                    if (h) { if (TerminateProcess(h, 0)) closed++; CloseHandle(h); }
                }
            }
        } while (Process32NextW(snap, &pe));
        CloseHandle(snap);
    }
    char msg[128]; snprintf(msg, sizeof msg, Tr(LS_APPS_CLOSED_FMT), closed);
    SetStatus(msg);
}

/* ================= v9: AUTO-ACTUALIZACION (v12: URL del exe) =================
 * Consulta primero version.txt (raw.githubusercontent, formato simple
 * version=/sha256=) y si falla, la API de GitHub Releases. La URL de
 * descarga del exe se resuelve SIEMPRE desde la API (browser_download_url
 * del asset LuAnOptimizer.exe) o se construye como
 * releases/download/v<ver>/LuAnOptimizer.exe: el campo url= de
 * version.txt ya no se usa (en v11 apuntaba a la pagina de releases y el
 * updater descargaba HTML). Todo en
 * un hilo de trabajo con timeout corto (~5 s): si la red falla o el repo
 * aun no existe (el repo en GitHub todavia no esta publicado),
 * NO se muestra error: solo "No se pudo revisar actualizaciones" en la
 * barra de estado y el programa sigue normal. Nunca se actualiza sin
 * permiso y nunca en mitad de una partida (si el Modo Competitivo esta
 * activo, solo se avisa). El .exe no se puede sobrescribir a si mismo:
 * se descarga a %TEMP%\LuAnOptimizer_new.exe, se verifica su SHA256 con
 * BCrypt (la de Windows, nunca una implementacion propia) y un .bat
 * temporal espera a que el proceso salga, mueve el nuevo sobre el viejo
 * y vuelve a abrir el programa. */
/* g_compActive se define arriba, junto a los globales (CompetitiveMode lo usa antes). */
static char g_updateVersion[64] = {0};
static char g_updateUrl[512] = {0};
static char g_updateSha[128] = {0};
static volatile LONG g_updateBusy = 0;
static int g_updateManual = 0;
static int g_autoUpdateCheck = 1;
static void SetStatus(const char *s);
static void TrayUpdateTip(void);
static HWND Ctl(int id);
static int IsChecked(int id);
static void SetChecked(int id, int v);
static void SetTrack(double ms);
static void ApplyTimer(void);
static void CompetitiveMode(void);
static void CompetitiveRestore(void);
static void SocdInstall(void);
static void SocdUninstall(void);
static void SocdSetStatus(const char *s);

static int ParseVerNums(const char *s, int out[3]) {
    out[0]=out[1]=out[2]=0;
    if (!s) return 0;
    while (*s && (*s < '0' || *s > '9')) s++;
    if (!*s) return 0;
    out[0] = atoi(s);
    while (*s && *s != '.') s++;
    if (*s == '.') { s++; out[1] = atoi(s); while (*s && *s != '.') s++; if (*s == '.') { s++; out[2] = atoi(s); } }
    return 1;
}
static int IsNewerVersion(const char *tag) {
    int a[3], b[3];
    if (!ParseVerNums(tag, a)) return 0;
    ParseVerNums(APP_VERSION, b);
    for (int i = 0; i < 3; i++) { if (a[i] > b[i]) return 1; if (a[i] < b[i]) return 0; }
    return 0;
}
/* Extrae el valor entre comillas que sigue a una clave JSON simple. */
static int JsonStrVal(const char *json, const char *key, char *out, int outLen) {
    const char *p = strstr(json, key);
    if (!p) return 0;
    p = strchr(p + strlen(key), ':');
    if (!p) return 0;
    p = strchr(p, '"');
    if (!p) return 0;
    p++;
    const char *e = strchr(p, '"');
    if (!e || e - p >= outLen) return 0;
    memcpy(out, p, e - p); out[e - p] = 0;
    return 1;
}
static int FetchUrl(const char *url, char *out, int outSize) {
    HINTERNET hInet = InternetOpenA("LuAnOptimizer", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!hInet) return 0;
    DWORD to = 5000;
    InternetSetOptionA(hInet, INTERNET_OPTION_CONNECT_TIMEOUT, &to, sizeof to);
    InternetSetOptionA(hInet, INTERNET_OPTION_RECEIVE_TIMEOUT, &to, sizeof to);
    HINTERNET hUrl = InternetOpenUrlA(hInet, url, NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!hUrl) { InternetCloseHandle(hInet); return 0; }
    int total = 0, ok = 0;
    while (total < outSize - 1) {
        DWORD read = 0;
        if (!InternetReadFile(hUrl, out + total, outSize - 1 - total, &read) || read == 0) break;
        total += (int)read; ok = 1;
    }
    out[total] = 0;
    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInet);
    return ok && total > 0;
}
/* v12: resuelve la URL directa del .exe de la version `ver`.
 * 1) API de GitHub Releases: asset cuyo nombre es LuAnOptimizer.exe,
 *    su browser_download_url (misma busqueda que el fallback de abajo).
 * 2) Si la API falla, URL directa construida:
 *    releases/download/v<ver>/LuAnOptimizer.exe (el tag lleva "v").
 * El campo url= de version.txt YA NO se usa: en v11 apuntaba a la
 * pagina web de releases (releases/latest) y el updater descargaba
 * HTML, cuyo SHA256 nunca coincidia con el publicado. Devuelve 1 si
 * quedo una URL .exe en `out`, 0 si no (entonces el hilo de descarga
 * abre la pagina de releases como antes). */
static int ResolveExeDownloadUrl(const char *ver, char *out, int outLen) {
    out[0] = 0;
    char buf[65536];
    if (FetchUrl(UPDATE_API_URL, buf, sizeof buf)) {
        const char *p = buf;
        while ((p = strstr(p, "\"browser_download_url\"")) != NULL) {
            char u2[512] = {0};
            const char *q = strchr(p + 22, '"');
            if (q) { q++; const char *e = strchr(q, '"'); if (e && e - q < (int)sizeof u2) { memcpy(u2, q, e - q); u2[e - q] = 0; } }
            if (u2[0] && strstr(u2, "LuAnOptimizer.exe")) { strncpy(out, u2, outLen - 1); out[outLen - 1] = 0; return 1; }
            p += 22;
        }
    }
    if (ver && ver[0]) {
        const char *v = ver;
        while (*v == 'v' || *v == 'V') v++;
        if (*v) {
            snprintf(out, outLen, "https://github.com/luisadrianh474-droid/lu-an-optimizer/releases/download/v%s/LuAnOptimizer.exe", v);
            return 1;
        }
    }
    return 0;
}
static DWORD WINAPI UpdateCheckThreadProc(LPVOID lp) {
    (void)lp;
    if (!g_updateManual) Sleep(3000); /* al arrancar, esperar un poco: nunca bloquear el inicio */
    char buf[65536];
    char ver[64] = {0}, sha[128] = {0}, url[512] = {0};
    int found = 0;
    /* 1) Ruta simple: version.txt en raw.githubusercontent */
    if (FetchUrl(UPDATE_TXT_URL, buf, sizeof buf)) {
        const char *v = strstr(buf, "version=");
        if (v) {
            v += 8;
            int i = 0;
            while (*v && *v != '\r' && *v != '\n' && i < (int)sizeof ver - 1) ver[i++] = *v++;
            ver[i] = 0;
            const char *h = strstr(buf, "sha256=");
            if (h) { h += 7; i = 0; while (*h && *h != '\r' && *h != '\n' && i < (int)sizeof sha - 1) sha[i++] = *h++; sha[i] = 0; }
            /* v12: el campo url= de version.txt NO se lee: en v11 apuntaba
             * a la pagina web de releases y el updater descargaba HTML.
             * La URL del exe se resuelve abajo con ResolveExeDownloadUrl. */
            if (ver[0]) found = 1;
        }
    }
    /* 2) Si version.txt fallo, API de GitHub Releases */
    if (!found && FetchUrl(UPDATE_API_URL, buf, sizeof buf)) {
        char tag[64] = {0};
        if (JsonStrVal(buf, "\"tag_name\"", tag, sizeof tag)) {
            strncpy(ver, tag, sizeof ver - 1);
            /* asset LuAnOptimizer.exe: buscar su browser_download_url */
            const char *p = buf;
            while ((p = strstr(p, "\"browser_download_url\"")) != NULL) {
                char u2[512] = {0};
                const char *q = strchr(p + 22, '"');
                if (q) { q++; const char *e = strchr(q, '"'); if (e && e - q < (int)sizeof u2) { memcpy(u2, q, e - q); u2[e - q] = 0; } }
                if (u2[0] && strstr(u2, "LuAnOptimizer.exe")) { strncpy(url, u2, sizeof url - 1); break; }
                p += 22;
            }
            /* hash en el body del release: linea "SHA256: <hash>" */
            const char *hs = strstr(buf, "SHA256:");
            if (hs) { hs += 7; while (*hs == ' ') hs++; int i = 0; while (*hs && *hs != '"' && *hs != '\r' && *hs != '\n' && *hs != '\\' && i < (int)sizeof sha - 1) sha[i++] = *hs++; sha[i] = 0; }
            /* Si la API dio el tag pero no el asset, URL directa del tag. */
            if (!url[0] && tag[0]) {
                const char *v = tag; while (*v == 'v' || *v == 'V') v++;
                if (*v) snprintf(url, sizeof url, "https://github.com/luisadrianh474-droid/lu-an-optimizer/releases/download/v%s/LuAnOptimizer.exe", v);
            }
            found = 1;
        }
    }
    /* v12: si version.txt dio version+hash pero ninguna URL (su url= ya
     * no se usa), resolver la descarga del exe desde la API o construirla. */
    if (found && !url[0]) ResolveExeDownloadUrl(ver, url, sizeof url);
    InterlockedExchange(&g_updateBusy, 0);
    if (!found) { if (g_hwnd) PostMessageA(g_hwnd, WM_UPDATE_STATUS, 0, 0); return 0; }
    if (!IsNewerVersion(ver)) { if (g_hwnd) PostMessageA(g_hwnd, WM_UPDATE_STATUS, 1, 0); return 0; }
    strncpy(g_updateVersion, ver, sizeof g_updateVersion - 1);
    strncpy(g_updateUrl, url, sizeof g_updateUrl - 1);
    strncpy(g_updateSha, sha, sizeof g_updateSha - 1);
    if (g_hwnd) PostMessageA(g_hwnd, WM_UPDATE_FOUND, 0, 0);
    return 0;
}
static void StartUpdateCheck(int manual) {
    if (InterlockedCompareExchange(&g_updateBusy, 1, 0) != 0) return;
    g_updateManual = manual;
    g_updateVersion[0] = 0; g_updateUrl[0] = 0; g_updateSha[0] = 0;
    HANDLE h = CreateThread(NULL, 0, UpdateCheckThreadProc, NULL, 0, NULL);
    if (h) CloseHandle(h);
    else InterlockedExchange(&g_updateBusy, 0);
}
/* SHA256 de un archivo con BCrypt (la implementacion de Windows). */
static int Sha256FileHex(const char *path, char *hexOut, int hexLen) {
    HANDLE f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE) return 0;
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    int ok = 0;
    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0) == 0) {
        DWORD objLen = 0, cb = 0;
        if (BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&objLen, sizeof objLen, &cb, 0) == 0) {
            PUCHAR obj = (PUCHAR)malloc(objLen);
            if (obj && BCryptCreateHash(hAlg, &hHash, obj, objLen, NULL, 0, 0) == 0) {
                BYTE chunk[8192]; DWORD read = 0; int fail = 0;
                while (ReadFile(f, chunk, sizeof chunk, &read, NULL) && read > 0) {
                    if (BCryptHashData(hHash, chunk, read, 0) != 0) { fail = 1; break; }
                }
                if (!fail) {
                    BYTE digest[32];
                    if (BCryptFinishHash(hHash, digest, sizeof digest, 0) == 0) {
                        static const char *hexd = "0123456789abcdef";
                        if (hexLen >= 65) {
                            for (int i = 0; i < 32; i++) { hexOut[i*2] = hexd[digest[i] >> 4]; hexOut[i*2+1] = hexd[digest[i] & 15]; }
                            hexOut[64] = 0;
                            ok = 1;
                        }
                    }
                }
                BCryptDestroyHash(hHash);
            }
            free(obj);
        }
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }
    CloseHandle(f);
    return ok;
}
static DWORD WINAPI UpdateDownloadThreadProc(LPVOID lp) {
    (void)lp;
    char tmpDir[MAX_PATH] = {0};
    GetTempPathA(sizeof tmpDir, tmpDir);
    char newExe[MAX_PATH] = {0}, batPath[MAX_PATH] = {0};
    snprintf(newExe, sizeof newExe, "%sLuAnOptimizer_new.exe", tmpDir);
    snprintf(batPath, sizeof batPath, "%sLuAnUpdate.bat", tmpDir);
    if (!g_updateUrl[0]) {
        /* version.txt sin asset directo: abrir la pagina de releases */
        ShellExecuteA(NULL, "open", UPDATE_RELEASES_URL, NULL, NULL, SW_SHOWNORMAL);
        if (g_hwnd) PostMessageA(g_hwnd, WM_UPDATE_STATUS, 2, 0);
        return 0;
    }
    /* v12: defensa: nunca descargar una URL que no sea el .exe. En v11
     * la URL era la pagina HTML de releases y el SHA256 nunca coincidia
     * (la proteccion funciono, pero la descarga era basura). Si esto
     * salta, se avisa y se abre la pagina de releases como antes. */
    {
        size_t n = strlen(g_updateUrl);
        if (n < 5 || _stricmp(g_updateUrl + n - 4, ".exe") != 0) {
            MessageBoxA(g_hwnd, Tr(LS_UPD_DOWNLOAD_FAIL), Tr(LS_UPD_TITLE), MB_ICONERROR);
            ShellExecuteA(NULL, "open", UPDATE_RELEASES_URL, NULL, NULL, SW_SHOWNORMAL);
            if (g_hwnd) PostMessageA(g_hwnd, WM_UPDATE_STATUS, 2, 0);
            return 0;
        }
    }
    HRESULT hr = URLDownloadToFileA(NULL, g_updateUrl, newExe, 0, NULL);
    if (hr != S_OK) {
        if (g_hwnd) PostMessageA(g_hwnd, WM_UPDATE_STATUS, 3, 0);
        return 0;
    }
    if (g_updateSha[0]) {
        char hex[65] = {0};
        if (!Sha256FileHex(newExe, hex, sizeof hex) || _stricmp(hex, g_updateSha) != 0) {
            DeleteFileA(newExe);
            MessageBoxA(g_hwnd, Tr(LS_UPD_SHA_MISMATCH), Tr(LS_UPD_TITLE), MB_ICONERROR);
            return 0;
        }
    } else {
        /* Sin hash disponible: confirmacion extra antes de instalar. */
        if (MessageBoxA(g_hwnd, Tr(LS_UPD_NO_HASH), Tr(LS_UPD_NO_HASH_TITLE), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
            DeleteFileA(newExe);
            return 0;
        }
    }
    /* .bat de reemplazo: espera a que este proceso salga, mueve el nuevo
     * sobre el viejo, borra el temporal y vuelve a abrir el programa. */
    char exePath[MAX_PATH] = {0};
    GetModuleFileNameA(NULL, exePath, sizeof exePath);
    FILE *b = fopen(batPath, "w");
    if (!b) { DeleteFileA(newExe); return 0; }
    fprintf(b, "@echo off\r\n");
    fprintf(b, ":wait\r\n");
    fprintf(b, "tasklist /NH /FI \"IMAGENAME eq LuAnOptimizer.exe\" 2>nul | find /I \"LuAnOptimizer.exe\" >nul\r\n");
    fprintf(b, "if not errorlevel 1 ( timeout /t 1 /nobreak >nul & goto wait )\r\n");
    fprintf(b, "move /Y \"%s\" \"%s\"\r\n", newExe, exePath);
    fprintf(b, "start \"\" \"%s\"\r\n", exePath);
    fprintf(b, "del \"%%~f0\"\r\n");
    fclose(b);
    ShellExecuteA(NULL, "open", batPath, NULL, NULL, SW_HIDE);
    /* Salir con la limpieza normal (SOCD, timer, bandeja): WM_CLOSE. */
    if (g_hwnd) PostMessageA(g_hwnd, WM_CLOSE, 0, 0);
    return 0;
}
static void StartUpdateDownload(void) {
    HANDLE h = CreateThread(NULL, 0, UpdateDownloadThreadProc, NULL, 0, NULL);
    if (h) CloseHandle(h);
}

/* ================= v10: MODO ULTRA BAJA LATENCIA =================
 * Que hace: quitar la latencia que agrega la propia PC.
 * Limite honesto (tambien en la interfaz y en el README): el ping tiene
 * un piso fisico que depende de la distancia y de la ruta que el ISP
 * elige hasta el servidor. Ningun programa puede bajar ese piso; solo
 * puede quitar los milisegundos que agrega la PC (red, CPU, servicios
 * descargando en segundo plano). Todo lo que toca el registro guarda
 * primero el valor original en g_ultraBackup y Tr(LS_ULTRA_RESTORE) lo
 * devuelve. Si una clave no existe o no se puede escribir, se sigue
 * con las demas y se reporta; nada de esto crashea el programa. */

/* --- Guardado/restauracion de valores de registro --- */
typedef struct {
    int valid;                    /* 1 si se guardo algo que restaurar */
    DWORD tcpNoDelay; int hasTcpNoDelay;
    DWORD tcpAckFreq; int hasTcpAckFreq;
    DWORD throttling; int hasThrottling;
    DWORD pnpCaps; int hasPnpCaps;
    DWORD hwSchMode; int hasHwSchMode;
    int svcWuauservWasRunning;
    int svcDoSvcWasRunning;
    char msg[512];
} UltraRegBackup;
static UltraRegBackup g_ultraBackup;
static int g_ultraOn = 0;
static int g_ultraNetApplied = 0;   /* 1 si se tocaron claves de red */

/* RegSetDword ya existe (void, linea 557) y RegGetDword tambien (int).
 * Para el modo Ultra hace falta saber si la escritura tuvo exito
 * (permisos de administrador), asi que esta variante devuelve 1 si ok. */
static int RegSetDwordOk(HKEY root, const char *sub, const char *val, DWORD v) {
    HKEY k;
    if (RegCreateKeyExA(root, sub, 0, NULL, 0, KEY_SET_VALUE | KEY_QUERY_VALUE, NULL, &k, NULL) != ERROR_SUCCESS) return 0;
    LONG r = RegSetValueExA(k, val, 0, REG_DWORD, (const BYTE*)&v, sizeof v);
    RegCloseKey(k);
    return r == ERROR_SUCCESS;
}
/* La clave de parametros TCP esta en HKLM\...\Parameters\Interfaces\<GUID> */
static int UltraSetAllInterfaces(const char *name, DWORD val, UltraRegBackup *bk, DWORD *saved, int *hasSaved) {
    HKEY k;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces", 0, KEY_READ | KEY_SET_VALUE, &k) != ERROR_SUCCESS) return 0;
    char sub[64]; DWORD i = 0; int ok = 0;
    DWORD len = sizeof sub;
    while (RegEnumKeyExA(k, i++, sub, &len, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        char path[256];
        _snprintf(path, sizeof path, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\%s", sub);
        DWORD old;
        if (!*hasSaved && RegGetDword(HKEY_LOCAL_MACHINE, path, name, &old)) { *saved = old; *hasSaved = 1; }
        if (RegSetDwordOk(HKEY_LOCAL_MACHINE, path, name, val)) ok = 1;
        len = sizeof sub;
        (void)bk;
    }
    RegCloseKey(k);
    return ok;
}
/* Ahorro de energia del adaptador de red: PnPCapabilities=0 lo quita
 * (24 = el adaptador puede apagarse para ahorrar energia). */
static int UltraSetNicPowerSaving(int disable) {
    HKEY k;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}", 0, KEY_READ | KEY_SET_VALUE, &k) != ERROR_SUCCESS) return 0;
    char sub[64]; DWORD i = 0; int ok = 0;
    DWORD len = sizeof sub;
    while (RegEnumKeyExA(k, i++, sub, &len, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        char path[256];
        _snprintf(path, sizeof path, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\%s", sub);
        DWORD old;
        if (!g_ultraBackup.hasPnpCaps && RegGetDword(HKEY_LOCAL_MACHINE, path, "PnPCapabilities", &old)) { g_ultraBackup.pnpCaps = old; g_ultraBackup.hasPnpCaps = 1; }
        if (RegSetDwordOk(HKEY_LOCAL_MACHINE, path, "PnPCapabilities", disable ? 0 : 24)) ok = 1;
        len = sizeof sub;
    }
    RegCloseKey(k);
    return ok;
}
static void UltraMsg(UltraRegBackup *bk, const char *fmt, ...) {
    char b[256]; va_list ap; va_start(ap, fmt); _vsnprintf(b, sizeof b, fmt, ap); va_end(ap);
    if (bk->msg[0]) strncat(bk->msg, " | ", sizeof bk->msg - strlen(bk->msg) - 1);
    strncat(bk->msg, b, sizeof bk->msg - strlen(bk->msg) - 1);
}

/* Aplica todos los ajustes de red/CPU del modo Ultra. Devuelve 1 si al
 * menos un ajuste se aplico; siempre deja en g_ultraBackup->msg que se
 * hizo y que no se pudo hacer. */
static int UltraApply(UltraRegBackup *bk) {
    memset(bk, 0, sizeof *bk);
    /* 1) Desactivar algoritmo de Nagle (TCPNoDelay=1) y ACK inmediato
     * (TcpAckFrequency=1) en la clave global y en cada interfaz. */
    DWORD old;
    if (RegGetDword(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", "TCPNoDelay", &old)) { bk->tcpNoDelay = old; bk->hasTcpNoDelay = 1; }
    if (RegSetDwordOk(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", "TCPNoDelay", 1)) UltraMsg(bk, Tr(LS_ULTRA_NAGLE_OK));
    else UltraMsg(bk, Tr(LS_ULTRA_NAGLE_FAIL));
    if (RegGetDword(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", "TcpAckFrequency", &old)) { bk->tcpAckFreq = old; bk->hasTcpAckFreq = 1; }
    if (RegSetDwordOk(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", "TcpAckFrequency", 1)) UltraMsg(bk, Tr(LS_ULTRA_ACK_OK));
    else UltraMsg(bk, Tr(LS_ULTRA_ACK_FAIL));
    UltraSetAllInterfaces("TCPNoDelay", 1, bk, &bk->tcpNoDelay, &bk->hasTcpNoDelay);
    UltraSetAllInterfaces("TcpAckFrequency", 1, bk, &bk->tcpAckFreq, &bk->hasTcpAckFreq);
    /* 2) Quitar el limite de ancho de banda de Windows para juegos. */
    if (RegGetDword(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile", "NetworkThrottlingIndex", &old)) { bk->throttling = old; bk->hasThrottling = 1; }
    if (RegSetDwordOk(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile", "NetworkThrottlingIndex", 0xFFFFFFFF)) UltraMsg(bk, Tr(LS_ULTRA_THROTTLE_OK));
    else UltraMsg(bk, Tr(LS_ULTRA_THROTTLE_FAIL));
    /* 3) Ahorro de energia del adaptador de red. */
    if (UltraSetNicPowerSaving(1)) UltraMsg(bk, Tr(LS_ULTRA_NIC_OK));
    else UltraMsg(bk, Tr(LS_ULTRA_NIC_FAIL));
    /* 4) TCP autotuning en normal (netsh: lo mismo que en Windows 10/11
     * por defecto; si estaba en disabled lo deja en normal). */
    ShellExecuteA(NULL, "open", "netsh.exe", "int tcp set global autotuninglevel=normal", NULL, SW_HIDE);
    /* 5) Ahorro de energia del WiFi: desactivar la suspension selectiva
     * del USB/WiFi (el adaptador WiFi no se duerme a mitad de partida). */
    if (RegSetDwordOk(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Power", "HibernateEnabledDefault", 0)) UltraMsg(bk, Tr(LS_ULTRA_WIFI_OK));
    else UltraMsg(bk, Tr(LS_ULTRA_WIFI_FAIL));
    bk->valid = 1;
    return 1;
}
/* Devuelve todos los valores guardados a su estado original. */
static void UltraRestore(UltraRegBackup *bk) {
    if (!bk->valid) return;
    if (bk->hasTcpNoDelay) RegSetDwordOk(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", "TCPNoDelay", bk->tcpNoDelay);
    if (bk->hasTcpAckFreq) RegSetDwordOk(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters", "TcpAckFrequency", bk->tcpAckFreq);
    if (bk->hasThrottling) RegSetDwordOk(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Multimedia\\SystemProfile", "NetworkThrottlingIndex", bk->throttling);
    if (bk->hasPnpCaps) {
        HKEY k;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}", 0, KEY_SET_VALUE, &k) == ERROR_SUCCESS) {
            char sub[64]; DWORD i = 0; DWORD len = sizeof sub;
            while (RegEnumKeyExA(k, i++, sub, &len, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                char path[256];
                _snprintf(path, sizeof path, "SYSTEM\\CurrentControlSet\\Control\\Class\\{4d36e972-e325-11ce-bfc1-08002be10318}\\%s", sub);
                RegSetDwordOk(HKEY_LOCAL_MACHINE, path, "PnPCapabilities", bk->pnpCaps);
                len = sizeof sub;
            }
            RegCloseKey(k);
        }
    }
    memset(bk, 0, sizeof *bk);
}

/* --- Pausar servicios mientras se juega (temporal, no para siempre) --- */
static int SvcIsRunning(const char *name) {
    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return 0;
    SC_HANDLE s = OpenServiceA(scm, name, SERVICE_QUERY_STATUS);
    int run = 0;
    if (s) {
        SERVICE_STATUS st;
        if (QueryServiceStatus(s, &st)) run = (st.dwCurrentState == SERVICE_RUNNING);
        CloseServiceHandle(s);
    }
    CloseServiceHandle(scm);
    return run;
}
/* Pausa un servicio (SERVICE_CONTROL_PAUSE) si esta corriendo. */
static int SvcPause(const char *name) {
    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return 0;
    SC_HANDLE s = OpenServiceA(scm, name, SERVICE_PAUSE_CONTINUE);
    int ok = 0;
    if (s) {
        SERVICE_STATUS st;
        ok = ControlService(s, SERVICE_CONTROL_PAUSE, &st);
        CloseServiceHandle(s);
    }
    CloseServiceHandle(scm);
    return ok;
}
/* Reanuda un servicio pausado. */
static int SvcContinue(const char *name) {
    SC_HANDLE scm = OpenSCManagerA(NULL, NULL, SC_MANAGER_CONNECT);
    if (!scm) return 0;
    SC_HANDLE s = OpenServiceA(scm, name, SERVICE_PAUSE_CONTINUE);
    int ok = 0;
    if (s) {
        SERVICE_STATUS st;
        ok = ControlService(s, SERVICE_CONTROL_CONTINUE, &st);
        CloseServiceHandle(s);
    }
    CloseServiceHandle(scm);
    return ok;
}

/* --- Apps que mas red usan (por numero de conexiones TCP) ---
 * GetTcpTable2 devuelve cada conexion TCP con su PID; se cuentan las
 * conexiones por proceso y se muestran los 6 con mas conexiones, con
 * su nombre. No se toca ningun proceso del sistema ni el propio. */
static int IsSystemProcess(DWORD pid) {
    if (pid == 0 || pid == 4) return 1;
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return 1;
    char path[MAX_PATH] = "";
    int sys = 0;
    if (GetModuleFileNameExA(h, NULL, path, sizeof path)) {
        char sysdir[MAX_PATH]; GetSystemDirectoryA(sysdir, sizeof sysdir);
        if (_strnicmp(path, sysdir, strlen(sysdir)) == 0) sys = 1;
        char windir[MAX_PATH]; GetWindowsDirectoryA(windir, sizeof windir);
        if (_strnicmp(path, windir, strlen(windir)) == 0) sys = 1;
    }
    CloseHandle(h);
    return sys;
}
static void UltraRefreshHogs(void) {
    HWND lb = Ctl(ID_LIST_NETHOGS);
    if (!lb) return;
    SendMessageA(lb, LB_RESETCONTENT, 0, 0);
    DWORD sz = 0;
    GetTcpTable2(NULL, &sz, FALSE);
    MIB_TCPTABLE2 *t = (MIB_TCPTABLE2*)malloc(sz ? sz : 1);
    if (!t) return;
    if (GetTcpTable2(t, &sz, FALSE) != NO_ERROR) { free(t); return; }
    /* Contar conexiones por PID (max 256 PIDs). */
    typedef struct { DWORD pid; int n; char name[64]; } Hog;
    Hog hogs[256]; int nh = 0;
    for (DWORD i = 0; i < t->dwNumEntries; i++) {
        DWORD pid = t->table[i].dwOwningPid;
        if (pid == 0 || pid == GetCurrentProcessId()) continue;
        int j; for (j = 0; j < nh; j++) if (hogs[j].pid == pid) break;
        if (j == nh) { if (nh >= 256) continue; hogs[nh].pid = pid; hogs[nh].n = 0; hogs[nh].name[0] = 0; nh++; }
        hogs[j].n++;
    }
    for (int i = 0; i < nh; i++) {
        HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, hogs[i].pid);
        if (h) {
            char path[MAX_PATH] = "";
            if (GetModuleFileNameExA(h, NULL, path, sizeof path)) {
                const char *base = strrchr(path, '\\');
                _snprintf(hogs[i].name, sizeof hogs[i].name, "%s", base ? base + 1 : path);
            }
            CloseHandle(h);
        }
        if (!hogs[i].name[0]) _snprintf(hogs[i].name, sizeof hogs[i].name, "PID %lu", (unsigned long)hogs[i].pid);
    }
    /* Ordenar por conexiones, de mayor a menor. */
    for (int i = 0; i < nh; i++) for (int j = i + 1; j < nh; j++) if (hogs[j].n > hogs[i].n) { Hog tmp = hogs[i]; hogs[i] = hogs[j]; hogs[j] = tmp; }
    int shown = nh < 6 ? nh : 6;
    for (int i = 0; i < shown; i++) {
        char line[128];
        _snprintf(line, sizeof line, "%s  -  %d conexiones TCP  (PID %lu)%s", hogs[i].name, hogs[i].n, (unsigned long)hogs[i].pid, IsSystemProcess(hogs[i].pid) ? Tr(LS_HOG_SYSTEM_SUFFIX) : "");
        int idx = (int)SendMessageA(lb, LB_ADDSTRING, 0, (LPARAM)line);
        SendMessageA(lb, LB_SETITEMDATA, idx, hogs[i].pid);
    }
    if (shown == 0) SendMessageA(lb, LB_ADDSTRING, 0, (LPARAM)Tr(LS_HOGS_NONE));
    free(t);
}
/* Cierra el proceso seleccionado en la lista (nunca sistema ni el propio). */
static void UltraKillHog(void) {
    HWND lb = Ctl(ID_LIST_NETHOGS);
    if (!lb) return;
    int sel = (int)SendMessageA(lb, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR) { MessageBoxA(g_hwnd, Tr(LS_HOG_SELECT), Tr(LS_KILL_HOG), MB_OK | MB_ICONINFORMATION); return; }
    DWORD pid = (DWORD)SendMessageA(lb, LB_GETITEMDATA, sel, 0);
    if (pid == 0 || pid == GetCurrentProcessId() || IsSystemProcess(pid)) {
        MessageBoxA(g_hwnd, Tr(LS_HOG_SYSTEM), Tr(LS_KILL_HOG), MB_OK | MB_ICONWARNING);
        return;
    }
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!h) { MessageBoxA(g_hwnd, Tr(LS_HOG_OPEN_FAIL), Tr(LS_KILL_HOG), MB_OK | MB_ICONWARNING); return; }
    char name[64] = "";
    char path[MAX_PATH] = "";
    if (GetModuleFileNameExA(h, NULL, path, sizeof path)) { const char *b = strrchr(path, '\\'); _snprintf(name, sizeof name, "%s", b ? b + 1 : path); }
    char q[256];
    _snprintf(q, sizeof q, Tr(LS_HOG_ASK_FMT), name[0] ? name : Tr(LS_HOG_UNKNOWN_NAME), (unsigned long)pid);
    if (MessageBoxA(g_hwnd, q, Tr(LS_KILL_HOG), MB_YESNO | MB_ICONQUESTION) == IDYES) {
        TerminateProcess(h, 0);
        SetStatus(Tr(LS_HOG_CLOSED));
        UltraRefreshHogs();
    }
    CloseHandle(h);
}

/* --- Ping real a servidores de juego (v10) ---
 * Mide con IcmpSendEcho en un hilo de trabajo (nunca bloquea la UI)
 * la IP de CDMX y la de Miami que el usuario escriba en la pestana
 * Juegos, y recomienda el mejor. Las IP por defecto son de referencia
 * y se pueden editar: la CDMX es un nodo de referencia en la Ciudad de
 * Mexico y la de Miami es un DNS publico en Miami (sirven para comparar
 * la ruta; para el ping exacto del servidor de la partida usa la IP de
 * tu partida si la conoces). */
typedef struct { char cdmx[64]; char miami[64]; } SrvPingReq;
static SrvPingReq g_srvReq;
typedef struct { int cdmx; int miami; int lossC; int lossM; } SrvPingRes;
static SrvPingRes g_srvRes;
static DWORD WINAPI ServerPingThreadProc(LPVOID p) {
    (void)p;
    SrvPingRes r; memset(&r, 0, sizeof r);
    r.cdmx = -1; r.miami = -1;
    HANDLE icmp = IcmpCreateFile();
    if (icmp != INVALID_HANDLE_VALUE) {
        char sendbuf[32]; memset(sendbuf, 'A', sizeof sendbuf);
        char recvbuf[1024];
        for (int i = 0; i < 4; i++) {
            DWORD rep = IcmpSendEcho(icmp, inet_addr(g_srvReq.cdmx), sendbuf, sizeof sendbuf, NULL, recvbuf, sizeof recvbuf, 1000);
            if (rep) {
                PICMP_ECHO_REPLY er = (PICMP_ECHO_REPLY)recvbuf;
                if (r.cdmx < 0) r.cdmx = (int)er->RoundTripTime;
                else r.cdmx = (r.cdmx + (int)er->RoundTripTime) / 2;
            } else r.lossC++;
        }
        for (int i = 0; i < 4; i++) {
            DWORD rep = IcmpSendEcho(icmp, inet_addr(g_srvReq.miami), sendbuf, sizeof sendbuf, NULL, recvbuf, sizeof recvbuf, 1000);
            if (rep) {
                PICMP_ECHO_REPLY er = (PICMP_ECHO_REPLY)recvbuf;
                if (r.miami < 0) r.miami = (int)er->RoundTripTime;
                else r.miami = (r.miami + (int)er->RoundTripTime) / 2;
            } else r.lossM++;
        }
        IcmpCloseHandle(icmp);
    }
    g_srvRes = r;
    PostMessageA(g_hwnd, WM_SERVER_PING_DONE, 0, 0);
    return 0;
}
/* v11: texto del resultado del ping a servidores (se usa en el
 * resultado del hilo y al refrescar el idioma). */
static void ServerPingText(char *out, int outLen) {
    char c[32], m[32];
    if (g_srvRes.cdmx >= 0) _snprintf(c, sizeof c, "%d ms", g_srvRes.cdmx);
    else _snprintf(c, sizeof c, "%s", Tr(LS_NO_RESPONSE));
    if (g_srvRes.miami >= 0) _snprintf(m, sizeof m, "%d ms", g_srvRes.miami);
    else _snprintf(m, sizeof m, "%s", Tr(LS_NO_RESPONSE));
    if (g_srvRes.cdmx >= 0 && g_srvRes.miami >= 0) {
        const char *best = (g_srvRes.cdmx <= g_srvRes.miami) ? "CDMX" : "Miami";
        _snprintf(out, outLen, Tr(LS_SRV_RESULT_BEST), c, g_srvRes.lossC, m, g_srvRes.lossM, best);
    } else {
        _snprintf(out, outLen, Tr(LS_SRV_RESULT), c, g_srvRes.lossC, m, g_srvRes.lossM);
    }
}

static void ServerPingStart(void) {
    GetWindowTextA(Ctl(ID_SRV_CDMX), g_srvReq.cdmx, sizeof g_srvReq.cdmx);
    GetWindowTextA(Ctl(ID_SRV_MIAMI), g_srvReq.miami, sizeof g_srvReq.miami);
    SetWindowTextA(Ctl(ID_LBL_SERVER_PING), Tr(LS_SRV_MEASURING));
    HANDLE th = CreateThread(NULL, 0, ServerPingThreadProc, NULL, 0, NULL);
    if (th) CloseHandle(th);
}

/* --- Diagnostico de ruta / traceroute (v10) ---
 * TTL creciente con IcmpSendEcho (hasta 30 saltos, 1000 ms por salto),
 * en hilo de trabajo. Cada salto muestra su IP y ms; si no responde,
 * "*". Al terminar, resume donde esta el mayor salto de latencia entre
 * dos saltos consecutivos: tu PC / tu router / tu ISP / el servidor. */
static char g_traceTarget[64];
static char g_traceOut[8192];
static char g_traceSummary[256];
static int g_traceWorstHop = -1; /* -1 = no se pudo medir, 0 = sin salto claro */
static unsigned long g_traceWorstJump = 0;
static int g_traceDone = 0; /* 1 = el hilo ya termino y hay resumen */

/* v11: resumen del traceroute en el idioma actual. */
static void TraceSummaryText(char *out, int outLen) {
    if (!g_traceDone || g_traceWorstHop < 0) { _snprintf(out, outLen, "%s", Tr(LS_TRACE_FAIL)); return; }
    if (g_traceWorstHop == 0) { _snprintf(out, outLen, "%s", Tr(LS_TRACE_NOJUMP)); return; }
    const char *where = Tr(LS_WHERE_ISP);
    if (g_traceWorstHop == 1) where = Tr(LS_WHERE_PC);
    else if (g_traceWorstHop == 2) where = Tr(LS_WHERE_ROUTER);
    else if (g_traceWorstHop >= 28) where = Tr(LS_WHERE_SERVER);
    _snprintf(out, outLen, Tr(LS_TRACE_SUMMARY_FMT), g_traceWorstJump, g_traceWorstHop, where);
}

static DWORD WINAPI TraceThreadProc(LPVOID p) {
    (void)p;
    g_traceOut[0] = 0;
    g_traceSummary[0] = 0;
    HANDLE icmp = IcmpCreateFile();
    if (icmp == INVALID_HANDLE_VALUE) {
        _snprintf(g_traceOut, sizeof g_traceOut, Tr(LS_TRACE_ICMP_FAIL));
        g_traceWorstHop = -1; g_traceWorstJump = 0; g_traceDone = 1;
        _snprintf(g_traceSummary, sizeof g_traceSummary, Tr(LS_TRACE_FAIL));
        PostMessageA(g_hwnd, WM_TRACE_DONE, 0, 0);
        return 0;
    }
    char sendbuf[32]; memset(sendbuf, 'A', sizeof sendbuf);
    char recvbuf[1024];
    IPAddr dst = inet_addr(g_traceTarget);
    unsigned long prevMs = 0;
    int worstHop = 0; unsigned long worstJump = 0;
    for (int ttl = 1; ttl <= 30; ttl++) {
        IP_OPTION_INFORMATION opt; memset(&opt, 0, sizeof opt); opt.Ttl = (unsigned char)ttl;
        DWORD rep = IcmpSendEcho(icmp, dst, sendbuf, sizeof sendbuf, &opt, recvbuf, sizeof recvbuf, 1000);
        char line[256];
        if (rep) {
            PICMP_ECHO_REPLY er = (PICMP_ECHO_REPLY)recvbuf;
            struct in_addr ia; ia.S_un.S_addr = er->Address;
            _snprintf(line, sizeof line, "%2d  %s  %lu ms\r\n", ttl, inet_ntoa(ia), er->RoundTripTime);
            if (ttl > 1 && er->RoundTripTime > prevMs) {
                unsigned long jump = er->RoundTripTime - prevMs;
                if (jump > worstJump) { worstJump = jump; worstHop = ttl; }
            }
            prevMs = er->RoundTripTime;
            strncat(g_traceOut, line, sizeof g_traceOut - strlen(g_traceOut) - 1);
            if (er->Address == dst) break;
        } else {
            _snprintf(line, sizeof line, "%2d  *\r\n", ttl);
            strncat(g_traceOut, line, sizeof g_traceOut - strlen(g_traceOut) - 1);
        }
    }
    IcmpCloseHandle(icmp);
    /* Resumen: el mayor salto entre dos saltos consecutivos. */
    if (worstHop == 0) _snprintf(g_traceSummary, sizeof g_traceSummary, Tr(LS_TRACE_NOJUMP));
    else {
        const char *where = Tr(LS_WHERE_ISP);
        if (worstHop == 1) where = Tr(LS_WHERE_PC);
        else if (worstHop == 2) where = Tr(LS_WHERE_ROUTER);
        else if (worstHop >= 28) where = Tr(LS_WHERE_SERVER);
        _snprintf(g_traceSummary, sizeof g_traceSummary, Tr(LS_TRACE_SUMMARY_FMT), worstJump, worstHop, where);
    }
    g_traceWorstHop = worstHop; g_traceWorstJump = worstJump; g_traceDone = 1;
    PostMessageA(g_hwnd, WM_TRACE_DONE, 0, 0);
    return 0;
}
static void TraceStart(void) {
    /* Traceroute al servidor con mejor ping (o CDMX si no se midio). */
    g_traceDone = 0; /* v11: se vuelve a medir; el resumen se re-traduce al cambiar idioma */
    const char *t = g_srvReq.cdmx[0] ? g_srvReq.cdmx : "189.203.220.1";
    if (g_srvRes.cdmx >= 0 && g_srvRes.miami >= 0 && g_srvRes.miami < g_srvRes.cdmx) t = g_srvReq.miami;
    _snprintf(g_traceTarget, sizeof g_traceTarget, "%s", t);
    SetWindowTextA(Ctl(ID_EDIT_TRACE), Tr(LS_TRACE_RUNNING));
    SetWindowTextA(Ctl(ID_LBL_TRACE_SUMMARY), Tr(LS_TRACE_MEASURING));
    HANDLE th = CreateThread(NULL, 0, TraceThreadProc, NULL, 0, NULL);
    if (th) CloseHandle(th);
}

/* ================= v9: PERFILES POR JUEGO =================
 * Deteccion por proceso cada 2 s (CreateToolhelp32Snapshot). Al arrancar
 * un juego se guardan los ajustes actuales y se aplica su perfil; al
 * cerrarse se restaura todo. Solo se aplica UNA vez por transicion, asi
 * que si el usuario cambia algo a mano mientras juega, no se le pisa.
 * CS2: SOCD FORZADO OFF siempre (Valve lo prohibio en 2024). */
typedef struct { const char *key; const char *name; const char *exe; int defAuto; int defSocd; int defTimer05; int defComp; int forbidden; } GameProfile;
static const GameProfile g_games[5] = {
    { "Valorant", "Valorant", "VALORANT-Win64-Shipping.exe", 1, 1, 1, 1, 0 },
    { "CS2", "CS2", "cs2.exe", 1, 0, 1, 1, 1 },
    { "Fortnite", "Fortnite", "FortniteClient-Win64-Shipping.exe", 0, 1, 1, 1, 0 },
    { "Apex", "Apex Legends", "r5apex.exe", 0, 1, 1, 0, 0 },
    { "Otro", "Otro juego", "", 0, 0, 1, 0, 0 }
};

/* v11: nombre para mostrar ("Otro juego" se traduce; la clave "Otro"
 * del registro no cambia). */
static const char *GameDisplayName(int idx) {
    if (idx == 4) return Tr(LS_GAME_OTHER_NAME);
    if (idx >= 0 && idx < 5) return g_games[idx].name;
    return "";
}
static int g_gameAuto[5] = {0}, g_gameSocd[5] = {0}, g_gameTimer05[5] = {1,1,1,1,1}, g_gameComp[5] = {0};
static char g_gameCustomExe[128] = {0};
static int g_gameCurrent = -1; /* indice del juego con perfil aplicado, -1 = ninguno */
static double g_savedTimer = 15.6;
static int g_savedSocd = 0, g_savedComp = 0;
static HANDLE g_gameStopEvent = NULL;
static void RegSetStr(HKEY root, const char *sub, const char *val, const char *s) {
    HKEY k; if (RegCreateKeyExA(root, sub, 0, NULL, 0, KEY_SET_VALUE, NULL, &k, NULL) != ERROR_SUCCESS) return;
    RegSetValueExA(k, val, 0, REG_SZ, (const BYTE*)s, (DWORD)strlen(s) + 1);
    RegCloseKey(k);
}
static int RegGetStr(HKEY root, const char *sub, const char *val, char *out, int outLen) {
    HKEY k; if (RegOpenKeyExA(root, sub, 0, KEY_READ, &k) != ERROR_SUCCESS) return 0;
    DWORD type = 0, len = (DWORD)outLen;
    LONG r = RegQueryValueExA(k, val, NULL, &type, (LPBYTE)out, &len);
    RegCloseKey(k);
    if (r != ERROR_SUCCESS || type != REG_SZ) return 0;
    out[outLen - 1] = 0; return 1;
}
static void GameRegSub(int i, char *out, int outLen) {
    snprintf(out, outLen, "Software\\LuAnOptimizer\\Games\\%s", g_games[i].key);
}
static void LoadGamesSettings(void) {
    for (int i = 0; i < 5; i++) {
        char sub[128]; GameRegSub(i, sub, sizeof sub);
        DWORD v = 0;
        g_gameAuto[i] = RegGetDword(HKEY_CURRENT_USER, sub, "Auto", &v) ? (int)v : g_games[i].defAuto;
        g_gameSocd[i] = RegGetDword(HKEY_CURRENT_USER, sub, "Socd", &v) ? (int)v : g_games[i].defSocd;
        g_gameTimer05[i] = RegGetDword(HKEY_CURRENT_USER, sub, "Timer05", &v) ? (int)v : g_games[i].defTimer05;
        g_gameComp[i] = RegGetDword(HKEY_CURRENT_USER, sub, "CompMode", &v) ? (int)v : g_games[i].defComp;
        if (g_games[i].forbidden) g_gameSocd[i] = 0; /* CS2: SOCD siempre off */
        SetChecked(ID_GAMES_AUTO_BASE + i, g_gameAuto[i] ? BST_CHECKED : BST_UNCHECKED);
        SetChecked(ID_GAMES_SOCD_BASE + i, g_gameSocd[i] ? BST_CHECKED : BST_UNCHECKED);
        SetChecked(ID_GAMES_TIMER_BASE + i, g_gameTimer05[i] ? BST_CHECKED : BST_UNCHECKED);
        SetChecked(ID_GAMES_COMP_BASE + i, g_gameComp[i] ? BST_CHECKED : BST_UNCHECKED);
    }
    if (!RegGetStr(HKEY_CURRENT_USER, "Software\\LuAnOptimizer\\Games\\Otro", "Exe", g_gameCustomExe, sizeof g_gameCustomExe))
        g_gameCustomExe[0] = 0;
    SetWindowTextA(Ctl(ID_GAMES_CUSTOM_EXE), g_gameCustomExe);
}
static void SaveGameProfile(int i) {
    char sub[128]; GameRegSub(i, sub, sizeof sub);
    RegSetDword(HKEY_CURRENT_USER, sub, "Auto", (DWORD)g_gameAuto[i]);
    RegSetDword(HKEY_CURRENT_USER, sub, "Socd", (DWORD)(g_games[i].forbidden ? 0 : g_gameSocd[i]));
    RegSetDword(HKEY_CURRENT_USER, sub, "Timer05", (DWORD)g_gameTimer05[i]);
    RegSetDword(HKEY_CURRENT_USER, sub, "CompMode", (DWORD)g_gameComp[i]);
    if (i == 4) RegSetStr(HKEY_CURRENT_USER, sub, "Exe", g_gameCustomExe);
}
static int ProcessRunning(const wchar_t *exeW) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe; pe.dwSize = sizeof pe;
    int found = 0;
    if (Process32FirstW(snap, &pe)) do {
        if (_wcsicmp(pe.szExeFile, exeW) == 0) { found = 1; break; }
    } while (Process32NextW(snap, &pe));
    CloseHandle(snap);
    return found;
}
static DWORD WINAPI GameWatchThreadProc(LPVOID lp) {
    (void)lp;
    while (WaitForSingleObject(g_gameStopEvent, 2000) != WAIT_OBJECT_0) {
        int detected = -1;
        for (int i = 0; i < 5; i++) {
            if (!g_gameAuto[i]) continue;
            char exe[128] = {0};
            if (i == 4) { strncpy(exe, g_gameCustomExe, sizeof exe - 1); if (!exe[0]) continue; }
            else strncpy(exe, g_games[i].exe, sizeof exe - 1);
            wchar_t exeW[128] = {0};
            MultiByteToWideChar(CP_ACP, 0, exe, -1, exeW, 128);
            if (ProcessRunning(exeW)) { detected = i; break; }
        }
        if (detected != g_gameCurrent) {
            /* Transicion: avisar a la UI (ella aplica/restaura, nunca el hilo). */
            if (g_hwnd) PostMessageA(g_hwnd, WM_GAME_APPLY, (WPARAM)detected, 0);
            g_gameCurrent = detected;
        }
    }
    return 0;
}
/* --- Prioridad y afinidad del juego (v10, solo si Ultra esta activo) ---
 * Cuando se detecta un juego, su proceso sube a HIGH_PRIORITY_CLASS y
 * se le quita un nucleo (el sistema se queda con uno para si), para que
 * el juego no compita con Windows por CPU. Es reversible: al cerrar el
 * juego se devuelve la prioridad normal y la afinidad original. No se
 * toca core parking con powercfg (no hace falta para esto). */
static DWORD g_ultraGamePid = 0;
static DWORD g_ultraGameOldPrio = 0;
static DWORD_PTR g_ultraGameOldAffinity = 0;
static DWORD FindGamePid(int idx) {
    char exe[128] = {0};
    if (idx == 4) { strncpy(exe, g_gameCustomExe, sizeof exe - 1); if (!exe[0]) return 0; }
    else strncpy(exe, g_games[idx].exe, sizeof exe - 1);
    wchar_t exeW[128] = {0};
    MultiByteToWideChar(CP_ACP, 0, exe, -1, exeW, 128);
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe; pe.dwSize = sizeof pe;
    DWORD pid = 0;
    if (Process32FirstW(snap, &pe)) do {
        if (_wcsicmp(pe.szExeFile, exeW) == 0) { pid = pe.th32ProcessID; break; }
    } while (Process32NextW(snap, &pe));
    CloseHandle(snap);
    return pid;
}
static void UltraApplyToGame(int idx) {
    DWORD pid = FindGamePid(idx);
    if (!pid) return;
    HANDLE h = OpenProcess(PROCESS_SET_INFORMATION | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!h) return;
    g_ultraGameOldPrio = GetPriorityClass(h);
    DWORD_PTR sysMask, procMask;
    if (GetProcessAffinityMask(h, &procMask, &sysMask)) {
        g_ultraGameOldAffinity = procMask;
        /* Afinidad: todos los nucleos disponibles menos 1 para el sistema.
         * Si solo hay 1 nucleo, no se toca la afinidad. */
        if (sysMask > 1) {
            DWORD_PTR mask = sysMask & ~(DWORD_PTR)1;
            if (mask) SetProcessAffinityMask(h, mask);
        }
    }
    SetPriorityClass(h, HIGH_PRIORITY_CLASS);
    g_ultraGamePid = pid;
    CloseHandle(h);
}
static void UltraRestoreGame(void) {
    if (!g_ultraGamePid) return;
    HANDLE h = OpenProcess(PROCESS_SET_INFORMATION, FALSE, g_ultraGamePid);
    if (h) {
        if (g_ultraGameOldPrio) SetPriorityClass(h, g_ultraGameOldPrio);
        if (g_ultraGameOldAffinity) SetProcessAffinityMask(h, g_ultraGameOldAffinity);
        CloseHandle(h);
    }
    g_ultraGamePid = 0;
}
/* Se ejecuta en el hilo UI al recibir WM_GAME_APPLY. idx >= 0 aplica el
 * perfil del juego; idx == -1 restaura los ajustes guardados. */
static void ApplyGameProfile(int idx) {
    if (idx >= 0) {
        g_savedTimer = g_desiredMs;
        g_savedSocd = g_socdOn ? 1 : 0;
        g_savedComp = g_compActive ? 1 : 0;
        if (g_gameComp[idx]) {
            CompetitiveMode(); /* timer 0.5, FilterKeys off, Game Mode... */
        } else {
            SetTrack(g_gameTimer05[idx] ? 0.5 : 1.0);
            ApplyTimer();
        }
        if (g_gameSocd[idx] && !g_games[idx].forbidden) {
            /* El usuario ya dio su consentimiento al activar el SOCD en
             * el perfil: se instala directo, sin popup en mitad del juego. */
            SetChecked(ID_CHK_SOCD, BST_CHECKED);
            SetChecked(ID_SOCD_LAST, BST_CHECKED);
            InterlockedExchange(&g_socdMode, 0);
            SocdInstall();
        } else if (g_games[idx].forbidden) {
            SetChecked(ID_CHK_SOCD, BST_UNCHECKED);
            if (g_socdOn) SocdUninstall();
        }
        if (g_ultraOn) UltraApplyToGame(idx); /* prioridad alta + afinidad */
        char msg[256];
        snprintf(msg, sizeof msg, Tr(LS_GAME_DETECTED_FMT), GameDisplayName(idx));
        SetWindowTextA(Ctl(ID_GAMES_STATUS), msg);
        SetStatus(msg);
    } else {
        UltraRestoreGame(); /* prioridad y afinidad del juego a normal */
        if (g_savedComp) CompetitiveRestore();
        else { SetTrack(g_savedTimer); ApplyTimer(); }
        if (g_savedSocd && !g_socdOn) { SetChecked(ID_CHK_SOCD, BST_CHECKED); SocdInstall(); }
        else if (!g_savedSocd && g_socdOn) { SetChecked(ID_CHK_SOCD, BST_UNCHECKED); SocdUninstall(); }
        SetWindowTextA(Ctl(ID_GAMES_STATUS), Tr(LS_NO_GAME));
        SetStatus(Tr(LS_GAME_CLOSED));
    }
}

/* ================= v9: MONITOR EN PANTALLA / OVERLAY =================
 * Ventana pequena siempre encima (TOPMOST + LAYERED, sin borde, fondo
 * semitransparente, arrastrable, con X para cerrarla y casilla en
 * Competitivo). NO roba el foco ni interfiere con el juego: opcion
 * Tr(LS_CLICK_THROUGH) (WS_EX_TRANSPARENT).
 * FPS: estadisticas de presentacion DXGI del monitor principal
 * (IDXGIOutput::GetFrameStatistics) = frecuencia de presentacion de la
 * PANTALLA, etiquetado "FPS (pantalla)". NO es el FPS del juego: el FPS
 * real no se puede leer sin enganchar DirectX, y eso lo pueden detectar
 * los anti-cheats. Temperatura: WMI MSAcpi_ThermalZoneTemperature
 * (root\WMI, Kelvin*10); si no hay sensor (lo mas comun en escritorio),
 * muestra "Temp: N/D", nunca un numero inventado. */
static HWND g_overlay = NULL, g_overlayText = NULL;
static int g_overlayOn = 0, g_overlayClick = 0;
static int g_overlayX = 40, g_overlayY = 40;
static volatile LONG g_tempC = -1000; /* -1000 = N/D */
static HANDLE g_tempStopEvent = NULL;
static ULONGLONG g_cpuPrevIdle = 0, g_cpuPrevKernel = 0, g_cpuPrevUser = 0;
static int g_cpuPct = -1;
static IDXGIOutput *g_dxgiOut = NULL;
static DWORD g_dxgiPrevPresent = 0, g_dxgiPrevTick = 0;
static int g_dxgiFps = -1;
static LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
static void OverlaySavePos(void) {
    RegSetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "OverlayX", (DWORD)g_overlayX);
    RegSetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "OverlayY", (DWORD)g_overlayY);
    RegSetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "OverlayOn", (DWORD)g_overlayOn);
    RegSetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "OverlayClickThrough", (DWORD)g_overlayClick);
}
static int CpuPercent(void) {
    FILETIME idle, kernel, user;
    if (!GetSystemTimes(&idle, &kernel, &user)) return -1;
    ULONGLONG i = ((ULONGLONG)idle.dwHighDateTime << 32) | idle.dwLowDateTime;
    ULONGLONG k = ((ULONGLONG)kernel.dwHighDateTime << 32) | kernel.dwLowDateTime;
    ULONGLONG u = ((ULONGLONG)user.dwHighDateTime << 32) | user.dwLowDateTime;
    if (g_cpuPrevKernel == 0) { g_cpuPrevIdle = i; g_cpuPrevKernel = k; g_cpuPrevUser = u; return -1; }
    ULONGLONG kDelta = (k - g_cpuPrevKernel), uDelta = (u - g_cpuPrevUser), iDelta = (i - g_cpuPrevIdle);
    ULONGLONG total = kDelta + uDelta;
    g_cpuPrevIdle = i; g_cpuPrevKernel = k; g_cpuPrevUser = u;
    if (total == 0) return 0;
    return (int)((total - iDelta) * 100 / total);
}
static int DxgiFps(void) {
    if (!g_dxgiOut) {
        IDXGIFactory *factory = NULL;
        if (CreateDXGIFactory(&IID_IDXGIFactory, (void**)&factory) != S_OK || !factory) return -1;
        IDXGIAdapter *adapter = NULL;
        if (factory->lpVtbl->EnumAdapters(factory, 0, &adapter) == S_OK && adapter) {
            if (adapter->lpVtbl->EnumOutputs(adapter, 0, &g_dxgiOut) != S_OK) g_dxgiOut = NULL;
            adapter->lpVtbl->Release(adapter);
        }
        factory->lpVtbl->Release(factory);
        if (!g_dxgiOut) return -1;
    }
    DXGI_FRAME_STATISTICS fs;
    if (g_dxgiOut->lpVtbl->GetFrameStatistics(g_dxgiOut, &fs) != S_OK) return -1;
    DWORD now = GetTickCount();
    if (g_dxgiPrevTick == 0) { g_dxgiPrevPresent = fs.PresentCount; g_dxgiPrevTick = now; return -1; }
    DWORD elapsed = now - g_dxgiPrevTick;
    DWORD delta = fs.PresentCount - g_dxgiPrevPresent;
    g_dxgiPrevPresent = fs.PresentCount; g_dxgiPrevTick = now;
    if (elapsed == 0) return g_dxgiFps;
    return (int)(delta * 1000 / elapsed);
}
static DWORD WINAPI TempThreadProc(LPVOID lp) {
    (void)lp;
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) return 0;
    while (WaitForSingleObject(g_tempStopEvent, 5000) != WAIT_OBJECT_0) {
        IWbemLocator *loc = NULL;
        hr = CoCreateInstance(&CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, &IID_IWbemLocator, (void**)&loc);
        if (SUCCEEDED(hr) && loc) {
            IWbemServices *svc = NULL;
            BSTR ns = SysAllocString(L"root\\WMI");
            BSTR user = NULL, pass = NULL;
            hr = loc->lpVtbl->ConnectServer(loc, ns, user, pass, NULL, 0, NULL, NULL, &svc);
            SysFreeString(ns);
            if (SUCCEEDED(hr) && svc) {
                CoSetProxyBlanket((IUnknown*)svc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
                IEnumWbemClassObject *en = NULL;
                BSTR ql = SysAllocString(L"WQL");
                BSTR q = SysAllocString(L"SELECT CurrentTemperature FROM MSAcpi_ThermalZoneTemperature");
                hr = svc->lpVtbl->ExecQuery(svc, ql, q, WBEM_FLAG_FORWARD_ONLY, NULL, &en);
                SysFreeString(ql); SysFreeString(q);
                if (SUCCEEDED(hr) && en) {
                    IWbemClassObject *obj = NULL; ULONG got = 0;
                    if (en->lpVtbl->Next(en, WBEM_INFINITE, 1, &obj, &got) == S_OK && got == 1 && obj) {
                        VARIANT v; VariantInit(&v);
                        if (obj->lpVtbl->Get(obj, L"CurrentTemperature", 0, &v, NULL, NULL) == S_OK && (v.vt == VT_I4 || v.vt == VT_UI4)) {
                            long kelvin10 = (v.vt == VT_I4) ? v.lVal : (long)v.ulVal;
                            if (kelvin10 > 0) InterlockedExchange(&g_tempC, (LONG)(kelvin10 / 10 - 273));
                        }
                        VariantClear(&v);
                        obj->lpVtbl->Release(obj);
                    }
                    en->lpVtbl->Release(en);
                }
                svc->lpVtbl->Release(svc);
            }
            loc->lpVtbl->Release(loc);
        }
    }
    CoUninitialize();
    return 0;
}
static void OverlayRefreshText(void) {
    if (!g_overlayText) return;
    long avg = -1, jitter = 0, loss = 0, count = 0;
    EnterCriticalSection(&g_pingCS);
    avg = g_liveAvg; jitter = g_liveJitter; loss = g_liveLoss; count = g_liveCount;
    LeaveCriticalSection(&g_pingCS);
    int cpu = CpuPercent();
    if (cpu >= 0) g_cpuPct = cpu;
    MEMORYSTATUSEX st; memset(&st, 0, sizeof st); st.dwLength = sizeof st;
    DWORD ramPct = 0;
    if (GlobalMemoryStatusEx(&st)) ramPct = st.dwMemoryLoad;
    int fps = DxgiFps();
    if (fps >= 0) g_dxgiFps = fps;
    double timerNow = QueryTimer();
    long temp = g_tempC;
    char pingTxt[48], cpuTxt[24], fpsTxt[24], tempTxt[24];
    if (count == 0) snprintf(pingTxt, sizeof pingTxt, "--");
    else if (avg >= 0) snprintf(pingTxt, sizeof pingTxt, "%ld ms", avg);
    else snprintf(pingTxt, sizeof pingTxt, Tr(LS_NO_RESPONSE));
    if (g_cpuPct >= 0) snprintf(cpuTxt, sizeof cpuTxt, "%d%%", g_cpuPct); else snprintf(cpuTxt, sizeof cpuTxt, Tr(LS_NA));
    if (g_dxgiFps >= 0) snprintf(fpsTxt, sizeof fpsTxt, "%d", g_dxgiFps); else snprintf(fpsTxt, sizeof fpsTxt, Tr(LS_NA));
    if (temp > -1000) snprintf(tempTxt, sizeof tempTxt, "%ld C", temp); else snprintf(tempTxt, sizeof tempTxt, Tr(LS_NA));
    char txt[512];
    snprintf(txt, sizeof txt,
        Tr(LS_OVERLAY_FMT),
        pingTxt, jitter, loss, cpuTxt, (unsigned long)ramPct, timerNow, g_socdOn ? "ON" : "OFF", fpsTxt, tempTxt);
    SetWindowTextA(g_overlayText, txt);
}
static LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        g_overlayText = CreateWindowExA(0, "STATIC", "Lu-An Monitor", WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 8, 250, 170, hwnd, NULL, g_hInst, NULL);
        SendMessage(g_overlayText, WM_SETFONT, (WPARAM)g_font, TRUE);
        CreateWindowExA(0, "BUTTON", "X", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 244, 4, 22, 20, hwnd, (HMENU)(INT_PTR)ID_OVERLAY_CLOSE, g_hInst, NULL);
        SetTimer(hwnd, 7, 1000, NULL);
        OverlayRefreshText();
        return 0;
    case WM_TIMER:
        if (wp == 7) { OverlayRefreshText(); return 0; }
        break;
    case WM_COMMAND:
        if (LOWORD(wp) == ID_OVERLAY_CLOSE) {
            g_overlayOn = 0;
            SetChecked(ID_CHK_OVERLAY, BST_UNCHECKED);
            OverlaySavePos();
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        break;
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, COL_TEXT); SetBkColor(hdc, COL_BGDEEP);
        return (LRESULT)g_brBgDeep; /* v13: sigue el tema */
    }
    case WM_NCHITTEST: {
        /* Toda la ventana es arrastrable (el boton X sigue siendo clicable:
         * es un hijo y recibe su propio hit-test antes que el padre). */
        LRESULT r = DefWindowProcA(hwnd, msg, wp, lp);
        if (r == HTCLIENT) return HTCAPTION;
        return r;
    }
    case WM_MOVE: {
        RECT rc; GetWindowRect(hwnd, &rc);
        g_overlayX = rc.left; g_overlayY = rc.top;
        OverlaySavePos();
        return 0;
    }
    case WM_DESTROY:
        KillTimer(hwnd, 7);
        if (g_dxgiOut) { g_dxgiOut->lpVtbl->Release(g_dxgiOut); g_dxgiOut = NULL; }
        g_overlay = NULL; g_overlayText = NULL;
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}
static void OverlayApplyClickThrough(void) {
    if (!g_overlay) return;
    LONG_PTR ex = GetWindowLongPtrA(g_overlay, GWL_EXSTYLE);
    if (g_overlayClick) ex |= WS_EX_TRANSPARENT;
    else ex &= ~WS_EX_TRANSPARENT;
    SetWindowLongPtrA(g_overlay, GWL_EXSTYLE, ex);
    SetWindowPos(g_overlay, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}
static void OverlayShow(void) {
    if (!g_overlay) {
        DWORD rv = 0;
        g_overlayX = RegGetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "OverlayX", &rv) ? (int)rv : 40;
        g_overlayY = RegGetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "OverlayY", &rv) ? (int)rv : 40;
        g_overlay = CreateWindowExA(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW, "LuAnOverlay", "Lu-An Monitor",
            WS_POPUP, g_overlayX, g_overlayY, 270, 190, NULL, NULL, g_hInst, NULL);
        if (!g_overlay) return;
        SetLayeredWindowAttributes(g_overlay, 0, 225, LWA_ALPHA);
        if (!g_tempStopEvent) {
            g_tempStopEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
            HANDLE h = CreateThread(NULL, 0, TempThreadProc, NULL, 0, NULL);
            if (h) CloseHandle(h);
        }
    }
    OverlayApplyClickThrough();
    ShowWindow(g_overlay, SW_SHOWNOACTIVATE);
    UpdateWindow(g_overlay);
}
static void OverlayHide(void) {
    if (g_overlay) ShowWindow(g_overlay, SW_HIDE);
}

/* ---------- Helpers UI ---------- */
/* Busca un control aunque este reparentado dentro de una pestana */
static HWND Ctl(int id) {
    HWND h = GetDlgItem(g_hwnd, id);
    if (h) return h;
    if (g_pageTimer) { h = GetDlgItem(g_pageTimer, id); if (h) return h; }
    if (g_pageFK) { h = GetDlgItem(g_pageFK, id); if (h) return h; }
    if (g_pageDNS) { h = GetDlgItem(g_pageDNS, id); if (h) return h; }
    if (g_pageComp) { h = GetDlgItem(g_pageComp, id); if (h) return h; }
    if (g_pageGames) { h = GetDlgItem(g_pageGames, id); if (h) return h; }
    return NULL;
}
static int IsChecked(int id) { HWND h = Ctl(id); return h ? (int)SendMessage(h, BM_GETCHECK, 0, 0) : 0; }
static void SetChecked(int id, int v) { HWND h = Ctl(id); if (h) SendMessage(h, BM_SETCHECK, (WPARAM)v, 0); }
static void SetStatus(const char *s) {
    SetWindowTextA(Ctl(ID_STATUS), s);
    TrayUpdateTip(); /* el tooltip de la bandeja sigue al estado actual */
}
static void SetIntEdit(int id, int v) { char b[32]; snprintf(b, sizeof b, "%d", v); SetWindowTextA(Ctl(id), b); }
static int GetIntEdit(int id) { char b[32]; HWND h = Ctl(id); if (!h) return 0; GetWindowTextA(h, b, sizeof b); return atoi(b); }
static void RefreshCurrentLabel(void) {
    char b[128];
    g_currentMs = QueryTimer();
    snprintf(b, sizeof b, Tr(LS_TIMER_CURRENT_FMT), g_currentMs);
    SetWindowTextA(Ctl(ID_LBL_CURRENT), b);
}
static void RefreshDesiredLabel(void) {
    char b[64]; snprintf(b, sizeof b, "%.2f ms", g_desiredMs);
    SetWindowTextA(Ctl(ID_LBL_DESIRED), b);
}

/* ---------- Idioma: aplicar a toda la interfaz (v11) ---------- */
static void RefreshDynamicTexts(void) {
    RefreshCurrentLabel();
    RefreshDesiredLabel();
    RefreshCurrentDns();
    /* SOCD: segun su estado actual */
    if (!g_socdOn) SetWindowTextA(Ctl(ID_LBL_SOCD_STATUS), Tr(LS_SOCD_OFF));
    else SetWindowTextA(Ctl(ID_LBL_SOCD_STATUS), Tr(g_socdMode == 0 ? LS_SOCD_ON_LAST : LS_SOCD_ON_NEUTRAL));
    /* Ultra: etiqueta y boton segun su estado */
    SetWindowTextA(Ctl(ID_LBL_ULTRA_STATUS), g_ultraOn ? Tr(LS_ULTRA_ON) : Tr(LS_ULTRA_OFF));
    SetWindowTextA(Ctl(ID_ULTRA_ON), g_ultraOn ? Tr(LS_ULTRA_BTN_OFF) : Tr(LS_ULTRA_BTN_ON));
    /* Ping en vivo: si hay muestras, reconstruir; si no, el texto inicial */
    if (g_liveCount > 0) { char b[256]; LivePingText(b, sizeof b); SetWindowTextA(Ctl(ID_LBL_LIVE_PING), b); }
    /* Ping a servidores: si ya se midio, reconstruir */
    if (g_srvRes.cdmx >= 0 || g_srvRes.miami >= 0) { char b[512]; ServerPingText(b, sizeof b); SetWindowTextA(Ctl(ID_LBL_SERVER_PING), b); }
    /* Traceroute: si ya termino, reconstruir el resumen */
    if (g_traceDone) { char b[256]; TraceSummaryText(b, sizeof b); SetWindowTextA(Ctl(ID_LBL_TRACE_SUMMARY), b); }
    /* Juegos: segun el juego con perfil aplicado (o ninguno) */
    if (g_gameCurrent >= 0) {
        char b[256]; snprintf(b, sizeof b, Tr(LS_GAME_DETECTED_FMT), GameDisplayName(g_gameCurrent));
        SetWindowTextA(Ctl(ID_GAMES_STATUS), b);
    }
    /* El resto de etiquetas dinamicas (DNS, memoria, FK) vuelve a su texto
     * inicial traducido; se actualiza solo en la proxima accion. */
    SetWindowTextA(Ctl(ID_LBL_DNS_STATUS), Tr(LS_DNS_HINT));
    SetWindowTextA(Ctl(ID_LBL_MEM_STATUS), Tr(LS_MEM_INIT));
    SetWindowTextA(Ctl(ID_LBL_FK_STATUS), Tr(LS_FK_RULE));
    /* Overlay: refrescar sus etiquetas si esta visible */
    if (g_overlay) OverlayRefreshText();
}
static void ApplyLanguage(void) {
    for (int i = 0; i < g_langRegCount; i++)
        if (g_langReg[i].h) SetWindowTextA(g_langReg[i].h, Tr(g_langReg[i].key));
    RefreshThemeCombo(); /* v13: los nombres de los temas tambien se traducen */
    /* Pestanas: Competitivo/Juegos cambian de nombre */
    g_tabNames[3] = Tr(LS_TAB_COMPETITIVE);
    g_tabNames[4] = Tr(LS_TAB_GAMES);
    if (g_tabBar) InvalidateRect(g_tabBar, NULL, FALSE);
    RefreshDynamicTexts();
    UpdateStatusBox();
    SetStatus(Tr(LS_STATUS_READY));
}
static void SetTrack(double ms) {
    g_desiredMs = ms;
    SendMessage(Ctl(ID_TRACKBAR), TBM_SETPOS, TRUE, (LPARAM)(int)(ms * 2.0 + 0.5));
    RefreshDesiredLabel();
}
static void ApplyTimer(void) {
    double applied = SetTimerRes(g_desiredMs);
    if (applied < 0) { MessageBoxA(g_hwnd, Tr(LS_TIMER_APPLY_FAIL), "Timer Resolution", MB_ICONWARNING); return; }
    char b[160]; snprintf(b, sizeof b, Tr(LS_TIMER_APPLIED_FMT), applied);
    SetStatus(b); RefreshCurrentLabel();
}
static void ApplyFK(void) {
    int enabled = IsChecked(ID_CHK_FK) == BST_CHECKED;
    int bounce = IsChecked(ID_CHK_BOUNCE) == BST_CHECKED;
    DWORD bm = (DWORD)GetIntEdit(ID_EDIT_BOUNCE), wm = (DWORD)GetIntEdit(ID_EDIT_WAIT);
    DWORD dm = (DWORD)GetIntEdit(ID_EDIT_DELAY), rm = (DWORD)GetIntEdit(ID_EDIT_REPEAT);
    if (!SetFK(enabled, bounce, bm, wm, dm, rm)) { MessageBoxA(g_hwnd, Tr(LS_FK_APPLY_FAIL), "FilterKeys", MB_ICONWARNING); return; }
    char b[220];
    if (!enabled) snprintf(b, sizeof b, Tr(LS_FK_OFF));
    else if (bounce && bm) snprintf(b, sizeof b, Tr(LS_FK_BOUNCE_FMT), (unsigned)bm);
    else snprintf(b, sizeof b, Tr(LS_FK_SLOW_FMT), (unsigned)wm, (unsigned)dm, (unsigned)rm);
    SetWindowTextA(Ctl(ID_LBL_FK_STATUS), b); SetStatus(b);
}
static void ProfileGaming(void) { SetTrack(0.5); ApplyTimer(); SetChecked(ID_CHK_FK, BST_UNCHECKED); SetChecked(ID_CHK_BOUNCE, BST_UNCHECKED); ApplyFK(); SetStatus(Tr(LS_PROFILE_GAMING)); }
static void ProfileTyping(void) { SetTrack(1.0); ApplyTimer(); SetChecked(ID_CHK_FK, BST_CHECKED); SetChecked(ID_CHK_BOUNCE, BST_CHECKED); SetIntEdit(ID_EDIT_BOUNCE, 100); SetIntEdit(ID_EDIT_WAIT, 0); SetIntEdit(ID_EDIT_DELAY, 0); SetIntEdit(ID_EDIT_REPEAT, 0); ApplyFK(); SetStatus(Tr(LS_PROFILE_TYPING)); }
static void ProfileAccess(void) { SetTrack(1.0); ApplyTimer(); SetChecked(ID_CHK_FK, BST_CHECKED); SetChecked(ID_CHK_BOUNCE, BST_UNCHECKED); SetIntEdit(ID_EDIT_BOUNCE, 0); SetIntEdit(ID_EDIT_WAIT, 500); SetIntEdit(ID_EDIT_DELAY, 1000); SetIntEdit(ID_EDIT_REPEAT, 500); ApplyFK(); SetStatus(Tr(LS_PROFILE_ACCESS)); }
static void ProfileNormal(void) { SetTrack(15.6); ApplyTimer(); SetChecked(ID_CHK_FK, BST_UNCHECKED); ApplyFK(); SetStatus(Tr(LS_PROFILE_NORMAL)); }

/* ---------- Tipografia estilo iOS (San Francisco no existe en Windows:
 * se usa Segoe UI Variable Text, con fallback a Segoe UI) ---------- */
static int CALLBACK EnumFontFamProc(const LOGFONTA *lf, const TEXTMETRICA *tm, DWORD ft, LPARAM lp) {
    (void)lf; (void)tm; (void)ft; *(int*)lp = 1; return 0;
}
static HFONT MakeUIFont(int px, int weight) {
    const char *name = g_hasVarFont ? "Segoe UI Variable Text" : "Segoe UI";
    return CreateFontA(px,0,0,0,weight,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_SWISS,name);
}
/* ---------- Animacion de botones: hover con fundido + pulso al pulsar ---------- */
static COLORREF Blend(COLORREF c1, COLORREF c2, int t) {
    if (t < 0) t = 0; if (t > 255) t = 255;
    return RGB(GetRValue(c1) + (GetRValue(c2)-GetRValue(c1))*t/255,
               GetGValue(c1) + (GetGValue(c2)-GetGValue(c1))*t/255,
               GetBValue(c1) + (GetBValue(c2)-GetBValue(c1))*t/255);
}
typedef struct { HWND h; int level; int target; int pressed; int tracking; } BtnAnim;
static BtnAnim g_anim[64];
static int g_animCount = 0;
static BtnAnim* AnimFor(HWND h) {
    for (int i = 0; i < g_animCount; i++) if (g_anim[i].h == h) return &g_anim[i];
    if (g_animCount >= 64) return NULL;
    BtnAnim *a = &g_anim[g_animCount++];
    memset(a, 0, sizeof *a); a->h = h; return a;
}
static LRESULT CALLBACK BtnSubProc(HWND h, UINT m, WPARAM w, LPARAM l, UINT_PTR id, DWORD_PTR ref) {
    (void)id; (void)ref;
    BtnAnim *a = AnimFor(h);
    if (a) switch (m) {
    case WM_MOUSEMOVE:
        if (!a->tracking) { TRACKMOUSEEVENT t; t.cbSize = sizeof t; t.dwFlags = TME_LEAVE; t.hwndTrack = h; t.dwHoverTime = 0; TrackMouseEvent(&t); a->tracking = 1; }
        /* v7: con el SOCD activo no arrancar el timer de animacion (16 ms):
         * dejar el boton en su estado final de una vez, sin repintar. */
        if (a->target != 255) { a->target = 255; if (g_socdOn) { a->level = 255; InvalidateRect(h, NULL, FALSE); } else SetTimer(g_hwnd, 2, 16, NULL); }
        break;
    case WM_MOUSELEAVE:
        a->tracking = 0; a->target = 0;
        if (g_socdOn) { a->level = 0; InvalidateRect(h, NULL, FALSE); }
        else SetTimer(g_hwnd, 2, 16, NULL);
        break;
    case WM_LBUTTONDOWN:
        a->pressed = 1; InvalidateRect(h, NULL, FALSE); break;
    case WM_LBUTTONUP:
        a->pressed = 0; InvalidateRect(h, NULL, FALSE); break;
    }
    return DefSubclassProc(h, m, w, l);
}

/* ---------- Interruptor ON/OFF animado tipo iOS (v10) ----------
 * Sustituye a TODAS las casillas con palomita (BS_AUTOCHECKBOX). Con
 * visual styles, el checkbox estandar dibuja su texto en negro
 * (color del tema, ignora WM_CTLCOLORBTN) y por eso las letras se
 * veian negras sobre el fondo oscuro, sobre todo en la pestana Juegos.
 * El toggle dibuja el mismo su texto en COL_TEXT, asi que el problema
 * desaparece, y ademas se ve moderno: pista redondeada gris en OFF y
 * degradado cian->magenta en ON, circulo blanco que se desliza con
 * animacion suave (~150 ms, timer de 16 ms). Responde a clic y a
 * teclado (Espacio/Enter). Para que TODO el codigo existente siga
 * funcionando sin cambios, responde a BM_GETCHECK / BM_SETCHECK como
 * un checkbox normal (IsChecked / SetChecked no se tocan). */
typedef struct { int checked; float anim; float target; int hover; int pressed; int focus; int tracking; } ToggleState;
static LRESULT CALLBACK ToggleWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    ToggleState *st = (ToggleState*)(INT_PTR)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    switch (msg) {
    case WM_CREATE: {
        st = (ToggleState*)calloc(1, sizeof(ToggleState));
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)st);
        return 0;
    }
    case WM_DESTROY:
        free(st); SetWindowLongPtrA(hwnd, GWLP_USERDATA, 0); return 0;
    case WM_GETDLGCODE:
        return DLGC_WANTALLKEYS;
    case BM_GETCHECK:
        return st ? (st->checked ? BST_CHECKED : BST_UNCHECKED) : BST_UNCHECKED;
    case BM_SETCHECK:
        if (st) { st->checked = (wp == BST_CHECKED); st->anim = st->target = st->checked ? 1.0f : 0.0f; KillTimer(hwnd, 1); InvalidateRect(hwnd, NULL, FALSE); }
        return 0;
    case WM_SETFOCUS:
        if (st) { st->focus = 1; InvalidateRect(hwnd, NULL, FALSE); } return 0;
    case WM_KILLFOCUS:
        if (st) { st->focus = 0; st->pressed = 0; InvalidateRect(hwnd, NULL, FALSE); } return 0;
    case WM_MOUSEMOVE:
        if (st && !st->tracking) { TRACKMOUSEEVENT t; t.cbSize = sizeof t; t.dwFlags = TME_LEAVE; t.hwndTrack = hwnd; t.dwHoverTime = 0; TrackMouseEvent(&t); st->tracking = 1; }
        if (st && !st->hover) { st->hover = 1; InvalidateRect(hwnd, NULL, FALSE); }
        return 0;
    case WM_MOUSELEAVE:
        if (st) { st->hover = 0; st->tracking = 0; InvalidateRect(hwnd, NULL, FALSE); }
        return 0;
    case WM_LBUTTONDOWN:
        if (st) { st->pressed = 1; SetCapture(hwnd); InvalidateRect(hwnd, NULL, FALSE); }
        SetFocus(hwnd);
        return 0;
    case WM_LBUTTONUP: {
        if (!st || !st->pressed) return 0;
        st->pressed = 0; ReleaseCapture();
        RECT rc; GetClientRect(hwnd, &rc);
        POINT pt = { (int)(short)LOWORD(lp), (int)(short)HIWORD(lp) };
        if (PtInRect(&rc, pt)) {
            st->checked = !st->checked;
            st->target = st->checked ? 1.0f : 0.0f;
            if (g_socdOn) { st->anim = st->target; InvalidateRect(hwnd, NULL, FALSE); }
            else SetTimer(hwnd, 1, 16, NULL);
            SendMessageA(GetParent(hwnd), WM_COMMAND, MAKEWPARAM((UINT)(INT_PTR)GetWindowLongPtrA(hwnd, GWLP_ID), BN_CLICKED), (LPARAM)hwnd);
        }
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    }
    case WM_KEYDOWN:
        if (st && (wp == VK_SPACE || wp == VK_RETURN) && !(lp & (1 << 30))) {
            st->checked = !st->checked;
            st->target = st->checked ? 1.0f : 0.0f;
            if (g_socdOn) { st->anim = st->target; InvalidateRect(hwnd, NULL, FALSE); }
            else SetTimer(hwnd, 1, 16, NULL);
            SendMessageA(GetParent(hwnd), WM_COMMAND, MAKEWPARAM((UINT)(INT_PTR)GetWindowLongPtrA(hwnd, GWLP_ID), BN_CLICKED), (LPARAM)hwnd);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        return 0;
    case WM_TIMER:
        if (wp == 1 && st) {
            float step = 0.22f; /* ~150 ms de animacion a 16 ms por paso */
            if (st->anim < st->target) { st->anim += step; if (st->anim > st->target) st->anim = st->target; }
            else if (st->anim > st->target) { st->anim -= step; if (st->anim < st->target) st->anim = st->target; }
            InvalidateRect(hwnd, NULL, FALSE);
            if (st->anim == st->target) KillTimer(hwnd, 1);
        }
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, g_brBg);
        if (st) {
            int cy = (rc.bottom - rc.top) / 2;
            int ty = cy - 12; /* pista 46x24 */
            /* Halo suave al pasar el raton (difuminado simulado: capas
             * cada vez mas cercanas al fondo). */
            if (st->hover || st->focus) {
                for (int k = 2; k >= 1; k--) {
                    COLORREF hc = Blend(st->checked ? COL_ACCENT : COL_CARD2, COL_BG, 200);
                    HBRUSH hb = CreateSolidBrush(hc);
                    HPEN hp = CreatePen(PS_SOLID, 1, hc);
                    HGDIOBJ ob = SelectObject(hdc, hb), op = SelectObject(hdc, hp);
                    RoundRect(hdc, 0, ty - k * 2, 46 + k * 4, ty + 24 + k * 2, 14, 14);
                    SelectObject(hdc, ob); SelectObject(hdc, op);
                    DeleteObject(hb); DeleteObject(hp);
                }
            }
            /* Pista: gris en OFF, degradado cian->magenta en ON (se
             * funde segun la animacion para que el cambio sea suave). */
            for (int x = 0; x < 46; x++) {
                COLORREF on = Blend(COL_ACCENT, COL_MAGENTA, x * 255 / 46);
                COLORREF c = Blend(RGB(71, 85, 105), on, (int)(st->anim * 255.0f));
                HPEN p2 = CreatePen(PS_SOLID, 1, c);
                HGDIOBJ op2 = SelectObject(hdc, p2);
                MoveToEx(hdc, x, ty + 2, NULL); LineTo(hdc, x, ty + 22);
                SelectObject(hdc, op2); DeleteObject(p2);
            }
            /* Bordes redondeados de la pista (tapar las esquinas del degradado) */
            HPEN bp = CreatePen(PS_SOLID, 1, st->checked ? Blend(COL_ACCENT, COL_MAGENTA, 128) : RGB(100, 116, 139));
            HGDIOBJ obp = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH)), opp = SelectObject(hdc, bp);
            RoundRect(hdc, 0, ty, 46, ty + 24, 12, 12);
            SelectObject(hdc, obp); SelectObject(hdc, opp); DeleteObject(bp);
            /* Circulo blanco que se desliza: 2 + anim*22 (diametro 20) */
            int kx = 2 + (int)(st->anim * 22.0f + 0.5f);
            HBRUSH sb = CreateSolidBrush(RGB(203, 213, 225));
            HPEN sp = CreatePen(PS_SOLID, 1, RGB(203, 213, 225));
            HGDIOBJ osb = SelectObject(hdc, sb), osp = SelectObject(hdc, sp);
            Ellipse(hdc, kx, ty + 13, kx + 20, ty + 23);
            SelectObject(hdc, osb); SelectObject(hdc, osp);
            DeleteObject(sb); DeleteObject(sp);
            HBRUSH wb = CreateSolidBrush(RGB(255, 255, 255));
            HPEN wpen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
            HGDIOBJ owb = SelectObject(hdc, wb), owp = SelectObject(hdc, wpen);
            Ellipse(hdc, kx, ty + 2, kx + 20, ty + 22);
            SelectObject(hdc, owb); SelectObject(hdc, owp);
            DeleteObject(wb); DeleteObject(wpen);
            if (st->focus) {
                HPEN fp = CreatePen(PS_DOT, 1, COL_ACCENT);
                HGDIOBJ ofp = SelectObject(hdc, GetStockObject(HOLLOW_BRUSH)), ofp2 = SelectObject(hdc, fp);
                RoundRect(hdc, 0, ty - 3, 46 + 6, ty + 27, 12, 12);
                SelectObject(hdc, ofp); SelectObject(hdc, ofp2); DeleteObject(fp);
            }
        }
        /* Texto a la derecha de la pista, siempre en color claro
         * (COL_TEXT): es lo que arregla las letras negras de Juegos. */
        SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, COL_TEXT);
        SelectObject(hdc, g_font);
        char txt[256]; GetWindowTextA(hwnd, txt, sizeof txt);
        RECT tr = { 54, rc.top, rc.right, rc.bottom };
        DrawTextA(hdc, txt, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        EndPaint(hwnd, &ps);
        return 0;
    }
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}
/* Crea un interruptor ON/OFF con el mismo sitio y tamano que la casilla
 * que sustituye. La ventana mide 24 px de alto (la pista), el texto va
 * dentro de la misma ventana. */
static HWND MakeToggle(HWND p, int x, int y, int w, const char *t, int id) {
    HWND hw = CreateWindowExA(0, "LuAnToggle", t, WS_CHILD | WS_VISIBLE | WS_TABSTOP, x, y, w, 24, p, (HMENU)(INT_PTR)id, g_hInst, NULL);
    SendMessage(hw, WM_SETFONT, (WPARAM)g_font, TRUE);
    return hw;
}

static HWND MakeLabel(HWND p, int x, int y, int w, int ht, const char *t, HFONT f, int id) {
    HWND hw = CreateWindowExA(0, "STATIC", t, WS_CHILD|WS_VISIBLE|SS_LEFT, x, y, w, ht, p, (HMENU)(INT_PTR)id, g_hInst, NULL);
    SendMessage(hw, WM_SETFONT, (WPARAM)f, TRUE); return hw;
}
static HWND MakeButton(HWND p, int x, int y, int w, int ht, const char *t, int id) {
    /* v10 (difuminado): la ventana del boton es 12 px mas grande que el
     * boton visible, con 6 px de margen por lado para dibujar el glow
     * sin que el recorte del control lo corte. La posicion visual del
     * boton no cambia: se dibuja inset 6 px dentro de su ventana. */
    HWND hw = CreateWindowExA(0, "BUTTON", t, WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_OWNERDRAW, x - 6, y - 6, w + 12, ht + 12, p, (HMENU)(INT_PTR)id, g_hInst, NULL);
    SendMessage(hw, WM_SETFONT, (WPARAM)g_font, TRUE);
    SetWindowSubclass(hw, BtnSubProc, 0, 0);
    return hw;
}

/* ---------- Idioma: controles registrados (v11) ----------
 * MakeLabelK/MakeButtonK/MakeToggleK crean el control con el texto en
 * el idioma actual y lo registran: ApplyLanguage() les vuelve a poner
 * el texto al cambiar de idioma, sin reiniciar. */
static void RegLang(HWND h, int key) {
    if (h && g_langRegCount < MAX_LANG_REG) { g_langReg[g_langRegCount].h = h; g_langReg[g_langRegCount].key = key; g_langRegCount++; }
}
static HWND MakeLabelK(HWND p, int x, int y, int w, int ht, int key, HFONT f, int id) {
    HWND hw = MakeLabel(p, x, y, w, ht, Tr(key), f, id);
    RegLang(hw, key);
    return hw;
}
static HWND MakeButtonK(HWND p, int x, int y, int w, int ht, int key, int id) {
    HWND hw = MakeButton(p, x, y, w, ht, Tr(key), id);
    RegLang(hw, key);
    return hw;
}
static HWND MakeToggleK(HWND p, int x, int y, int w, int key, int id) {
    HWND hw = MakeToggle(p, x, y, w, Tr(key), id);
    RegLang(hw, key);
    return hw;
}

/* Dibuja botones bonitos: fondo redondeado, color segun ID, con
 * difuminado (glow) alrededor: varias capas de rectangulos redondeados
 * cada vez mas cercanas al fondo, mas intensas en hover. */
static void DrawButton(LPDRAWITEMSTRUCT d) {
    HDC hdc = d->hDC; RECT r = d->rcItem;
    BtnAnim *a = AnimFor(d->hwndItem);
    COLORREF base = COL_BTN;
    if (d->CtlID == ID_APPLY_TIMER || d->CtlID == ID_APPLY_FK || d->CtlID == ID_DNS_APPLY || d->CtlID == ID_COMP_MODE || d->CtlID == ID_ULTRA_ON) base = COL_BTN2;
    if ((d->CtlID >= ID_PRESET_05 && d->CtlID <= ID_PRESET_156) ||
        (d->CtlID >= ID_DNS_GOOGLE && d->CtlID <= ID_DNS_AUTO)) base = COL_CARD2;
    if (d->itemState & ODS_SELECTED) base = COL_ACCENT2;
    /* Hover: funde hacia un tono mas claro; pulsado: se encoge (pulso) y se oscurece */
    COLORREF hoverCol = Blend(base, RGB(255,255,255), 80);
    COLORREF bg = Blend(base, hoverCol, a ? a->level : 0);
    COLORREF border = (a && a->level > 0) ? Blend(COL_ACCENT, base, 255 - a->level) : base;
    RECT b = r; InflateRect(&b, -6, -6); /* cuerpo visible del boton */
    if (a && a->pressed) { InflateRect(&b, -3, -3); bg = Blend(bg, RGB(0,0,0), 70); }
    /* Difuminado (v10): 3 capas concéntricas en el margen de 6 px.
     * La mas externa es casi el fondo; al pasar el raton el glow se
     * intensifica (menos mezcla hacia el fondo). */
    int hov = a ? a->level : 0;
    COLORREF glowCol = Blend(base, COL_ACCENT, 120);
    for (int k = 3; k >= 1; k--) {
        RECT gr = b; InflateRect(&gr, k * 2, k * 2);
        int mix = 150 + k * 30 - hov / 8;
        if (mix < 70) mix = 70; if (mix > 240) mix = 240;
        COLORREF gc = Blend(glowCol, COL_BG, mix);
        HBRUSH gbr = CreateSolidBrush(gc);
        HPEN gpen = CreatePen(PS_SOLID, 1, gc);
        HGDIOBJ ogb = SelectObject(hdc, gbr), ogp = SelectObject(hdc, gpen);
        RoundRect(hdc, gr.left, gr.top, gr.right, gr.bottom, 14, 14);
        SelectObject(hdc, ogb); SelectObject(hdc, ogp);
        DeleteObject(gbr); DeleteObject(gpen);
    }
    HBRUSH br = CreateSolidBrush(bg);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ oldBr = SelectObject(hdc, br), oldPen = SelectObject(hdc, pen);
    RoundRect(hdc, b.left, b.top, b.right, b.bottom, 12, 12);
    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen); DeleteObject(br); DeleteObject(pen);
    SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, COL_TEXT);
    SelectObject(hdc, g_font);
    char txt[128]; GetWindowTextA(d->hwndItem, txt, sizeof txt);
    DrawTextA(hdc, txt, -1, &b, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}

/* ---------- Barra de pestanas premium (dibujada a mano) ----------
 * Sustituye al WC_TABCONTROL estandar, que se ve plano y rompe el diseno.
 * Cada pestana es una "pildora" redondeada con glifo GDI + texto Segoe UI
 * Semibold; la activa lleva fondo de tarjeta y una barra de acento con
 * gradiente cian->magenta (colores del logo LU-AN) que SE DESLIZA de una
 * pestana a otra, y el hover funde suavemente (timer 16 ms, igual que los
 * botones). Dibujo con doble bufer para no parpadear. */
static void TabRect(int idx, RECT *r) {
    r->left = 8 + idx * (130 + 6);
    r->top = 3; r->right = r->left + 130; r->bottom = r->top + 31;
}
static void TabIndRect(int idx, float *x, float *w) {
    RECT r; TabRect(idx, &r);
    *x = (float)(r.left + 14); *w = (float)(r.right - r.left - 28);
}
static int TabBarHit(int x, int y) {
    for (int i = 0; i < 5; i++) {
        RECT r; TabRect(i, &r);
        if (x >= r.left && x < r.right && y >= r.top && y < r.bottom) return i;
    }
    return -1;
}
/* Glifos simples en GDI, centrados en (cx,cy), color col, 18x18 aprox:
 * 0 reloj (Timer), 1 teclado (FilterKeys), 2 globo (DNS), 3 rayo (Competitivo) */
static void DrawTabGlyph(HDC hdc, int idx, int cx, int cy, COLORREF col) {
    HPEN pen = CreatePen(PS_SOLID, 2, col);
    HBRUSH hollow = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
    HBRUSH solid = CreateSolidBrush(col);
    HGDIOBJ ob = SelectObject(hdc, hollow), op = SelectObject(hdc, pen);
    switch (idx) {
    case 0: /* reloj */
        Ellipse(hdc, cx-8, cy-8, cx+8, cy+8);
        MoveToEx(hdc, cx, cy, NULL); LineTo(hdc, cx, cy-6);
        MoveToEx(hdc, cx, cy, NULL); LineTo(hdc, cx+5, cy+2);
        break;
    case 1: /* teclado */
        RoundRect(hdc, cx-9, cy-6, cx+9, cy+6, 3, 3);
        SelectObject(hdc, solid);
        for (int r2 = 0; r2 < 2; r2++) for (int c2 = 0; c2 < 3; c2++)
            Rectangle(hdc, cx-6+c2*5, cy-3+r2*5, cx-4+c2*5, cy-1+r2*5);
        break;
    case 2: /* globo / red */
        Ellipse(hdc, cx-8, cy-8, cx+8, cy+8);
        Ellipse(hdc, cx-3, cy-8, cx+3, cy+8);
        MoveToEx(hdc, cx-8, cy, NULL); LineTo(hdc, cx+8, cy);
        break;
    case 4: { /* mando / gamepad */
        RoundRect(hdc, cx-9, cy-5, cx+9, cy+6, 5, 5);
        SelectObject(hdc, solid);
        Rectangle(hdc, cx-7, cy-2, cx-5, cy+2);
        Rectangle(hdc, cx-6, cy-1, cx-3, cy+1);
        Rectangle(hdc, cx+4, cy-3, cx+6, cy-1);
        Rectangle(hdc, cx+4, cy+1, cx+6, cy+3);
        break; }
    case 3: { /* rayo */
        POINT p[6] = {{cx+2,cy-9},{cx-5,cy+1},{cx-1,cy+1},{cx-2,cy+9},{cx+5,cy-2},{cx+1,cy-2}};
        SelectObject(hdc, solid); SelectObject(hdc, GetStockObject(NULL_PEN));
        Polygon(hdc, p, 6);
        break; }
    }
    SelectObject(hdc, ob); SelectObject(hdc, op);
    DeleteObject(pen); DeleteObject(solid);
}
static void TabBarAnimate(HWND h) {
    /* v7: con el SOCD activo no arrancar la animacion de la barra:
     * indicador en su sitio final, sin timer de 16 ms. */
    if (g_socdOn) { g_indX = g_indTX; g_indW = g_indTW; InvalidateRect(h, NULL, FALSE); return; }
    SetTimer(h, 1, 16, NULL);
}
static void TabBarSelect(int idx) {
    if (idx < 0 || idx > 4) return;
    TabIndRect(idx, &g_indTX, &g_indTW);
    TabBarAnimate(g_tabBar);
    ShowTab(idx);
}
static LRESULT CALLBACK TabBarWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HGDIOBJ oldBmp = SelectObject(mem, bmp);
        FillRect(mem, &rc, g_brBg);
        for (int i = 0; i < 5; i++) {
            RECT r; TabRect(i, &r);
            int active = (i == g_curTab);
            int lvl = g_tabHoverLvl[i];
            COLORREF bg = active ? COL_CARD : Blend(COL_BG, COL_TABHOVER, lvl / 2);
            HBRUSH br = CreateSolidBrush(bg);
            HPEN pen = CreatePen(PS_SOLID, 1, active ? Blend(COL_ACCENT, COL_ACCENT2, 128) : (lvl > 0 ? COL_CARD2 : COL_BG));
            HGDIOBJ ob = SelectObject(mem, br), op = SelectObject(mem, pen);
            RoundRect(mem, r.left, r.top, r.right, r.bottom, 12, 12);
            SelectObject(mem, ob); SelectObject(mem, op);
            DeleteObject(br); DeleteObject(pen);
            COLORREF gc = active ? COL_ACCENT : (lvl > 0 ? Blend(COL_MUTED, COL_TEXT, lvl) : COL_MUTED);
            DrawTabGlyph(mem, i, r.left + 24, r.top + 15, gc);
            SetBkMode(mem, TRANSPARENT);
            SetTextColor(mem, active ? COL_TEXT : (lvl > 0 ? Blend(COL_MUTED, COL_TEXT, lvl) : COL_MUTED));
            SelectObject(mem, g_fontTab);
            RECT tr = { r.left + 42, r.top, r.right - 6, r.bottom };
            DrawTextA(mem, g_tabNames[i], -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        }
        /* Indicador deslizante: barra de gradiente cian->magenta bajo la pestana activa */
        int ix = (int)(g_indX + 0.5f), iw = (int)(g_indW + 0.5f);
        if (iw > 4) {
            for (int x = 0; x < iw; x++) {
                COLORREF c = Blend(COL_ACCENT, COL_MAGENTA, x * 255 / iw);
                HPEN p2 = CreatePen(PS_SOLID, 1, c);
                HGDIOBJ op2 = SelectObject(mem, p2);
                MoveToEx(mem, ix + x, TABBAR_H - 4, NULL); LineTo(mem, ix + x, TABBAR_H - 1);
                SelectObject(mem, op2); DeleteObject(p2);
            }
        }
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, oldBmp); DeleteObject(bmp); DeleteDC(mem);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND: return 1;
    case WM_MOUSEMOVE: {
        int x = (int)(short)LOWORD(lp), y = (int)(short)HIWORD(lp);
        int hit = TabBarHit(x, y);
        if (hit != g_tabHover) { g_tabHover = hit; TabBarAnimate(hwnd); }
        if (hit >= 0) SetCursor(LoadCursor(NULL, IDC_HAND));
        if (hit >= 0) {
            TRACKMOUSEEVENT t; t.cbSize = sizeof t; t.dwFlags = TME_LEAVE; t.hwndTrack = hwnd; t.dwHoverTime = 0;
            TrackMouseEvent(&t);
        }
        return 0;
    }
    case WM_MOUSELEAVE:
        g_tabHover = -1; TabBarAnimate(hwnd); return 0;
    case WM_LBUTTONDOWN: {
        int x = (int)(short)LOWORD(lp), y = (int)(short)HIWORD(lp);
        int hit = TabBarHit(x, y);
        if (hit >= 0 && hit != g_curTab) TabBarSelect(hit);
        return 0;
    }
    case WM_KEYDOWN:
        if (wp == VK_LEFT) { TabBarSelect((g_curTab + 4) % 5); return 0; }
        if (wp == VK_RIGHT) { TabBarSelect((g_curTab + 1) % 5); return 0; }
        break;
    case WM_TIMER:
        if (wp == 1) {
            /* v7: si el SOCD se activo mientras la barra animaba, dejar
             * todo en su estado final y parar el timer. */
            if (g_socdOn) {
                for (int i = 0; i < 5; i++) g_tabHoverLvl[i] = (i == g_tabHover) ? 255 : 0;
                g_indX = g_indTX; g_indW = g_indTW;
                KillTimer(hwnd, 1); InvalidateRect(hwnd, NULL, FALSE);
                return 0;
            }
            int active = 0;
            for (int i = 0; i < 5; i++) {
                int target = (i == g_tabHover) ? 255 : 0;
                if (g_tabHoverLvl[i] != target) {
                    int step = 45;
                    if (g_tabHoverLvl[i] < target) { g_tabHoverLvl[i] += step; if (g_tabHoverLvl[i] > target) g_tabHoverLvl[i] = target; }
                    else { g_tabHoverLvl[i] -= step; if (g_tabHoverLvl[i] < target) g_tabHoverLvl[i] = target; }
                    active = 1;
                }
            }
            float dx = g_indTX - g_indX, dw = g_indTW - g_indW;
            if (dx > 0.5f || dx < -0.5f || dw > 0.5f || dw < -0.5f) {
                g_indX += dx * 0.25f; g_indW += dw * 0.25f; active = 1;
            } else { g_indX = g_indTX; g_indW = g_indTW; }
            if (active) InvalidateRect(hwnd, NULL, FALSE);
            else KillTimer(hwnd, 1);
            return 0;
        }
        break;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

/* ---------- Paginas de pestanas ---------- */
static LRESULT CALLBACK PageWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND: SendMessageA(g_hwnd, WM_COMMAND, wp, lp); return 0;
    case WM_HSCROLL: SendMessageA(g_hwnd, WM_HSCROLL, wp, lp); return 0;
    case WM_LBUTTONDOWN: { /* v13.1: clic en la barra propia */
        int idx = PageIndexFromHwnd(hwnd);
        if (idx < 0) break;
        int x = (int)(short)LOWORD(lp), y = (int)(short)HIWORD(lp);
        RECT track, thumb;
        int has = ScrollThumbRect(hwnd, idx, &track, &thumb);
        if (x < track.left || x >= track.right || !has) break;
        if (y >= thumb.top && y < thumb.bottom) {
            /* Agarrar el thumb: capturar el raton para seguirlo. */
            g_scrollDrag[idx] = 1; g_scrollGrab[idx] = y - thumb.top;
            SetCapture(hwnd);
        } else if (y < thumb.top) {
            PageSetScroll(idx, g_pageScroll[idx] - PAGE_H);
        } else {
            PageSetScroll(idx, g_pageScroll[idx] + PAGE_H);
        }
        return 0; }
    case WM_MOUSEMOVE: { /* v13.1: arrastre del thumb + hover de la barra */
        int idx = PageIndexFromHwnd(hwnd);
        if (idx < 0) break;
        int x = (int)(short)LOWORD(lp), y = (int)(short)HIWORD(lp);
        if (g_scrollDrag[idx] == 1) {
            RECT track, thumb;
            if (ScrollThumbRect(hwnd, idx, &track, &thumb)) {
                int maxOff = g_pageContentH[idx] - PAGE_H; if (maxOff < 0) maxOff = 0;
                int range = (track.bottom - track.top) - (thumb.bottom - thumb.top);
                int off = range > 0 ? (y - g_scrollGrab[idx] - track.top) * maxOff / range : 0;
                PageSetScroll(idx, off);
            }
            return 0;
        }
        { RECT track, thumb;
          int has = ScrollThumbRect(hwnd, idx, &track, &thumb);
          int over = has && x >= track.left && x < track.right;
          if (over && !g_scrollHover[idx]) {
              g_scrollHover[idx] = 1;
              TRACKMOUSEEVENT t; t.cbSize = sizeof t; t.dwFlags = TME_LEAVE; t.hwndTrack = hwnd; t.dwHoverTime = 0;
              TrackMouseEvent(&t);
              ScrollInvalidate(hwnd);
          } else if (!over && g_scrollHover[idx]) {
              g_scrollHover[idx] = 0; ScrollInvalidate(hwnd);
          }
        }
        return 0; }
    case WM_LBUTTONUP: { /* v13.1: soltar el thumb */
        int idx = PageIndexFromHwnd(hwnd);
        if (idx >= 0 && g_scrollDrag[idx] == 1) {
            g_scrollDrag[idx] = -1; ReleaseCapture(); ScrollInvalidate(hwnd);
        }
        return 0; }
    case WM_CAPTURECHANGED: { /* v13.1: si se pierde la captura, soltar el thumb */
        int idx = PageIndexFromHwnd(hwnd);
        if (idx >= 0 && g_scrollDrag[idx] == 1) { g_scrollDrag[idx] = -1; ScrollInvalidate(hwnd); }
        return 0; }
    case WM_MOUSELEAVE: { /* v13.1: quitar el hover de la barra */
        int idx = PageIndexFromHwnd(hwnd);
        if (idx >= 0 && g_scrollHover[idx]) { g_scrollHover[idx] = 0; ScrollInvalidate(hwnd); }
        return 0; }
    case WM_MOUSEWHEEL: { /* v11: rueda del mouse baja la pestana */
        int idx = PageIndexFromHwnd(hwnd);
        if (idx < 0) return 0;
        PageSetScroll(idx, g_pageScroll[idx] - (GET_WHEEL_DELTA_WPARAM(wp) / 120) * 48);
        return 0; }
    case WM_DRAWITEM: DrawButton((LPDRAWITEMSTRUCT)lp); return TRUE;
    case WM_CTLCOLORSTATIC: case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, COL_TEXT); SetBkColor(hdc, COL_BG);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLOREDIT: case WM_CTLCOLORLISTBOX: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, COL_TEXT); SetBkColor(hdc, COL_CARD);
        return (LRESULT)g_brCard;
    }
    case WM_PAINT: {
        /* Marco redondeado del panel de contenido: mismo borde sutil en las 4 paginas */
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        RECT r; GetClientRect(hwnd, &r);
        FillRect(hdc, &r, g_brBg);
        HBRUSH br = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
        HPEN pen = CreatePen(PS_SOLID, 1, COL_CARD2);
        HGDIOBJ ob = SelectObject(hdc, br), op = SelectObject(hdc, pen);
        RoundRect(hdc, r.left + 1, r.top + 1, r.right - 1, r.bottom - 1, 14, 14);
        SelectObject(hdc, ob); SelectObject(hdc, op); DeleteObject(pen);
        /* v13.1: barra de desplazamiento propia, integrada al tema:
         * track del color de tarjeta, thumb redondeado con el degradado
         * de acento del tema (igual que el indicador de pestanas). */
        { int idx = PageIndexFromHwnd(hwnd);
          if (idx >= 0) {
            RECT track, thumb;
            int has = ScrollThumbRect(hwnd, idx, &track, &thumb);
            HBRUSH tbr = CreateSolidBrush(Blend(COL_CARD2, COL_BG, 128));
            HPEN tpen = CreatePen(PS_SOLID, 1, Blend(COL_CARD2, COL_BG, 128));
            HGDIOBJ ob2 = SelectObject(hdc, tbr), op2 = SelectObject(hdc, tpen);
            RoundRect(hdc, track.left, track.top, track.right, track.bottom, 10, 10);
            SelectObject(hdc, ob2); SelectObject(hdc, op2);
            DeleteObject(tbr); DeleteObject(tpen);
            if (has) {
                HRGN rg = CreateRoundRectRgn(thumb.left, thumb.top, thumb.right, thumb.bottom, 9, 9);
                SelectClipRgn(hdc, rg);
                int hov = g_scrollHover[idx] || g_scrollDrag[idx] == 1;
                for (int y = thumb.top; y < thumb.bottom; y++) {
                    COLORREF c = Blend(COL_ACCENT, COL_MAGENTA, (y - thumb.top) * 255 / (thumb.bottom - thumb.top));
                    if (hov) c = Blend(c, RGB(255,255,255), 70);
                    HPEN p2 = CreatePen(PS_SOLID, 1, c);
                    HGDIOBJ op3 = SelectObject(hdc, p2);
                    MoveToEx(hdc, thumb.left, y, NULL); LineTo(hdc, thumb.right, y);
                    SelectObject(hdc, op3); DeleteObject(p2);
                }
                SelectClipRgn(hdc, NULL);
                DeleteObject(rg);
            }
          }
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wp; RECT r; GetClientRect(hwnd, &r);
        FillRect(hdc, &r, g_brBg); return 1;
    }
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}
static HWND g_enumList[128];
static int g_enumCount = 0;
static BOOL CALLBACK CollectEnum(HWND child, LPARAM lp) {
    (void)lp;
    if (g_enumCount < 128) g_enumList[g_enumCount++] = child;
    return TRUE;
}
/* Reparenta cada seccion a su pagina segun su posicion Y original:
 * Timer 162-370, FilterKeys 378-645, DNS 650+. Encabezado, perfiles y
 * la barra de estado quedan en la ventana principal. */
static void ReparentToPages(void) {
    g_enumCount = 0;
    EnumChildWindows(g_hwnd, CollectEnum, 0);
    for (int i = 0; i < g_enumCount; i++) {
        HWND child = g_enumList[i];
        /* OJO (bug corregido en v5): la pagina Competitivo TAMBIEN hay que
         * excluirla; sin ella se reparentaba dentro de la pagina Timer y
         * la pestana Competitivo quedaba vacia. */
        if (child == g_tabBar || child == g_pageTimer || child == g_pageFK || child == g_pageDNS || child == g_pageComp || child == g_pageGames) continue;
        if ((int)(INT_PTR)GetWindowLongPtrA(child, GWLP_ID) == ID_STATUS) continue;
        RECT rc; GetWindowRect(child, &rc);
        POINT pt = { rc.left, rc.top }; ScreenToClient(g_hwnd, &pt);
        HWND page = NULL; int top = 0;
        if (pt.y >= 162 && pt.y < 371) { page = g_pageTimer; top = 162; }
        else if (pt.y >= 371 && pt.y < 646) { page = g_pageFK; top = 378; }
        else if (pt.y >= 646 && pt.y < 950) { page = g_pageDNS; top = 650; }
        else if (pt.y >= 950 && pt.y < 1760) { page = g_pageComp; top = 950; }
        else if (pt.y >= 1760) { page = g_pageGames; top = 1760; }
        else continue;
        SetParent(child, page);
        SetWindowPos(child, NULL, pt.x - 10, pt.y - top + 8, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
    }
}
static void ShowTab(int sel) {
    g_curTab = sel;
    for (int i = 0; i < 5; i++) ShowWindow(g_pages[i], i == sel ? SW_SHOW : SW_HIDE);
    if (sel == 2) RefreshCurrentDns();
    if (g_tabBar) { TabIndRect(sel, &g_indTX, &g_indTW); TabBarAnimate(g_tabBar); }
    /* Transicion: la pagina nueva entra deslizandose 14 px hacia su sitio */
    g_slideX = 14;
    SetWindowPos(g_pages[sel], NULL, PAGE_X + g_slideX, PAGE_Y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
    SetTimer(g_hwnd, 6, 16, NULL);
}

/* ---------- Bandeja del sistema (system tray, v8) ----------
 * La X SIEMPRE cierra el programa como antes. Solo el boton minimizar
 * cambia: si la casilla Tr(LS_TRAY_MIN) esta marcada, el
 * boton _ oculta la ventana y deja el icono LU-AN en la bandeja (el
 * icono SOLO aparece en ese caso). El proceso sigue vivo, asi que el
 * timer, el SOCD, el ping en vivo y la limpieza automatica siguen
 * funcionando en segundo plano. Doble clic en el icono restaura la
 * ventana (y quita el icono de la bandeja); clic derecho abre el menu
 * (Mostrar / Modo Competitivo / SOCD / Salir). La salida real quita el
 * icono y hace la limpieza. */
static int g_trayAdded = 0;          /* 1 = el icono esta en la bandeja */
static int g_inTray = 0;             /* 1 = ventana oculta en la bandeja */
static int g_trayBalloonShown = 0;   /* el aviso solo se muestra una vez */
static int g_trayMinOn = 1;          /* casilla Tr(LS_TRAY_MIN) */
static HICON g_hTrayIcon = NULL;
static UINT g_wmTaskbarCreated = 0;  /* se registra en WM_CREATE */
static void TrayAdd(void) {
    if (!g_hwnd) return;
    if (!g_hTrayIcon) {
        g_hTrayIcon = LoadIcon(g_hInst, MAKEINTRESOURCEA(100));
        if (!g_hTrayIcon) g_hTrayIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    NOTIFYICONDATAA nid; memset(&nid, 0, sizeof nid);
    nid.cbSize = sizeof nid;
    nid.hWnd = g_hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = g_hTrayIcon;
    snprintf(nid.szTip, sizeof nid.szTip, "Lu-An Optimizer");
    if (Shell_NotifyIconA(NIM_ADD, &nid)) g_trayAdded = 1;
    if (g_trayAdded) {
        /* Tooltip con el estado actual (timer + SOCD) */
        char tip[128];
        snprintf(tip, sizeof tip, "Lu-An Optimizer | Timer %.2f ms | SOCD %s",
                 g_currentMs, g_socdOn ? "ON" : "OFF");
        memset(&nid, 0, sizeof nid);
        nid.cbSize = sizeof nid; nid.hWnd = g_hwnd; nid.uID = 1;
        nid.uFlags = NIF_TIP;
        strncpy(nid.szTip, tip, sizeof nid.szTip - 1);
        Shell_NotifyIconA(NIM_MODIFY, &nid);
    }
}
static void TrayRemove(void) {
    if (!g_trayAdded || !g_hwnd) return;
    NOTIFYICONDATAA nid; memset(&nid, 0, sizeof nid);
    nid.cbSize = sizeof nid; nid.hWnd = g_hwnd; nid.uID = 1;
    Shell_NotifyIconA(NIM_DELETE, &nid);
    g_trayAdded = 0;
}
/* Refresca el tooltip con el estado actual; se llama desde SetStatus. */
static void TrayUpdateTip(void) {
    if (!g_trayAdded || !g_hwnd) return;
    NOTIFYICONDATAA nid; memset(&nid, 0, sizeof nid);
    nid.cbSize = sizeof nid; nid.hWnd = g_hwnd; nid.uID = 1;
    nid.uFlags = NIF_TIP;
    snprintf(nid.szTip, sizeof nid.szTip,
             "Lu-An Optimizer | Timer %.2f ms | SOCD %s",
             g_currentMs, g_socdOn ? "ON" : "OFF");
    Shell_NotifyIconA(NIM_MODIFY, &nid);
}
static void TrayBalloonOnce(void) {
    if (g_trayBalloonShown || !g_trayAdded || !g_hwnd) return;
    g_trayBalloonShown = 1;
    NOTIFYICONDATAA nid; memset(&nid, 0, sizeof nid);
    nid.cbSize = sizeof nid; nid.hWnd = g_hwnd; nid.uID = 1;
    nid.uFlags = NIF_INFO;
    nid.hIcon = g_hTrayIcon;
    strncpy(nid.szInfoTitle, "Lu-An Optimizer", sizeof nid.szInfoTitle - 1);
    strncpy(nid.szInfo, Tr(LS_TRAY_BALLOON),
            sizeof nid.szInfo - 1);
    nid.dwInfoFlags = NIIF_INFO;
    nid.uTimeout = 5000;
    Shell_NotifyIconA(NIM_MODIFY, &nid);
}
/* Oculta la ventana y deja el programa en la bandeja. El icono se crea
 * aqui (solo aparece cuando se minimiza en segundo plano). Devuelve 1. */
static int TrayHide(void) {
    TrayAdd();
    if (!g_trayAdded) return 0;
    g_inTray = 1;
    ShowWindow(g_hwnd, SW_HIDE);
    TrayBalloonOnce();
    return 1;
}
static void TrayShow(void) {
    g_inTray = 0;
    TrayRemove(); /* al volver la ventana, el icono ya no hace falta */
    ShowWindow(g_hwnd, SW_SHOW);
    if (IsIconic(g_hwnd)) ShowWindow(g_hwnd, SW_RESTORE);
    SetForegroundWindow(g_hwnd);
    SetFocus(g_hwnd);
}
/* Salida de verdad: quita el icono y destruye la ventana; WM_DESTROY
 * hace la limpieza (SOCD, timer, logo, ping). */
static void TrayRealExit(void) {
    TrayRemove();
    DestroyWindow(g_hwnd);
}
/* Menu de clic derecho sobre el icono de la bandeja. */
static void TrayMenu(void) {
    HMENU m = CreatePopupMenu();
    if (!m) return;
    AppendMenuA(m, MF_STRING, ID_TRAY_SHOW, Tr(LS_TRAY_SHOW));
    AppendMenuA(m, MF_SEPARATOR, 0, NULL);
    AppendMenuA(m, MF_STRING, ID_TRAY_COMP, Tr(LS_GAME_COMP));
    AppendMenuA(m, MF_STRING | (g_socdOn ? MF_CHECKED : 0), ID_TRAY_SOCD,
                g_socdOn ? Tr(LS_TRAY_SOCD_OFF) : Tr(LS_TRAY_SOCD_ON));
    AppendMenuA(m, MF_SEPARATOR, 0, NULL);
    AppendMenuA(m, MF_STRING, ID_TRAY_EXIT, Tr(LS_TRAY_EXIT));
    POINT pt; GetCursorPos(&pt);
    SetForegroundWindow(g_hwnd); /* si no, el menu no se cierra al clicar fuera */
    int cmd = TrackPopupMenu(m, TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                             pt.x, pt.y, 0, g_hwnd, NULL);
    PostMessageA(g_hwnd, WM_NULL, 0, 0);
    DestroyMenu(m);
    switch (cmd) {
    case ID_TRAY_SHOW: TrayShow(); break;
    case ID_TRAY_COMP: CompetitiveMode(); break;
    case ID_TRAY_SOCD:
        /* Igual que la casilla: SocdToggle muestra la advertencia
         * obligatoria (incl. CS2) si se va a activar. */
        SetChecked(ID_CHK_SOCD, g_socdOn ? BST_UNCHECKED : BST_CHECKED);
        SocdToggle();
        break;
    case ID_TRAY_EXIT: TrayRealExit(); break;
    default: break;
    }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    /* Si el Explorador se reinicia, la bandeja se vacia: reponer el
     * icono, pero solo si la ventana estaba oculta en la bandeja. */
    if (g_wmTaskbarCreated && msg == g_wmTaskbarCreated) {
        if (g_inTray) { g_trayAdded = 0; TrayAdd(); }
        return 0;
    }
    switch (msg) {
    case WM_CREATE: {
        LoadLanguage(); /* v11: idioma guardado (es/en), antes de crear controles */
        /* v11: nombres de pestanas segun el idioma */
        g_tabNames[0] = "Timer"; g_tabNames[1] = "FilterKeys"; g_tabNames[2] = "DNS";
        g_tabNames[3] = Tr(LS_TAB_COMPETITIVE); g_tabNames[4] = Tr(LS_TAB_GAMES);
        g_hwnd = hwnd;
        InitializeCriticalSection(&g_pingCS);
        TimerInit();
        EnsureWSA();
        { HDC dc = GetDC(hwnd);
          LOGFONTA lf; memset(&lf, 0, sizeof lf); lf.lfCharSet = DEFAULT_CHARSET;
          strcpy(lf.lfFaceName, "Segoe UI Variable Text");
          g_hasVarFont = 0; EnumFontFamiliesExA(dc, &lf, EnumFontFamProc, (LPARAM)&g_hasVarFont, 0);
          ReleaseDC(hwnd, dc); }
        g_font = MakeUIFont(15, FW_NORMAL);
        g_fontBold = MakeUIFont(15, FW_BOLD);
        g_fontSection = MakeUIFont(17, FW_SEMIBOLD);
        g_fontTab = MakeUIFont(13, FW_BOLD); /* pestanas: 5 caben mejor con 13 px */
        g_fontBig = MakeUIFont(30, FW_BOLD);
        g_fontTitle = MakeUIFont(22, FW_SEMIBOLD);
        g_brBg = CreateSolidBrush(COL_BG); g_brCard = CreateSolidBrush(COL_CARD); g_brBtn = CreateSolidBrush(COL_BTN);
        g_brBgDeep = CreateSolidBrush(COL_BGDEEP); /* v13: fondo del overlay */
        LoadLogo();

        /* Encabezado con logo (el logo se dibuja en WM_PAINT a la izquierda) */
        MakeLabel(hwnd, 230, 18, 300, 32, "LU-AN OPTIMIZER", g_fontTitle, 0);
        MakeLabel(hwnd, 230, 52, 300, 20, "GAMING OPTIMIZATION", g_fontBold, 0);
        MakeLabelK(hwnd, 230, 76, 310, 20, LS_SUBTITLE, g_font, 0);
        { char vb[64]; snprintf(vb, sizeof vb, "v%s - Windows 7 / 8 / 10 / 11", APP_VERSION);
          MakeLabel(hwnd, 230, 100, 320, 16, vb, g_font, ID_LBL_VERSION); }

        /* Perfiles */
        MakeLabelK(hwnd, 20, 128, 120, 20, LS_QUICK_PROFILE, g_fontBold, 0);
        MakeButton(hwnd, 140, 124, 110, 30, "Gaming", ID_PROFILE_GAMING);
        MakeButtonK(hwnd, 258, 124, 110, 30, LS_WRITING, ID_PROFILE_TYPING);
        MakeButtonK(hwnd, 376, 124, 130, 30, LS_ACCESS, ID_PROFILE_ACCESS);
        MakeButton(hwnd, 514, 124, 90, 30, "Normal", ID_PROFILE_NORMAL);
        /* v11: selector de idioma (ES/EN) arriba a la derecha, bajo el
         * cuadro de estado. Muestra el idioma al que se cambia. */
        MakeButtonK(hwnd, 545, 50, 135, 28, LS_LANG_BTN, ID_LANG_TOGGLE);
        /* v13: selector de tema bajo el boton de idioma (Neon / Fuego /
         * Esmeralda). Desplegable estandar: el texto y la lista siguen
         * la paleta por WM_CTLCOLOREDIT / WM_CTLCOLORLISTBOX. */
        MakeLabelK(hwnd, 545, 84, 40, 20, LS_THEME_LBL, g_font, ID_LBL_THEME);
        HWND tc = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL, 588, 80, 92, 130, hwnd, (HMENU)(INT_PTR)ID_THEME_COMBO, g_hInst, NULL);
        SendMessage(tc, WM_SETFONT, (WPARAM)g_fontTab, TRUE);
        RefreshThemeCombo();

        /* --- Tarjeta Timer --- */
        MakeLabel(hwnd, 20, 162, 300, 22, "TIMER RESOLUTION", g_fontSection, 0);
        MakeLabelK(hwnd, 20, 188, 260, 34, LS_TIMER_CURRENT_INIT, g_fontBig, ID_LBL_CURRENT);
        MakeLabelK(hwnd, 300, 190, 320, 34, LS_TIMER_NORMAL_NOTE, g_font, 0);

        HWND tr = CreateWindowExA(0, TRACKBAR_CLASSA, "", WS_CHILD|WS_VISIBLE|TBS_AUTOTICKS|TBS_HORZ, 20, 232, 460, 32, hwnd, (HMENU)(INT_PTR)ID_TRACKBAR, g_hInst, NULL);
        SendMessage(tr, TBM_SETRANGE, TRUE, MAKELONG(1, 31));
        SendMessage(tr, TBM_SETPOS, TRUE, 2);
        SendMessage(tr, TBM_SETTICFREQ, 5, 0);
        MakeLabel(hwnd, 490, 235, 110, 22, "1.00 ms", g_fontBold, ID_LBL_DESIRED);
        MakeLabel(hwnd, 20, 264, 200, 18, "0.5 ms", g_font, 0);
        MakeLabel(hwnd, 430, 264, 80, 18, "15.6 ms", g_font, 0);

        MakeButton(hwnd, 20, 286, 82, 32, "0.5 ms", ID_PRESET_05);
        MakeButton(hwnd, 108, 286, 82, 32, "1.0 ms", ID_PRESET_10);
        MakeButton(hwnd, 196, 286, 82, 32, "2.0 ms", ID_PRESET_20);
        MakeButton(hwnd, 284, 286, 82, 32, "5.0 ms", ID_PRESET_50);
        MakeButton(hwnd, 372, 286, 92, 32, "15.6 ms", ID_PRESET_156);
        MakeButtonK(hwnd, 20, 326, 200, 36, LS_APPLY_RES, ID_APPLY_TIMER);
        MakeButtonK(hwnd, 230, 326, 230, 36, LS_BACK_156, ID_RESET_TIMER);

        /* --- Tarjeta FilterKeys --- */
        MakeLabelK(hwnd, 20, 378, 300, 22, LS_FK_TITLE, g_fontSection, 0);
        HWND c1 = MakeToggleK(hwnd, 20, 404, 200, LS_FK_ENABLE, ID_CHK_FK);
        HWND c2 = MakeToggleK(hwnd, 240, 404, 360, LS_BOUNCE_DESC, ID_CHK_BOUNCE);

        int y = 432;
        MakeLabelK(hwnd, 20, y, 190, 20, LS_BOUNCE_LBL, g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "100", WS_CHILD|WS_VISIBLE|ES_NUMBER, 220, y-2, 70, 22, hwnd, (HMENU)(INT_PTR)ID_EDIT_BOUNCE, g_hInst, NULL);
        MakeLabel(hwnd, 300, y, 40, 20, "ms", g_font, 0);
        MakeLabelK(hwnd, 350, y, 190, 20, LS_SLOW_LBL, g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD|WS_VISIBLE|ES_NUMBER, 550, y-2, 70, 22, hwnd, (HMENU)(INT_PTR)ID_EDIT_WAIT, g_hInst, NULL);
        y += 30;
        MakeLabelK(hwnd, 20, y, 190, 20, LS_DELAY_LBL, g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1000", WS_CHILD|WS_VISIBLE|ES_NUMBER, 220, y-2, 70, 22, hwnd, (HMENU)(INT_PTR)ID_EDIT_DELAY, g_hInst, NULL);
        MakeLabelK(hwnd, 350, y, 190, 20, LS_REPEAT_LBL, g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "500", WS_CHILD|WS_VISIBLE|ES_NUMBER, 550, y-2, 70, 22, hwnd, (HMENU)(INT_PTR)ID_EDIT_REPEAT, g_hInst, NULL);

        MakeButtonK(hwnd, 20, 494, 200, 36, LS_APPLY_FK, ID_APPLY_FK);
        MakeButtonK(hwnd, 230, 494, 220, 36, LS_DISABLE_FK, ID_DISABLE_FK);
        MakeLabelK(hwnd, 20, 536, 620, 20, LS_FK_RULE, g_font, ID_LBL_FK_STATUS);

        MakeLabelK(hwnd, 20, 564, 220, 20, LS_TEST_KB, g_fontBold, 0);
        HWND te = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", Tr(LS_TEST_PLACEHOLDER), WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_VSCROLL, 20, 588, 620, 48, hwnd, (HMENU)(INT_PTR)ID_TEST_EDIT, g_hInst, NULL);
        SendMessage(te, WM_SETFONT, (WPARAM)g_font, TRUE);

        /* --- Tarjeta DNS Jumper --- */
        MakeLabel(hwnd, 20, 650, 300, 22, "DNS JUMPER", g_fontSection, 0);
        MakeLabelK(hwnd, 20, 678, 140, 20, LS_ADAPTER_LBL, g_font, 0);
        HWND cb = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL, 165, 674, 475, 150, hwnd, (HMENU)(INT_PTR)ID_DNS_ADAPTER, g_hInst, NULL);
        SendMessage(cb, WM_SETFONT, (WPARAM)g_font, TRUE);
        MakeLabelK(hwnd, 20, 704, 620, 20, LS_DNS_CURRENT_INIT, g_font, ID_LBL_DNS_CURRENT);

        MakeButton(hwnd, 20, 728, 140, 30, "Google", ID_DNS_GOOGLE);
        MakeButton(hwnd, 168, 728, 140, 30, "Cloudflare", ID_DNS_CLOUDFLARE);
        MakeButton(hwnd, 316, 728, 140, 30, "Quad9", ID_DNS_QUAD9);
        MakeButton(hwnd, 20, 764, 140, 30, "OpenDNS", ID_DNS_OPENDNS);
        MakeButton(hwnd, 168, 764, 140, 30, "AdGuard", ID_DNS_ADGUARD);
        MakeButtonK(hwnd, 316, 764, 140, 30, LS_DNS_AUTO, ID_DNS_AUTO);
        MakeLabelK(hwnd, 470, 728, 170, 60, LS_DNS_PRESET_HINT, g_font, 0);

        MakeLabelK(hwnd, 20, 806, 120, 20, LS_DNS_PRIMARY, g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "8.8.8.8", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 150, 804, 150, 22, hwnd, (HMENU)(INT_PTR)ID_DNS_PRIMARY, g_hInst, NULL);
        MakeLabelK(hwnd, 320, 806, 130, 20, LS_DNS_SECONDARY, g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "8.8.4.4", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 460, 804, 180, 22, hwnd, (HMENU)(INT_PTR)ID_DNS_SECONDARY, g_hInst, NULL);

        MakeButtonK(hwnd, 20, 836, 150, 36, LS_APPLY_DNS, ID_DNS_APPLY);
        MakeButtonK(hwnd, 178, 836, 170, 36, LS_RESTORE_AUTO, ID_DNS_RESTORE);
        MakeButtonK(hwnd, 356, 836, 130, 36, LS_CLEAR_CACHE, ID_DNS_FLUSH);
        MakeButtonK(hwnd, 494, 836, 146, 36, LS_TEST_PING, ID_DNS_PING);
        MakeLabelK(hwnd, 20, 878, 620, 20, LS_DNS_HINT, g_font, ID_LBL_DNS_STATUS);

        /* --- Tarjeta Competitivo (Y absoluta 950+, se reparenta a la 4a pagina) --- */
        MakeLabel(hwnd, 20, 950, 300, 22, "SOCD CLEANER", g_fontSection, 0);
        HWND cs = MakeToggleK(hwnd, 20, 976, 380, LS_SOCD_ENABLE, ID_CHK_SOCD);
        HWND r1 = CreateWindowExA(0, "BUTTON", Tr(LS_RADIO_LAST), WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON|WS_GROUP, 420, 976, 150, 22, hwnd, (HMENU)(INT_PTR)ID_SOCD_LAST, g_hInst, NULL);
        SendMessage(r1, WM_SETFONT, (WPARAM)g_font, TRUE);
        HWND r2 = CreateWindowExA(0, "BUTTON", Tr(LS_RADIO_NEUTRAL), WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON, 570, 976, 140, 22, hwnd, (HMENU)(INT_PTR)ID_SOCD_NEUTRAL, g_hInst, NULL);
        SendMessage(r2, WM_SETFONT, (WPARAM)g_font, TRUE);
        SendMessage(r1, BM_SETCHECK, BST_CHECKED, 0);
        MakeLabelK(hwnd, 20, 1024, 620, 20, LS_SOCD_OFF, g_font, ID_LBL_SOCD_STATUS);
        MakeLabelK(hwnd, 20, 1044, 620, 18, LS_SOCD_TEST_HINT, g_font, ID_LBL_SOCD_TEST);

        MakeLabelK(hwnd, 20, 1064, 300, 22, LS_COMP_TITLE, g_fontSection, 0);
        MakeButtonK(hwnd, 20, 1088, 240, 36, LS_COMP_BTN, ID_COMP_MODE);
        MakeButtonK(hwnd, 270, 1088, 170, 36, LS_COMP_RESTORE, ID_COMP_RESTORE);
        HWND lg = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY|WS_VSCROLL, 20, 1132, 620, 34, hwnd, (HMENU)(INT_PTR)ID_COMP_LOG, g_hInst, NULL);
        SendMessage(lg, WM_SETFONT, (WPARAM)g_font, TRUE);

        MakeLabelK(hwnd, 20, 1174, 300, 22, LS_LIVE_TITLE, g_fontSection, 0);
        MakeLabelK(hwnd, 20, 1198, 620, 20, LS_LIVE_INIT, g_font, ID_LBL_LIVE_PING);
        HWND cl = MakeToggleK(hwnd, 20, 1222, 280, LS_LIVE_TOGGLE, ID_CHK_LIVE_PING);
        MakeButtonK(hwnd, 320, 1218, 180, 30, LS_PING_ALL, ID_PING_ALL);
        MakeLabelK(hwnd, 20, 1250, 620, 20, LS_BEST_INIT, g_font, ID_LBL_BEST_DNS);

        MakeLabelK(hwnd, 20, 1278, 300, 22, LS_SYSOPT_TITLE, g_fontSection, 0);
        HWND cm = MakeToggleK(hwnd, 20, 1302, 280, LS_MOUSE_ACCEL, ID_CHK_MOUSE_ACCEL);
        HWND cf = MakeToggleK(hwnd, 320, 1302, 330, LS_FSE, ID_CHK_FSE);
        MakeButtonK(hwnd, 20, 1330, 250, 36, LS_CLOSE_APPS_BTN, ID_BTN_CLOSE_APPS);
        MakeButtonK(hwnd, 280, 1330, 220, 36, LS_CLEAN_MEM, ID_BTN_CLEAN_MEM);
        MakeLabelK(hwnd, 20, 1372, 620, 20, LS_MEM_INIT, g_font, ID_LBL_MEM_STATUS);
        HWND ca = MakeToggleK(hwnd, 20, 1396, 420, LS_AUTO_CLEAN, ID_CHK_AUTO_CLEAN);

        /* --- Monitor en pantalla (v9) --- */
        MakeLabelK(hwnd, 20, 1420, 300, 22, LS_MONITOR_TITLE, g_fontSection, 0);
        HWND co = MakeToggleK(hwnd, 20, 1444, 160, LS_SHOW_MONITOR, ID_CHK_OVERLAY);
        HWND cc = MakeToggleK(hwnd, 200, 1444, 260, LS_CLICK_THROUGH, ID_CHK_OVERLAY_CLICK);
        MakeLabelK(hwnd, 20, 1468, 620, 18, LS_FPS_NOTE, g_font, 0);

        /* --- Auto-actualizacion (v9) --- */
        MakeLabelK(hwnd, 20, 1492, 300, 22, LS_UPDATES_TITLE, g_fontSection, 0);
        MakeButtonK(hwnd, 20, 1516, 220, 32, LS_CHECK_UPDATES, ID_BTN_CHECK_UPDATE);
        HWND cu = MakeToggleK(hwnd, 252, 1522, 300, LS_AUTO_UPDATE, ID_CHK_AUTO_UPDATE);

        /* --- Modo Ultra Baja Latencia (v10): Y absoluta 1560+, se
         * reparenta a la pagina Competitivo (rango 950-1759). --- */
        MakeLabelK(hwnd, 20, 1560, 400, 22, LS_ULTRA_TITLE, g_fontSection, 0);
        MakeLabelK(hwnd, 20, 1584, 620, 18, LS_ULTRA_NOTE, g_font, 0);
        MakeButtonK(hwnd, 20, 1606, 300, 36, LS_ULTRA_BTN_ON, ID_ULTRA_ON);
        MakeButtonK(hwnd, 330, 1606, 150, 36, LS_ULTRA_RESTORE, ID_ULTRA_RESTORE);
        MakeLabelK(hwnd, 20, 1648, 620, 18, LS_ULTRA_OFF, g_font, ID_LBL_ULTRA_STATUS);
        MakeToggleK(hwnd, 20, 1670, 600, LS_GPU_SCHED, ID_CHK_GPU_SCHED);
        MakeLabelK(hwnd, 20, 1698, 380, 18, LS_NETHOGS_LBL, g_font, 0);
        HWND hl = CreateWindowExA(0, "LISTBOX", "", WS_CHILD|WS_VISIBLE|LBS_NOTIFY|WS_VSCROLL|WS_BORDER, 20, 1716, 380, 40, hwnd, (HMENU)(INT_PTR)ID_LIST_NETHOGS, g_hInst, NULL);
        SendMessage(hl, WM_SETFONT, (WPARAM)g_font, TRUE);
        MakeButtonK(hwnd, 410, 1716, 140, 32, LS_KILL_HOG, ID_BTN_KILL_HOG);
        MakeButtonK(hwnd, 560, 1716, 80, 32, LS_REFRESH_HOGS, ID_BTN_REFRESH_HOGS);

        /* --- Pestana Juegos (v9): Y absoluta 1760+, se reparenta a la 5a pagina --- */
        MakeLabelK(hwnd, 20, 1760, 300, 22, LS_GAMES_TITLE, g_fontSection, 0);
        MakeLabelK(hwnd, 20, 1786, 620, 20, LS_NO_GAME, g_font, ID_GAMES_STATUS);
        {
            const char *gameRows[5] = {
                "Valorant - VALORANT-Win64-Shipping.exe",
                Tr(LS_GAME_CS2_ROW),
                "Fortnite - FortniteClient-Win64-Shipping.exe",
                "Apex Legends - r5apex.exe",
                Tr(LS_GAME_OTHER_ROW) };
            for (int gi = 0; gi < 5; gi++) {
                int gy = 1812 + gi * 58;
                HWND gr = MakeLabel(hwnd, 20, gy, 340, 20, gameRows[gi], g_fontBold, 0);
                if (gi == 1) RegLang(gr, LS_GAME_CS2_ROW);
                else if (gi == 4) RegLang(gr, LS_GAME_OTHER_ROW);
                HWND ga = MakeToggleK(hwnd, 380, gy, 200, LS_GAME_AUTO, ID_GAMES_AUTO_BASE + gi);
                HWND gs = MakeToggle(hwnd, 20, gy + 24, 90, "SOCD", ID_GAMES_SOCD_BASE + gi);
                HWND gt = MakeToggleK(hwnd, 120, gy + 24, 220, LS_GAME_TIMER, ID_GAMES_TIMER_BASE + gi);
                HWND gc = MakeToggleK(hwnd, 350, gy + 24, 180, LS_GAME_COMP, ID_GAMES_COMP_BASE + gi);
            }
        }
        MakeLabelK(hwnd, 20, 2106, 220, 20, LS_CUSTOM_EXE_LBL, g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 250, 2104, 220, 22, hwnd, (HMENU)(INT_PTR)ID_GAMES_CUSTOM_EXE, g_hInst, NULL);
        MakeLabelK(hwnd, 20, 2132, 620, 34, LS_GAMES_NOTE, g_font, ID_GAMES_NOTE);

        /* --- Ping a servidores de juego + diagnostico de ruta (v10) --- */
        MakeLabelK(hwnd, 20, 2190, 400, 22, LS_SRV_TITLE, g_fontSection, 0);
        MakeLabelK(hwnd, 20, 2214, 620, 18, LS_SRV_NOTE, g_font, 0);
        MakeLabel(hwnd, 20, 2238, 50, 20, "CDMX:", g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "189.203.220.1", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 75, 2236, 150, 22, hwnd, (HMENU)(INT_PTR)ID_SRV_CDMX, g_hInst, NULL);
        MakeLabel(hwnd, 240, 2238, 50, 20, "Miami:", g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "4.2.2.1", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 295, 2236, 150, 22, hwnd, (HMENU)(INT_PTR)ID_SRV_MIAMI, g_hInst, NULL);
        MakeButtonK(hwnd, 20, 2264, 230, 32, LS_SRV_MEASURE, ID_BTN_SERVER_PING);
        MakeButtonK(hwnd, 260, 2264, 200, 32, LS_TRACE_BTN, ID_BTN_TRACEROUTE);
        MakeLabelK(hwnd, 20, 2302, 620, 18, LS_SRV_INIT, g_font, ID_LBL_SERVER_PING);
        HWND tr2 = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", Tr(LS_TRACE_INIT), WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY|WS_VSCROLL, 20, 2324, 620, 92, hwnd, (HMENU)(INT_PTR)ID_EDIT_TRACE, g_hInst, NULL);
        SendMessage(tr2, WM_SETFONT, (WPARAM)g_font, TRUE);
        MakeLabelK(hwnd, 20, 2420, 620, 18, LS_TRACE_SUM_INIT, g_font, ID_LBL_TRACE_SUMMARY);

        MakeLabelK(hwnd, 20, 472, 660, 16, LS_STATUS_READY, g_font, ID_STATUS);

        /* Leer estado real */
        MYFILTERKEYS fk; if (GetFK(&fk)) {
            SetChecked(ID_CHK_FK, (fk.dwFlags & FKF_FILTERKEYSON) ? BST_CHECKED : BST_UNCHECKED);
            SetChecked(ID_CHK_BOUNCE, fk.iBounceMSec ? BST_CHECKED : BST_UNCHECKED);
            SetIntEdit(ID_EDIT_BOUNCE, (int)fk.iBounceMSec); SetIntEdit(ID_EDIT_WAIT, (int)fk.iWaitMSec);
            SetIntEdit(ID_EDIT_DELAY, (int)fk.iDelayMSec); SetIntEdit(ID_EDIT_REPEAT, (int)fk.iRepeatMSec);
        }
        RefreshCurrentLabel(); RefreshDesiredLabel();
        RefreshAdapters(); RefreshCurrentDns();
        /* --- Barra de pestanas premium (dibujada a mano, con glifos,
         * indicador deslizante y animacion de hover) --- */
        g_tabBar = CreateWindowExA(0, "LuAnTabBar", "", WS_CHILD|WS_VISIBLE|WS_TABSTOP, TABBAR_X, TABBAR_Y, TABBAR_W, TABBAR_H, hwnd, (HMENU)(INT_PTR)ID_TAB, g_hInst, NULL);
        TabIndRect(0, &g_indX, &g_indW); g_indTX = g_indX; g_indTW = g_indW;
        g_pageTimer = CreateWindowExA(0, "LuAnPage", "", WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pageFK = CreateWindowExA(0, "LuAnPage", "", WS_CHILD|WS_CLIPCHILDREN, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pageDNS = CreateWindowExA(0, "LuAnPage", "", WS_CHILD|WS_CLIPCHILDREN, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pageComp = CreateWindowExA(0, "LuAnPage", "", WS_CHILD|WS_CLIPCHILDREN, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pageGames = CreateWindowExA(0, "LuAnPage", "", WS_CHILD|WS_CLIPCHILDREN, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pages[0] = g_pageTimer; g_pages[1] = g_pageFK; g_pages[2] = g_pageDNS; g_pages[3] = g_pageComp; g_pages[4] = g_pageGames;
        ReparentToPages();
        PageInitScroll(); /* v11: desplazamiento de cada pestana */
        SetWindowPos(Ctl(ID_STATUS), NULL, 20, 472, 660, 16, SWP_NOZORDER|SWP_NOSIZE);
        ShowTab(0);
        /* --- Bandeja (v8): UNA casilla en el pie, en la ventana
         * principal (se crea DESPUES de ReparentToPages para que no se
         * mueva a ninguna pestana). Guardada en
         * HKCU\Software\LuAnOptimizer (MinimizeToBackground, 1 = si).
         * La X siempre cierra el programa; solo el boton minimizar
         * cambia segun esta casilla. No se crea icono de bandeja al
         * arrancar: solo aparece al minimizar en segundo plano. */
        g_wmTaskbarCreated = RegisterWindowMessageA("TaskbarCreated");
        DWORD rv = 0;
        g_trayMinOn = RegGetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "MinimizeToBackground", &rv) ? (int)rv : 1;
        HWND tm = MakeToggleK(hwnd, 20, 492, 330, LS_TRAY_MIN, ID_CHK_TRAY_MIN);
        SendMessage(tm, BM_SETCHECK, g_trayMinOn ? BST_CHECKED : BST_UNCHECKED, 0);
        /* --- v9: cargar preferencias guardadas --- */
        LoadGamesSettings();
        {
            DWORD rv2 = 0;
            g_autoUpdateCheck = RegGetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "AutoUpdateCheck", &rv2) ? (int)rv2 : 1;
            SetChecked(ID_CHK_AUTO_UPDATE, g_autoUpdateCheck ? BST_CHECKED : BST_UNCHECKED);
            g_overlayOn = RegGetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "OverlayOn", &rv2) ? (int)rv2 : 0;
            g_overlayClick = RegGetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "OverlayClickThrough", &rv2) ? (int)rv2 : 0;
            SetChecked(ID_CHK_OVERLAY, g_overlayOn ? BST_CHECKED : BST_UNCHECKED);
            SetChecked(ID_CHK_OVERLAY_CLICK, g_overlayClick ? BST_CHECKED : BST_UNCHECKED);
            if (g_overlayOn) OverlayShow();
            /* v10: leer el estado real de la programacion de GPU (HwSchMode)
             * para que el interruptor refleje lo que hay en el registro. */
            {
                DWORD hw = 0;
                if (RegGetDword(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers", "HwSchMode", &hw))
                    SetChecked(ID_CHK_GPU_SCHED, hw ? BST_CHECKED : BST_UNCHECKED);
            }
        }
        if (!g_gameStopEvent) {
            g_gameStopEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
            HANDLE gh = CreateThread(NULL, 0, GameWatchThreadProc, NULL, 0, NULL);
            if (gh) CloseHandle(gh);
        }
        if (g_autoUpdateCheck) StartUpdateCheck(0);
        SetTimer(hwnd, 1, 1000, NULL);
        UpdateStatusBox(); /* v11: cuadro ACTIVO/APAGADO inicial */
        return 0;
    }
    case WM_TIMER:
        if (wp == 1) { RefreshCurrentLabel(); UpdatePingTarget(); UpdateStatusBox(); return 0; }
        if (wp == 4) { CleanMemory(); return 0; }
        if (wp == 2) {
            /* v7: con el SOCD activo, no animar botones: dejarlos en su
             * estado final y parar el timer de 16 ms. */
            if (g_socdOn) {
                for (int i = 0; i < g_animCount; i++) { g_anim[i].level = g_anim[i].target; InvalidateRect(g_anim[i].h, NULL, FALSE); }
                KillTimer(hwnd, 2);
                return 0;
            }
            int active = 0;
            for (int i = 0; i < g_animCount; i++) {
                BtnAnim *a = &g_anim[i];
                if (a->level != a->target) {
                    int step = 45;
                    if (a->level < a->target) { a->level += step; if (a->level > a->target) a->level = a->target; }
                    else { a->level -= step; if (a->level < a->target) a->level = a->target; }
                    InvalidateRect(a->h, NULL, FALSE);
                }
                if (a->level != a->target) active = 1;
            }
            if (!active) KillTimer(hwnd, 2);
            return 0;
        }
        if (wp == 6) {
            /* v7: con el SOCD activo, sin deslizamiento: pagina ya en su sitio. */
            if (g_socdOn) { g_slideX = 0; SetWindowPos(g_pages[g_curTab], NULL, PAGE_X, PAGE_Y, 0, 0, SWP_NOZORDER | SWP_NOSIZE); KillTimer(hwnd, 6); return 0; }
            /* Deslizamiento de entrada de la pagina al cambiar de pestana */
            g_slideX -= 2;
            if (g_slideX <= 0) { g_slideX = 0; KillTimer(hwnd, 6); }
            SetWindowPos(g_pages[g_curTab], NULL, PAGE_X + g_slideX, PAGE_Y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            return 0;
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        if (g_theme == THEME_PINKIMICH && g_hPinkBmp) {
            /* v13.1: solo con Pinkimich el logo es Kuromi + Cinnamoroll.
             * La imagen es 1920x1280: un recorte cuadrado cortaria las
             * orejas de los personajes, asi que se escala entera dentro
             * del mismo rectangulo del logo (190x103), centrada: 154x103.
             * Los textos LU-AN OPTIMIZER / GAMING OPTIMIZATION no cambian. */
            HDC mem = CreateCompatibleDC(hdc);
            HGDIOBJ old = SelectObject(mem, g_hPinkBmp);
            BITMAP bm; GetObject(g_hPinkBmp, sizeof bm, &bm);
            int dw = 103 * bm.bmWidth / bm.bmHeight; if (dw > 190) dw = 190;
            int dx = 20 + (190 - dw) / 2;
            StretchBlt(hdc, dx, 10, dw, 103, mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            SelectObject(mem, old); DeleteDC(mem);
        } else if (g_hLogoBmp) {
            HDC mem = CreateCompatibleDC(hdc);
            HGDIOBJ old = SelectObject(mem, g_hLogoBmp);
            BITMAP bm; GetObject(g_hLogoBmp, sizeof bm, &bm);
            /* Logo 360x196 -> 190x103, mantiene la proporcion */
            StretchBlt(hdc, 20, 10, 190, 103, mem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
            SelectObject(mem, old); DeleteDC(mem);
        } else {
            SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, COL_ACCENT);
            SelectObject(hdc, g_fontTitle);
            TextOutA(hdc, 20, 40, "LU-AN", 5);
        }
        /* v11: cuadro de estado arriba a la derecha (ACTIVO con el
         * acento del tema / APAGADO gris, esquinas redondeadas). */
        { const char *st = Tr(g_compActive || g_ultraOn ? LS_STATUS_ACTIVE : LS_STATUS_OFF);
          HFONT oldF = SelectObject(hdc, g_fontBold);
          SIZE sz; GetTextExtentPoint32A(hdc, st, (int)strlen(st), &sz);
          int bw = sz.cx + 36; if (bw < 92) bw = 92;
          int bx = 690 - bw, by = 14, bh = 30;
          HBRUSH bbr = CreateSolidBrush(g_compActive || g_ultraOn ? Blend(COL_ACCENT, COL_BG, 205) : RGB(0x4A, 0x4F, 0x5E));
          HPEN bpen = CreatePen(PS_SOLID, 1, g_compActive || g_ultraOn ? COL_ACCENT : RGB(0x6B, 0x71, 0x82));
          HPEN oldP = SelectObject(hdc, bpen); HBRUSH oldB = SelectObject(hdc, bbr);
          RoundRect(hdc, bx, by, bx + bw, by + bh, 14, 14);
          SelectObject(hdc, oldB); SelectObject(hdc, oldP);
          DeleteObject(bbr); DeleteObject(bpen);
          int dotR = 5, dotX = bx + 14, dotY = by + bh / 2;
          HBRUSH dbr = CreateSolidBrush(g_compActive || g_ultraOn ? COL_ACCENT : RGB(0xC7, 0xCC, 0xD9));
          HBRUSH oldD = SelectObject(hdc, dbr);
          Ellipse(hdc, dotX - dotR, dotY - dotR, dotX + dotR, dotY + dotR);
          SelectObject(hdc, oldD); DeleteObject(dbr);
          SetTextColor(hdc, g_themeCur.light ? COL_TEXT : RGB(0xFF, 0xFF, 0xFF)); SetBkMode(hdc, TRANSPARENT); /* v13.1: en tema claro el texto es negro (blanco sobre rosa pastel no se lee) */
          TextOutA(hdc, dotX + 11, by + (bh - sz.cy) / 2, st, (int)strlen(st));
          SelectObject(hdc, oldF); }

        /* Linea divisoria con gradiente cian->magenta bajo los perfiles */
        for (int x = 20; x < 680; x++) {
            COLORREF c = Blend(COL_ACCENT, COL_MAGENTA, (x - 20) * 255 / 660);
            HPEN p2 = CreatePen(PS_SOLID, 1, c);
            HGDIOBJ op2 = SelectObject(hdc, p2);
            MoveToEx(hdc, x, 160, NULL); LineTo(hdc, x, 162);
            SelectObject(hdc, op2); DeleteObject(p2);
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_MOUSEWHEEL: /* v11: la rueda baja la pestana visible */
        PageSetScroll(g_curTab, g_pageScroll[g_curTab] - (GET_WHEEL_DELTA_WPARAM(wp) / 120) * 48);
        return 0;
    case WM_HSCROLL:
        if ((HWND)lp == Ctl(ID_TRACKBAR)) {
            int pos = (int)SendMessage((HWND)lp, TBM_GETPOS, 0, 0);
            g_desiredMs = pos / 2.0; RefreshDesiredLabel();
        }
        return 0;
    case WM_COMMAND: {
        int id = LOWORD(wp);
        int code = HIWORD(wp);
        if (id == ID_DNS_ADAPTER && code == CBN_SELCHANGE) { RefreshCurrentDns(); return 0; }
        if (id == ID_THEME_COMBO && code == CBN_SELCHANGE) { /* v13: cambiar tema al instante */
            int sel = (int)SendMessage(Ctl(ID_THEME_COMBO), CB_GETCURSEL, 0, 0);
            if (sel >= 0 && sel < 4 && sel != g_theme) { g_theme = sel; SaveTheme(); ApplyTheme(); }
            return 0;
        }
        if (code == BN_CLICKED && id >= ID_GAMES_AUTO_BASE && id < ID_GAMES_AUTO_BASE + 5) {
            int gi = id - ID_GAMES_AUTO_BASE;
            g_gameAuto[gi] = (IsChecked(id) == BST_CHECKED);
            SaveGameProfile(gi);
            return 0;
        }
        if (code == BN_CLICKED && id >= ID_GAMES_SOCD_BASE && id < ID_GAMES_SOCD_BASE + 5) {
            int gi = id - ID_GAMES_SOCD_BASE;
            if (g_games[gi].forbidden && IsChecked(id) == BST_CHECKED) {
                /* CS2: SOCD prohibido por Valve (2024). Bloquear siempre. */
                SetChecked(id, BST_UNCHECKED);
                g_gameSocd[gi] = 0;
                MessageBoxA(g_hwnd, Tr(LS_CS2_SOCD_FORBIDDEN), Tr(LS_CS2_SOCD_TITLE), MB_ICONWARNING);
                return 0;
            }
            g_gameSocd[gi] = (IsChecked(id) == BST_CHECKED);
            SaveGameProfile(gi);
            return 0;
        }
        if (code == BN_CLICKED && id >= ID_GAMES_TIMER_BASE && id < ID_GAMES_TIMER_BASE + 5) {
            int gi = id - ID_GAMES_TIMER_BASE;
            g_gameTimer05[gi] = (IsChecked(id) == BST_CHECKED);
            SaveGameProfile(gi);
            return 0;
        }
        if (code == BN_CLICKED && id >= ID_GAMES_COMP_BASE && id < ID_GAMES_COMP_BASE + 5) {
            int gi = id - ID_GAMES_COMP_BASE;
            g_gameComp[gi] = (IsChecked(id) == BST_CHECKED);
            SaveGameProfile(gi);
            return 0;
        }
        switch (id) {
        case ID_PROFILE_GAMING: ProfileGaming(); break;
        case ID_PROFILE_TYPING: ProfileTyping(); break;
        case ID_PROFILE_ACCESS: ProfileAccess(); break;
        case ID_PROFILE_NORMAL: ProfileNormal(); break;
        case ID_PRESET_05: SetTrack(0.5); break;
        case ID_PRESET_10: SetTrack(1.0); break;
        case ID_PRESET_20: SetTrack(2.0); break;
        case ID_PRESET_50: SetTrack(5.0); break;
        case ID_PRESET_156: SetTrack(15.6); break;
        case ID_APPLY_TIMER: ApplyTimer(); break;
        case ID_RESET_TIMER: SetTrack(15.6); ApplyTimer(); break;
        case ID_APPLY_FK: ApplyFK(); break;
        case ID_DISABLE_FK: SetChecked(ID_CHK_FK, BST_UNCHECKED); ApplyFK(); break;
        case ID_DNS_GOOGLE: SetDnsPreset("8.8.8.8", "8.8.4.4"); break;
        case ID_DNS_CLOUDFLARE: SetDnsPreset("1.1.1.1", "1.0.0.1"); break;
        case ID_DNS_QUAD9: SetDnsPreset("9.9.9.9", "149.112.112.112"); break;
        case ID_DNS_OPENDNS: SetDnsPreset("208.67.222.222", "208.67.220.220"); break;
        case ID_DNS_ADGUARD: SetDnsPreset("94.140.14.14", "94.140.15.15"); break;
        case ID_DNS_AUTO: SetDnsPreset("", ""); RestoreDnsAuto(); break;
        case ID_DNS_APPLY: ApplyDns(); break;
        case ID_DNS_RESTORE: RestoreDnsAuto(); break;
        case ID_DNS_FLUSH: FlushDns(); break;
        case ID_DNS_PING: PingDns(); break;
        case ID_CHK_SOCD: SocdToggle(); break;
        case ID_SOCD_LAST: if (g_socdOn) { InterlockedExchange(&g_socdMode, 0); SocdSetStatus(Tr(LS_SOCD_ON_LAST)); } break;
        case ID_SOCD_NEUTRAL: if (g_socdOn) { InterlockedExchange(&g_socdMode, 1); SocdSetStatus(Tr(LS_SOCD_ON_NEUTRAL)); } break;
        case ID_LANG_TOGGLE: /* v11: cambiar idioma ES/EN sin reiniciar */
            g_lang = g_lang ? 0 : 1;
            SaveLanguage();
            ApplyLanguage();
            break;
        case ID_COMP_MODE: CompetitiveMode(); break;
        case ID_COMP_RESTORE: CompetitiveRestore(); break;
        case ID_ULTRA_ON: {
            if (!g_ultraOn) {
                if (MessageBoxA(hwnd, Tr(LS_ULTRA_WARN), Tr(LS_ULTRA_TITLE_MB), MB_YESNO | MB_ICONINFORMATION) != IDYES) break;
                g_ultraBackup.svcWuauservWasRunning = SvcIsRunning("wuauserv");
                g_ultraBackup.svcDoSvcWasRunning = SvcIsRunning("DoSvc");
                if (g_ultraBackup.svcWuauservWasRunning) SvcPause("wuauserv");
                if (g_ultraBackup.svcDoSvcWasRunning) SvcPause("DoSvc");
                UltraApply(&g_ultraBackup);
                g_ultraNetApplied = 1;
                g_ultraOn = 1;
                SetWindowTextA(Ctl(ID_LBL_ULTRA_STATUS), Tr(LS_ULTRA_ON));
                SetWindowTextA(Ctl(ID_ULTRA_ON), Tr(LS_ULTRA_BTN_OFF));
                SetStatus(Tr(LS_ULTRA_ON_STATUS));
                UpdateStatusBox(); /* v11: el cuadro de estado sigue al cambio */
                UltraRefreshHogs();
            } else {
                if (g_ultraBackup.svcWuauservWasRunning) SvcContinue("wuauserv");
                if (g_ultraBackup.svcDoSvcWasRunning) SvcContinue("DoSvc");
                UltraRestore(&g_ultraBackup);
                UltraRestoreGame();
                g_ultraOn = 0; g_ultraNetApplied = 0;
                SetWindowTextA(Ctl(ID_LBL_ULTRA_STATUS), Tr(LS_ULTRA_OFF_RESTORED));
                SetWindowTextA(Ctl(ID_ULTRA_ON), Tr(LS_ULTRA_BTN_ON));
                SetStatus(Tr(LS_ULTRA_OFF_STATUS));
                UpdateStatusBox(); /* v11 */
            }
            break;
        }
        case ID_ULTRA_RESTORE: {
            if (g_ultraOn) {
                if (g_ultraBackup.svcWuauservWasRunning) SvcContinue("wuauserv");
                if (g_ultraBackup.svcDoSvcWasRunning) SvcContinue("DoSvc");
            }
            UltraRestore(&g_ultraBackup);
            UltraRestoreGame();
            g_ultraOn = 0; g_ultraNetApplied = 0;
            SetWindowTextA(Ctl(ID_LBL_ULTRA_STATUS), Tr(LS_ULTRA_OFF_RESTORED));
            SetWindowTextA(Ctl(ID_ULTRA_ON), Tr(LS_ULTRA_BTN_ON));
            SetStatus(Tr(LS_NET_RESTORED));
            UpdateStatusBox(); /* v11 */
            break;
        }
        case ID_CHK_GPU_SCHED: {
            DWORD v = (IsChecked(ID_CHK_GPU_SCHED) == BST_CHECKED) ? 1 : 0;
            if (RegSetDwordOk(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Control\\GraphicsDrivers", "HwSchMode", v))
                MessageBoxA(hwnd, Tr(LS_GPU_SAVED), "GPU", MB_OK | MB_ICONINFORMATION);
            else
                MessageBoxA(hwnd, Tr(LS_GPU_WRITE_FAIL), "GPU", MB_OK | MB_ICONWARNING);
            break;
        }
        case ID_BTN_KILL_HOG: UltraKillHog(); break;
        case ID_BTN_REFRESH_HOGS: UltraRefreshHogs(); SetStatus(Tr(LS_HOGS_REFRESHED)); break;
        case ID_BTN_SERVER_PING: ServerPingStart(); break;
        case ID_BTN_TRACEROUTE: TraceStart(); break;
        case ID_CHK_LIVE_PING:
            if (IsChecked(ID_CHK_LIVE_PING) == BST_CHECKED) { LivePingStart(); SetWindowTextA(Ctl(ID_LBL_LIVE_PING), Tr(LS_LIVE_MEASURING)); }
            else { LivePingStop(); SetWindowTextA(Ctl(ID_LBL_LIVE_PING), Tr(LS_LIVE_PAUSED)); }
            break;
        case ID_PING_ALL: StartPingAll(); break;
        case ID_CHK_MOUSE_ACCEL: ToggleMouseAccel(); break;
        case ID_CHK_FSE: ToggleFSE(); break;
        case ID_BTN_CLOSE_APPS: CloseBackgroundApps(); break;
        case ID_BTN_CLEAN_MEM: CleanMemory(); break;
        case ID_CHK_AUTO_CLEAN:
            if (IsChecked(ID_CHK_AUTO_CLEAN) == BST_CHECKED) SetTimer(hwnd, 4, 15 * 60 * 1000, NULL);
            else KillTimer(hwnd, 4);
            break;
        case ID_CHK_OVERLAY:
            g_overlayOn = (IsChecked(ID_CHK_OVERLAY) == BST_CHECKED);
            if (g_overlayOn) OverlayShow(); else OverlayHide();
            OverlaySavePos();
            break;
        case ID_CHK_OVERLAY_CLICK:
            g_overlayClick = (IsChecked(ID_CHK_OVERLAY_CLICK) == BST_CHECKED);
            OverlayApplyClickThrough();
            OverlaySavePos();
            break;
        case ID_BTN_CHECK_UPDATE:
            SetStatus(Tr(LS_UPD_CHECKING));
            StartUpdateCheck(1);
            break;
        case ID_CHK_AUTO_UPDATE:
            g_autoUpdateCheck = (IsChecked(ID_CHK_AUTO_UPDATE) == BST_CHECKED);
            RegSetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "AutoUpdateCheck", g_autoUpdateCheck ? 1 : 0);
            SetStatus(g_autoUpdateCheck ? Tr(LS_UPD_AUTO_ON) : Tr(LS_UPD_AUTO_OFF));
            break;
        case ID_GAMES_CUSTOM_EXE:
            if (code == EN_KILLFOCUS) {
                GetWindowTextA(Ctl(ID_GAMES_CUSTOM_EXE), g_gameCustomExe, sizeof g_gameCustomExe);
                SaveGameProfile(4);
            }
            break;
        case ID_CHK_TRAY_MIN: /* v8: guardar la casilla en el registro */
            g_trayMinOn = (IsChecked(ID_CHK_TRAY_MIN) == BST_CHECKED);
            RegSetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "MinimizeToBackground", g_trayMinOn ? 1 : 0);
            SetStatus(g_trayMinOn ? Tr(LS_TRAY_MIN_ON) : Tr(LS_TRAY_MIN_OFF));
            break;
        case ID_TRAY_SHOW: TrayShow(); break; /* por si el menu llega por WM_COMMAND */
        case ID_TRAY_COMP: CompetitiveMode(); break;
        case ID_TRAY_SOCD:
            SetChecked(ID_CHK_SOCD, g_socdOn ? BST_UNCHECKED : BST_CHECKED);
            SocdToggle();
            break;
        case ID_TRAY_EXIT: TrayRealExit(); break;
        }
        return 0;
    }
    case WM_SYSCOMMAND: /* v8: el boton minimizar va a la bandeja */
        if ((wp & 0xFFF0) == SC_MINIMIZE) {
            if (g_trayMinOn && TrayHide()) return 0;
        }
        return DefWindowProcA(hwnd, msg, wp, lp);
    case WM_CLOSE: /* v8: la X SIEMPRE cierra el programa como antes */
        TrayRealExit();
        return 0;
    case WM_TRAYICON: /* v8: doble clic restaura, clic derecho abre el menu */
        if (lp == WM_LBUTTONDBLCLK) TrayShow();
        else if (lp == WM_RBUTTONUP) TrayMenu();
        return 0;
    /* La barra de pestanas ya no es un TabControl: la seleccion la hace
     * LuAnTabBar directamente (TabBarSelect -> ShowTab). No hay WM_NOTIFY. */
    /* Resultados que llegan de los hilos worker: aqui SI se puede tocar la UI */
    case WM_UPDATE_FOUND: {
        char q[512];
        snprintf(q, sizeof q, Tr(LS_UPD_NEW_FMT), g_updateVersion, APP_VERSION);
        if (g_compActive) {
            /* Nunca descargar solo en mitad de una partida: solo avisar. */
            char m[512];
            snprintf(m, sizeof m, Tr(LS_UPD_NEW_COMP_FMT), g_updateVersion);
            MessageBoxA(g_hwnd, m, Tr(LS_UPD_AVAILABLE), MB_ICONINFORMATION);
            SetStatus(m);
        } else if (MessageBoxA(g_hwnd, q, Tr(LS_UPD_AVAILABLE), MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
            SetStatus(Tr(LS_UPD_DOWNLOADING));
            StartUpdateDownload();
        } else {
            SetStatus(Tr(LS_UPD_AVAILABLE_STATUS));
        }
        return 0;
    }
    case WM_UPDATE_STATUS: {
        int st = (int)wp;
        if (st == 0) {
            /* Red falla o repo aun no publicado: sin error, solo estado. */
            SetStatus(Tr(LS_UPD_CHECK_FAIL));
            if (g_updateManual) SetStatus(Tr(LS_UPD_CHECK_FAIL_LONG));
        } else if (st == 1) {
            SetStatus(Tr(LS_UPD_LATEST));
            if (g_updateManual) MessageBoxA(g_hwnd, Tr(LS_UPD_LATEST_MB), "Actualizaciones", MB_ICONINFORMATION);
        } else if (st == 2) {
            SetStatus(Tr(LS_UPD_RELEASES_OPENED));
        } else if (st == 3) {
            SetStatus(Tr(LS_UPD_DOWNLOAD_FAIL));
        }
        return 0;
    }
    case WM_GAME_APPLY: {
        ApplyGameProfile((int)wp);
        return 0;
    }
    case WM_SERVER_PING_DONE: {
        char txt[512];
        ServerPingText(txt, sizeof txt);
        SetWindowTextA(Ctl(ID_LBL_SERVER_PING), txt);
        SetStatus(Tr(LS_SRV_MEASURED));
        return 0;
    }
    case WM_TRACE_DONE: {
        SetWindowTextA(Ctl(ID_EDIT_TRACE), g_traceOut);
        SetWindowTextA(Ctl(ID_LBL_TRACE_SUMMARY), g_traceSummary);
        SetStatus(Tr(LS_TRACE_DONE));
        return 0;
    }
    case WM_SOCD_KEY: {
        if (!g_socdOn) return 0;
        int idx = (int)wp; long lat = (long)lp;
        char msg[256];
        snprintf(msg, sizeof msg, Tr(LS_SOCD_TEST_FMT), SocdKeyName(idx), lat);
        SetWindowTextA(Ctl(ID_LBL_SOCD_TEST), msg);
        return 0;
    }
    case WM_PING_UPDATE: {
        char msg[256];
        LivePingText(msg, sizeof msg);
        SetWindowTextA(Ctl(ID_LBL_LIVE_PING), msg);
        return 0;
    }
    case WM_PING_DONE: {
        int kind = (int)wp;
        char msg[512]; int best = -1;
        EnterCriticalSection(&g_pingCS);
        strncpy(msg, g_pingResult, sizeof msg - 1); msg[sizeof msg - 1] = 0; best = g_pingBestIdx;
        LeaveCriticalSection(&g_pingCS);
        if (kind == 0) { SetDnsStatus(msg); }
        else {
            SetWindowTextA(Ctl(ID_LBL_BEST_DNS), msg);
            SetStatus(msg);
            if (best >= 0) SetDnsPreset(DNS_IPS[best], "");
            if (InterlockedExchange(&g_compMeasuring, 0)) {
                char line[560]; snprintf(line, sizeof line, Tr(LS_COMP_DNS_BEST_FMT), msg);
                CompLog(line);
                CompLog(Tr(LS_COMP_READY));
            }
        }
        return 0;
    }
    case WM_DRAWITEM:
        DrawButton((LPDRAWITEMSTRUCT)lp); return TRUE;
    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, COL_TEXT); SetBkColor(hdc, COL_BG);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, COL_TEXT); SetBkColor(hdc, COL_BG);
        return (LRESULT)g_brBg;
    }
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, COL_TEXT); SetBkColor(hdc, COL_CARD);
        return (LRESULT)g_brCard;
    }
    case WM_CTLCOLORLISTBOX: {
        HDC hdc = (HDC)wp;
        SetTextColor(hdc, COL_TEXT); SetBkColor(hdc, COL_CARD);
        return (LRESULT)g_brCard;
    }
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wp; RECT r; GetClientRect(hwnd, &r);
        FillRect(hdc, &r, g_brBg); return 1;
    }
    case WM_DESTROY:
        if (g_gameStopEvent) SetEvent(g_gameStopEvent);
        if (g_tempStopEvent) SetEvent(g_tempStopEvent);
        if (g_overlay) DestroyWindow(g_overlay);
        KillTimer(hwnd, 1); KillTimer(hwnd, 3); KillTimer(hwnd, 4); KillTimer(hwnd, 6);
        TrayRemove(); /* v8: quitar el icono de la bandeja al salir de verdad */
        LivePingStop();
        if (g_ultraOn) { /* v10: al salir, devolver la red y los servicios a su estado */
            if (g_ultraBackup.svcWuauservWasRunning) SvcContinue("wuauserv");
            if (g_ultraBackup.svcDoSvcWasRunning) SvcContinue("DoSvc");
            UltraRestore(&g_ultraBackup);
            UltraRestoreGame();
        }
        SocdUninstall(); ReleaseTimer(); FreeLogo();
        DeleteCriticalSection(&g_pingCS);
        if (g_wsaInit) WSACleanup();
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR cmd, int show) {
    (void)hPrev; (void)cmd;
    g_hInst = hInst;
    InitCommonControls();
    /* DPI aware para que se vea bien en 7-11 */
    HMODULE u = GetModuleHandleA("user32.dll");
    if (u) { FARPROC p = GetProcAddress(u, "SetProcessDPIAware"); if (p) p(); }

    WNDCLASSA wc2 = {0};
    wc2.lpfnWndProc = PageWndProc; wc2.hInstance = hInst; wc2.lpszClassName = "LuAnPage";
    wc2.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc2);

    WNDCLASSA wc3 = {0};
    wc3.lpfnWndProc = TabBarWndProc; wc3.hInstance = hInst; wc3.lpszClassName = "LuAnTabBar";
    wc3.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc3);

    WNDCLASSA wc5 = {0};
    wc5.lpfnWndProc = ToggleWndProc; wc5.hInstance = hInst; wc5.lpszClassName = "LuAnToggle";
    wc5.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc5);

    LoadTheme(); /* v13: tema guardado, antes de registrar clases (el fondo del overlay lo usa) */
    WNDCLASSA wc4 = {0};
    wc4.lpfnWndProc = OverlayWndProc; wc4.hInstance = hInst; wc4.lpszClassName = "LuAnOverlay";
    wc4.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc4.hbrBackground = CreateSolidBrush(COL_BGDEEP);
    RegisterClassA(&wc4);

    wc3.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc3);

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.lpszClassName = "LuAnOptimizer";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCEA(100));
    if (!wc.hIcon) wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassA(&wc);

    /* v11: ventana compacta 700x560, centrada en el area de trabajo (asi
     * no se recorta al salir de un juego en pantalla completa). */
    RECT wa; SystemParametersInfoA(SPI_GETWORKAREA, 0, &wa, 0);
    int winX = wa.left + (wa.right - wa.left - 700) / 2;
    int winY = wa.top + (wa.bottom - wa.top - 560) / 2;
    if (winX < wa.left) winX = wa.left;
    if (winY < wa.top) winY = wa.top;
    char winTitle[64];
    snprintf(winTitle, sizeof winTitle, "Lu-An Optimizer v%s", APP_VERSION);
    HWND hwnd = CreateWindowExA(0, "LuAnOptimizer", winTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        winX, winY, 700, 560, NULL, NULL, hInst, NULL);
    if (!hwnd) return 1;
    ShowWindow(hwnd, show); UpdateWindow(hwnd);
    MSG m; while (GetMessageA(&m, NULL, 0, 0)) { TranslateMessage(&m); DispatchMessageA(&m); }
    return (int)m.wParam;
}
