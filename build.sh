#!/bin/bash
set -e
cd "$(dirname "$0")"
ZIG=${ZIG:-/tmp/zig-x86_64-linux-0.15.1/zig}
# Si Zig no esta en /tmp (es efimero), descomprime la copia guardada:
#   tar -xf ~/workspace/tools/zig-0.15.1.tar.xz -C /tmp
# 1) Recursos (icono LU-AN + manifiesto + logo.jpg + logo Pinkimich + version 13.1.0.0):
#    la salida es POSICIONAL, no /fo (con /fo zig rc sale con 0 pero no escribe nada).
"$ZIG" rc /v resource.rc lu-an.res
# 2) Compila el .exe enlazando el .res (icono incrustado verificado).
#    v9 anade: wininet (auto-actualizacion), urlmon (descarga),
#    bcrypt (SHA256), dxgi (FPS de pantalla del overlay),
#    wbemuuid + oleaut32 (temperatura por WMI).
#    v10 anade: psapi (GetModuleFileNameExA, lista de apps con red),
#    shell32 (netsh via ShellExecuteA).
#    v11 anade: powrprof (PowerGetActiveScheme, sin ventana de cmd).
"$ZIG" cc -target x86_64-windows-gnu -O2 -D_WIN32_WINNT=0x0601 main.c lu-an.res \
  -o LuAnOptimizer.exe -lcomctl32 -lgdi32 -luser32 -lkernel32 \
  -liphlpapi -lws2_32 -lgdiplus -lole32 -ladvapi32 \
  -lwininet -lurlmon -lbcrypt -ldxgi -lwbemuuid -loleaut32 -lpsapi -lshell32 -lpowrprof \
  -Wl,--subsystem,windows
cp app.manifest LuAnOptimizer.exe.manifest
rm -f LuAnOptimizer.pdb
ls -lh LuAnOptimizer.exe LuAnOptimizer.exe.manifest lu-an.res
sha256sum LuAnOptimizer.exe
echo "OK - LuAnOptimizer.exe listo"
# Verificar el icono incrustado (directorio de recursos del PE):
#   python3 -c "import pefile; pe=pefile.PE('LuAnOptimizer.exe'); ..."
# Debe listar RT_GROUP_ICON (14) con 6 tamanos y RT_ICON (3) con 6 imagenes.
# Verificar la version incrustada:
#   python3 -c "import pefile; pe=pefile.PE('LuAnOptimizer.exe'); print([e for e in pe.FileInfo])"
