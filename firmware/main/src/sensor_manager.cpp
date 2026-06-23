/*
 * ============================================================================
 * ARCHIVO: sensor_manager.cpp
 * PROPÓSITO: Implementación del gestor de sensor LDR
 * 
 * Contiene la lógica real de lectura, normalización y procesamiento
 * de datos del sensor LDR del ESP32.
 * ============================================================================
 */

#include "sensor_manager.h"
#include "config.h"

// ============================================================================
// VARIABLES GLOBALES DEL MÓDULO
// ============================================================================

// Buffer para suavizado del valor del LDR (media móvil)
#define SENSOR_BUFFER_SIZE 5
int sensor_buffer[SENSOR_BUFFER_SIZE] = {0};
int buffer_index = 0;

// ============================================================================
// FUNCIONES PRIVADAS
// ============================================================================

/*
 * FUNCIÓN: read_adc_raw()
 * PROPÓSITO: Leer valor bruto del ADC del GPIO LDR
 * RETORNA: int - valor entre 0 y 4095
 * 
 * NOTAS TÉCNICAS:
 * - El ESP32 tiene ADC de 12 bits
 * - Rango de voltaje: 0 a 3.3V
 * - Valor 0 = 0V, Valor 4095 = 3.3V
 * - Esta función es el punto de lectura física del hardware
 */
static int read_adc_raw() {
    // Leer valor del ADC en el pin LDR_PIN
    // analogRead() automáticamente mapea 0-3.3V a 0-4095
    int raw = analogRead(LDR_PIN);
    return raw;
}

/*
 * FUNCIÓN: apply_smoothing(int raw_value)
 * PROPÓSITO: Suavizar lectura del ADC usando media móvil
 * PARÁMETROS: raw_value - valor bruto del ADC
 * RETORNA: int - valor suavizado
 * 
 * WHY (POR QUÉ):
 * - Los sensores analógicos tienen ruido
 * - Cambios bruscos causarían parpadeo del LED
 * - La media móvil filtra ruido sin retardo importante
 */
static int apply_smoothing(int raw_value) {
    // Guardar valor en el buffer circular
    sensor_buffer[buffer_index] = raw_value;
    buffer_index = (buffer_index + 1) % SENSOR_BUFFER_SIZE;
    
    // Calcular promedio de todos los valores en el buffer
    long sum = 0;
    for (int i = 0; i < SENSOR_BUFFER_SIZE; i++) {
        sum += sensor_buffer[i];
    }
    
    // Retornar promedio entero
    return sum / SENSOR_BUFFER_SIZE;
}

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

void sensor_init() {
    /*
     * Inicializar el sensor LDR
     * 
     * El LDR no necesita configuración especial en GPIO
     * porque analogRead() automáticamente configura el modo ADC
     */
    
    // Mostrar en serial que se inicializó
    if (DEBUG_ENABLED) {
        Serial.println("[SENSOR] Inicializando sensor LDR en GPIO " + String(LDR_PIN));
    }
    
    // Pre-llenar el buffer con valores iniciales
    for (int i = 0; i < SENSOR_BUFFER_SIZE; i++) {
        sensor_buffer[i] = analogRead(LDR_PIN);
        delay(10); // Pequeña pausa entre lecturas
    }
}

SensorData sensor_read() {
    /*
     * Leer el sensor LDR y retornar estructura con datos procesados
     * 
     * Proceso:
     * 1. Leer valor bruto del ADC
     * 2. Aplicar suavizado
     * 3. Normalizar a porcentaje
     * 4. Retornar estructura con todos los datos
     */
    
    // Crear estructura que vamos a retornar
    SensorData data;
    
    // Paso 1: Leer valor bruto del ADC
    int raw = read_adc_raw();
    
    // Paso 2: Aplicar suavizado para eliminar ruido
    int smoothed = apply_smoothing(raw);
    
    // Paso 3: Guardar en estructura
    data.raw_adc_value = smoothed;
    
    // Paso 4: Normalizar a porcentaje (0-100%)
    data.light_percentage = normalize_adc_value(smoothed);
    
    // Paso 5: Temperatura (simulada por ahora)
    // En futuro: conectar sensor DHT o similar
    data.temperature = 25.0 + (smoothed / 4095.0) * 5.0;  // Rango 25-30°C simulado
    
    // Paso 6: Timestamp actual en milisegundos
    data.timestamp = millis();
    
    return data;
}

float normalize_adc_value(int raw_value) {
    /*
     * Convertir valor bruto del ADC (0-4095) a porcentaje (0-100)
     * 
     * FÓRMULA:
     * light_percentage = ((raw_value - LDR_MIN) / (LDR_MAX - LDR_MIN)) * 100
     * 
     * NOTAS:
     * - LDR_MIN_VALUE y LDR_MAX_VALUE se definen en config.h
     * - Estos valores deben CALIBRARSE con tu sensor específico
     * - Un LDR tiene comportamiento no-lineal
     */
    
    // Limitar el valor al rango configurado
    if (raw_value < LDR_MIN_VALUE) {
        raw_value = LDR_MIN_VALUE;
    }
    if (raw_value > LDR_MAX_VALUE) {
        raw_value = LDR_MAX_VALUE;
    }
    
    // Calcular porcentaje dentro del rango
    float percentage = ((float)(raw_value - LDR_MIN_VALUE) / (LDR_MAX_VALUE - LDR_MIN_VALUE)) * 100.0;
    
    // Asegurar que esté entre 0 y 100
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;
    
    return percentage;
}

