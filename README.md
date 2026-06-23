# Sistema de Control Adaptativo de LED con IoT

**Control inteligente de iluminación basado en luz ambiente - Red Local WiFi**

## Características

- ESP32 configurado como Punto de Acceso WiFi (AP)
- Sensor LDR para medición de luz ambiente
- Regulación automática de tira LED por PWM
- Servidor backend en Python Flask
- Interfaz gráfica (Dashboard) con visualización en tiempo real
- Almacenamiento local persistente en formato CSV
- Estructura de código desacoplada y modular
- Documentación y comentarios en español

## Estructura del Proyecto

```
IoT-Control-LED/
├── firmware/          → Código fuente para el ESP32 (PlatformIO / Arduino)
├── backend/           → Servidor REST API en Python Flask
├── frontend/          → Dashboard web de monitoreo (HTML, CSS y JS)
├── data/              → Carpeta de base de datos local en formato CSV
├── docs/              → Documentación técnica adicional
└── README.md          → Archivo de documentación general
```

## Guía de Instalación y Configuración

### Paso 1: Preparación del Hardware

**Componentes necesarios:**
- ESP32 (DevKit v1 o compatible)
- Sensor de luz LDR
- Resistencia de 10kΩ (para el divisor de tensión del LDR)
- Tira LED o diodo LED PWM
- Transistor MOSFET (por ejemplo, módulo IRF520) si se utiliza tira LED de potencia
- Cables de conexión y fuente de alimentación adecuada

**Conexiones básicas:**
- Divisor de tensión del LDR → Entrada analógica GPIO 34 (ADC)
- Señal de control del LED → Salida digital GPIO 25 (PWM)
- Conexión común a Tierra → GND

### Paso 2: Carga del Firmware

```bash
# 1. Instalar PlatformIO CLI
# 2. Navegar a la carpeta del firmware
cd firmware

# 3. Compilar y subir el código al microcontrolador
pio run -t upload

# 4. Iniciar el monitor serial para depuración
pio device monitor --baud 115200
```

### Paso 3: Configuración del Backend

```bash
# 1. Acceder al directorio del backend
cd backend

# 2. Crear y activar el entorno virtual
python -m venv venv
# En Windows:
venv\Scripts\activate
# En macOS/Linux:
source venv/bin/activate

# 3. Instalar las librerías necesarias
pip install -r requirements.txt

# 4. Iniciar el servidor Flask
python app.py
```

El servidor local se levantará por defecto en `http://localhost:5000`.

### Paso 4: Configuración del Frontend

1. El frontend es estático y no requiere dependencias de servidor complejas.
2. Puede abrirse directamente haciendo doble clic sobre el archivo `frontend/index.html` en el navegador web.

### Paso 5: Conectividad y Acceso en Red Local

Una vez encendido el ESP32, creará su propio Punto de Acceso WiFi con las siguientes credenciales:

- **WiFi SSID**: `Esp32-Tópicos IV`
- **Contraseña**: `12345678`

Al conectar tu laptop a esta red local:
- La laptop recibirá automáticamente la IP `192.168.4.2`.
- El ESP32 operará en la IP `192.168.4.1`.
- El flujo de comunicación se establecerá enviando las peticiones HTTP desde el ESP32 hacia el servidor en `http://192.168.4.2:5000`.

---

## Flujo de Operación

1. **Inicialización**: El ESP32 inicia en modo de Punto de Acceso WiFi.
2. **Adquisición**: Realiza lecturas continuas del sensor LDR cada 1 segundo.
3. **Procesamiento**: Determina el nivel de salida PWM óptimo para controlar la tira LED.
4. **Actuación**: Modifica la luminosidad física del LED de manera automática.
5. **Transmisión**: Cada 5 segundos, el ESP32 realiza un envío HTTP POST en JSON a la laptop.
6. **Almacenamiento**: El backend en Flask recibe los datos y los escribe incrementalmente en el archivo CSV local.
7. **Monitoreo**: El frontend en Javascript realiza peticiones periódicas al backend para refrescar y graficar los datos en tiempo real.

---

## API REST del Backend

### Recibir datos del ESP32
- **Método**: `POST`
- **Ruta**: `/api/sensor/data`
- **Cabecera**: `Content-Type: application/json`
- **Formato del JSON**:
```json
{
  "timestamp": "2026-06-18T10:30:45",
  "lux": 450.0,
  "led_intensity": 75,
  "temperature": 28.5
}
```

### Obtener últimos datos registrados
- **Método**: `GET`
- **Ruta**: `/api/data/latest?limit=20`

### Obtener datos filtrados por rango de fechas
- **Método**: `GET`
- **Ruta**: `/api/data/range?from=2026-06-18T00:00:00&to=2026-06-18T23:59:59`

### Obtener estadísticas básicas agregadas
- **Método**: `GET`
- **Ruta**: `/api/stats`

---

## Base de Datos (CSV)

Ruta del archivo físico: `data/sensor_data.csv`

Estructura de almacenamiento:
```csv
timestamp,lux,led_intensity,temperature
2026-06-18T10:30:45,450.0,75,28.5
2026-06-18T10:31:45,480.0,80,28.7
```

---

## Mantenimiento y Extensibilidad

### Integración de Nuevos Sensores
1. **Firmware**: Modificar `firmware/main/src/main.cpp` para declarar el pin del nuevo sensor y la función de lectura correspondiente.
2. **Backend**: Actualizar el modelo y la validación en `backend/models/sensor_data.py` y `backend/utils/validators.py`.
3. **Frontend**: Actualizar las tarjetas métricas y gráficos en `frontend/index.html` y `frontend/js/app.js`.

### Escalabilidad de la Arquitectura
La solución está preparada para migrarse a entornos con bases de datos relacionales (como SQLite o PostgreSQL) y soporta la conexión paralela de múltiples nodos ESP32.

---

## Resolución de Problemas (Troubleshooting)

**Fallo de comunicación del ESP32:**
- Verificar que el microcontrolador recibe alimentación estable (3.3V / 5V).
- Verificar que la laptop está efectivamente conectada al SSID `Esp32-Tópicos IV`.
- Monitorear la salida serial para verificar la existencia de códigos de error de conexión HTTP.

**El backend no recibe información:**
- Confirmar que la dirección IP configurada en el firmware del ESP32 corresponde a la IP de la laptop (`192.168.4.2`).
- Comprobar que el firewall del sistema operativo no esté bloqueando el tráfico entrante en el puerto `5000`.

**Dashboard sin visualización de datos:**
- Abrir la consola del desarrollador en el navegador (tecla F12) para descartar errores de CORS.
- Asegurarse de que el backend Flask esté en ejecución y respondiendo en el puerto configurado.

---

## Referencias Técnicas

- [Documentación Oficial de ESP32](https://docs.espressif.com/projects/esp-idf/)
- [Guía de Uso de Flask](https://flask.palletsprojects.com/)
- [Visualización Interactiva con Chart.js](https://www.chartjs.org/)
- [Manual de PlatformIO CLI](https://docs.platformio.org/)

Ver más información en: [docs/ARQUITECTURA.md](docs/ARQUITECTURA.md)
