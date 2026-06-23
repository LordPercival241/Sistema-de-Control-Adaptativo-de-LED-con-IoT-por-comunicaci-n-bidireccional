# Arquitectura del Sistema IoT - Control Adaptativo de LED

## 1. Visión General

El sistema consta de una solución IoT distribuida en red local que automatiza el control de iluminación LED basada en luz ambiente:
- **Sensor**: LDR (Light Dependent Resistor) - Mide la iluminancia ambiental.
- **Actuador**: Tira LED PWM - Ajusta de forma automática la potencia de salida lumínica.
- **Hub local**: El microcontrolador ESP32 actúa como Punto de Acceso (AP) WiFi.
- **Servidor Backend**: Recibe, almacena y procesa la telemetría en tiempo real.
- **Frontend**: Dashboard para la visualización gráfica de históricos.

## 2. Flujo de Datos General

```
┌─────────────────────────────────────────────────────────┐
│                      RED LOCAL WiFi                      │
│                   (AP del ESP32)                          │
└─────────────────────────────────────────────────────────┘
         │                           │                        │
    ┌────▼────┐              ┌──────▼──────┐        ┌───────▼────┐
    │  ESP32   │◄────────────►│  Backend    │◄──────►│  Frontend  │
    │(AP + I/O)│   HTTP/JSON  │(Flask)      │  JSON  │ (Browser)  │
    └────┬────┘              └──────┬──────┘        └───────┬────┘
         │                          │                        │
    ┌────▼────┐              ┌──────▼──────┐        ┌───────▼────┐
    │ LDR+LED │              │  CSV File   │        │  Gráficos  │
    │(I/O)    │              │ (Datos)     │        │ (Chart.js) │
    └─────────┘              └─────────────┘        └────────────┘
```

## 3. Componentes y Responsabilidades

### A. Firmware ESP32 (C++)
**Responsabilidades:**
- Adquisición analógica mediante el sensor LDR conectado a GPIO 34.
- Regulación del ciclo de trabajo PWM de la señal de salida para el LED en GPIO 25.
- Levantamiento y gestión de la red inalámbrica en modo softAP.
- Serialización y transmisión de telemetría mediante peticiones HTTP POST JSON hacia el backend.
- Ejecución del algoritmo local de control inverso (mapeo de porcentaje de luz a ciclo PWM).

**Esquema de datos JSON enviado:**
```json
POST /api/sensor/data
{
  "timestamp": "2026-06-18T10:30:45",
  "lux": 450.0,
  "led_intensity": 75,
  "temperature": 28.5
}
```

### B. Backend (Python Flask)
**Responsabilidades:**
- Exposición de la interfaz de programación REST API.
- Validación de datos recibidos y control de excepciones.
- Persistencia en disco del histórico mediante formato CSV.
- Consolidación y procesamiento analítico de datos estadísticos para el dashboard.

**Endpoints expuestos:**
- `POST /api/sensor/data` - Recepción de telemetría.
- `GET /api/data/latest` - Retorno de las últimas mediciones.
- `GET /api/data/range` - Retorno de datos en un intervalo de tiempo.
- `GET /api/stats` - Retorno de promedios, valores mínimos y máximos.

### C. Frontend Web (Javascript Vanilla / Chart.js)
**Responsabilidades:**
- Presentación visual de la telemetría en tiempo real.
- Renderizado interactivo de gráficos temporales de luz ambiente e intensidad del LED.
- Generación y descarga local del archivo CSV para el usuario.
- Indicador visual del estado de comunicación con la API.

---

## 4. Principios del Diseño

- **Desacoplamiento**: Los módulos son independientes; el frontend y el firmware interactúan exclusivamente mediante la interfaz de la REST API expuesta por el backend.
- **Modularidad**: El software se divide en subsistemas dedicados (adquisición, control de potencia, conectividad y transmisión de datos).
- **Escalabilidad**: El diseño contempla la incorporación directa de nuevos sensores sin alterar el núcleo de la arquitectura.
- **Operación Local**: Sistema autónomo configurado para operar sin necesidad de conexión externa a Internet.

---

## 5. Secuencia de Ejecución

1. El microcontrolador ESP32 arranca la red softAP `Esp32-Tópicos IV`.
2. Se inicia el bucle de sensado del LDR a intervalos fijos de 1 segundo.
3. Se calcula la acción de control de la salida de potencia PWM.
4. Se actualiza el ciclo de trabajo del LED (rango PWM de 0 a 255).
5. Transcurridos 5 segundos, se compila la estructura JSON y se realiza el HTTP POST.
6. El backend Flask valida y guarda el registro en `sensor_data.csv`.
7. El dashboard frontend realiza consultas GET automatizadas y actualiza las gráficas.

---

## 6. Mapeo Físico del Hardware

| Sensor / Actuador | Pin GPIO | Función | Rango Operativo |
|-------------------|----------|---------|-----------------|
| Sensor LDR | GPIO 34 | Entrada analógica ADC | 0 - 4095 |
| Entrada LED PWM | GPIO 25 | Salida analógica PWM | 0 - 255 |
| Conexión de Tierra | GND | Referencia común 0V | - |
| Alimentación LDR | 3.3V | Voltaje de referencia | - |
