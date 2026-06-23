/*
 * ============================================================================
 * ARCHIVO: http_client.cpp
 * PROPÓSITO: Implementación de cliente HTTP para comunicación con backend
 * 
 * Envía datos del sensor al backend mediante HTTP POST con JSON
 * ============================================================================
 */

#include "http_client.h"
#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ============================================================================
// FUNCIONES PRIVADAS
// ============================================================================

/*
 * FUNCIÓN: build_sensor_json(float lux, int led_intensity, float temp)
 * PROPÓSITO: Construir string JSON con los datos del sensor
 * RETORNA: String - JSON formateado
 * 
 * NOTAS:
 * - Usa ArduinoJson para serialización
 * - Añade timestamp automático
 * - Incluye ID del dispositivo
 */
static String build_sensor_json(float lux, int led_intensity, float temp) {
    // Crear documento JSON con buffer para 256 bytes
    StaticJsonDocument<256> doc;
    
    // Agregar campos al JSON
    // Timestamp en formato ISO 8601 (YYYY-MM-DDTHH:MM:SS)
    // Por ahora usamos millis() convertido, en futuro agregar RTC
    doc["timestamp"] = String(millis());
    
    // Nivel de luz en porcentaje
    doc["lux"] = lux;
    
    // Intensidad actual del LED
    doc["led_intensity"] = led_intensity;
    
    // Temperatura
    doc["temperature"] = temp;
    
    // ID del dispositivo para identificación
    doc["device_id"] = DEVICE_NAME;
    
    // Versión del firmware
    doc["firmware_version"] = FIRMWARE_VERSION;
    
    // Serializar JSON a string
    String json_string;
    serializeJson(doc, json_string);
    
    return json_string;
}

/*
 * FUNCIÓN: http_response_code_to_result(int code)
 * PROPÓSITO: Convertir código HTTP a resultado
 * RETORNA: HTTPResult - resultado normalizado
 */
static HTTPResult http_response_code_to_result(int code) {
    if (code >= 200 && code < 300) {
        return HTTP_SUCCESS;           // 2xx = Éxito
    } else if (code >= 400 && code < 500) {
        return HTTP_CLIENT_ERROR;      // 4xx = Error cliente
    } else if (code >= 500) {
        return HTTP_SERVER_ERROR;      // 5xx = Error servidor
    } else {
        return HTTP_CONNECTION_ERROR;  // Otro = Error conexión
    }
}

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

HTTPResult http_send_sensor_data(float lux, int led_intensity, float temp, String& out_led_command) {
    /*
     * Enviar datos del sensor al backend
     * 
     * PROCESO:
     * 1. Construir URL completa
     * 2. Crear cliente HTTP
     * 3. Establecer headers
     * 4. Construir JSON
     * 5. Enviar POST request
     * 6. Procesar respuesta
     * 7. Limpiar recursos
     */
    
    if (DEBUG_ENABLED) {
        Serial.println("[HTTP] Enviando datos al backend...");
    }
    
    // Paso 1: Construir URL completa
    String url = "http://" + String(BACKEND_IP) + ":" + String(BACKEND_PORT) + 
                 String(BACKEND_ENDPOINT);
    
    // Paso 2: Crear cliente HTTP
    HTTPClient http;
    
    // Paso 3: Especificar timeout y configurar conexión
    http.setTimeout(HTTP_TIMEOUT);
    
    // Paso 4: Iniciar conexión HTTP
    if (!http.begin(url)) {
        if (DEBUG_ENABLED) {
            Serial.println("[HTTP] Error al conectar a: " + url);
        }
        return HTTP_CONNECTION_ERROR;
    }
    
    // Paso 5: Establecer headers HTTP
    // Content-Type: JSON
    http.addHeader("Content-Type", "application/json");
    // User-Agent para identificación
    http.addHeader("User-Agent", "ESP32-IoT/1.0");
    
    // Paso 6: Construir JSON
    String json_data = build_sensor_json(lux, led_intensity, temp);
    
    if (DEBUG_ENABLED) {
        Serial.println("[HTTP] JSON: " + json_data);
    }
    
    // Paso 7: Enviar POST request
    int http_response_code = http.POST(json_data);
    
    // Paso 8: Procesar respuesta
    HTTPResult result = HTTP_CONNECTION_ERROR;
    
    if (http_response_code > 0) {
        // Conexión exitosa, procesar código de respuesta
        result = http_response_code_to_result(http_response_code);
        
        String response_body = http.getString();
        
        // Extraer el led_command del JSON si la respuesta fue exitosa
        if (result == HTTP_SUCCESS) {
            StaticJsonDocument<512> response_doc;
            DeserializationError error = deserializeJson(response_doc, response_body);
            if (!error && response_doc.containsKey("led_command")) {
                out_led_command = response_doc["led_command"].as<String>();
            }
        }
        
        if (DEBUG_ENABLED) {
            Serial.println("[HTTP] Codigo: " + String(http_response_code));
            Serial.println("[HTTP] Respuesta: " + response_body);
        }
    } else {
        // Error en la conexión
        if (DEBUG_ENABLED) {
            Serial.println("[HTTP] Error en conexion: " + String(http.errorToString(http_response_code)));
        }
        result = HTTP_CONNECTION_ERROR;
    }
    
    // Paso 9: Limpiar recursos HTTP
    http.end();
    
    return result;
}

HTTPResult http_get_config() {
    /*
     * Obtener configuración del backend (futuro)
     * 
     * RESERVADO para expansiones futuras
     * Podría usarse para:
     * - Obtener nuevos parámetros de control
     * - Actualizar firmware OTA
     * - Cambiar configuración remota
     */
    
    if (DEBUG_ENABLED) {
        Serial.println("[HTTP] GET config reservado para futuro");
    }
    
    return HTTP_SUCCESS;
}

