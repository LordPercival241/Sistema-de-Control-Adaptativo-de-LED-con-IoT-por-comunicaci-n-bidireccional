# ============================================================================
# ARCHIVO: models/sensor_data.py
# PROPÓSITO: Modelo de datos para las lecturas del sensor PID
# ============================================================================

from typing import Dict, Any
from datetime import datetime

class SensorData:
    """
    Representa una lectura individual proveniente del sistema ESP32.
    """
    def __init__(self, timestamp: str, setpoint: float, lux: float, pwm_output: int, device_id: str = "ESP32_PID"):
        self.timestamp = timestamp
        self.setpoint = setpoint
        self.lux = lux
        self.pwm_output = pwm_output
        self.device_id = device_id

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'SensorData':
        """
        Crea una instancia desde un diccionario, por ejemplo el payload de una petición HTTP POST.
        """
        return cls(
            timestamp=data.get('timestamp', datetime.now().isoformat()),
            setpoint=float(data.get('setpoint', 0.0)),
            lux=float(data.get('lux', 0.0)),
            pwm_output=int(data.get('pwm_output', 0)),
            device_id=data.get('device_id', 'ESP32_PID')
        )

    def to_dict(self) -> Dict[str, Any]:
        """
        Convierte la instancia a un diccionario para devolver en respuestas JSON o guardar en CSV.
        """
        return {
            'timestamp': self.timestamp,
            'setpoint': self.setpoint,
            'lux': self.lux,
            'pwm_output': self.pwm_output,
            'device_id': self.device_id
        }

    def __str__(self) -> str:
        """
        Representación legible de los datos para logs.
        """
        return f"SensorData(timestamp={self.timestamp}, setpoint={self.setpoint}, lux={self.lux}, pwm={self.pwm_output})"
