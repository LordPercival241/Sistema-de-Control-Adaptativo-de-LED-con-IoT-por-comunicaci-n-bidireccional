/*
 * ============================================================================
 * ARCHIVO: wifi_manager.h
 * PROPÓSITO: Interfaz para gestionar conectividad WiFi
 * 
 * Este módulo maneja la configuración del ESP32 como punto de acceso (AP)
 * WiFi y proporciona funciones para comunicación de red.
 * ============================================================================
 */

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

// ============================================================================
// TIPOS Y ESTRUCTURAS
// ============================================================================

/*
 * Enumeración para estados de conexión WiFi
 */
typedef enum {
    WIFI_STATE_IDLE,         // No iniciado
    WIFI_STATE_AP_ACTIVE,    // Punto de acceso activo
    WIFI_STATE_CONNECTED,    // Conectado como cliente
    WIFI_STATE_ERROR         // Error en la conexión
} WiFiState;

// Estructura para información del WiFi
typedef struct {
    // Estado actual
    WiFiState state;
    
    // Número de clientes conectados
    int connected_clients;
    
    // IP del punto de acceso
    char ap_ip[16];
    
    // SSID del AP
    char ap_ssid[32];
    
    // Señal de fuerza (RSSI)
    int signal_strength;
} WiFiInfo;

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

/*
 * FUNCIÓN: wifi_init_ap()
 * PROPÓSITO: Inicializar ESP32 como punto de acceso (AP) WiFi
 * PARÁMETROS: ninguno
 * RETORNA: bool - true si fue exitoso, false si falló
 * 
 * NOTAS:
 * - Configura el SSID definido en config.h
 * - Establece contraseña configurada
 * - Activa el AP en 192.168.4.1
 */
bool wifi_init_ap();

/*
 * FUNCIÓN: wifi_get_state()
 * PROPÓSITO: Obtener información actual del WiFi
 * PARÁMETROS: ninguno
 * RETORNA: WiFiInfo - estructura con información del WiFi
 */
WiFiInfo wifi_get_state();

/*
 * FUNCIÓN: wifi_is_ap_active()
 * PROPÓSITO: Verificar si el AP WiFi está activo
 * PARÁMETROS: ninguno
 * RETORNA: bool - true si AP está activo
 */
bool wifi_is_ap_active();

/*
 * FUNCIÓN: wifi_get_client_count()
 * PROPÓSITO: Obtener número de clientes conectados al AP
 * PARÁMETROS: ninguno
 * RETORNA: int - número de clientes
 */
int wifi_get_client_count();

#endif // WIFI_MANAGER_H

