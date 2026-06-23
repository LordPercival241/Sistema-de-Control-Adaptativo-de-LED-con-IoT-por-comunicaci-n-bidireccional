/*
 * ============================================================================
 * ARCHIVO: config.h
 * PROPÓSITO: Constantes y configuración centralizada del sistema
 * 
 * Este archivo contiene todas las constantes y configuraciones del proyecto
 * para facilitar cambios sin modificar el código principal.
 * ============================================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURACIÓN WiFi AP (Punto de Acceso)
// ============================================================================

// SSID del punto de acceso WiFi del ESP32
#define WIFI_SSID "Esp32-Tópicos IV"

// Contraseña del punto de acceso
#define WIFI_PASSWORD "12345678"

// Rango de direcciones IP del AP
#define WIFI_GATEWAY "192.168.4.1"
#define WIFI_SUBNET "255.255.255.0"

// ============================================================================
// CONFIGURACIÓN DEL BACKEND
// ============================================================================

// IP del servidor backend (en la red local)
#define BACKEND_IP "192.168.4.2"

// Puerto del servidor backend
#define BACKEND_PORT 5000

// Endpoint para enviar datos
#define BACKEND_ENDPOINT "/api/sensor/data"

// ============================================================================
// CONFIGURACIÓN DE HARDWARE - GPIO
// ============================================================================

// GPIO 34: Entrada analógica para el sensor LDR
#define LDR_PIN 34

// GPIO 25: Salida PWM para control de la tira LED
#define LED_PIN 25

// ============================================================================
// CONFIGURACIÓN DE SENSORES
// ============================================================================

// Rango de valores analógicos del LDR (0-4095 para ADC de 12 bits)
#define ADC_MIN 0
#define ADC_MAX 4095

// Valores calibrados del LDR (ajustar según tu sensor)
// Valor mínimo: cuando está oscuro
#define LDR_MIN_VALUE 100

// Valor máximo: cuando hay mucha luz
#define LDR_MAX_VALUE 3800

// ============================================================================
// CONFIGURACIÓN DEL CONTROL LED
// ============================================================================

// Intensidad mínima del LED (0-255, PWM 8 bits)
#define LED_MIN_INTENSITY 0

// Intensidad máxima del LED (0-255)
#define LED_MAX_INTENSITY 255

// Frecuencia PWM en Hz
#define LED_PWM_FREQUENCY 1000

// Canal PWM (ESP32 tiene 16 canales, 0-15)
#define LED_PWM_CHANNEL 0

// Resolución PWM en bits (8 bits = 0-255)
#define LED_PWM_RESOLUTION 8

// ============================================================================
// CONFIGURACIÓN DE TIEMPOS
// ============================================================================

// Intervalo de lectura del LDR en milisegundos
#define SENSOR_READ_INTERVAL 1000  // Cada 1 segundo

// Intervalo de envío de datos al backend en milisegundos
#define DATA_SEND_INTERVAL 5000    // Cada 5 segundos

// Timeout de conexión HTTP en milisegundos
#define HTTP_TIMEOUT 5000          // 5 segundos

// ============================================================================
// OTROS PARÁMETROS
// ============================================================================

// Habilitar debug serial
#define DEBUG_ENABLED true

// Baudrate de comunicación serial
#define SERIAL_BAUDRATE 115200

// Nombre del dispositivo para identificación
#define DEVICE_NAME "ESP32_LUZ_LED"

// Versión del firmware
#define FIRMWARE_VERSION "1.0.0"

#endif // CONFIG_H

