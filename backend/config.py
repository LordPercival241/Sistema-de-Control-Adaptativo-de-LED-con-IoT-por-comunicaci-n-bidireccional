# ============================================================================
# ARCHIVO: config.py
# PROPÓSITO: Configuración centralizada del backend
#
# Contiene todas las configuraciones del proyecto para facilitar cambios
# sin modificar el código principal.
# ============================================================================

import os
from datetime import datetime

# ============================================================================
# CONFIGURACIÓN BÁSICA
# ============================================================================

# Entorno de ejecución (development, testing, production)
ENVIRONMENT = os.getenv('ENVIRONMENT', 'development')

# Debug habilitado
DEBUG = ENVIRONMENT == 'development'

# Puerto de ejecución
PORT = int(os.getenv('PORT', 5000))

# Host de ejecución
HOST = os.getenv('HOST', '0.0.0.0')

# ============================================================================
# CONFIGURACIÓN DE ARCHIVO CSV
# ============================================================================

# Directorio donde se almacenan los datos
DATA_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'data')

# Nombre del archivo CSV
CSV_FILENAME = 'sensor_data.csv'

# Ruta completa del archivo CSV
CSV_FILE_PATH = os.path.join(DATA_DIR, CSV_FILENAME)

# Crear directorio si no existe
os.makedirs(DATA_DIR, exist_ok=True)

# ============================================================================
# CONFIGURACIÓN DE VALIDACIÓN DE DATOS
# ============================================================================

# Rango válido de valores LUX (lux reales del sensor, no porcentaje)
LUX_MIN = 0.0
LUX_MAX = 5000.0

# Rango válido de Setpoint (lux objetivo del controlador PID)
SETPOINT_MIN = 0.0
SETPOINT_MAX = 5000.0

# Rango válido de PWM Output (salida del controlador PID)
PWM_OUTPUT_MIN = 0
PWM_OUTPUT_MAX = 255

# ============================================================================
# CONFIGURACIÓN DE LOGGING
# ============================================================================

# Nivel de logging (DEBUG, INFO, WARNING, ERROR, CRITICAL)
LOG_LEVEL = 'DEBUG' if DEBUG else 'INFO'

# Formato de logs
LOG_FORMAT = '[%(asctime)s] %(levelname)s - %(message)s'

# ============================================================================
# CONFIGURACIÓN DE API
# ============================================================================

# Límite de registros por página en paginación
PAGE_SIZE = 100

# Número máximo de registros a retornar en un request
MAX_RECORDS = 10000

# Timeout en segundos para validación de datos
VALIDATION_TIMEOUT = 5

# ============================================================================
# CONFIGURACIÓN DE CORS
# ============================================================================

# Orígenes permitidos (cambiar en producción)
CORS_ORIGINS = [
    '*',
    'http://localhost:3000',
    'http://localhost:5000',
    'http://127.0.0.1:5000',
    'http://192.168.4.2:3000',
    'http://192.168.4.2:5000',
    'http://192.168.4.2',
    'http://10.22.4.178:3000',
    'http://10.22.4.178:5000',
    'http://10.22.4.178',
    'null'  # Para abrir frontend desde archivos locales (file://)
]

# ============================================================================
# INFORMACIÓN DEL SISTEMA
# ============================================================================

# Versión del backend
BACKEND_VERSION = "2.0.0"

# Nombre del dispositivo
DEVICE_NAME = "Backend-IoT-PID"

# Versión mínima de API soportada
MIN_API_VERSION = "2.0.0"

print(f"""
========================================================
    CONFIGURACION BACKEND CARGADA                        
    Entorno: {ENVIRONMENT:40} 
    Debug: {str(DEBUG):46} 
    Puerto: {PORT}                                        
    Archivo CSV: {os.path.basename(CSV_FILE_PATH):37} 
========================================================
""")
