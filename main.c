/* Lu-An Optimizer v9 - Timer Resolution + FilterKeys + DNS Jumper + Competitivo
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
 * Correccion v7 (delay RESIDUAL, Luis: sigue algo de delay en v6, y el
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

/* ---------- Version (v9) ---------- */
#define APP_VERSION "9.0.0"
#define APP_VERSION_NUM 9,0,0,0
#define UPDATE_API_URL "https://api.github.com/repos/luisadrianh474-droid/lu-an-optimizer/releases/latest"
#define UPDATE_TXT_URL "https://raw.githubusercontent.com/luisadrianh474-droid/lu-an-optimizer/main/version.txt"
#define UPDATE_RELEASES_URL "https://github.com/luisadrianh474-droid/lu-an-optimizer/releases/latest"

/* ---------- Colores (tema oscuro bonito) ---------- */
#define COL_BG      RGB(15,23,42)
#define COL_CARD    RGB(30,41,59)
#define COL_CARD2   RGB(51,65,85)
#define COL_TEXT    RGB(241,245,249)
#define COL_MUTED   RGB(148,163,184)
#define COL_ACCENT  RGB(34,211,238)
#define COL_ACCENT2 RGB(99,102,241)
#define COL_GREEN   RGB(52,211,153)
#define COL_BTN     RGB(99,102,241)
#define COL_BTN2    RGB(14,165,233)
#define COL_MAGENTA RGB(217,70,239)   /* rosa/magenta del logo LU-AN */
#define COL_TABHOVER RGB(56,72,102)  /* tarjeta intermedia para hover de pestana */

static HINSTANCE g_hInst;
static HWND g_hwnd;
static HFONT g_font, g_fontBold, g_fontBig, g_fontTitle, g_fontSection, g_fontTab;
static HWND g_tabBar, g_pageTimer, g_pageFK, g_pageDNS, g_pageComp, g_pageGames;
static HWND g_pages[5];
static int g_curTab = 0;
static volatile LONG g_compActive = 0; /* v9: 1 = Modo Competitivo activo */
/* Barra de pestanas premium dibujada a mano (sustituye al TabControl estandar) */
static const char *g_tabNames[5] = {"Timer", "FilterKeys", "DNS", "Competitivo", "Juegos"};
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
#define PAGE_H 620
static int g_hasVarFont = 0;
static HBRUSH g_brBg, g_brCard, g_brBtn;
static double g_desiredMs = 1.0;
static double g_currentMs = 15.6;
static HBITMAP g_hLogoBmp = NULL;
static ULONG_PTR g_gdiplusToken = 0;
static void SetStatus(const char *s); /* definida en Helpers UI, usada por DNS */
static void TrayUpdateTip(void); /* bandeja (v8): refresca el tooltip */
static void ShowTab(int sel); /* definida en Paginas de pestanas, usada por la barra premium */
static HWND Ctl(int id);
static int IsChecked(int id);
static void SetChecked(int id, int v);

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
#define ID_CHK_TRAY_MIN 1401 /* "Minimizar en segundo plano" */
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
#define WM_UPDATE_FOUND (WM_APP+5)
#define WM_UPDATE_STATUS (WM_APP+6)
#define WM_GAME_APPLY (WM_APP+7)

/* ---------- Logo LU-AN (GDI+, desde logo.jpg en memoria) ---------- */
static void LoadLogo(void) {
    GdiplusStartupInput input;
    input.GdiplusVersion = 1;
    input.DebugEventCallback = NULL;
    input.SuppressBackgroundThread = FALSE;
    input.SuppressExternalCodecs = FALSE;
    if (GdiplusStartup(&g_gdiplusToken, &input, NULL) != 0) return;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, g_logoJpgLen);
    if (!hMem) return;
    void *p = GlobalLock(hMem);
    if (!p) { GlobalFree(hMem); return; }
    memcpy(p, g_logoJpg, g_logoJpgLen);
    GlobalUnlock(hMem);
    IStream *stream = NULL;
    if (CreateStreamOnHGlobal(hMem, TRUE, &stream) != S_OK) { GlobalFree(hMem); return; }
    GpBitmap *bmp = NULL;
    if (GdipCreateBitmapFromStream(stream, &bmp) != 0) { stream->lpVtbl->Release(stream); return; }
    stream->lpVtbl->Release(stream);
    HBITMAP hbm = NULL;
    if (GdipCreateHBITMAPFromBitmap(bmp, &hbm, 0) != 0) { GdipDisposeImage((GpImage*)bmp); return; }
    GdipDisposeImage((GpImage*)bmp);
    g_hLogoBmp = hbm;
}
static void FreeLogo(void) {
    if (g_hLogoBmp) { DeleteObject(g_hLogoBmp); g_hLogoBmp = NULL; }
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
    if (!EnumAdapters(&buf, &len)) { SendMessage(cb, CB_ADDSTRING, 0, (LPARAM)"Sin adaptadores"); SendMessage(cb, CB_SETCURSEL, 0, 0); return; }
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
    else { SendMessage(cb, CB_ADDSTRING, 0, (LPARAM)"Sin adaptadores activos"); SendMessage(cb, CB_SETCURSEL, 0, 0); }
}
static void RefreshCurrentDns(void) {
    char msg[512];
    int sel = (int)SendMessage(Ctl(ID_DNS_ADAPTER), CB_GETCURSEL, 0, 0);
    if (sel < 0 || sel >= g_adapterCount) { SetWindowTextA(Ctl(ID_LBL_DNS_CURRENT), "DNS actual: elige un adaptador."); return; }
    PIP_ADAPTER_ADDRESSES buf = NULL; ULONG len = 0;
    if (!EnumAdapters(&buf, &len)) { SetWindowTextA(Ctl(ID_LBL_DNS_CURRENT), "DNS actual: no se pudo leer."); return; }
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
    if (!found) { SetWindowTextA(Ctl(ID_LBL_DNS_CURRENT), "DNS actual: adaptador no encontrado."); return; }
    if (d1[0] && d2[0]) snprintf(msg, sizeof msg, "DNS actual: %s, %s", d1, d2);
    else if (d1[0]) snprintf(msg, sizeof msg, "DNS actual: %s", d1);
    else snprintf(msg, sizeof msg, "DNS actual: automatico (DHCP) o sin DNS.");
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
    if (!adapter[0]) { SetDnsStatus("DNS: elige un adaptador de red primero."); return; }
    char p[64], s[64]; GetEditText(ID_DNS_PRIMARY, p, sizeof p); GetEditText(ID_DNS_SECONDARY, s, sizeof s);
    if (!IsValidIPv4(p)) { MessageBoxA(g_hwnd, "El DNS primario no es una IP valida (ej. 8.8.8.8).", "DNS", MB_ICONWARNING); return; }
    char cmd[1024]; DWORD code = 0;
    snprintf(cmd, sizeof cmd, "cmd.exe /c netsh interface ip set dns name=\"%s\" static %s primary", adapter, p);
    if (!RunHidden(cmd, &code)) { SetDnsStatus("DNS: fallo al aplicar. Ejecuta como administrador."); MessageBoxA(g_hwnd, "No se pudo aplicar el DNS. Prueba ejecutar Lu-An Optimizer como administrador.", "DNS", MB_ICONWARNING); return; }
    if (IsValidIPv4(s)) {
        snprintf(cmd, sizeof cmd, "cmd.exe /c netsh interface ip add dns name=\"%s\" %s index=2", adapter, s);
        RunHidden(cmd, &code);
    }
    char msg[256]; snprintf(msg, sizeof msg, "DNS aplicado en %s: %s%s%s", adapter, p, IsValidIPv4(s) ? ", " : "", IsValidIPv4(s) ? s : "");
    SetDnsStatus(msg); RefreshCurrentDns();
}
static void RestoreDnsAuto(void) {
    char adapter[256]; GetSelectedAdapter(adapter, sizeof adapter);
    if (!adapter[0]) { SetDnsStatus("DNS: elige un adaptador de red primero."); return; }
    char cmd[1024]; DWORD code = 0;
    snprintf(cmd, sizeof cmd, "cmd.exe /c netsh interface ip set dns name=\"%s\" dhcp", adapter);
    if (!RunHidden(cmd, &code)) { SetDnsStatus("DNS: fallo al restaurar. Ejecuta como administrador."); MessageBoxA(g_hwnd, "No se pudo restaurar el DNS. Prueba ejecutar como administrador.", "DNS", MB_ICONWARNING); return; }
    SetDnsStatus("DNS restaurado a automatico (DHCP)."); RefreshCurrentDns();
}
static void FlushDns(void) {
    DWORD code = 0;
    if (RunHidden("cmd.exe /c ipconfig /flushdns", &code)) SetDnsStatus("Cache DNS limpiada (ipconfig /flushdns).");
    else SetDnsStatus("No se pudo limpiar la cache DNS.");
}
/* Declaraciones adelantadas: el ping corre en hilos worker (definidos mas abajo) */
static int StartPingSingle(void);
static int StartPingAll(void);
/* "Probar ping" ya no mide en el hilo UI (4 x 1000 ms lo congelaban):
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
 * desbloqueara: ese era el delay (Luis lo noto justo al pulsar Modo
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
    if (!g_socdThread) { InterlockedExchange(&g_socdOn, 0); SetChecked(ID_CHK_SOCD, BST_UNCHECKED); SocdSetStatus("SOCD: no se pudo crear el hilo del hook."); return; }
    DWORD w = WaitForSingleObject(g_socdReadyEvent, 500); /* v7: antes 1000 ms */
    if (w != WAIT_OBJECT_0 || !g_socdHook) {
        SocdSetStatus("SOCD: no se pudo instalar el hook de teclado.");
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
    SocdSetStatus(g_socdMode == 0 ? "SOCD ACTIVADO (hilo dedicado, prioridad alta): ultima tecla gana (A/D, W/S, flechas)." : "SOCD ACTIVADO (hilo dedicado, prioridad alta): modo neutro (ninguna al pulsar las dos).");
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
    SocdSetStatus("SOCD desactivado.");
}
static void SocdToggle(void) {
    if (IsChecked(ID_CHK_SOCD) != BST_CHECKED) { SocdUninstall(); return; }
    /* Advertencia obligatoria, siempre, antes de activar */
    char warn[700];
    snprintf(warn, sizeof warn,
        "SOCD Cleaner: al pulsar A y D (o W y S) a la vez, solo cuenta la ultima tecla.\n\n"
        "ADVERTENCIA: en algunos juegos esta PROHIBIDO. En CS2, Valve lo prohibio en 2024\n"
        "y puede expulsarte de la partida o banearte. En otros juegos las reglas cambian.\n"
        "Usalo bajo tu propia responsabilidad y solo en juegos donde este permitido.%s\n\n"
        "Quieres activar el SOCD Cleaner?",
        IsChecked(ID_CHK_FK) == BST_CHECKED ? "\n\nOJO: tienes FilterKeys activado; SOCD + FilterKeys a la vez pueden chocar." : "");
    if (MessageBoxA(g_hwnd, warn, "SOCD Cleaner - Advertencia", MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
        SetChecked(ID_CHK_SOCD, BST_UNCHECKED);
        SocdSetStatus("SOCD desactivado (no aceptaste la advertencia).");
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
        snprintf(msg, sizeof msg, "Memoria standby limpiada. Disponible: %llu MB -> %llu MB.", before, after);
        SetWindowTextA(Ctl(ID_LBL_MEM_STATUS), msg); SetStatus(msg);
    } else {
        SetWindowTextA(Ctl(ID_LBL_MEM_STATUS), "No se pudo limpiar la memoria standby (prueba como administrador).");
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
static void GetActivePowerGuid(char *out, int outLen) {
    out[0] = 0;
    FILE *f = _popen("powercfg /getactivescheme", "r");
    if (!f) return;
    char line[256];
    while (fgets(line, sizeof line, f)) {
        char *g = strstr(line, "GUID:");
        if (g) { g += 5; while (*g == ' ') g++; strncpy(out, g, outLen - 1); out[outLen - 1] = 0; char *e = strchr(out, '\r'); if (e) *e = 0; e = strchr(out, '\n'); if (e) *e = 0; e = strchr(out, ' '); if (e) *e = 0; break; }
    }
    _pclose(f);
}
static void CompetitiveMode(void) {
    InterlockedExchange(&g_compActive, 1);
    HWND h = Ctl(ID_COMP_LOG); if (h) SetWindowTextA(h, "");
    CompLog("Modo Competitivo activado:");
    /* 1) Timer 0.5 ms */
    SetTrack(0.5); ApplyTimer();
    CompLog("- Timer Resolution: 0.5 ms.");
    /* 2) FilterKeys apagado */
    SetChecked(ID_CHK_FK, BST_UNCHECKED); SetChecked(ID_CHK_BOUNCE, BST_UNCHECKED); ApplyFK();
    CompLog("- FilterKeys desactivado (sin filtros en el teclado).");
    /* 3) Limpiar cache DNS */
    FlushDns();
    CompLog("- Cache DNS limpiada.");
    /* 4) Plan de energia Alto rendimiento */
    GetActivePowerGuid(g_prevPowerGuid, sizeof g_prevPowerGuid);
    DWORD code = 0;
    if (RunHidden("cmd.exe /c powercfg /setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c", &code))
        CompLog("- Plan de energia: Alto rendimiento.");
    else CompLog("- Plan de energia: no se pudo cambiar (prueba como administrador).");
    /* 5) Game Mode */
    DWORD gm = 0;
    g_prevGameMode = RegGetDword(HKEY_CURRENT_USER, "Software\\Microsoft\\GameBar", "AllowAutoGameMode", &gm) ? (int)gm : -1;
    RegSetDword(HKEY_CURRENT_USER, "Software\\Microsoft\\GameBar", "AllowAutoGameMode", 1);
    CompLog("- Game Mode activado.");
    /* 6) DNS mas rapido: medir los 5 presets y recomendar (sin aplicar solo).
     * En v5 esto se hacia en el hilo UI (hasta 7,5 s) y era lo que frenaba
     * el teclado con el SOCD activo: ahora es asincrono, el resultado
     * llega con WM_PING_DONE y lo registra el log. */
    if (StartPingAll()) {
        InterlockedExchange(&g_compMeasuring, 1);
        CompLog("- Midiendo los 5 DNS para recomendar el mas rapido (en segundo plano, sin congelar el teclado)...");
    } else {
        CompLog("- Medicion de DNS: ya habia una medicion en curso.");
        CompLog("Modo Competitivo listo. Cierra el programa o pulsa Restaurar normal para volver.");
    }
    SetStatus("Modo Competitivo activado: timer 0.5 ms, sin FilterKeys, alto rendimiento, Game Mode.");
}
static void CompetitiveRestore(void) {
    InterlockedExchange(&g_compActive, 0);
    HWND h = Ctl(ID_COMP_LOG); if (h) SetWindowTextA(h, "");
    CompLog("Restaurando modo normal:");
    SetTrack(15.6); ApplyTimer();
    CompLog("- Timer Resolution: 15.6 ms (normal de Windows).");
    if (g_prevPowerGuid[0]) {
        char cmd[256]; snprintf(cmd, sizeof cmd, "cmd.exe /c powercfg /setactive %s", g_prevPowerGuid);
        DWORD code = 0;
        if (RunHidden(cmd, &code)) CompLog("- Plan de energia restaurado al anterior.");
        else CompLog("- Plan de energia: no se pudo restaurar.");
    } else {
        DWORD code = 0;
        RunHidden("cmd.exe /c powercfg /setactive 381b4222-f694-41f0-9685-ff5bb260df2e", &code);
        CompLog("- Plan de energia: Equilibrado.");
    }
    if (g_prevGameMode < 0) RegDelValue(HKEY_CURRENT_USER, "Software\\Microsoft\\GameBar", "AllowAutoGameMode");
    else RegSetDword(HKEY_CURRENT_USER, "Software\\Microsoft\\GameBar", "AllowAutoGameMode", (DWORD)g_prevGameMode);
    CompLog("- Game Mode restaurado como estaba.");
    SetStatus("Modo normal restaurado.");
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
    if (!g_liveThread) SetWindowTextA(Ctl(ID_LBL_LIVE_PING), "Ping en vivo: no se pudo crear el hilo.");
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
        if (ok) snprintf(msg, sizeof msg, "Ping a %s: %d/4 respuestas, media %ld ms.", ip, ok, avg);
        else snprintf(msg, sizeof msg, "Ping a %s: 0/4 respuestas (sin respuesta).", ip);
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
        if (bestIdx >= 0) snprintf(msg, sizeof msg, "El mas rapido: %s %s (%ld ms). %s", DNS_NAMES[bestIdx], DNS_IPS[bestIdx], bestMs, detail);
        else snprintf(msg, sizeof msg, "Ningun DNS respondio. %s", detail);
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
        SetDnsStatus("Ya hay una medicion de ping en curso, espera a que termine.");
        return 0;
    }
    PingJob *job = (PingJob*)malloc(sizeof(PingJob));
    if (!job) { InterlockedExchange(&g_pingBusy, 0); return 0; }
    job->kind = kind;
    strncpy(job->ip, ip ? ip : "", sizeof job->ip - 1); job->ip[sizeof job->ip - 1] = 0;
    HANDLE h = CreateThread(NULL, 0, PingJobThreadProc, job, 0, NULL);
    if (!h) { free(job); InterlockedExchange(&g_pingBusy, 0); SetDnsStatus("No se pudo iniciar la medicion de ping."); return 0; }
    CloseHandle(h); /* el hilo se libera solo al terminar */
    return 1;
}
static int StartPingSingle(void) {
    char p[64]; GetEditText(ID_DNS_PRIMARY, p, sizeof p);
    if (!IsValidIPv4(p)) { MessageBoxA(g_hwnd, "El DNS primario no es una IP valida para probar el ping.", "DNS", MB_ICONWARNING); return 0; }
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
        SetStatus("Aceleracion del mouse desactivada (precision mejorada).");
    } else if (g_mouseSaved) {
        SystemParametersInfoA(SPI_SETMOUSE, 0, g_savedMouse, SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
        SetStatus("Aceleracion del mouse restaurada.");
    }
}
static void ToggleFSE(void) {
    if (IsChecked(ID_CHK_FSE) == BST_CHECKED) {
        DWORD v = 0;
        g_prevFSE = RegGetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_FSEBehaviorMode", &v) ? (int)v : -1;
        g_prevFSEHonor = RegGetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_HonorUserFSEBehaviorMode", &v) ? (int)v : -1;
        RegSetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_FSEBehaviorMode", 2);
        RegSetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_HonorUserFSEBehaviorMode", 1);
        SetStatus("Optimizaciones de pantalla completa desactivadas.");
    } else {
        if (g_prevFSE < 0) RegDelValue(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_FSEBehaviorMode");
        else RegSetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_FSEBehaviorMode", (DWORD)g_prevFSE);
        if (g_prevFSEHonor < 0) RegDelValue(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_HonorUserFSEBehaviorMode");
        else RegSetDword(HKEY_CURRENT_USER, "System\\GameConfigStore", "GameDVR_HonorUserFSEBehaviorMode", (DWORD)g_prevFSEHonor);
        SetStatus("Optimizaciones de pantalla completa restauradas.");
    }
}
/* MinGW/Zig solo expone la version ancha de Toolhelp32: PROCESSENTRY32W */
static const wchar_t *BG_APPS[10] = { L"chrome.exe", L"msedge.exe", L"firefox.exe", L"discord.exe", L"spotify.exe", L"steam.exe", L"EpicGamesLauncher.exe", L"opera.exe", L"brave.exe", L"obs64.exe" };
static void CloseBackgroundApps(void) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) { SetStatus("No se pudo listar los procesos."); return; }
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
    if (!nFound) { MessageBoxA(g_hwnd, "No hay apps en segundo plano de la lista abiertas (navegadores, Discord, Spotify, launchers...).", "Cerrar apps", MB_ICONINFORMATION); return; }
    if (found[0]) found[strlen(found) - 2] = 0;
    char q[700];
    snprintf(q, sizeof q, "Se encontraron estas apps en segundo plano:\n\n%s\n\nCerrarlas libera CPU y RAM para el juego. Se cerraran sin guardar (cierra tus paginas o partidas antes si hace falta).\n\nQuieres cerrarlas?", found);
    if (MessageBoxA(g_hwnd, q, "Cerrar apps en segundo plano", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) != IDYES) return;
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
    char msg[128]; snprintf(msg, sizeof msg, "Apps cerradas: %d proceso(s).", closed);
    SetStatus(msg);
}

/* ================= v9: AUTO-ACTUALIZACION =================
 * Consulta primero version.txt (raw.githubusercontent, formato simple
 * version=/sha256=/url=) y si falla, la API de GitHub Releases. Todo en
 * un hilo de trabajo con timeout corto (~5 s): si la red falla o el repo
 * aun no existe (GitHub de Luis todavia no esta conectado ni publicado),
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
            const char *u = strstr(buf, "url=");
            if (u) { u += 4; i = 0; while (*u && *u != '\r' && *u != '\n' && i < (int)sizeof url - 1) url[i++] = *u++; url[i] = 0; }
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
            found = 1;
        }
    }
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
    HRESULT hr = URLDownloadToFileA(NULL, g_updateUrl, newExe, 0, NULL);
    if (hr != S_OK) {
        if (g_hwnd) PostMessageA(g_hwnd, WM_UPDATE_STATUS, 3, 0);
        return 0;
    }
    if (g_updateSha[0]) {
        char hex[65] = {0};
        if (!Sha256FileHex(newExe, hex, sizeof hex) || _stricmp(hex, g_updateSha) != 0) {
            DeleteFileA(newExe);
            MessageBoxA(g_hwnd, "La descarga no coincide con el SHA256 publicado. Actualizacion cancelada por seguridad.", "Actualizacion", MB_ICONERROR);
            return 0;
        }
    } else {
        /* Sin hash disponible: confirmacion extra antes de instalar. */
        if (MessageBoxA(g_hwnd, "No se pudo verificar el SHA256 de la descarga (el release no publica hash).\n\nInstalar de todas formas?", "Actualizacion - sin verificar", MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
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
        char msg[256];
        snprintf(msg, sizeof msg, "Juego detectado: %s - perfil aplicado.", g_games[idx].name);
        SetWindowTextA(Ctl(ID_GAMES_STATUS), msg);
        SetStatus(msg);
    } else {
        if (g_savedComp) CompetitiveRestore();
        else { SetTrack(g_savedTimer); ApplyTimer(); }
        if (g_savedSocd && !g_socdOn) { SetChecked(ID_CHK_SOCD, BST_CHECKED); SocdInstall(); }
        else if (!g_savedSocd && g_socdOn) { SetChecked(ID_CHK_SOCD, BST_UNCHECKED); SocdUninstall(); }
        SetWindowTextA(Ctl(ID_GAMES_STATUS), "Ningun juego detectado.");
        SetStatus("Juego cerrado: ajustes restaurados.");
    }
}

/* ================= v9: MONITOR EN PANTALLA / OVERLAY =================
 * Ventana pequena siempre encima (TOPMOST + LAYERED, sin borde, fondo
 * semitransparente, arrastrable, con X para cerrarla y casilla en
 * Competitivo). NO roba el foco ni interfiere con el juego: opcion
 * "No interferir (click-through)" (WS_EX_TRANSPARENT).
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
    else snprintf(pingTxt, sizeof pingTxt, "sin respuesta");
    if (g_cpuPct >= 0) snprintf(cpuTxt, sizeof cpuTxt, "%d%%", g_cpuPct); else snprintf(cpuTxt, sizeof cpuTxt, "N/D");
    if (g_dxgiFps >= 0) snprintf(fpsTxt, sizeof fpsTxt, "%d", g_dxgiFps); else snprintf(fpsTxt, sizeof fpsTxt, "N/D");
    if (temp > -1000) snprintf(tempTxt, sizeof tempTxt, "%ld C", temp); else snprintf(tempTxt, sizeof tempTxt, "N/D");
    char txt[512];
    snprintf(txt, sizeof txt,
        "Lu-An Monitor\r\nPing: %s | Jitter: %ld ms | Perdida: %ld%%\r\nCPU: %s | RAM: %lu%%\r\nTimer: %.2f ms | SOCD: %s\r\nFPS (pantalla): %s\r\nTemp: %s\r\nFPS = pantalla, no del juego.",
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
        SetTextColor(hdc, COL_TEXT); SetBkColor(hdc, RGB(10, 16, 30));
        static HBRUSH brOv = NULL;
        if (!brOv) brOv = CreateSolidBrush(RGB(10, 16, 30));
        return (LRESULT)brOv;
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
    snprintf(b, sizeof b, "Actual: %.2f ms", g_currentMs);
    SetWindowTextA(Ctl(ID_LBL_CURRENT), b);
}
static void RefreshDesiredLabel(void) {
    char b[64]; snprintf(b, sizeof b, "%.2f ms", g_desiredMs);
    SetWindowTextA(Ctl(ID_LBL_DESIRED), b);
}
static void SetTrack(double ms) {
    g_desiredMs = ms;
    SendMessage(Ctl(ID_TRACKBAR), TBM_SETPOS, TRUE, (LPARAM)(int)(ms * 2.0 + 0.5));
    RefreshDesiredLabel();
}
static void ApplyTimer(void) {
    double applied = SetTimerRes(g_desiredMs);
    if (applied < 0) { MessageBoxA(g_hwnd, "No se pudo aplicar. Prueba ejecutar como administrador.", "Timer Resolution", MB_ICONWARNING); return; }
    char b[160]; snprintf(b, sizeof b, "Timer aplicado: %.2f ms - se mantiene mientras la app este abierta.", applied);
    SetStatus(b); RefreshCurrentLabel();
}
static void ApplyFK(void) {
    int enabled = IsChecked(ID_CHK_FK) == BST_CHECKED;
    int bounce = IsChecked(ID_CHK_BOUNCE) == BST_CHECKED;
    DWORD bm = (DWORD)GetIntEdit(ID_EDIT_BOUNCE), wm = (DWORD)GetIntEdit(ID_EDIT_WAIT);
    DWORD dm = (DWORD)GetIntEdit(ID_EDIT_DELAY), rm = (DWORD)GetIntEdit(ID_EDIT_REPEAT);
    if (!SetFK(enabled, bounce, bm, wm, dm, rm)) { MessageBoxA(g_hwnd, "No se pudo aplicar FilterKeys.", "FilterKeys", MB_ICONWARNING); return; }
    char b[220];
    if (!enabled) snprintf(b, sizeof b, "FilterKeys desactivado.");
    else if (bounce && bm) snprintf(b, sizeof b, "FilterKeys: BounceKeys %u ms (Slow/Repeat apagados por la regla de Windows).", (unsigned)bm);
    else snprintf(b, sizeof b, "FilterKeys: Slow %u ms, retardo %u ms, repeticion %u ms.", (unsigned)wm, (unsigned)dm, (unsigned)rm);
    SetWindowTextA(Ctl(ID_LBL_FK_STATUS), b); SetStatus(b);
}
static void ProfileGaming(void) { SetTrack(0.5); ApplyTimer(); SetChecked(ID_CHK_FK, BST_UNCHECKED); SetChecked(ID_CHK_BOUNCE, BST_UNCHECKED); ApplyFK(); SetStatus("Perfil Gaming: timer 0.5 ms, FilterKeys desactivado."); }
static void ProfileTyping(void) { SetTrack(1.0); ApplyTimer(); SetChecked(ID_CHK_FK, BST_CHECKED); SetChecked(ID_CHK_BOUNCE, BST_CHECKED); SetIntEdit(ID_EDIT_BOUNCE, 100); SetIntEdit(ID_EDIT_WAIT, 0); SetIntEdit(ID_EDIT_DELAY, 0); SetIntEdit(ID_EDIT_REPEAT, 0); ApplyFK(); SetStatus("Perfil Escritura: timer 1.0 ms, rebote 100 ms."); }
static void ProfileAccess(void) { SetTrack(1.0); ApplyTimer(); SetChecked(ID_CHK_FK, BST_CHECKED); SetChecked(ID_CHK_BOUNCE, BST_UNCHECKED); SetIntEdit(ID_EDIT_BOUNCE, 0); SetIntEdit(ID_EDIT_WAIT, 500); SetIntEdit(ID_EDIT_DELAY, 1000); SetIntEdit(ID_EDIT_REPEAT, 500); ApplyFK(); SetStatus("Perfil Accesibilidad: teclas lentas 500 ms, sin rebote."); }
static void ProfileNormal(void) { SetTrack(15.6); ApplyTimer(); SetChecked(ID_CHK_FK, BST_UNCHECKED); ApplyFK(); SetStatus("Perfil Normal: todo como viene en Windows."); }

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

static HWND MakeLabel(HWND p, int x, int y, int w, int ht, const char *t, HFONT f, int id) {
    HWND hw = CreateWindowExA(0, "STATIC", t, WS_CHILD|WS_VISIBLE|SS_LEFT, x, y, w, ht, p, (HMENU)(INT_PTR)id, g_hInst, NULL);
    SendMessage(hw, WM_SETFONT, (WPARAM)f, TRUE); return hw;
}
static HWND MakeButton(HWND p, int x, int y, int w, int ht, const char *t, int id) {
    HWND hw = CreateWindowExA(0, "BUTTON", t, WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON|BS_OWNERDRAW, x, y, w, ht, p, (HMENU)(INT_PTR)id, g_hInst, NULL);
    SendMessage(hw, WM_SETFONT, (WPARAM)g_font, TRUE);
    SetWindowSubclass(hw, BtnSubProc, 0, 0);
    return hw;
}

/* Dibuja botones bonitos: fondo redondeado, color segun ID */
static void DrawButton(LPDRAWITEMSTRUCT d) {
    HDC hdc = d->hDC; RECT r = d->rcItem;
    BtnAnim *a = AnimFor(d->hwndItem);
    COLORREF base = COL_BTN;
    if (d->CtlID == ID_APPLY_TIMER || d->CtlID == ID_APPLY_FK || d->CtlID == ID_DNS_APPLY || d->CtlID == ID_COMP_MODE) base = COL_BTN2;
    if ((d->CtlID >= ID_PRESET_05 && d->CtlID <= ID_PRESET_156) ||
        (d->CtlID >= ID_DNS_GOOGLE && d->CtlID <= ID_DNS_AUTO)) base = COL_CARD2;
    if (d->itemState & ODS_SELECTED) base = COL_ACCENT2;
    /* Hover: funde hacia un tono mas claro; pulsado: se encoge (pulso) y se oscurece */
    COLORREF hoverCol = Blend(base, RGB(255,255,255), 80);
    COLORREF bg = Blend(base, hoverCol, a ? a->level : 0);
    COLORREF border = (a && a->level > 0) ? Blend(COL_ACCENT, base, 255 - a->level) : base;
    if (a && a->pressed) { InflateRect(&r, -3, -3); bg = Blend(bg, RGB(0,0,0), 70); }
    HBRUSH br = CreateSolidBrush(bg);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ oldBr = SelectObject(hdc, br), oldPen = SelectObject(hdc, pen);
    RoundRect(hdc, r.left, r.top, r.right, r.bottom, 12, 12);
    SelectObject(hdc, oldBr); SelectObject(hdc, oldPen); DeleteObject(br); DeleteObject(pen);
    SetBkMode(hdc, TRANSPARENT); SetTextColor(hdc, COL_TEXT);
    SelectObject(hdc, g_font);
    char txt[128]; GetWindowTextA(d->hwndItem, txt, sizeof txt);
    DrawTextA(hdc, txt, -1, &r, DT_CENTER|DT_VCENTER|DT_SINGLELINE);
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
        else if (pt.y >= 950 && pt.y < 1560) { page = g_pageComp; top = 950; }
        else if (pt.y >= 1560) { page = g_pageGames; top = 1560; }
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
 * cambia: si la casilla "Minimizar en segundo plano" esta marcada, el
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
static int g_trayMinOn = 1;          /* casilla "Minimizar en segundo plano" */
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
    strncpy(nid.szInfo, "Sigue corriendo en segundo plano. Doble clic en el icono para mostrarlo.",
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
    AppendMenuA(m, MF_STRING, ID_TRAY_SHOW, "Mostrar Lu-An Optimizer");
    AppendMenuA(m, MF_SEPARATOR, 0, NULL);
    AppendMenuA(m, MF_STRING, ID_TRAY_COMP, "Modo Competitivo");
    AppendMenuA(m, MF_STRING | (g_socdOn ? MF_CHECKED : 0), ID_TRAY_SOCD,
                g_socdOn ? "Desactivar SOCD" : "Activar SOCD");
    AppendMenuA(m, MF_SEPARATOR, 0, NULL);
    AppendMenuA(m, MF_STRING, ID_TRAY_EXIT, "Salir");
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
        LoadLogo();

        /* Encabezado con logo (el logo se dibuja en WM_PAINT a la izquierda) */
        MakeLabel(hwnd, 230, 18, 440, 32, "LU-AN OPTIMIZER", g_fontTitle, 0);
        MakeLabel(hwnd, 230, 52, 440, 20, "GAMING OPTIMIZATION", g_fontBold, 0);
        MakeLabel(hwnd, 230, 76, 440, 20, "Timer + FilterKeys + DNS  -  Windows 7 / 8 / 10 / 11", g_font, 0);

        /* Perfiles */
        MakeLabel(hwnd, 20, 128, 120, 20, "Perfil rapido:", g_fontBold, 0);
        MakeButton(hwnd, 140, 124, 110, 30, "Gaming", ID_PROFILE_GAMING);
        MakeButton(hwnd, 258, 124, 110, 30, "Escritura", ID_PROFILE_TYPING);
        MakeButton(hwnd, 376, 124, 130, 30, "Accesibilidad", ID_PROFILE_ACCESS);
        MakeButton(hwnd, 514, 124, 90, 30, "Normal", ID_PROFILE_NORMAL);

        /* --- Tarjeta Timer --- */
        MakeLabel(hwnd, 20, 162, 300, 22, "TIMER RESOLUTION", g_fontSection, 0);
        MakeLabel(hwnd, 20, 188, 260, 34, "Actual: 15.60 ms", g_fontBig, ID_LBL_CURRENT);
        MakeLabel(hwnd, 300, 190, 320, 34, "Normal de Windows: 15.6 ms\nSe mantiene mientras la app este abierta.", g_font, 0);

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
        MakeButton(hwnd, 20, 326, 200, 36, "Aplicar resolucion", ID_APPLY_TIMER);
        MakeButton(hwnd, 230, 326, 230, 36, "Volver a 15.6 ms (normal)", ID_RESET_TIMER);

        /* --- Tarjeta FilterKeys --- */
        MakeLabel(hwnd, 20, 378, 300, 22, "FILTERKEYS  (TECLADO)", g_fontSection, 0);
        HWND c1 = CreateWindowExA(0, "BUTTON", "Activar FilterKeys", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 20, 404, 200, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_FK, g_hInst, NULL);
        SendMessage(c1, WM_SETFONT, (WPARAM)g_font, TRUE);
        HWND c2 = CreateWindowExA(0, "BUTTON", "BounceKeys: ignorar pulsaciones repetidas", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 240, 404, 360, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_BOUNCE, g_hInst, NULL);
        SendMessage(c2, WM_SETFONT, (WPARAM)g_font, TRUE);

        int y = 432;
        MakeLabel(hwnd, 20, y, 190, 20, "Rebote (BounceKeys)", g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "100", WS_CHILD|WS_VISIBLE|ES_NUMBER, 220, y-2, 70, 22, hwnd, (HMENU)(INT_PTR)ID_EDIT_BOUNCE, g_hInst, NULL);
        MakeLabel(hwnd, 300, y, 40, 20, "ms", g_font, 0);
        MakeLabel(hwnd, 350, y, 190, 20, "SlowKeys: mantener pulsada", g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "0", WS_CHILD|WS_VISIBLE|ES_NUMBER, 550, y-2, 70, 22, hwnd, (HMENU)(INT_PTR)ID_EDIT_WAIT, g_hInst, NULL);
        y += 30;
        MakeLabel(hwnd, 20, y, 190, 20, "Retardo antes de repetir", g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "1000", WS_CHILD|WS_VISIBLE|ES_NUMBER, 220, y-2, 70, 22, hwnd, (HMENU)(INT_PTR)ID_EDIT_DELAY, g_hInst, NULL);
        MakeLabel(hwnd, 350, y, 190, 20, "Intervalo entre repeticiones", g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "500", WS_CHILD|WS_VISIBLE|ES_NUMBER, 550, y-2, 70, 22, hwnd, (HMENU)(INT_PTR)ID_EDIT_REPEAT, g_hInst, NULL);

        MakeButton(hwnd, 20, 494, 200, 36, "Aplicar FilterKeys", ID_APPLY_FK);
        MakeButton(hwnd, 230, 494, 220, 36, "Desactivar FilterKeys", ID_DISABLE_FK);
        MakeLabel(hwnd, 20, 536, 620, 20, "Si activas BounceKeys, Slow/Repeat se apagan solos (regla de Windows).", g_font, ID_LBL_FK_STATUS);

        MakeLabel(hwnd, 20, 564, 220, 20, "Prueba tu teclado aqui:", g_fontBold, 0);
        HWND te = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "Escribe aqui para probar el rebote y la repeticion...", WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_VSCROLL, 20, 588, 620, 48, hwnd, (HMENU)(INT_PTR)ID_TEST_EDIT, g_hInst, NULL);
        SendMessage(te, WM_SETFONT, (WPARAM)g_font, TRUE);

        /* --- Tarjeta DNS Jumper --- */
        MakeLabel(hwnd, 20, 650, 300, 22, "DNS JUMPER", g_fontSection, 0);
        MakeLabel(hwnd, 20, 678, 140, 20, "Adaptador de red:", g_font, 0);
        HWND cb = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL, 165, 674, 475, 150, hwnd, (HMENU)(INT_PTR)ID_DNS_ADAPTER, g_hInst, NULL);
        SendMessage(cb, WM_SETFONT, (WPARAM)g_font, TRUE);
        MakeLabel(hwnd, 20, 704, 620, 20, "DNS actual: ...", g_font, ID_LBL_DNS_CURRENT);

        MakeButton(hwnd, 20, 728, 140, 30, "Google", ID_DNS_GOOGLE);
        MakeButton(hwnd, 168, 728, 140, 30, "Cloudflare", ID_DNS_CLOUDFLARE);
        MakeButton(hwnd, 316, 728, 140, 30, "Quad9", ID_DNS_QUAD9);
        MakeButton(hwnd, 20, 764, 140, 30, "OpenDNS", ID_DNS_OPENDNS);
        MakeButton(hwnd, 168, 764, 140, 30, "AdGuard", ID_DNS_ADGUARD);
        MakeButton(hwnd, 316, 764, 140, 30, "Automatico (DHCP)", ID_DNS_AUTO);
        MakeLabel(hwnd, 470, 728, 170, 60, "Pulsa un preset para\nrellenar el DNS.", g_font, 0);

        MakeLabel(hwnd, 20, 806, 120, 20, "DNS primario:", g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "8.8.8.8", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 150, 804, 150, 22, hwnd, (HMENU)(INT_PTR)ID_DNS_PRIMARY, g_hInst, NULL);
        MakeLabel(hwnd, 320, 806, 130, 20, "DNS secundario:", g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "8.8.4.4", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 460, 804, 180, 22, hwnd, (HMENU)(INT_PTR)ID_DNS_SECONDARY, g_hInst, NULL);

        MakeButton(hwnd, 20, 836, 150, 36, "Aplicar DNS", ID_DNS_APPLY);
        MakeButton(hwnd, 178, 836, 170, 36, "Restaurar automatico", ID_DNS_RESTORE);
        MakeButton(hwnd, 356, 836, 130, 36, "Limpiar cache", ID_DNS_FLUSH);
        MakeButton(hwnd, 494, 836, 146, 36, "Probar ping", ID_DNS_PING);
        MakeLabel(hwnd, 20, 878, 620, 20, "El DNS se aplica con netsh. Si falla, ejecuta como administrador.", g_font, ID_LBL_DNS_STATUS);

        /* --- Tarjeta Competitivo (Y absoluta 950+, se reparenta a la 4a pagina) --- */
        MakeLabel(hwnd, 20, 950, 300, 22, "SOCD CLEANER", g_fontSection, 0);
        HWND cs = CreateWindowExA(0, "BUTTON", "Activar SOCD Cleaner  (WASD + flechas)", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 20, 976, 380, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_SOCD, g_hInst, NULL);
        SendMessage(cs, WM_SETFONT, (WPARAM)g_font, TRUE);
        HWND r1 = CreateWindowExA(0, "BUTTON", "Ultima tecla gana", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON|WS_GROUP, 420, 976, 150, 22, hwnd, (HMENU)(INT_PTR)ID_SOCD_LAST, g_hInst, NULL);
        SendMessage(r1, WM_SETFONT, (WPARAM)g_font, TRUE);
        HWND r2 = CreateWindowExA(0, "BUTTON", "Neutro (ninguna)", WS_CHILD|WS_VISIBLE|BS_AUTORADIOBUTTON, 570, 976, 140, 22, hwnd, (HMENU)(INT_PTR)ID_SOCD_NEUTRAL, g_hInst, NULL);
        SendMessage(r2, WM_SETFONT, (WPARAM)g_font, TRUE);
        SendMessage(r1, BM_SETCHECK, BST_CHECKED, 0);
        MakeLabel(hwnd, 20, 1024, 620, 20, "SOCD desactivado.", g_font, ID_LBL_SOCD_STATUS);
        MakeLabel(hwnd, 20, 1044, 620, 18, "Prueba SOCD: pulsa A o D con el SOCD activo.", g_font, ID_LBL_SOCD_TEST);

        MakeLabel(hwnd, 20, 1064, 300, 22, "MODO COMPETITIVO", g_fontSection, 0);
        MakeButton(hwnd, 20, 1088, 240, 36, "ACTIVAR MODO COMPETITIVO", ID_COMP_MODE);
        MakeButton(hwnd, 270, 1088, 170, 36, "Restaurar normal", ID_COMP_RESTORE);
        HWND lg = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_MULTILINE|ES_READONLY|WS_VSCROLL, 20, 1132, 620, 34, hwnd, (HMENU)(INT_PTR)ID_COMP_LOG, g_hInst, NULL);
        SendMessage(lg, WM_SETFONT, (WPARAM)g_font, TRUE);

        MakeLabel(hwnd, 20, 1174, 300, 22, "PING EN VIVO", g_fontSection, 0);
        MakeLabel(hwnd, 20, 1198, 620, 20, "Ping: -- ms | Media: -- | Jitter: -- | Perdida: --%", g_font, ID_LBL_LIVE_PING);
        HWND cl = CreateWindowExA(0, "BUTTON", "Medir cada 1 s (al DNS primario)", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 20, 1222, 280, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_LIVE_PING, g_hInst, NULL);
        SendMessage(cl, WM_SETFONT, (WPARAM)g_font, TRUE);
        MakeButton(hwnd, 320, 1218, 180, 30, "Probar todos los DNS", ID_PING_ALL);
        MakeLabel(hwnd, 20, 1250, 620, 20, "El mas rapido: aun no medido. Pulsa Probar todos los DNS.", g_font, ID_LBL_BEST_DNS);

        MakeLabel(hwnd, 20, 1278, 300, 22, "OPTIMIZACIONES DEL SISTEMA", g_fontSection, 0);
        HWND cm = CreateWindowExA(0, "BUTTON", "Quitar aceleracion del mouse", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 20, 1302, 280, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_MOUSE_ACCEL, g_hInst, NULL);
        SendMessage(cm, WM_SETFONT, (WPARAM)g_font, TRUE);
        HWND cf = CreateWindowExA(0, "BUTTON", "Quitar optimizaciones de pantalla completa", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 320, 1302, 330, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_FSE, g_hInst, NULL);
        SendMessage(cf, WM_SETFONT, (WPARAM)g_font, TRUE);
        MakeButton(hwnd, 20, 1330, 250, 36, "Cerrar apps en segundo plano...", ID_BTN_CLOSE_APPS);
        MakeButton(hwnd, 280, 1330, 220, 36, "Limpiar memoria standby", ID_BTN_CLEAN_MEM);
        MakeLabel(hwnd, 20, 1372, 620, 20, "Memoria standby: aun no limpiada en esta sesion.", g_font, ID_LBL_MEM_STATUS);
        HWND ca = CreateWindowExA(0, "BUTTON", "Limpiar memoria standby cada 15 min automaticamente", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 20, 1396, 420, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_AUTO_CLEAN, g_hInst, NULL);
        SendMessage(ca, WM_SETFONT, (WPARAM)g_font, TRUE);

        /* --- Monitor en pantalla (v9) --- */
        MakeLabel(hwnd, 20, 1420, 300, 22, "MONITOR EN PANTALLA", g_fontSection, 0);
        HWND co = CreateWindowExA(0, "BUTTON", "Mostrar monitor", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 20, 1444, 160, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_OVERLAY, g_hInst, NULL);
        SendMessage(co, WM_SETFONT, (WPARAM)g_font, TRUE);
        HWND cc = CreateWindowExA(0, "BUTTON", "No interferir (click-through)", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 200, 1444, 260, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_OVERLAY_CLICK, g_hInst, NULL);
        SendMessage(cc, WM_SETFONT, (WPARAM)g_font, TRUE);
        MakeLabel(hwnd, 20, 1468, 620, 18, "FPS (pantalla) = frecuencia de la pantalla, no el FPS del juego.", g_font, 0);

        /* --- Auto-actualizacion (v9) --- */
        MakeLabel(hwnd, 20, 1492, 300, 22, "ACTUALIZACIONES", g_fontSection, 0);
        MakeButton(hwnd, 20, 1516, 220, 32, "Buscar actualizaciones", ID_BTN_CHECK_UPDATE);
        HWND cu = CreateWindowExA(0, "BUTTON", "Buscar actualizaciones al iniciar", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 252, 1522, 300, 22, hwnd, (HMENU)(INT_PTR)ID_CHK_AUTO_UPDATE, g_hInst, NULL);
        SendMessage(cu, WM_SETFONT, (WPARAM)g_font, TRUE);

        /* --- Pestana Juegos (v9): Y absoluta 1560+, se reparenta a la 5a pagina --- */
        MakeLabel(hwnd, 20, 1560, 300, 22, "PERFILES POR JUEGO", g_fontSection, 0);
        MakeLabel(hwnd, 20, 1586, 620, 20, "Ningun juego detectado.", g_font, ID_GAMES_STATUS);
        {
            static const char *gameRows[5] = {
                "Valorant - VALORANT-Win64-Shipping.exe",
                "CS2 - cs2.exe  (SOCD prohibido, Valve 2024)",
                "Fortnite - FortniteClient-Win64-Shipping.exe",
                "Apex Legends - r5apex.exe",
                "Otro juego - escribe su .exe abajo" };
            for (int gi = 0; gi < 5; gi++) {
                int gy = 1612 + gi * 58;
                MakeLabel(hwnd, 20, gy, 340, 20, gameRows[gi], g_fontBold, 0);
                HWND ga = CreateWindowExA(0, "BUTTON", "Activar automaticamente", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 380, gy, 200, 20, hwnd, (HMENU)(INT_PTR)(ID_GAMES_AUTO_BASE + gi), g_hInst, NULL);
                SendMessage(ga, WM_SETFONT, (WPARAM)g_font, TRUE);
                HWND gs = CreateWindowExA(0, "BUTTON", "SOCD", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 20, gy + 24, 90, 20, hwnd, (HMENU)(INT_PTR)(ID_GAMES_SOCD_BASE + gi), g_hInst, NULL);
                SendMessage(gs, WM_SETFONT, (WPARAM)g_font, TRUE);
                HWND gt = CreateWindowExA(0, "BUTTON", "Timer 0.5 ms (si no, 1.0 ms)", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 120, gy + 24, 220, 20, hwnd, (HMENU)(INT_PTR)(ID_GAMES_TIMER_BASE + gi), g_hInst, NULL);
                SendMessage(gt, WM_SETFONT, (WPARAM)g_font, TRUE);
                HWND gc = CreateWindowExA(0, "BUTTON", "Modo Competitivo", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 350, gy + 24, 180, 20, hwnd, (HMENU)(INT_PTR)(ID_GAMES_COMP_BASE + gi), g_hInst, NULL);
                SendMessage(gc, WM_SETFONT, (WPARAM)g_font, TRUE);
            }
        }
        MakeLabel(hwnd, 20, 1906, 220, 20, "Otro juego - nombre del .exe:", g_font, 0);
        CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL, 250, 1904, 220, 22, hwnd, (HMENU)(INT_PTR)ID_GAMES_CUSTOM_EXE, g_hInst, NULL);
        MakeLabel(hwnd, 20, 1932, 620, 34, "Al arrancar un juego con su perfil activo se aplica solo; al cerrarse se restaura todo. CS2: el SOCD esta siempre bloqueado.", g_font, ID_GAMES_NOTE);

        MakeLabel(hwnd, 20, 906, 620, 20, "Listo. Elige un perfil o ajusta y pulsa Aplicar.", g_font, ID_STATUS);

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
        g_pageTimer = CreateWindowExA(0, "LuAnPage", "", WS_CHILD|WS_VISIBLE, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pageFK = CreateWindowExA(0, "LuAnPage", "", WS_CHILD, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pageDNS = CreateWindowExA(0, "LuAnPage", "", WS_CHILD, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pageComp = CreateWindowExA(0, "LuAnPage", "", WS_CHILD, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pageGames = CreateWindowExA(0, "LuAnPage", "", WS_CHILD, PAGE_X, PAGE_Y, PAGE_W, PAGE_H, hwnd, NULL, g_hInst, NULL);
        g_pages[0] = g_pageTimer; g_pages[1] = g_pageFK; g_pages[2] = g_pageDNS; g_pages[3] = g_pageComp; g_pages[4] = g_pageGames;
        ReparentToPages();
        SetWindowPos(Ctl(ID_STATUS), NULL, 20, 834, 620, 18, SWP_NOZORDER|SWP_NOSIZE);
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
        HWND tm = CreateWindowExA(0, "BUTTON", "Minimizar en segundo plano", WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX, 20, 858, 330, 20, hwnd, (HMENU)(INT_PTR)ID_CHK_TRAY_MIN, g_hInst, NULL);
        SendMessage(tm, WM_SETFONT, (WPARAM)g_font, TRUE);
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
        }
        if (!g_gameStopEvent) {
            g_gameStopEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
            HANDLE gh = CreateThread(NULL, 0, GameWatchThreadProc, NULL, 0, NULL);
            if (gh) CloseHandle(gh);
        }
        if (g_autoUpdateCheck) StartUpdateCheck(0);
        SetTimer(hwnd, 1, 1000, NULL);
        return 0;
    }
    case WM_TIMER:
        if (wp == 1) { RefreshCurrentLabel(); UpdatePingTarget(); return 0; }
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
        if (g_hLogoBmp) {
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
                MessageBoxA(g_hwnd, "SOCD prohibido en CS2 (Valve, 2024).\nEl SOCD no se puede activar para CS2 bajo ningun concepto.", "CS2 - SOCD prohibido", MB_ICONWARNING);
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
        case ID_SOCD_LAST: if (g_socdOn) { InterlockedExchange(&g_socdMode, 0); SocdSetStatus("SOCD ACTIVADO (hilo dedicado): ultima tecla gana (A/D, W/S, flechas)."); } break;
        case ID_SOCD_NEUTRAL: if (g_socdOn) { InterlockedExchange(&g_socdMode, 1); SocdSetStatus("SOCD ACTIVADO (hilo dedicado): modo neutro (ninguna al pulsar las dos)."); } break;
        case ID_COMP_MODE: CompetitiveMode(); break;
        case ID_COMP_RESTORE: CompetitiveRestore(); break;
        case ID_CHK_LIVE_PING:
            if (IsChecked(ID_CHK_LIVE_PING) == BST_CHECKED) { LivePingStart(); SetWindowTextA(Ctl(ID_LBL_LIVE_PING), "Ping en vivo: midiendo cada 1 s (hilo aparte, sin frenar el teclado)..."); }
            else { LivePingStop(); SetWindowTextA(Ctl(ID_LBL_LIVE_PING), "Ping en vivo: pausado."); }
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
            SetStatus("Buscando actualizaciones...");
            StartUpdateCheck(1);
            break;
        case ID_CHK_AUTO_UPDATE:
            g_autoUpdateCheck = (IsChecked(ID_CHK_AUTO_UPDATE) == BST_CHECKED);
            RegSetDword(HKEY_CURRENT_USER, "Software\\LuAnOptimizer", "AutoUpdateCheck", g_autoUpdateCheck ? 1 : 0);
            SetStatus(g_autoUpdateCheck ? "Buscar actualizaciones al iniciar: activado." : "Buscar actualizaciones al iniciar: desactivado (usa el boton).");
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
            SetStatus(g_trayMinOn ? "Minimizar en segundo plano: activado (el boton _ esconde la ventana a la bandeja)." : "Minimizar en segundo plano: desactivado (minimizar es el normal a la barra de tareas).");
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
        snprintf(q, sizeof q, "Hay una v%s nueva de Lu-An Optimizer (tu tienes la v%s).\n\nQuieres actualizar?", g_updateVersion, APP_VERSION);
        if (g_compActive) {
            /* Nunca descargar solo en mitad de una partida: solo avisar. */
            char m[512];
            snprintf(m, sizeof m, "Hay una v%s nueva de Lu-An Optimizer.\nEl Modo Competitivo esta activo: no se descargara nada hasta que lo desactives.", g_updateVersion);
            MessageBoxA(g_hwnd, m, "Actualizacion disponible", MB_ICONINFORMATION);
            SetStatus(m);
        } else if (MessageBoxA(g_hwnd, q, "Actualizacion disponible", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDYES) {
            SetStatus("Descargando la actualizacion...");
            StartUpdateDownload();
        } else {
            SetStatus("Actualizacion disponible: la puedes descargar cuando quieras con el boton.");
        }
        return 0;
    }
    case WM_UPDATE_STATUS: {
        int st = (int)wp;
        if (st == 0) {
            /* Red falla o repo aun no publicado: sin error, solo estado. */
            SetStatus("No se pudo revisar actualizaciones.");
            if (g_updateManual) SetStatus("No se pudo revisar actualizaciones (revisa tu internet o que el repo este publicado).");
        } else if (st == 1) {
            SetStatus("Ya tienes la ultima version.");
            if (g_updateManual) MessageBoxA(g_hwnd, "Ya tienes la ultima version de Lu-An Optimizer.", "Actualizaciones", MB_ICONINFORMATION);
        } else if (st == 2) {
            SetStatus("Se abrio la pagina de releases para descargar la actualizacion.");
        } else if (st == 3) {
            SetStatus("No se pudo descargar la actualizacion.");
        }
        return 0;
    }
    case WM_GAME_APPLY: {
        ApplyGameProfile((int)wp);
        return 0;
    }
    case WM_SOCD_KEY: {
        if (!g_socdOn) return 0;
        int idx = (int)wp; long lat = (long)lp;
        char msg[256];
        snprintf(msg, sizeof msg, "Prueba SOCD (hilo dedicado): ultima tecla %s, latencia ~%ld ms. Pulsa A/D o W/S.", SOCD_NAMES[idx], lat);
        SetWindowTextA(Ctl(ID_LBL_SOCD_TEST), msg);
        return 0;
    }
    case WM_PING_UPDATE: {
        long avg, mean, jitter, loss, count;
        EnterCriticalSection(&g_pingCS);
        avg = g_liveAvg; mean = g_liveMean; jitter = g_liveJitter; loss = g_liveLoss; count = g_liveCount;
        LeaveCriticalSection(&g_pingCS);
        char msg[256];
        if (avg >= 0) snprintf(msg, sizeof msg, "Ping: %ld ms | Media: %ld ms | Jitter: %ld ms | Perdida: %ld%% (%ld muestras)", avg, mean, jitter, loss, count);
        else snprintf(msg, sizeof msg, "Ping: sin respuesta | Media: %ld ms | Jitter: %ld ms | Perdida: %ld%% (%ld muestras)", mean, jitter, loss, count);
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
                char line[560]; snprintf(line, sizeof line, "- DNS mas rapido: %s.", msg);
                CompLog(line);
                CompLog("Modo Competitivo listo. Cierra el programa o pulsa Restaurar normal para volver.");
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

    WNDCLASSA wc4 = {0};
    wc4.lpfnWndProc = OverlayWndProc; wc4.hInstance = hInst; wc4.lpszClassName = "LuAnOverlay";
    wc4.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc4.hbrBackground = CreateSolidBrush(RGB(10, 16, 30));
    RegisterClassA(&wc4);

    wc3.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc3);

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WndProc; wc.hInstance = hInst; wc.lpszClassName = "LuAnOptimizer";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCEA(100));
    if (!wc.hIcon) wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(0, "LuAnOptimizer", "Lu-An Optimizer - Timer + FilterKeys + DNS + Competitivo + Juegos",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 700, 910, NULL, NULL, hInst, NULL);
    if (!hwnd) return 1;
    ShowWindow(hwnd, show); UpdateWindow(hwnd);
    MSG m; while (GetMessageA(&m, NULL, 0, 0)) { TranslateMessage(&m); DispatchMessageA(&m); }
    return (int)m.wParam;
}
