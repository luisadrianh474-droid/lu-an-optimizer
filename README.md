# Lu-An Optimizer — Timer + FilterKeys + DNS + Competitivo + Juegos (v13)

Un solo archivo `LuAnOptimizer.exe`. Doble clic y se abre.
No necesita Python, no necesita .NET, no necesita instalar nada.

Compatible con **Windows 7, 8, 10 y 11** (64 bits).
Escrito en C con la API de Windows, por eso es un .exe nativo y ligero
(icono incluido). Logo LU-AN incluido dentro de la ventana
**y como icono del archivo en el Explorador**.
Versión incrustada **13.0.0.0** (se ve en Propiedades del .exe > Detalles).

## Novedades de la v13 (temas de colores)

Arriba a la derecha, bajo el botón de idioma, hay un selector **Tema**
con 3 temas pre-seleccionados. Cambiar de tema se aplica al instante,
sin reiniciar, y se guarda en el registro
(`HKCU\Software\LuAnOptimizer Theme`) para la próxima vez:

- **Neón** (el de siempre, por defecto): fondo azul oscuro, acentos
  cian → magenta.
- **Fuego**: fondo negro, acentos naranja → rojo.
- **Esmeralda**: fondo negro verdoso, acentos verde → lima.

Todo sigue la paleta del tema: barra de pestañas con su indicador en
degradado, botones con brillo, interruptores ON/OFF, cuadro de estado
ACTIVO/APAGADO, marcos de página, divisores y textos de las 5 pestañas.
Los nombres de los temas también se traducen con el idioma (ES/EN). Nada
más cambió en v13.

## Novedades de la v12 (arreglo del auto-update)

En v10/v11, al pulsar **Actualizar** salía "La descarga no coincide con
el SHA256 publicado. Actualización cancelada por seguridad." La causa: el
campo `url=` de `version.txt` apuntaba a la página web de releases
(`.../releases/latest`), así que el programa descargaba el HTML de esa
página y su SHA256 nunca coincidía con el publicado. La protección
funcionó como debía (no instaló nada), pero la descarga era basura.

Ahora la URL del `.exe` se resuelve **desde la API de GitHub Releases**
(`browser_download_url` del asset `LuAnOptimizer.exe`); si la API falla,
se construye la URL directa
`.../releases/download/v<versión>/LuAnOptimizer.exe`. El campo `url=` de
`version.txt` ya no se usa como descarga. Además, el hilo de descarga se
**niega a bajar cualquier URL que no termine en `.exe`**: si eso pasara,
avisa y abre la página de releases, sin descargar páginas HTML. Nada más
cambió en v12: misma ventana 700x560 con desplazamiento, cuadro
ACTIVO/APAGADO, idioma ES/EN, SOCD, Ultra, ping/traceroute y bandeja.

## Novedades de la v11

### Ventana compacta con desplazamiento (700x560)
La ventana pasó de 700x1110 a **700x560** y se abre **centrada** en el
área de trabajo, así ya no se recorta al salir de un juego en pantalla
completa. Cada pestaña tiene **barra de desplazamiento vertical + rueda
del mouse**: todo el contenido sigue ahí (Timer, FilterKeys, DNS,
Competitivo con Modo Ultra, Juegos con ping/traceroute, overlay,
bandeja), solo se baja la pestaña para verlo. Nada se quitó ni se
escondió. El diseño premium se mantiene (barra de pestañas, glow de
botones, interruptores ON/OFF).

### Cuadro de estado ACTIVO / APAGADO
Arriba a la derecha, frente al logo, un cuadro redondeado que dice
**ACTIVO** con fondo verde cuando el Modo Competitivo o el Modo Ultra
Baja Latencia está encendido, y **APAGADO** con fondo gris cuando
ninguno está encendido. Cambia solo al activar/desactivar cualquiera
de los dos modos o al restaurar.

### Idioma español / inglés
Botón **"English"** arriba a la derecha, bajo el cuadro de estado.
Cambia **toda** la interfaz entre español e inglés al instante, sin
reiniciar: pestañas, etiquetas, botones, mensajes de estado, avisos
(incluida la advertencia del SOCD), menú de la bandeja y overlay.
El idioma elegido se guarda en el registro
(`HKCU\Software\LuAnOptimizer Language`, "es" o "en") y se carga al
arrancar. Por defecto español. Los nombres propios no cambian
(Lu-An Optimizer, Valorant, CS2, SOCD Cleaner).

### Versión visible
La versión **v11.0.0** se ve en la barra de título y en una etiqueta
pequeña junto al logo ("v11.0.0 - Windows 7 / 8 / 10 / 11"). La
etiqueta se genera desde `APP_VERSION`, así cambia sola al actualizar
la versión.

### Sin ventana de cmd al abrir
El plan de energía activo ya no se lee con `powercfg` (que abría una
consola que aparecía y desaparecía un instante), sino con la API
`PowerGetActiveScheme` (powrprof): ya no aparece ningún cmd al abrir
el programa, ni al cambiar de pestaña, ni al activar el Modo
Competitivo. Verificado: no queda ningún `_popen`, `system(` ni
`WinExec` en el código.

## Novedades de la v10

### Modo Ultra Baja Latencia (pestaña Competitivo)
Botón grande **"ACTIVAR ULTRA BAJA LATENCIA"** + **"Restaurar red"**.
Quita la latencia que agrega tu propia PC:
- Registro de red con respaldo de los valores originales: desactiva el
  algoritmo de Nagle (`TCPNoDelay=1`), ACK inmediato (`TcpAckFrequency=1`,
  también por interfaz), quita el límite de ancho de banda de Windows
  para juegos (`NetworkThrottlingIndex=0xFFFFFFFF`), desactiva el ahorro
  de energía del adaptador de red y del WiFi, y deja el autotuning TCP
  en normal. Si una clave no existe o no se puede escribir (falta de
  permisos de administrador), se sigue con las demás y se reporta.
- Mientras juegas, pausa temporalmente Windows Update (`wuauserv`) y
  Delivery Optimization (`DoSvc`); al desactivar el modo o al cerrar el
  programa se reanudan. No se desactivan para siempre.
- Lista de las apps que más red usan (por conexiones TCP) con botón
  **"Cerrar app"** por proceso. Nunca se tocan procesos del sistema ni
  el propio programa.
- **Aviso honesto: el ping tiene un piso físico.** Depende de la
  distancia al servidor y de la ruta que elige tu ISP. Ningún programa
  puede bajar ese piso: el modo Ultra solo quita los milisegundos que
  agrega tu PC (red, CPU, servicios descargando en segundo plano).
  Si tu ISP te manda a CDMX pasando por Estados Unidos, eso no lo
  arregla ningún ajuste local.

### Ping a servidores de juego + diagnóstico de ruta (pestaña Juegos)
- **"Medir ping a servidores"**: mide con ICMP el ping real a los
  servidores de CDMX y Miami (las IP se pueden editar) y dice cuál te
  conviene, con pérdida. Etiquetado claro: es ping al **servidor**,
  no al DNS.
- **"Diagnóstico de ruta"**: traceroute al mejor servidor (hasta 30
  saltos, 1 s por salto, en hilo aparte para no congelar la ventana),
  salto por salto con IP y ms; los saltos que no responden salen como
  `*`. Al final resume dónde está el mayor salto de latencia:
  tu PC / tu router / tu ISP / el servidor.

### Prioridad del juego y GPU
- Cuando se detecta un juego con su perfil activo **y** el modo Ultra
  está encendido, el proceso del juego sube a prioridad alta y se le
  quita un núcleo para el sistema (afinidad). Al cerrar el juego se
  restaura la prioridad normal y la afinidad original.
- Interruptor **"Programación de GPU acelerada por hardware"**
  (clave `HwSchMode`): requiere reiniciar Windows para aplicar.

### Interruptores ON/OFF animados y botones con difuminado
- **Todas** las casillas con palomita ahora son interruptores tipo iOS:
  pista redondeada gris en OFF y degradado cian→magenta en ON, con el
  círculo blanco que se desliza con animación suave (~150 ms). Responden
  a clic y a teclado (Espacio/Enter) y guardan su estado donde ya se
  guardaba (registro). Ninguna función cambió, solo la apariencia.
- Los botones ahora tienen **difuminado (glow)** suave cian→magenta
  alrededor, más intenso al pasar el ratón.
- **Texto de Juegos corregido**: todas las letras de la pestaña Juegos
  (etiquetas, descripciones de perfiles, "Juego detectado") ahora son
  blancas o gris claro sobre el fondo oscuro. La causa del bug era que
  las casillas estándar de Windows dibujan su texto en negro (color del
  tema) y no respetan el color del fondo oscuro; al sustituirlas por
  interruptores dibujados por el programa, el texto ya es claro.

## Solo hace falta

- `LuAnOptimizer.exe` — el programa, es lo único obligatorio.
- `LuAnOptimizer.exe.manifest` — opcional, ponlo en la misma carpeta
  que el .exe para el tema moderno de Windows.

Los demás archivos (`main.c`, `logo.jpg`, `luan.ico`, `resource.rc`,
`app.manifest`, `lu-an.res`) son el código fuente y los recursos,
no hacen falta para que corra.

## La ventana: 5 pestañas

Arriba, fijo, el logo LU-AN y el título. Debajo, los perfiles rápidos
y cinco pestañas para que nada se junte en una sola pantalla
(Timer, FilterKeys, DNS, Competitivo y **Juegos**, nueva en v9):

### Pestaña Timer
Baja el temporizador de Windows de 15.6 ms a 0.5 / 1.0 / 2.0 / 5.0 ms
con slider y presets. Usa `NtSetTimerResolution` de ntdll.dll.
Se mantiene mientras la app está abierta; al cerrarla, Windows
vuelve a 15.6 ms.

Perfiles rápidos (siempre visibles, arriba de las pestañas):
**Gaming** (timer 0.5 ms, FilterKeys apagado),
**Escritura** (timer 1.0 ms, rebote 100 ms),
**Accesibilidad** (teclas lentas 500 ms, sin rebote),
**Normal** (15.6 ms, todo como viene en Windows).

### Pestaña FilterKeys (teclado)
- **BounceKeys:** ignora pulsaciones repetidas muy rápidas.
- **SlowKeys:** la tecla solo cuenta si la mantienes pulsada.
- **RepeatKeys:** retardo antes de repetir y velocidad de repetición.
- Respeta la regla de Microsoft: BounceKeys o Slow/Repeat, nunca
  los dos a la vez. El programa lo impone solo.
- Cajita para probar el teclado.

### Pestaña DNS (DNS Jumper)
- Elige tu adaptador de red activo (Wi-Fi, Ethernet...) en el combo.
- Muestra el DNS actual de ese adaptador.
- Presets: **Google** 8.8.8.8 / 8.8.4.4, **Cloudflare** 1.1.1.1 / 1.0.0.1,
  **Quad9** 9.9.9.9 / 149.112.112.112, **OpenDNS** 208.67.222.222 / 208.67.220.220,
  **AdGuard** 94.140.14.14 / 94.140.15.15 y **Automático (DHCP)**.
- Puedes editar el DNS primario y secundario a mano.
- **Aplicar DNS:** lo cambia con `netsh` (oculto, sin ventana negra).
- **Restaurar automático:** vuelve a DHCP.
- **Limpiar cache:** `ipconfig /flushdns`.
- **Probar ping:** mide la latencia al DNS primario, 4 intentos,
  y muestra la media en ms.
- Si aplicar el DNS falla, ejecuta el programa como administrador
  (clic derecho > Ejecutar como administrador).

### Pestaña Competitivo (v4, para gaming competitivo)
- **SOCD Cleaner:** hook de teclado de bajo nivel (WH_KEYBOARD_LL) para
  WASD y flechas, en su **propio hilo dedicado** con prioridad
  **TIME_CRITICAL** y proceso en **HIGH_PRIORITY_CLASS** mientras está
  activo (v7, ver abajo). Modos: **Última tecla gana** (por defecto) y
  **Neutro (ninguna)**. **Apagado por defecto.** Al activarlo aparece
  SIEMPRE un cuadro de advertencia que debes aceptar: en algunos juegos
  está prohibido, en CS2 Valve lo prohibió en 2024 y puede expulsarte
  o banearte, y lo usas bajo tu responsabilidad. Se puede desactivar
  al instante con el mismo checkbox. No toca teclas fuera de WASD/flechas.
  El cambio de tecla es **atómico**: al pulsar D con A pulsada, el
  programa traga el D-down original e inyecta en una sola llamada
  `SendInput` el lote `[A-up, D-down]`, en ese orden, con scan code
  real (`KEYEVENTF_SCANCODE`). Debajo del estado hay un medidor de
  prueba: al pulsar A/D o W/S con el SOCD activo muestra la última
  tecla y su latencia aproximada en ms (con throttle: solo en cambio
  de tecla opuesta o 1 vez cada 100 ms, para no meter tráfico UI en
  cada tecla). Mientras el SOCD está activo, las animaciones de
  botones y pestañas se pausan (la ventana queda estática; lo
  importante es el teclado) y se reanudan solas al desactivarlo.
- **Modo Competitivo (1 clic):** timer 0.5 ms, FilterKeys apagado,
  limpiar caché DNS, plan de energía Alto rendimiento, Game Mode
  activado, y mide los 5 DNS para recomendarte el más rápido.
  La medición de DNS ahora es **asíncrona** (v6): ya no congela la
  ventana ni frena el teclado mientras mide; el resultado llega solo
  y se registra en el log de la pestaña.
  Muestra qué cambió línea por línea en el registro de la pestaña.
  **Restaurar normal** vuelve a timer 15.6 ms, al plan de energía
  anterior y a Game Mode como estaba.
- **Ping en vivo:** mide cada 1 s el ping al DNS primario y muestra
  ping actual, media, jitter y pérdida de paquetes (%).
  **Probar todos los DNS** hace ping a los 5 presets y te recomienda
  el de menor media (y lo deja puesto en el campo de DNS).
- **Optimizaciones de sistema** (cada una con su checkbox, reversible):
  quitar la aceleración del mouse (precisión mejorada),
  quitar las optimizaciones de pantalla completa,
  **cerrar apps en segundo plano** (te muestra la lista —navegadores,
  Discord, Spotify, launchers— y pide confirmación antes de cerrar;
  nunca cierra el sistema ni el propio programa).
- **Limpieza de memoria standby (tipo ISLC):** botón que purga la
  lista standby con `NtSetSystemInformation` y muestra la memoria
  disponible antes/después. Opción de limpiarla cada 15 min
  automáticamente con un checkbox.
- El DNS y el plan de energía necesitan administrador para cambiar;
  el programa avisa si falla.

## Corrección v7: el delay RESIDUAL del SOCD (causa real y arreglo)

**Síntoma:** en v6 el delay grande ya no estaba, pero Luis notó que
todavía quedaba algo de delay, y el medidor de prueba marcaba **0 ms**.

**Causa (verificada en el código):** el medidor en 0 ms significa que
el hook recibía la tecla al instante (`ahora - k->time = 0`), así que
el delay NO estaba en recibir la tecla ni en el hilo del hook. Estaba
aguas abajo, en **cómo se entregaba el cambio a Windows/juego**. En
modo última tecla gana, al pulsar D con A pulsada el código hacía
`SendInput(A-up)` y luego dejaba pasar el D-down original con
`CallNextHookEx`. El A-up sintético quedaba en cola **detrás** del
D-down original, así que el juego veía D-down primero y A-up después:
un instante con las dos pulsadas, que se siente como delay al hacer
counter-strafe. El medidor no lo veía porque mide hasta el hook, no
hasta el juego.

**Arreglo (el cambio clave):** al pulsar la tecla opuesta, ahora se
**traga el evento original** (`return 1`) y se inyecta en **una sola
llamada `SendInput` con 2 INPUT** el lote `[opuesta-up, nueva-down]`,
en ese orden, atómico. El juego ya no ve nunca las dos a la vez.
Lo mismo al soltar: al reactivar la opuesta se inyecta el lote
`[esta-up, opuesta-down]` en una sola llamada (o solo el down de la
opuesta si Windows ya creía esta tecla suelta). Los eventos inyectados
vuelven por el hook marcados `LLKHF_INJECTED` y se dejan pasar sin
procesar, así no hay bucles.

**Arreglos secundarios (v7):**
- El hilo del SOCD ahora corre con prioridad **TIME_CRITICAL** y,
  mientras el SOCD está activo, el proceso sube a
  **HIGH_PRIORITY_CLASS** (se restaura al desactivar). Con prioridad
  normal el hilo competía con la UI y el juego.
- Los eventos sintéticos llevan el **scan code real** con
  `MapVirtualKeyA` y la bandera `KEYEVENTF_SCANCODE` (más
  `KEYEVENTF_EXTENDEDKEY` en las flechas). Muchos juegos leen el
  teclado por Raw Input / scan code; solo con `wVk` el evento tarda
  más en traducirse.
- El **medidor de prueba** ya no hace `PostMessage` a la UI en cada
  tecla: solo avisa cuando hay cambio de tecla opuesta (A→D o D→A)
  o, como mucho, 1 vez cada 100 ms (throttle con `GetTickCount`).
- Las **animaciones se pausan mientras el SOCD está activo**: los
  timers de 16 ms de botones, barra de pestañas y deslizamiento de
  página se detienen (dejando todo en su estado final) y se reanudan
  solos al desactivar el SOCD. La ventana puede quedarse estática
  mientras se juega; lo importante es el teclado.
- La espera al instalar el hook bajó de 1000 a 500 ms.

**Límite honesto:** un hook `WH_KEYBOARD_LL` + `SendInput` siempre
añade algo de latencia frente al SOCD por firmware/driver de un
teclado con SOCD nativo (como el de Wooting o Razer). Con el cambio
atómico el delay que quedaba por el orden de entrega desaparece, pero
no se puede llegar a 0 ms con un hook de software: la única forma de
cero latencia añadida es el SOCD en el firmware del teclado.

## Corrección v6: el delay del SOCD (causa y arreglo)

**Síntoma:** al pulsar Modo Competitivo y luego activar el SOCD, las
teclas llegaban con mucho delay.

**Causa (verificada en el código):** los hooks `WH_KEYBOARD_LL` se
ejecutan en el contexto del hilo que los instaló. En v5 el hook vivía
en el hilo de la UI, y ese hilo se bloqueaba hasta 1 s en cada tick
del ping en vivo (`IcmpSendEcho` con timeout de 1000 ms) y hasta
7,5 s al medir los 5 DNS del Modo Competitivo (5 × 3 intentos × 500 ms).
Mientras la UI estaba bloqueada, cada tecla tenía que esperar su turno:
ese era el delay. El hook en sí era ligero, el problema era dónde vivía.

**Arreglo:**
- El SOCD ahora corre en un **hilo dedicado** con su propio bucle de
  mensajes: el hook se instala dentro de ese hilo (así los callbacks
  corren ahí, no en la UI) y se desinstala ahí al desactivar, soltando
  antes cualquier tecla que quedara suprimida. La UI solo le pide
  activar/desactivar; el estado se comparte con variables `volatile`
  e `InterlockedExchange`. El hook no toca la UI, no hace I/O ni Sleep,
  y la repetición automática de tecla sigue filtrada como antes.
- El **ping en vivo** corre en su propio hilo worker (timeout de
  300 ms por ping) y el **"Probar todos los DNS"** y el **"Probar ping"**
  corren en hilos de un solo uso; la UI solo recibe los resultados
  con `PostMessage` (`WM_PING_UPDATE` / `WM_PING_DONE`) y nunca se
  congela mientras mide. Si ya hay una medición en curso, el programa
  avisa y no lanza otra encima.
- La **medición de DNS del Modo Competitivo es asíncrona**: el modo
  se activa al instante y el DNS más rápido se registra en el log
  cuando la medición termina.
- **Medidor de prueba:** en la pestaña Competitivo, debajo del estado
  del SOCD, al pulsar A/D o W/S con el SOCD activo se muestra la
  última tecla y su latencia aproximada (tiempo entre que se generó
  la tecla y la procesó el hook). Si todo va bien debe marcar pocos ms.

## Aviso legal del SOCD

El SOCD Cleaner modifica cómo Windows interpreta dos teclas opuestas
pulsadas a la vez. Algunos juegos y anti-cheats lo consideran trampa:
**en CS2 está prohibido desde 2024** y puede costarte una expulsión
o un baneo. En otros juegos las reglas cambian con el tiempo.
Lu-An Optimizer lo trae **apagado por defecto** y te avisa cada vez
que lo activas. Úsalo solo en juegos donde esté permitido y bajo tu
propia responsabilidad.

## Diseño (v5 premium)

- **Pestañas premium:** la barra de pestañas ya no es el control
  estándar de Windows (que se veía plano y rompía el diseño). Ahora
  es una barra dibujada a mano (`LuAnTabBar`) con el mismo tema
  oscuro del resto del programa:
  - Cada pestaña es una "píldora" redondeada con su **glifo dibujado**
    (reloj para Timer, teclado para FilterKeys, globo para DNS,
    rayo para Competitivo) + texto en Segoe UI Semibold.
  - **Pestaña activa:** fondo de tarjeta con borde acento, texto
    blanco brillante y una barra con **gradiente cian → magenta**
    (los colores del logo LU-AN).
  - **Indicador deslizante:** esa barra de gradiente no salta de una
    pestaña a otra, **se desliza** con animación (timer de 16 ms).
  - **Hover con fundido:** al pasar el mouse la pestaña se ilumina
    poco a poco, igual que los botones.
  - **Transición al cambiar de pestaña:** la página nueva entra
    deslizándose suavemente desde la derecha, sin parpadeo
    (dibujo con doble búfer).
  - Se puede cambiar de pestaña con clic, y también con las
    **flechas izquierda/derecha** del teclado cuando la barra
    tiene el foco. El cursor cambia a mano sobre las pestañas.
  - Cada página tiene un **marco redondeado** con borde sutil,
    y bajo los perfiles hay una **línea divisoria con gradiente
    cian → magenta**.
- **Icono:** el logo del planeta LU-AN está incrustado en el .exe
  (6 tamaños: 16, 32, 48, 64, 128 y 256 px), así se ve en el
  Explorador, en la barra de tareas y en la ventana.
- **Tipografía:** estilo iOS. San Francisco es la fuente de Apple y
  no se puede distribuir en Windows, así que se usa su equivalente
  limpio en Windows: **Segoe UI Variable Text** (con fallback a
  Segoe UI en equipos antiguos): título grande en semibold, secciones
  en 17 semibold, cuerpo en 15 regular.
- **Botones animados:** botones con esquinas redondeadas que se
  iluminan con un fundido suave al pasar el mouse y se encogen un
  poco (pulso) al pulsarlos. El clic sigue funcionando igual.
  (v7: mientras el SOCD está activo las animaciones se pausan para
  no robarle CPU al teclado; se reanudan solas al desactivarlo.)
- **Bug corregido en v5:** al reparentar los controles a sus páginas,
  la página Competitivo no estaba en la lista de exclusión y se
  reparentaba dentro de la página Timer, así que su pestaña quedaba
  vacía. Ahora las 4 páginas se muestran correctamente.

## Bandeja del sistema (v8): minimizar en segundo plano

La **X siempre cierra el programa como antes** (desinstala el SOCD,
restaura el timer a 15.6 ms y sale de verdad). Lo nuevo es una casilla
en el pie de la ventana, **"Minimizar en segundo plano"** (marcada por
defecto, se recuerda en `HKCU\Software\LuAnOptimizer`):

- **Marcada:** el botón minimizar (_) oculta la ventana y deja el
  icono LU-AN en la bandeja del sistema, junto al reloj. Todo sigue
  funcionando en segundo plano: el timer (0.5 ms se mantiene mientras
  el proceso vive), el SOCD, el ping en vivo y la limpieza automática
  de memoria. El icono de la bandeja **solo** aparece cuando se
  minimiza en segundo plano.
- **Desmarcada:** minimizar es el minimizar normal a la barra de
  tareas, como siempre.

Con el icono en la bandeja:

- **Doble clic** en el icono: restaura la ventana (y el icono de la
  bandeja desaparece, porque la ventana ya está visible).
- **Clic derecho** en el icono: menú con **Mostrar Lu-An Optimizer**,
  **Modo Competitivo** (lo activa sin abrir la ventana), **Activar /
  Desactivar SOCD** (al activar muestra la misma advertencia
  obligatoria, incl. CS2) y **Salir** (esta sí cierra de verdad).
- El tooltip del icono muestra el estado actual: timer activo y si
  el SOCD está ON u OFF. La primera vez que se minimiza a la bandeja
  sale un aviso: "Sigue corriendo en segundo plano".
- Si el Explorador de Windows se reinicia, el icono se repone solo.

## Novedades v9

### Auto-actualización (pestaña Competitivo, sección Actualizaciones)
- Al arrancar, en un hilo de trabajo (nunca bloquea la UI ni el juego),
  revisa si hay versión nueva: primero consulta `version.txt` en
  `raw.githubusercontent.com/LuAnOptimizer/lu-an-optimizer/main/version.txt`
  (formato simple `version=` / `sha256=` / `url=`) y si falla, la API de
  GitHub Releases (`api.github.com/repos/LuAnOptimizer/lu-an-optimizer/releases/latest`,
  con WinINet, timeout ~5 s, User-Agent "LuAnOptimizer"). Compara el
  `tag_name` ("v9", "v10"...) con la versión del programa (`APP_VERSION "9.0.0"`).
- **Casilla "Buscar actualizaciones al iniciar"** (marcada por defecto,
  guardada en `HKCU\Software\LuAnOptimizer`, `AutoUpdateCheck`). Si la
  desmarcas, solo se revisa con el botón **"Buscar actualizaciones"**.
- Si hay versión nueva, pregunta con un cuadro: "Hay una vX nueva,
  ¿quieres actualizar?". **Nunca actualiza sin permiso, y nunca en mitad
  de una partida**: si el Modo Competitivo está activo, solo avisa, no
  descarga nada solo.
- Si aceptas, descarga el `LuAnOptimizer.exe` del release a
  `%TEMP%\LuAnOptimizer_new.exe` (con `URLDownloadToFile`) y verifica su
  **SHA256 con BCrypt** (la implementación de Windows, nunca una propia)
  contra el hash publicado en el release (línea `SHA256: <hash>` en el
  body o `version.txt`). Si no hay hash disponible, avisa "no se pudo
  verificar" y pide confirmación extra antes de instalar. Si el hash no
  coincide, cancela la actualización.
- El .exe no se puede sobrescribir a sí mismo: se escribe un `.bat`
  temporal en `%TEMP%` que espera a que el proceso salga, mueve el exe
  nuevo sobre el viejo, borra el temporal y vuelve a abrir el programa.
  El programa, antes de salir, hace su limpieza normal (desinstala el
  SOCD, restaura el timer).
- **Limitación honesta (importante): el repo aún NO está publicado en
  GitHub** (la cuenta de Luis todavía no está conectada), así que hoy la
  consulta falla y el programa solo pone en la barra de estado
  "No se pudo revisar actualizaciones" y sigue normal. **Eso es lo
  esperado**: la auto-actualización no encontrará nada hasta que Luis
  conecte GitHub, cree el repo `LuAnOptimizer/lu-an-optimizer` y publique
  un release (con el .exe y su SHA256, o con `version.txt` en la rama
  principal). Cuando el repo exista, funcionará sin tocar el programa.
  En el repo local ya está `version.txt` con la v9 y su SHA256 real.

### Perfiles por juego (pestaña Juegos, nueva, 5ª pestaña)
- Perfiles predefinidos, cada uno con casillas **"Activar
  automáticamente"**, **SOCD**, **"Timer 0.5 ms (si no, 1.0 ms)"** y
  **"Modo Competitivo"**, guardados en
  `HKCU\Software\LuAnOptimizer\Games\<juego>`:
  - **Valorant** (`VALORANT-Win64-Shipping.exe`): timer 0.5, FilterKeys
    off, SOCD ON (última tecla gana), Game Mode on.
  - **CS2** (`cs2.exe`): timer 0.5, FilterKeys off, **SOCD FORZADO OFF**
    con el texto "SOCD prohibido en CS2 (Valve, 2024)": si intentas
    prenderlo, se bloquea y muestra la advertencia. **Nunca se aplica
    SOCD en CS2 bajo ningún concepto.**
  - **Fortnite** (`FortniteClient-Win64-Shipping.exe`): timer 0.5,
    SOCD ON, Game Mode on.
  - **Apex Legends** (`r5apex.exe`): timer 0.5, SOCD ON.
  - **Otro juego**: genérico, escribes cualquier `.exe` en el campo.
- Detección: un hilo de trabajo revisa cada 2 s los procesos
  (`CreateToolhelp32Snapshot` + `Process32First/Next`). Al arrancar un
  juego con perfil activo: guarda los ajustes actuales, aplica su perfil
  (con el mismo código del Modo Competitivo, sin bloquear la UI) y avisa
  en la barra de estado y en la pestaña: "Perfil Valorant aplicado".
  Al cerrarse el juego: **restaura los ajustes guardados**. Solo se
  aplica una vez por cambio (arranca/cierra), así que si cambias algo a
  mano mientras juegas, no se te pisa. Arriba de la pestaña se muestra
  el estado: "Juego detectado: Valorant" o "Ningún juego detectado".
- El SOCD automático no muestra el popup de advertencia en mitad del
  juego: el consentimiento es la casilla del perfil (y la advertencia
  sigue apareciendo siempre al activar el SOCD a mano en Competitivo).

### Monitor en pantalla / overlay (casilla "Mostrar monitor" en Competitivo)
- Ventana pequeña **siempre encima** (TOPMOST + LAYERED, sin borde,
  fondo oscuro semitransparente, texto claro), **arrastrable con el
  ratón** por cualquier parte, con botón **X** chiquito para cerrarla.
  Recuerda su posición en el registro (`OverlayX` / `OverlayY`).
  Se prende/apaga con la casilla **"Mostrar monitor"** en la pestaña
  Competitivo (también se recuerda).
- Contenido, actualizado cada 1 s: **Ping** (el ping en vivo que ya
  existe), **Jitter**, **Pérdida %**, **CPU %** (`GetSystemTimes`,
  delta), **RAM %** (`GlobalMemoryStatusEx`), **Timer actual** (0.5 ms
  etc.), **SOCD ON/OFF** y **FPS (pantalla)**.
- **FPS (pantalla):** usa las estadísticas de presentación de DXGI del
  monitor principal (`IDXGIOutput::GetFrameStatistics`, contador de
  presents por segundo) como aproximación de la frecuencia de
  presentación de la pantalla. **NO es el FPS del juego**: el FPS real
  del juego no se puede leer sin enganchar DirectX, lo que los
  anti-cheats pueden detectar, así que no se intenta. El overlay lo
  etiqueta "FPS (pantalla)" y lo aclara en su propio texto
  ("FPS = pantalla, no del juego").
- **Temperatura:** se intenta leer por WMI
  (`MSAcpi_ThermalZoneTemperature` en `root\WMI`, Kelvin×10). Si no hay
  sensor disponible (lo más común en PC de escritorio), muestra
  **"Temp: N/D"** en vez de inventar un número. Si algún valor no se
  puede medir, muestra N/D, nunca un valor falso.
- **No interfiere con el juego:** no roba el foco (se muestra con
  `SW_SHOWNOACTIVATE`) y tiene la opción **"No interferir
  (click-through)"** que lo pone `WS_EX_TRANSPARENT` (los clics pasan
  a través de él; para moverlo o cerrarlo, quita esa casilla en la
  ventana principal).

## Si Windows te avisa

Como es un .exe nuevo sin firma, Windows SmartScreen puede decir
"Windows protegió su PC". Pulsa **Más información** y luego
**Ejecutar de todas formas**. Es normal en programas caseros sin
certificado de pago.

## Cómo se compiló

En Linux, con Zig 0.15.1 (incluye el compilador C para Windows).
Primero los recursos (icono + manifiesto + logo) y luego el .exe:

```
zig rc /v resource.rc lu-an.res
zig cc -target x86_64-windows-gnu -O2 -D_WIN32_WINNT=0x0601 main.c lu-an.res \
  -o LuAnOptimizer.exe -lcomctl32 -lgdi32 -luser32 -lkernel32 \
  -liphlpapi -lws2_32 -lgdiplus -lole32 -ladvapi32 \
  -lwininet -lurlmon -lbcrypt -ldxgi -lwbemuuid -loleaut32 -Wl,--subsystem,windows
cp app.manifest LuAnOptimizer.exe.manifest
```

Notas de compilación:
- `zig rc` con `/fo` sale con código 0 pero **no escribe el .res**;
  la salida es el argumento posicional (`zig rc resource.rc lu-an.res`).
  La primera ejecución de `zig rc` puede tardar ~1-2 min por la caché.
- `-lole32` hace falta por `CreateStreamOnHGlobal` (logo GDI+).
- v9 añade: `-lwininet` (consulta de actualizaciones) y `-lurlmon`
  (descarga del .exe nuevo), `-lbcrypt` (SHA256), `-ldxgi` (FPS de
  pantalla del overlay) y `-lwbemuuid` + `-loleaut32` (temperatura por WMI).
- `resource.rc` ahora incluye también el recurso de versión
  (`VS_VERSION_INFO`, FileVersion 9.0.0.0).
- `winsock2.h` debe incluirse antes que `iphlpapi.h`, si no
  `IP_ADAPTER_ADDRESSES` no se declara en los headers de MinGW/Zig.
- Zig vive en `/tmp` y se borra al reiniciar la máquina; hay una copia
  del instalador en `~/workspace/tools/zig-0.15.1.tar.xz`.

## Verificación (v9)

- PE32+ x64 (0x8664), subsistema 2 (GUI Windows), OS version 6.0.
- SHA256: `f45f19150ded3a5d4457a0203b4281e5754719dafecb305882b893c6e6ca9ac1`
- Tamaño: 498 KB (509.952 bytes). Ventana 700x910, 5 pestañas premium.
- Versión incrustada: FileVersion **9.0.0.0** (VS_VERSION_INFO, verificado
  con pefile) y `APP_VERSION "9.0.0"` en el código.
- La X **siempre** cierra el programa: verificado con grep, no existe
  `CloseToTray` ni ninguna lógica de cerrar-a-bandeja en `main.c`.
- **Icono verificado en el directorio de recursos del PE** (con pefile):
  `RT_GROUP_ICON` (tipo 14, id 100) con 6 entradas y `RT_ICON`
  (tipo 3) con 6 imágenes: 16, 32, 48, 64, 128 y 256 px a 32 bpp.
  El icono de 256 px extraído del .exe es el planeta LU-AN.
  También están `RT_MANIFEST` (id 1) y `RT_RCDATA` (id 101, logo.jpg).
  Además, `RT_VERSION` (tipo 16) con FileVersion 9.0.0.0.
- Contiene los strings `NtSetTimerResolution`, `SystemParametersInfoW`,
  `Lu-An Optimizer`, `Segoe UI Variable`, `8.8.8.8`, `1.1.1.1`,
  `GetAdaptersAddresses`, `IcmpSendEcho`, `SOCD`, `Competitivo`,
  `NtSetSystemInformation`, `powercfg`, `AllowAutoGameMode`,
  `GameDVR_FSEBehaviorMode`.
- Solo depende de DLLs del sistema (USER32, GDI32, KERNEL32,
  COMCTL32, IPHLPAPI, WS2_32, gdiplus, ole32, ADVAPI32, WININET, URLMON,
  bcrypt, dxgi, OLEAUT32 y UCRT `api-ms-win-crt-*`).
- **No probado corriendo en Windows real**: este servidor es Linux
  y no tiene Wine. La primera prueba debe ser en tu PC, en este orden:
  1. que se vean las 4 pestañas y el icono en el Explorador;
  2. Modo Competitivo y luego Restaurar normal;
  3. ping en vivo y Probar todos los DNS;
  4. SOCD solo en un juego donde esté permitido (NUNCA en CS2),
     y comprobar que A/D responde a la última tecla y que al
     desactivarlo el teclado vuelve a lo normal.
  5. **Prueba del delay (v6):** pulsa Modo Competitivo, luego activa
     el SOCD y pulsa A/D: ya no debe haber delay, y el medidor de la
     pestaña Competitivo debe marcar pocos ms. Repite la prueba con
     el ping en vivo activado y mientras se mide "Probar todos los DNS".
  6. **Prueba del delay residual (v7):** con el SOCD activo, haz
     counter-strafe A/D rápido varias veces: el cambio debe sentirse
     instantáneo, sin ese instante en que el personaje se queda quieto
     o se mueve a los dos lados. El medidor seguirá marcando 0 ms
     (mide hasta el hook, no hasta el juego); lo que importa es cómo
     se siente en el juego. Si todavía se nota algo, lo que queda es
     la latencia propia del hook de software (ver el límite honesto
     en la sección v7): ningún programa con hook puede igualar el
     SOCD por firmware del teclado.
  7. **Prueba de la bandeja (v8):** con la casilla "Minimizar en
     segundo plano" marcada, pulsa el botón minimizar (_): la ventana
     debe desaparecer y quedar solo el icono LU-AN en la bandeja; el
     timer y el SOCD deben seguir funcionando. Doble clic en el icono
     restaura la ventana. Con la casilla desmarcada, minimizar debe
     ser el normal a la barra de tareas. La X debe cerrar el programa
     siempre (el icono de la bandeja no debe quedar).
  8. **Prueba de la auto-actualización (v9):** al arrancar, la barra de
     estado debe decir "No se pudo revisar actualizaciones" (el repo
     aún no está publicado: **eso es lo esperado hoy**). Pulsa "Buscar
     actualizaciones" en Competitivo: debe decir lo mismo, sin ningún
     error ni ventana. Cuando el repo esté publicado con un release
     v10, debe avisar y, si aceptas, descargar, verificar el SHA256,
     reemplazar el .exe y reabrir el programa. Prueba también con el
     Modo Competitivo activo: solo debe avisar, sin descargar.
  9. **Prueba de perfiles por juego (v9):** en la pestaña Juegos,
     activa "Activar automáticamente" en Valorant y abre Valorant: debe
     aparecer "Juego detectado: Valorant" y aplicarse su perfil (timer
     0.5, SOCD ON). Al cerrar Valorant, debe restaurar tus ajustes.
     En CS2, intenta marcar su SOCD: debe bloquearse con la advertencia
     "SOCD prohibido en CS2 (Valve, 2024)", y al abrir CS2 el SOCD debe
     quedar OFF aunque su perfil esté activo.
  10. **Prueba del monitor (v9):** marca "Mostrar monitor" en
     Competitivo: debe aparecer la ventanita siempre encima con ping,
     CPU, RAM, timer, SOCD y FPS (pantalla). Arrástrala a otra esquina,
     cierra y vuelve a abrir el programa: debe recordar dónde estaba.
     Activa "No interferir (click-through)": los clics deben pasar a
     través de ella al juego. La temperatura probablemente dirá N/D
     (normal en escritorio sin sensor).
- En Windows 7 el UCRT (`api-ms-win-crt-*`) viene con Windows Update
  (KB2999226); en un Win7 sin actualizar podría faltar.
- La limpieza de memoria standby y el cambio de plan de energía
  pueden necesitar administrador.

## Code signing policy

Free code signing provided by SignPath.io, certificate by SignPath Foundation.

- **Authors / committers and reviewers:** the Lu-An Gaming Optimization maintainer(s) — the repository owner account. Every change is built by GitHub Actions from the source in this repository; no binary is signed that was not built by that automated build.
- **Approvers:** the repository owner approves each signing request before a release is signed.
- **Privacy policy:** This program will not transfer any information to other networked systems unless specifically requested by the user or the person installing or operating it. It sends ICMP pings to DNS and game servers you choose, and, when the update check is on, it asks GitHub for the latest release information and downloads a new version only with your permission.

Releases are code-signed for free by **SignPath Foundation** (see `.github/workflows/build.yml`). The publisher shown by Windows is therefore "SignPath Foundation", not "Lu-An".

## License

MIT — see [LICENSE](LICENSE).
