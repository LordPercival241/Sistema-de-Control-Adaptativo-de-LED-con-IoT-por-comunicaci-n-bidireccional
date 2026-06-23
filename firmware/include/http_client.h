/*
 * ============================================================================
 * ARCHIVO: http_client.h
 * PROPÓSITO: Interfaz para comunicación HTTP con el backend
 * 
 * Este módulo proporciona funciones para enviar datos del sensor al backend
 * mediante HTTP POST con formato JSON.
 * ============================================================================
 */

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <Arduino.h>

// ============================================================================
// TIPOS Y ESTRUCTURAS
// ============================================================================

/*
 * Enumeración para códigos de resultado HTTP
 */
typedef enum {
    HTTP_SUCCESS = 0,        // Envío exitoso (200-299)
    HTTP_CLIENT_ERROR,       // Error del cliente (400-499)
    HTTP_SERVER_ERROR,       // Error del servidor (500-599)
    HTTP_CONNECTION_ERROR,   // Error de conexión
    HTTP_TIMEOUT             // Timeout de la solicitud
} HTTPResult;

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

/*
 * FUNCIÓN: http_send_sensor_data(float lux, int led_intensity, float temp)
 * PROPÓSITO: Enviar datos del sensor al backend mediante POST HTTP
 * 
 * PARÁMETROS:
 *   - lux: nivel de luz ambiente (0-100%)
 *   - led_intensity: intensidad actual del LED (0-255)
 *   - temp: temperatura actual en grados Celsius
 * 
 * RETORNA: HTTPResult - código de resultado
 * 
 * DETALLES:
 * - Construye JSON con los datos
 * - Añade timestamp automático
 * - Envía a: BACKEND_IP:BACKEND_PORT/BACKEND_ENDPOINT
 * - Timeouts configurados en config.h
 * 
 * JSON ENVIADO:
 * {
 *   "timestamp": "2026-06-18T10:30:45",
 *   "lux": 45.5,
 *   "led_intensity": 128,
 *   "temperature": 28.5,
 *   "device_id": "ESP32_LUZ_LED"
 * }
 */
HTTPResult http_send_sensor_data(float lux, int led_intensity, float temp, String& out_led_command);

/*
 * FUNCIÓN: http_get_config()
 * PROPÓSITO: Obtener configuración del backend (futuro uso)
 * PARÁMETROS: ninguno
 * RETORNA: HTTPResult - código de resultado
 * 
 * NOTA: Función reservada para expansiones futuras
 */
HTTPResult http_get_config();

#endif // HTTP_CLIENT_H

