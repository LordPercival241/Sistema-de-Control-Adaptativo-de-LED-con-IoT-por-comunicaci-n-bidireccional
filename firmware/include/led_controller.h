/*
 * ============================================================================
 * ARCHIVO: led_controller.h
 * PROPÓSITO: Interfaz para controlar la tira LED mediante PWM
 * 
 * Este módulo encapsula la lógica de control PWM del LED, permitiendo
 * establecer la intensidad de forma simple y desacoplada.
 * ============================================================================
 */

#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

// ============================================================================
// TIPOS Y ESTRUCTURAS
// ============================================================================

/*
 * Enumeración para diferentes modos de control del LED
 */
typedef enum {
    LED_MODE_AUTO,       // Modo automático (basado en luz ambiente)
    LED_MODE_MANUAL,     // Modo manual (intensidad fija)
    LED_MODE_OFF         // LED apagado
} LEDMode;

// Estructura para almacenar estado del LED
typedef struct {
    // Intensidad actual (0-255)
    int current_intensity;
    
    // Modo actual de operación
    LEDMode mode;
    
    // Indicador si está encendido
    bool is_on;
    
    // Timestamp del último cambio
    unsigned long last_change_time;
} LEDState;

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

/*
 * FUNCIÓN: led_init()
 * PROPÓSITO: Inicializar el controlador LED (configurar GPIO y PWM)
 * PARÁMETROS: ninguno
 * RETORNA: void
 * 
 * NOTAS:
 * - Configura el pin GPIO para PWM
 * - Establece frecuencia y resolución del PWM
 * - Comienza con LED apagado (intensidad = 0)
 */
void led_init();

/*
 * FUNCIÓN: led_set_intensity(int intensity)
 * PROPÓSITO: Establecer la intensidad del LED (0-255)
 * PARÁMETROS:
 *   - intensity: valor entre 0 (apagado) y 255 (máximo brillo)
 * RETORNA: void
 * 
 * EJEMPLOS:
 * - led_set_intensity(0)   → LED apagado
 * - led_set_intensity(128) → LED al 50% de brillo
 * - led_set_intensity(255) → LED a máximo brillo
 */
void led_set_intensity(int intensity);

/*
 * FUNCIÓN: led_set_mode(LEDMode mode)
 * PROPÓSITO: Cambiar el modo de operación del LED
 * PARÁMETROS:
 *   - mode: el nuevo modo (AUTO, MANUAL, OFF)
 * RETORNA: void
 */
void led_set_mode(LEDMode mode);

/*
 * FUNCIÓN: led_get_state()
 * PROPÓSITO: Obtener el estado actual del LED
 * PARÁMETROS: ninguno
 * RETORNA: LEDState - estructura con el estado actual
 */
LEDState led_get_state();

/*
 * FUNCIÓN: map_light_to_intensity(float light_percentage)
 * PROPÓSITO: Convertir porcentaje de luz a intensidad LED (0-255)
 * PARÁMETROS:
 *   - light_percentage: valor entre 0 y 100
 * RETORNA: int - intensidad PWM entre 0 y 255
 * 
 * LÓGICA:
 * - Luz baja (0-20%) → LED alto (200-255)
 * - Luz media (40-60%) → LED medio (100-155)
 * - Luz alta (80-100%) → LED bajo (0-50)
 * 
 * Esta función es el CORAZÓN del control adaptativo
 */
int map_light_to_intensity(float light_percentage);

#endif // LED_CONTROLLER_H

