# ============================================================================
# ARCHIVO: utils/validators.py
# PROPÓSITO: Funciones para validar datos del ESP32 (Sistema PID)
#
# Verifica que los datos recibidos sean válidos antes de guardarlos
# ============================================================================

from typing import Dict, Any, Tuple
from datetime import datetime
import config

def validate_sensor_data(data: Dict[str, Any]) -> Tuple[bool, str]:
    """
    FUNCIÓN: validate_sensor_data()
    PROPÓSITO: Validar que los datos del sensor PID sean correctos
    
    PARÁMETROS:
    - data: Diccionario con los datos a validar
    
    RETORNA: Tupla (válido: bool, mensaje: str)
    
    VALIDACIONES:
    - Campos requeridos presentes (timestamp es opcional, se genera si falta)
    - Tipos de datos correctos
    - Valores dentro de rangos válidos
    """
    
    # Si no viene timestamp, generarlo automáticamente
    if 'timestamp' not in data or not data['timestamp']:
        data['timestamp'] = datetime.now().isoformat()
    
    # Lista de campos requeridos
    required_fields = ['setpoint', 'lux', 'pwm_output']
    
    # Verificar que todos los campos están presentes
    for field in required_fields:
        if field not in data:
            return False, f"Campo requerido faltante: {field}"
    
    # Validar timestamp
    timestamp = data.get('timestamp')
    if not isinstance(timestamp, str):
        return False, "timestamp debe ser string"
    
    # Validar setpoint
    try:
        setpoint = float(data.get('setpoint'))
        if not (config.SETPOINT_MIN <= setpoint <= config.SETPOINT_MAX):
            return False, f"setpoint debe estar entre {config.SETPOINT_MIN} y {config.SETPOINT_MAX}"
    except (TypeError, ValueError):
        return False, "setpoint debe ser numérico"
    
    # Validar lux (luz real en Lux)
    try:
        lux = float(data.get('lux'))
        if not (config.LUX_MIN <= lux <= config.LUX_MAX):
            return False, f"lux debe estar entre {config.LUX_MIN} y {config.LUX_MAX}"
    except (TypeError, ValueError):
        return False, "lux debe ser numérico"
    
    # Validar pwm_output
    try:
        pwm_output = int(data.get('pwm_output'))
        if not (config.PWM_OUTPUT_MIN <= pwm_output <= config.PWM_OUTPUT_MAX):
            return False, f"pwm_output debe estar entre {config.PWM_OUTPUT_MIN} y {config.PWM_OUTPUT_MAX}"
    except (TypeError, ValueError):
        return False, "pwm_output debe ser entero"
    
    # Si llegamos aquí, todo es válido
    return True, "Datos válidos"
