# ============================================================================
# ARCHIVO: services/csv_service.py
# PROPÓSITO: Servicio para gestionar persistencia en archivo CSV
#
# Encapsula toda la lógica de lectura y escritura en CSV
# Adaptado para el sistema PID Daylight Harvesting
# ============================================================================

import os
import csv
from typing import List, Optional
from datetime import datetime
from models.sensor_data import SensorData
import config

class CSVService:
    """
    CLASE: CSVService
    PROPÓSITO: Servicio para gestionar archivo CSV de datos
    
    RESPONSABILIDADES:
    - Crear archivo CSV si no existe
    - Guardar nuevas lecturas del sistema PID
    - Leer datos históricos
    - Filtrar por rango de fechas
    - Calcular estadísticas
    """
    
    # Encabezados del archivo CSV
    CSV_HEADERS = ['timestamp', 'setpoint', 'lux', 'pwm_output']
    
    @staticmethod
    def _ensure_csv_exists() -> bool:
        """
        FUNCIÓN PRIVADA: _ensure_csv_exists()
        PROPÓSITO: Verificar que el archivo CSV existe, si no crearlo
        
        RETORNA: True si existe o fue creado exitosamente
        """
        if os.path.exists(config.CSV_FILE_PATH):
            return True
        
        try:
            # Crear archivo nuevo con encabezados
            with open(config.CSV_FILE_PATH, 'w', newline='', encoding='utf-8') as csvfile:
                writer = csv.DictWriter(csvfile, fieldnames=CSVService.CSV_HEADERS)
                writer.writeheader()
            print(f"[CSV] Archivo creado: {config.CSV_FILE_PATH}")
            return True
        except Exception as e:
            print(f"[CSV] Error creando archivo: {e}")
            return False
    
    @staticmethod
    def save_sensor_data(sensor_data: SensorData) -> bool:
        """
        FUNCIÓN: save_sensor_data()
        PROPÓSITO: Guardar una lectura del sistema PID en el CSV
        
        PARÁMETROS:
        - sensor_data: Instancia de SensorData a guardar
        
        RETORNA: True si se guardó exitosamente, False si hubo error
        
        PROCESO:
        1. Verificar que archivo CSV existe
        2. Abrir en modo append (añadir)
        3. Escribir los datos del sensor
        4. Cerrar archivo
        """
        
        # Paso 1: Asegurar que el archivo existe
        if not CSVService._ensure_csv_exists():
            return False
        
        try:
            # Paso 2: Abrir en modo append (añadir al final)
            with open(config.CSV_FILE_PATH, 'a', newline='', encoding='utf-8') as csvfile:
                writer = csv.DictWriter(csvfile, fieldnames=CSVService.CSV_HEADERS)
                
                # Paso 3: Escribir fila con datos del sensor
                writer.writerow({
                    'timestamp': sensor_data.timestamp,
                    'setpoint': sensor_data.setpoint,
                    'lux': sensor_data.lux,
                    'pwm_output': sensor_data.pwm_output
                })
            
            return True
        
        except Exception as e:
            print(f"[CSV] Error guardando datos: {e}")
            return False
    
    @staticmethod
    def get_all_data() -> List[SensorData]:
        """
        FUNCIÓN: get_all_data()
        PROPÓSITO: Obtener todos los datos del archivo CSV
        
        RETORNA: Lista de objetos SensorData
        
        NOTAS:
        - Si hay muchos datos, esto puede ser lento
        - Para grandes volúmenes, considerar base de datos real
        """
        
        # Asegurar que el archivo existe
        if not CSVService._ensure_csv_exists():
            return []
        
        data_list = []
        
        try:
            with open(config.CSV_FILE_PATH, 'r', encoding='utf-8') as csvfile:
                # Usar csv.DictReader para leer como diccionarios
                reader = csv.DictReader(csvfile)
                
                for row in reader:
                    try:
                        # Convertir fila CSV a objeto SensorData
                        sensor_data = SensorData(
                            timestamp=row['timestamp'],
                            setpoint=float(row['setpoint']),
                            lux=float(row['lux']),
                            pwm_output=int(row['pwm_output'])
                        )
                        data_list.append(sensor_data)
                    except (ValueError, KeyError) as e:
                        print(f"[CSV] Fila inválida: {e}")
                        continue
        
        except Exception as e:
            print(f"[CSV] Error leyendo datos: {e}")
        
        return data_list
    
    @staticmethod
    def get_latest(limit: int = 1) -> List[SensorData]:
        """
        FUNCIÓN: get_latest()
        PROPÓSITO: Obtener los últimos N registros
        
        PARÁMETROS:
        - limit: Número de registros a retornar
        
        RETORNA: Lista con los últimos 'limit' registros
        """
        
        all_data = CSVService.get_all_data()
        return all_data[-limit:] if all_data else []
    
    @staticmethod
    def get_by_date_range(start_date: str, end_date: str) -> List[SensorData]:
        """
        FUNCIÓN: get_by_date_range()
        PROPÓSITO: Obtener datos en un rango de fechas
        
        PARÁMETROS:
        - start_date: Fecha inicial (formato: YYYY-MM-DD HH:MM:SS)
        - end_date: Fecha final (formato: YYYY-MM-DD HH:MM:SS)
        
        RETORNA: Lista de SensorData dentro del rango
        
        NOTAS:
        - Las fechas se comparan como strings (funciona con ISO 8601)
        - En producción, usar datetime proper para comparación
        """
        
        all_data = CSVService.get_all_data()
        
        # Filtrar por rango de fechas
        filtered = [
            data for data in all_data
            if start_date <= data.timestamp <= end_date
        ]
        
        return filtered
    
    @staticmethod
    def get_statistics() -> dict:
        """
        FUNCIÓN: get_statistics()
        PROPÓSITO: Calcular estadísticas de los datos del sistema PID
        
        RETORNA: Diccionario con estadísticas
        
        ESTADÍSTICAS CALCULADAS:
        - total_records: Número total de registros
        - avg_lux: Promedio de luz real (Lux)
        - avg_pwm_output: Promedio de salida PWM
        - avg_setpoint: Promedio de setpoint
        - min_lux / max_lux: Valores extremos de iluminación
        - min_pwm / max_pwm: Valores extremos de PWM
        """
        
        all_data = CSVService.get_all_data()
        
        if not all_data:
            return {
                'total_records': 0,
                'error': 'No hay datos'
            }
        
        # Calcular promedios
        total_records = len(all_data)
        avg_lux = sum(d.lux for d in all_data) / total_records
        avg_pwm_output = sum(d.pwm_output for d in all_data) / total_records
        avg_setpoint = sum(d.setpoint for d in all_data) / total_records
        
        # Encontrar valores extremos
        min_lux = min(d.lux for d in all_data)
        max_lux = max(d.lux for d in all_data)
        min_pwm = min(d.pwm_output for d in all_data)
        max_pwm = max(d.pwm_output for d in all_data)
        
        return {
            'total_records': total_records,
            'avg_lux': round(avg_lux, 2),
            'avg_pwm_output': round(avg_pwm_output, 1),
            'avg_setpoint': round(avg_setpoint, 2),
            'min_lux': round(min_lux, 2),
            'max_lux': round(max_lux, 2),
            'min_pwm': min_pwm,
            'max_pwm': max_pwm
        }
