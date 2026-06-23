# Resumen Ejecutivo del Proyecto

## Objetivo

Desarrollar un Sistema IoT de Control Adaptativo de LED basado en luz ambiente, el cual realiza las siguientes funciones:
- Medir la iluminancia del entorno utilizando un sensor LDR.
- Controlar de manera automática la intensidad lumínica de un LED por PWM.
- Transmitir la telemetría en tiempo real a un servidor backend local.
- Visualizar los datos históricos en un dashboard interactivo web.

---

## Componentes del Proyecto

### 1. Firmware ESP32
**Ubicación**: `/firmware`

**Funcionalidades**:
- Configura e inicia un Punto de Acceso (AP) WiFi.
- Realiza lecturas analógicas del sensor LDR con un intervalo de 1 segundo.
- Regula la luminosidad del LED a través de modulación por ancho de pulso (PWM).
- Envía peticiones HTTP POST con carga JSON al backend cada 5 segundos.
- Arquitectura de software modular orientada a tareas.
- Código fuente implementado en C++ para entorno PlatformIO / Arduino.

**Especificaciones de hardware**:
- GPIO 34: Entrada analógica (Conversor ADC) para el LDR.
- GPIO 25: Salida digital PWM para control del LED.
- Red WiFi local: SSID `Esp32-Tópicos IV` / Contraseña `12345678`.

### 2. Backend en Python y Flask
**Ubicación**: `/backend`

**Funcionalidades**:
- Recibe peticiones HTTP POST del ESP32 con formato JSON.
- Valida la integridad y rangos físicos de los datos recibidos.
- Almacena la telemetría de forma persistente y estructurada en formato CSV.
- Expone endpoints REST para la consulta de información histórica.
- Configuración de CORS habilitada para permitir accesos remotos del frontend.
- Proporciona cálculos de estadísticas agregadas en tiempo real.

**Endpoints disponibles**:
- `POST /api/sensor/data` - Recepción de datos del sensor.
- `GET /api/data/latest` - Recuperación de los últimos registros.
- `GET /api/data/range` - Consulta filtrada por rango de fechas.
- `GET /api/stats` - Estadísticas agregadas.
- `GET /api/health` - Health check de estado del servidor.

### 3. Frontend Web de Monitoreo
**Ubicación**: `/frontend`

**Funcionalidades**:
- Panel de control de interfaz de usuario de página única (SPA).
- Gráficos interactivos en tiempo real a través de Chart.js.
- Tabla dinámica de registros históricos.
- Tarjetas informativas con métricas del sensor y PWM en vivo.
- Controles interactivos para habilitar/pausar la actualización de datos.
- Función de descarga de históricos en formato CSV.
- Indicador de estado de conexión del backend.
- Diseño fluido adaptado a múltiples dispositivos (móvil, tableta y ordenador).

---

## Arquitectura General

```
┌─────────────────────────────────────────────────────┐
│              RED LOCAL WiFi (AP ESP32)              │
│                  192.168.4.1/24                      │
└─────────────────────────────────────────────────────┘
         │                           │
    ┌────▼────┐              ┌──────▼──────┐
    │  ESP32   │◄────────────►│  Backend    │
    │(AP+I/O)  │  HTTP JSON   │  Flask/CSV  │
    └────┬────┘              └──────┬──────┘
         │                          │
    ┌────▼────┐              ┌──────▼──────┐
    │ LDR+LED  │              │  Frontend   │
    │          │              │  HTML/JS    │
    └──────────┘              └─────────────┘
```

---

## Flujo de Datos

1. **Adquisición**: El sensor LDR captura el voltaje del divisor de tensión en el pin GPIO 34.
2. **Normalización**: El ESP32 convierte el nivel analógico a un rango porcentual (0-100%).
3. **Control**: Se calcula la acción de control inversa (a menor luz ambiente, mayor intensidad LED).
4. **PWM**: Se ajusta el ciclo de trabajo en el canal de salida GPIO 25 (rango 0-255).
5. **Transmisión**: Los datos de estado se transmiten vía HTTP POST cada 5 segundos al backend.
6. **Persistencia**: El backend añade el registro en el archivo de base de datos local `sensor_data.csv`.
7. **Consumo**: El frontend web solicita periódicamente información mediante peticiones GET.
8. **Renderizado**: La interfaz del navegador web actualiza los gráficos e indicadores en tiempo real.

---

## Tecnologías Utilizadas

| Capa | Tecnología | Función |
|------|-----------|---------|
| Firmware | C++ / framework Arduino | Control de hardware e interfaz física |
| Backend | Python | Lógica de servidor y control del almacenamiento |
| Framework Backend | Flask 3.0.0 | Definición y ruteo de la REST API |
| Frontend | JavaScript (Vanilla) | Interacción dinámica de la vista cliente |
| Gráficos | Chart.js 4.4.0 | Visualización estadística temporal |
| Persistencia | Formato CSV plano | Almacenamiento simple de registros |
| Red local | Red WiFi local (softAP) | Medio físico de transporte de datos |

---

## Instrucciones de Ejecución

### Paso 1: Cargar el Firmware en el ESP32
```bash
cd firmware
pio run -t upload
```

### Paso 2: Iniciar el Servidor Backend
```bash
cd backend
python -m venv venv
# En Windows:
venv\Scripts\activate
# En macOS/Linux:
source venv/bin/activate
pip install -r requirements.txt
python app.py
```

### Paso 3: Abrir la Interfaz de Monitoreo
Abra el archivo `frontend/index.html` en un navegador web o inicie un servidor local:
```bash
cd frontend
python -m http.server 3000
```

### Paso 4: Conexión de Red
- Conecte su computadora a la red WiFi del ESP32: SSID: `Esp32-Tópicos IV` / Contraseña: `12345678`.
- Asegúrese de que el backend esté ejecutándose en el puerto 5000 de la laptop (IP asignada en red AP: `192.168.4.2`).

---

## Consideraciones de Seguridad y Mejoras Futuras

### Seguridad en Desarrollo
- Acceso CORS restringido a orígenes de prueba locales.
- Validaciones de tipo y rango en el esquema JSON recibido en el backend.

### Hoja de Ruta del Proyecto
1. **Migración del Almacenamiento**: Transicionar de archivos CSV planos a bases de datos SQL relacionales (SQLite / PostgreSQL).
2. **Protocolos Robustos**: Integrar soporte para protocolos de mensajería ligera de IoT como MQTT o flujos bidireccionales por WebSockets.
3. **Firmware Avanzado**: Añadir actualizaciones de firmware por aire (OTA) y reconexión dinámica de red.
4. **Seguridad en Producción**: Implementar autenticación JWT en endpoints críticos y habilitar encriptación de tráfico SSL/TLS (HTTPS).
