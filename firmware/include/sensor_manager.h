/*
 * ============================================================================
 * ARCHIVO: sensor_manager.h
 * PROPÓSITO: Interfaz para gestionar el sensor LDR
 * 
 * Este módulo encapsula toda la lógica de lectura y procesamiento del
 * sensor LDR, proporcionando una interfaz limpia para obtener valores.
 * ============================================================================
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

// Estructura para almacenar datos del sensor
typedef struct {
    // Valor bruto del ADC (0-4095)
    int raw_adc_value;
    
    // Valor normalizado de luz (0-100, porcentaje)
    float light_percentage;
    
    // Temperatura (si hay sensor disponible)
    float temperature;
    
    // Timestamp del millis cuando se leyó
    unsigned long timestamp;
} SensorData;

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

/*
 * FUNCIÓN: sensor_init()
 * PROPÓSITO: Inicializar el sensor LDR (configurar GPIO y ADC)
 * PARÁMETROS: ninguno
 * RETORNA: void
 */
void sensor_init();

/*
 * FUNCIÓN: sensor_read()
 * PROPÓSITO: Leer el sensor LDR y retornar estructura con datos
 * PARÁMETROS: ninguno
 * RETORNA: SensorData - estructura con valores actuales del sensor
 * 
 * NOTAS:
 * - Normaliza el valor bruto del ADC a porcentaje (0-100)
 * - Aplica suavizado si es necesario
 * - Timestamp se asigna automáticamente
 */
SensorData sensor_read();

/*
 * FUNCIÓN: normalize_adc_value(int raw_value)
 * PROPÓSITO: Convertir valor bruto del ADC a porcentaje (0-100)
 * PARÁMETROS: 
 *   - raw_value: valor bruto del ADC (0-4095)
 * RETORNA: float - valor normalizado entre 0 y 100
 * 
 * EJEMPLO:
 * - raw_value = 0      → retorna 0.0%
 * - raw_value = 2048   → retorna 50.0%
 * - raw_value = 4095   → retorna 100.0%
 */
float normalize_adc_value(int raw_value);

#endif // SENSOR_MANAGER_H

