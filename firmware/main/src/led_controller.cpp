/*
 * ============================================================================
 * ARCHIVO: led_controller.cpp
 * PROPÓSITO: Implementación del controlador PWM de LED
 * 
 * Gestiona el control de la tira LED mediante PWM del ESP32
 * incluyendo intensidad y modos de operación.
 * ============================================================================
 */

#include "led_controller.h"
#include "config.h"

// ============================================================================
// VARIABLES GLOBALES DEL MÓDULO
// ============================================================================

// Estado actual del LED
LEDState current_led_state = {
    .current_intensity = 0,
    .mode = LED_MODE_AUTO,
    .is_on = false,
    .last_change_time = 0
};

// ============================================================================
// FUNCIONES PRIVADAS
// ============================================================================

/*
 * FUNCIÓN: apply_pwm(int intensity)
 * PROPÓSITO: Aplicar el valor PWM al GPIO del LED
 * PARÁMETROS: intensity - valor entre 0 y 255
 * 
 * NOTAS:
 * - PWM (Pulse Width Modulation) simula voltaje analógico con pulsos
 * - 0 = 0% duty cycle = apagado
 * - 255 = 100% duty cycle = brillo máximo
 * - 128 = 50% duty cycle = brillo medio
 */
static void apply_pwm(int intensity) {
    // ledcWrite(canal, valor) - escribe valor PWM en el canal
    // El ESP32 mapea automáticamente 0-255 a 0-100% duty cycle
    ledcWrite(LED_PWM_CHANNEL, intensity);
}

/*
 * FUNCIÓN: constrain_intensity(int intensity)
 * PROPÓSITO: Asegurar que la intensidad esté en rango válido
 * RETORNA: int - valor limitado entre LED_MIN_INTENSITY y LED_MAX_INTENSITY
 */
static int constrain_intensity(int intensity) {
    if (intensity < LED_MIN_INTENSITY) return LED_MIN_INTENSITY;
    if (intensity > LED_MAX_INTENSITY) return LED_MAX_INTENSITY;
    return intensity;
}

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

void led_init() {
    /*
     * Inicializar el controlador LED
     * 
     * Configuración necesaria:
     * 1. Configurar el pin GPIO
     * 2. Configurar PWM
     * 3. Establecer frecuencia y resolución
     */
    
    if (DEBUG_ENABLED) {
        Serial.println("[LED] Inicializando controlador LED en GPIO " + String(LED_PIN));
        Serial.println("[LED] Frecuencia: " + String(LED_PWM_FREQUENCY) + " Hz");
        Serial.println("[LED] Resolución: " + String(LED_PWM_RESOLUTION) + " bits");
    }
    
    // Configurar el pin GPIO como salida
    pinMode(LED_PIN, OUTPUT);
    
    // Configurar PWM en ESP32
    // ledcSetup(canal, frecuencia, resolución)
    // - canal: número del canal PWM (0-15)
    // - frecuencia: en Hz (1000 Hz es común para LED)
    // - resolución: en bits (8 bits = 0-255)
    ledcSetup(LED_PWM_CHANNEL, LED_PWM_FREQUENCY, LED_PWM_RESOLUTION);
    
    // Asociar el pin GPIO al canal PWM
    // ledcAttachPin(pin, canal)
    ledcAttachPin(LED_PIN, LED_PWM_CHANNEL);
    
    // Apagar LED inicialmente
    led_set_intensity(0);
    
    if (DEBUG_ENABLED) {
        Serial.println("[LED] LED inicializado correctamente");
    }
}

void led_set_intensity(int intensity) {
    /*
     * Establecer la intensidad del LED (0-255)
     * 
     * LÓGICA:
     * 1. Limitar valor al rango válido
     * 2. Aplicar PWM al GPIO
     * 3. Actualizar estado interno
     * 4. Log opcional
     */
    
    // Paso 1: Limitar al rango válido
    intensity = constrain_intensity(intensity);
    
    // Paso 2: Aplicar PWM
    apply_pwm(intensity);
    
    // Paso 3: Actualizar estado
    current_led_state.current_intensity = intensity;
    current_led_state.is_on = (intensity > 0);
    current_led_state.last_change_time = millis();
    
    // Paso 4: Logging
    if (DEBUG_ENABLED) {
        int percentage = (intensity * 100) / 255;  // Convertir a porcentaje
        Serial.println("[LED] Intensidad establecida: " + String(intensity) + "/255 (" + String(percentage) + "%)");
    }
}

void led_set_mode(LEDMode mode) {
    /*
     * Cambiar modo de operación del LED
     * 
     * MODOS:
     * - LED_MODE_AUTO: LED controlado por sensor (recomendado)
     * - LED_MODE_MANUAL: Intensidad fija (para pruebas)
     * - LED_MODE_OFF: LED apagado (ahorro de energía)
     */
    
    current_led_state.mode = mode;
    
    if (DEBUG_ENABLED) {
        String mode_name;
        switch (mode) {
            case LED_MODE_AUTO:
                mode_name = "AUTO";
                break;
            case LED_MODE_MANUAL:
                mode_name = "MANUAL";
                break;
            case LED_MODE_OFF:
                mode_name = "OFF";
                led_set_intensity(0);  // Apagar LED inmediatamente
                break;
            default:
                mode_name = "UNKNOWN";
        }
        Serial.println("[LED] Modo cambiado a: " + mode_name);
    }
}

LEDState led_get_state() {
    /*
     * Obtener el estado actual del LED
     * Simplemente retorna la estructura con estado
     */
    return current_led_state;
}

int map_light_to_intensity(float light_percentage) {
    /*
     * FUNCIÓN CRÍTICA: Convertir luz ambiente a intensidad LED
     * 
     * LÓGICA DE CONTROL ADAPTATIVO:
     * Esta es la inteligencia del sistema. Define cómo reacciona
     * el LED a cambios de luz ambiente.
     * 
     * ESTRATEGIA (INVERSIÓN FOTOADAPTATIVA):
     * - Luz BAJA (0-25%) → LED ALTO (150-255) para iluminar
     * - Luz MEDIA (25-75%) → LED MEDIO (50-150) para complementar
     * - Luz ALTA (75-100%) → LED BAJO (0-50) para no interferir
     * 
     * FÓRMULA MATEMÁTICA (Lineal Inversa):
     * intensity = 255 - (light_percentage * 2.55)
     * 
     * NOTAS TÉCNICAS:
     * - La relación es inversa (más luz = menos LED)
     * - Esto evita deslumbramiento
     * - Favorece ahorro de energía durante el día
     * - Proporciona iluminación en la noche
     */
    
    // Asegurar que el porcentaje esté entre 0 y 100
    if (light_percentage < 0) light_percentage = 0;
    if (light_percentage > 100) light_percentage = 100;
    
    // FÓRMULA INVERSA: Más luz = Menos intensidad LED
    // Multiplicamos por 2.55 porque 100 * 2.55 = 255
    int intensity = 255 - (int)(light_percentage * 2.55);
    
    // Aplicar suavización con constrain
    intensity = constrain_intensity(intensity);
    
    return intensity;
}

