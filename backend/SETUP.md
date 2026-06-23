# Guía de Instalación y Ejecución del Backend

## Requisitos Previos

- Python 3.8 o superior
- pip (gestor de paquetes de Python)
- Acceso a interfaz de línea de comandos (Terminal o CMD)

---

## Instalación Paso a Paso

### Paso 1: Crear Entorno Virtual

```bash
# Windows
python -m venv venv
venv\Scripts\activate

# macOS / Linux
python3 -m venv venv
source venv/bin/activate
```

**Propósito del entorno virtual:**
- Aísla las dependencias específicas de esta aplicación.
- Evita conflictos de versiones con otros paquetes de Python instalados en el sistema.
- Garantiza la reproducibilidad del entorno de ejecución.

### Paso 2: Instalar Dependencias

```bash
pip install -r requirements.txt
```

**Dependencias principales instaladas:**
- Flask 3.0.0 (Framework de desarrollo web)
- Flask-CORS 4.0.0 (Soporte para Cross-Origin Resource Sharing)
- python-dateutil 2.8.2 (Utilidades analíticas de fecha y hora)
- requests 2.31.0 (Cliente HTTP)

### Paso 3: Ejecución del Servidor

```bash
python app.py
```

**Salida esperada en consola:**
```
 * Serving Flask app 'app'
 * Debug mode: on
 * Running on http://0.0.0.0:5000
```

**Direcciones de acceso:**
- Acceso local (en la misma laptop): `http://localhost:5000` o `http://127.0.0.1:5000`
- Acceso en red local (según la IP de tu PC): `http://[IP_DE_TU_PC]:5000`
- Conexión del ESP32 en la red AP: `http://192.168.4.2:5000`

---

## Endpoints Disponibles (API REST)

### 1. Recepción de datos del sensor
- **Método**: `POST`
- **Ruta**: `/api/sensor/data`
- **Cabecera**: `Content-Type: application/json`
- **Cuerpo (JSON)**:
```json
{
  "timestamp": "2026-06-18T10:30:45",
  "lux": 45.5,
  "led_intensity": 128,
  "temperature": 28.5,
  "device_id": "ESP32_LUZ_LED"
}
```

**Respuesta de éxito (HTTP 200/201):**
```json
{
  "success": true,
  "message": "Datos guardados correctamente",
  "data": {
    "timestamp": "2026-06-18T10:30:45",
    "lux": 45.5,
    "led_intensity": 128,
    "temperature": 28.5,
    "device_id": "ESP32_LUZ_LED"
  }
}
```

### 2. Consulta del último registro
- **Método**: `GET`
- **Ruta**: `/api/data/latest?limit=10`
- **Descripción**: Retorna la cantidad especificada de últimos registros de telemetría guardados.

### 3. Filtro por rango de fechas
- **Método**: `GET`
- **Ruta**: `/api/data/range?from=2026-06-18T00:00:00&to=2026-06-18T23:59:59`

### 4. Estadísticas del sistema
- **Método**: `GET`
- **Ruta**: `/api/stats`
- **Descripción**: Devuelve métricas agregadas (promedios, mínimos y máximos de luminosidad, intensidad PWM y temperatura).

### 5. Comprobación de estado (Health Check)
- **Método**: `GET`
- **Ruta**: `/api/health`

---

## Estructura del Módulo Backend

```
backend/
├── app.py                 # Inicialización y factoría de la aplicación Flask
├── config.py             # Parámetros y constantes del sistema
├── requirements.txt      # Archivo de definición de dependencias
├── models/
│   └── sensor_data.py   # Estructura del modelo de datos de telemetría
├── services/
│   └── csv_service.py   # Servicio de persistencia en disco (CSV)
├── utils/
│   └── validators.py    # Funciones de validación de esquemas JSON
└── routes/
    └── sensor_routes.py # Definición de endpoints y rutas de API
```

---

## Parámetros de Configuración

Es posible modificar el comportamiento del backend editando el archivo `config.py`:
- `PORT`: Define el puerto TCP de escucha (por defecto `5000`).
- `HOST`: Dirección de escucha (por defecto `0.0.0.0` para admitir tráfico externo).
- `CSV_FILENAME`: Nombre del archivo donde se almacena el histórico de datos.
- `LUX_MIN`, `LUX_MAX`, `PWM_OUTPUT_MIN`, `PWM_OUTPUT_MAX`: Límites físicos admitidos para la validación de telemetría.

---

## Resolución de Problemas (Troubleshooting)

### Error: "Port 5000 already in use"
Si el puerto 5000 está ocupado por otro proceso del sistema operativo, puedes redefinirlo en `config.py`:
```python
PORT = 5001
```

### Error: "ModuleNotFoundError"
Asegúrate de haber activado correctamente el entorno virtual (`venv`) antes de ejecutar el comando de instalación de dependencias:
```bash
pip install -r requirements.txt
```

### Error: "Connection refused" en el ESP32
- Verifica que el servidor Flask se esté ejecutando en la laptop.
- Asegúrate de que la IP especificada en el código del ESP32 coincide con la IP local de la laptop en la red local (`192.168.4.2`).
- Desactiva temporalmente el firewall de Windows o añade una regla de entrada para permitir el tráfico en el puerto `5000`.

### Error de origen cruzado (CORS) en el Frontend
- Verifica que la dirección origen de tu frontend se encuentre registrada dentro de la lista `CORS_ORIGINS` en `config.py` o utiliza el comodín `'*'`.

---

## Servidor en Producción

Para despliegues formales en producción, no debe utilizarse el servidor de desarrollo integrado de Flask (`app.run()`). En su lugar, se recomienda utilizar un servidor WSGI como Gunicorn:

```bash
pip install gunicorn
gunicorn -w 4 -b 0.0.0.0:5000 app:create_app()
```

---

## Notas de Persistencia

- El archivo de base de datos se genera automáticamente en la ruta `/data/sensor_data.csv`.
- Las lecturas se añaden de forma incremental al final del archivo físico sin sobrescribir registros previos.
