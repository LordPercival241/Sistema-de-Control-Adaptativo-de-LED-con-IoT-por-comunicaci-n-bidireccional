# ============================================================================
# ARCHIVO: routes/sensor_routes.py
# PROPÓSITO: Rutas API para recibir y consultar datos del sistema PID
#
# Define los endpoints REST del sistema Daylight Harvesting
# ============================================================================

from flask import Blueprint, request, jsonify
from models.sensor_data import SensorData
from services.csv_service import CSVService
from utils.validators import validate_sensor_data
import config

# Crear blueprint para las rutas de sensores
# Blueprint: permite organizar rutas en módulos separados
sensor_bp = Blueprint('sensors', __name__, url_prefix='/api')

# Variable global para almacenar el modo deseado del LED
current_led_mode = "AUTO"

# ============================================================================
# ENDPOINTS
# ============================================================================

@sensor_bp.route('/sensor/data', methods=['POST'])
def receive_sensor_data():
    """
    ENDPOINT: POST /api/sensor/data
    PROPÓSITO: Recibir datos del sistema PID desde el ESP32
    
    BODY (JSON):
    {
      "timestamp": "2026-06-20T12:00:00",
      "setpoint": 400.0,
      "lux": 385.7,
      "pwm_output": 128,
      "device_id": "ESP32_PID"
    }
    
    RETORNA: JSON con confirmación
    
    CÓDIGO DE RESPUESTA:
    - 200: Datos recibidos correctamente
    - 400: Datos inválidos
    - 500: Error del servidor
    """
    
    try:
        # Paso 1: Obtener JSON del request
        if not request.is_json:
            return jsonify({
                'success': False,
                'error': 'Content-Type debe ser application/json'
            }), 400
        
        data = request.get_json()
        
        # Paso 2: Validar datos
        is_valid, validation_message = validate_sensor_data(data)
        
        if not is_valid:
            return jsonify({
                'success': False,
                'error': validation_message
            }), 400
        
        # Paso 3: Crear objeto SensorData
        sensor_data = SensorData.from_dict(data)
        
        # Paso 4: Guardar en CSV
        saved = CSVService.save_sensor_data(sensor_data)
        
        if not saved:
            return jsonify({
                'success': False,
                'error': 'Error guardando datos en la base de datos'
            }), 500
        
        # Paso 5: Retornar confirmación
        if config.DEBUG:
            print(f"[API] Datos recibidos: {sensor_data}")
        
        return jsonify({
            'success': True,
            'message': 'Datos guardados correctamente',
            'data': sensor_data.to_dict(),
            'led_command': current_led_mode
        }), 200
    
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Error interno: {str(e)}'
        }), 500


@sensor_bp.route('/data/latest', methods=['GET'])
def get_latest_data():
    """
    ENDPOINT: GET /api/data/latest
    PROPÓSITO: Obtener el último registro de datos
    
    PARÁMETROS QUERY (opcionales):
    - limit: Número de últimos registros a retornar (default: 1)
    
    EJEMPLO:
    GET /api/data/latest?limit=10
    
    RETORNA: JSON con lista de últimos registros
    """
    
    try:
        # Obtener parámetro limit
        limit = request.args.get('limit', default=1, type=int)
        
        # Limitar al máximo permitido
        if limit > config.MAX_RECORDS:
            limit = config.MAX_RECORDS
        
        # Obtener datos
        latest_data = CSVService.get_latest(limit)
        
        # Convertir a diccionarios
        data_list = [d.to_dict() for d in latest_data]
        
        return jsonify({
            'success': True,
            'data': data_list,
            'count': len(data_list)
        }), 200
    
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Error: {str(e)}'
        }), 500


@sensor_bp.route('/data/range', methods=['GET'])
def get_data_range():
    """
    ENDPOINT: GET /api/data/range
    PROPÓSITO: Obtener datos en un rango de fechas
    
    PARÁMETROS QUERY (requeridos):
    - from: Fecha inicial (formato: YYYY-MM-DD HH:MM:SS)
    - to: Fecha final (formato: YYYY-MM-DD HH:MM:SS)
    
    EJEMPLO:
    GET /api/data/range?from=2026-06-18T00:00:00&to=2026-06-18T23:59:59
    
    RETORNA: JSON con datos en el rango
    """
    
    try:
        # Obtener parámetros
        start_date = request.args.get('from')
        end_date = request.args.get('to')
        
        # Validar que ambos parámetros existen
        if not start_date or not end_date:
            return jsonify({
                'success': False,
                'error': 'Parámetros requeridos: from y to'
            }), 400
        
        # Obtener datos
        data = CSVService.get_by_date_range(start_date, end_date)
        
        # Convertir a diccionarios
        data_list = [d.to_dict() for d in data]
        
        return jsonify({
            'success': True,
            'data': data_list,
            'count': len(data_list),
            'date_range': {
                'from': start_date,
                'to': end_date
            }
        }), 200
    
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Error: {str(e)}'
        }), 500


@sensor_bp.route('/stats', methods=['GET'])
def get_statistics():
    """
    ENDPOINT: GET /api/stats
    PROPÓSITO: Obtener estadísticas del sistema PID
    
    RETORNA: JSON con estadísticas agregadas
    
    ESTADÍSTICAS:
    - total_records: Total de registros
    - avg_lux: Promedio de iluminación real (Lux)
    - avg_pwm_output: Promedio de salida PWM
    - avg_setpoint: Promedio de setpoint
    - min/max de valores
    """
    
    try:
        # Obtener estadísticas
        stats = CSVService.get_statistics()
        
        return jsonify({
            'success': True,
            'statistics': stats
        }), 200
    
    except Exception as e:
        return jsonify({
            'success': False,
            'error': f'Error: {str(e)}'
        }), 500


@sensor_bp.route('/health', methods=['GET'])
def health_check():
    """
    ENDPOINT: GET /api/health
    PROPÓSITO: Verificar que el backend está en línea
    
    RETORNA: JSON con estado del sistema
    
    NOTAS:
    - Útil para debugging
    - Verifica disponibilidad del servicio
    """
    
    return jsonify({
        'success': True,
        'message': 'Backend en línea',
        'version': config.BACKEND_VERSION,
        'environment': config.ENVIRONMENT
    }), 200

@sensor_bp.route('/led/control', methods=['POST'])
def control_led():
    """
    ENDPOINT: POST /api/led/control
    PROPÓSITO: Cambiar el modo de operación del LED
    """
    global current_led_mode
    try:
        data = request.get_json()
        if 'mode' in data:
            mode = data['mode'].upper()
            if mode in ['AUTO', 'OFF', 'MANUAL']:
                current_led_mode = mode
                if config.DEBUG:
                    print(f"[API] Modo LED cambiado a: {current_led_mode}")
                return jsonify({'success': True, 'mode': current_led_mode}), 200
            else:
                return jsonify({'success': False, 'error': 'Modo inválido'}), 400
        return jsonify({'success': False, 'error': 'No se proporcionó el modo'}), 400
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500
