/*
 * ============================================================================
 * ARCHIVO: wifi_manager.cpp
 * PROPÓSITO: Implementación de gestión WiFi AP (Punto de Acceso)
 * 
 * Configura el ESP32 como punto de acceso WiFi para que otros dispositivos
 * se conecten a él y reciban datos del sistema.
 * ============================================================================
 */

#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

// ============================================================================
// VARIABLES GLOBALES DEL MÓDULO
// ============================================================================

// Estado actual del WiFi
static WiFiState current_state = WIFI_STATE_IDLE;

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

bool wifi_init_ap() {
    /*
     * Inicializar ESP32 como Punto de Acceso (AP) WiFi
     * 
     * PROCESO:
     * 1. Apagar modo cliente WiFi
     * 2. Activar modo AP
     * 3. Configurar red local (IP, gateway, subnet)
     * 4. Establecer SSID y contraseña
     * 5. Iniciar AP
     * 6. Mostrar información de conexión
     */
    
    if (DEBUG_ENABLED) {
        Serial.println("[WiFi] Inicializando modo AP (Punto de Acceso)...");
    }
    
    // Paso 1: Configurar modo WiFi (AP solamente)
    // WIFI_MODE_AP: Solo punto de acceso
    // WIFI_MODE_STA: Solo cliente
    // WIFI_MODE_APSTA: Ambos modos
    WiFi.mode(WIFI_MODE_AP);
    
    // Paso 2: Configurar IP estática del AP
    // softAPConfig(localIP, gateway, subnet)
    IPAddress ap_ip(192, 168, 4, 1);           // IP del AP
    IPAddress gateway(192, 168, 4, 1);         // Gateway (mismo que AP)
    IPAddress subnet(255, 255, 255, 0);        // Máscara de subred /24
    
    WiFi.softAPConfig(ap_ip, gateway, subnet);
    
    // Paso 3: Iniciar el Punto de Acceso
    // softAP(ssid, password, channel, hidden, max_connections)
    // - ssid: nombre de la red
    // - password: contraseña (mínimo 8 caracteres)
    // - channel: canal WiFi (1-13)
    // - hidden: false = visible, true = oculta
    // - max_connections: máximo de clientes (1-4)
    bool ap_started = WiFi.softAP(
        WIFI_SSID,           // SSID configurado en config.h
        WIFI_PASSWORD,       // Contraseña configurada
        1,                   // Canal 1 (menos interferencia)
        false,               // Red visible (no oculta)
        4                    // Máximo 4 clientes
    );
    
    if (!ap_started) {
        if (DEBUG_ENABLED) {
            Serial.println("[WiFi] Error al iniciar AP");
        }
        current_state = WIFI_STATE_ERROR;
        return false;
    }
    
    // Paso 4: Actualizar estado
    current_state = WIFI_STATE_AP_ACTIVE;
    
    // Paso 5: Mostrar información de conexión
    if (DEBUG_ENABLED) {
        Serial.println("[WiFi] Punto de Acceso iniciado");
        Serial.println("[WiFi] SSID: " + String(WIFI_SSID));
        Serial.println("[WiFi] Contraseña: " + String(WIFI_PASSWORD));
        Serial.println("[WiFi] IP del AP: " + WiFi.softAPIP().toString());
        Serial.println("[WiFi] Clientes máximos: 4");
        Serial.println("[WiFi] Esperando conexiones...");
    }
    
    return true;
}

WiFiInfo wifi_get_state() {
    /*
     * Obtener información actual del WiFi
     * 
     * RETORNA: Estructura con estado, IPs, clientes, etc.
     */
    
    WiFiInfo info;
    info.state = current_state;
    info.connected_clients = WiFi.softAPgetStationNum();  // Número de clientes
    
    // Obtener IP del AP
    IPAddress ip = WiFi.softAPIP();
    snprintf(info.ap_ip, sizeof(info.ap_ip), "%d.%d.%d.%d",
             ip[0], ip[1], ip[2], ip[3]);
    
    // Copiar SSID
    strncpy(info.ap_ssid, WIFI_SSID, sizeof(info.ap_ssid) - 1);
    
    // Signal strength (RSSI) - para futuros usos
    info.signal_strength = 0;  // No aplica en modo AP
    
    return info;
}

bool wifi_is_ap_active() {
    /*
     * Verificar si el AP WiFi está activo
     */
    return (current_state == WIFI_STATE_AP_ACTIVE);
}

int wifi_get_client_count() {
    /*
     * Obtener número de clientes conectados al AP
     * 
     * RETORNA: int - número de dispositivos conectados
     */
    return WiFi.softAPgetStationNum();
}

