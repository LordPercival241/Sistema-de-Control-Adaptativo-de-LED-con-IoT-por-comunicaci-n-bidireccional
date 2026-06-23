/*
 * ============================================================================
 * ARCHIVO: main.cpp
 * PROPÓSITO: Punto de entrada principal del firmware ESP32
 * 
 * Este es el archivo principal que orquesta todo el sistema:
 * - Inicializa todos los módulos
 * - Implementa la máquina de estados principal
 * - Gestiona timers y lecturas periódicas
 * ============================================================================
 */

// Incluir todas las cabeceras de módulos
#include "config.h"
#include "sensor_manager.h"
#include "led_controller.h"
#include "wifi_manager.h"
#include "http_client.h"

// ============================================================================
// VARIABLES GLOBALES DEL SISTEMA
// ============================================================================

// Estructuras de datos globales
SensorData last_sensor_data;
LEDState last_led_state;

// Variables para control de tiempos
unsigned long last_sensor_read_time = 0;      // Última lectura del sensor
unsigned long last_data_send_time = 0;        // Último envío de datos
unsigned long system_start_time = 0;          // Tiempo de inicio del sistema

// Contador de lecturas y envíos (para estadísticas)
int sensor_read_count = 0;
int data_send_count = 0;
int data_send_success_count = 0;

// ============================================================================
// FUNCIONES SETUP Y LOOP
// ============================================================================

/*
 * FUNCIÓN: setup()
 * PROPÓSITO: Inicialización única al arrancar el ESP32
 * 
 * ORDEN CRÍTICO:
 * 1. Serial para debug
 * 2. Inicializar hardware (GPIO)
 * 3. Configurar WiFi AP
 * 4. Imprimir información de inicio
 */
void setup() {
    /*
     * Inicializar comunicación serial
     * Velocidad: 115200 baud (estándar ESP32)
     * Esto permite ver logs en el monitor serial
     */
    Serial.begin(SERIAL_BAUDRATE);
    
    // Pequeña pausa para que Serial se establezca
    delay(500);
    
    // Imprimir encabezado de bienvenida
    if (DEBUG_ENABLED) {
        Serial.println("\n\n");
        Serial.println("╔════════════════════════════════════════════════════════╗");
        Serial.println("║   SISTEMA IoT - CONTROL ADAPTATIVO DE LED             ║");
        Serial.println("║   Inicializando ESP32...                              ║");
        Serial.println("╚════════════════════════════════════════════════════════╝");
        Serial.println();
        Serial.println("[SISTEMA] Dispositivo: " + String(DEVICE_NAME));
        Serial.println("[SISTEMA] Versión firmware: " + String(FIRMWARE_VERSION));
        Serial.println("[SISTEMA] Compilado: " + String(__DATE__) + " " + String(__TIME__));
    }
    
    // Registrar tiempo de inicio
    system_start_time = millis();
    
    // Paso 1: Inicializar sensor LDR
    if (DEBUG_ENABLED) {
        Serial.println("\n[SETUP] Inicializando componentes de hardware...");
    }
    sensor_init();
    
    // Paso 2: Inicializar controlador LED
    led_init();
    
    // Paso 3: Configurar WiFi como Punto de Acceso
    if (DEBUG_ENABLED) {
        Serial.println("[SETUP] Configurando WiFi AP...");
    }
    bool wifi_ok = wifi_init_ap();
    
    if (!wifi_ok) {
        if (DEBUG_ENABLED) {
            Serial.println("[SETUP] Error critico: no se pudo inicializar WiFi");
            Serial.println("[SETUP] Sistema en FALLO - Apagando...");
        }
        // En producción, podrías reintentar o reiniciar el dispositivo
        while (true) {
            delay(1000);  // Esperar eternamente
        }
    }
    
    // Paso 4: Mensaje final de inicialización
    if (DEBUG_ENABLED) {
        Serial.println("\n╔════════════════════════════════════════════════════════╗");
        Serial.println("║   SISTEMA INICIALIZADO CORRECTAMENTE                 ║");
        Serial.println("║   Esperando conexiones WiFi...                       ║");
        Serial.println("╚════════════════════════════════════════════════════════╝");
        Serial.println();
        Serial.println("Conectarse a: SSID='" + String(WIFI_SSID) + "' Contraseña='" + String(WIFI_PASSWORD) + "'");
        Serial.println("IP del AP: " + WiFi.softAPIP().toString());
        Serial.println();
    }
}

/*
 * FUNCIÓN: loop()
 * PROPÓSITO: Bucle principal que se ejecuta continuamente
 * 
 * MÁQUINA DE ESTADOS PRINCIPAL:
 * 1. Leer sensor cada SENSOR_READ_INTERVAL
 * 2. Controlar LED basado en lectura
 * 3. Enviar datos al backend cada DATA_SEND_INTERVAL
 * 4. Mantener información de estado
 */
void loop() {
    // Obtener tiempo actual
    unsigned long now = millis();
    
    // ========================================================================
    // TAREA 1: Lectura periódica del sensor
    // ========================================================================
    
    // Verificar si es tiempo de leer el sensor
    if ((now - last_sensor_read_time) >= SENSOR_READ_INTERVAL) {
        // Actualizar tiempo de última lectura
        last_sensor_read_time = now;
        
        // Leer sensor LDR
        last_sensor_data = sensor_read();
        
        // Incrementar contador
        sensor_read_count++;
        
        // Debug
        if (DEBUG_ENABLED) {
            Serial.print("[SENSOR] Lectura #" + String(sensor_read_count) + " - ");
            Serial.print("ADC: " + String(last_sensor_data.raw_adc_value) + " - ");
            Serial.print("Luz: " + String(last_sensor_data.light_percentage, 1) + "% - ");
            Serial.println("Temp: " + String(last_sensor_data.temperature, 1) + "°C");
        }
        
        // ====================================================================
        // TAREA 2: Control del LED basado en lectura del sensor
        // ====================================================================
        
        // Obtener modo actual del LED
        LEDState led_state = led_get_state();
        
        if (led_state.mode == LED_MODE_AUTO) {
            // Modo automático: calcular intensidad basada en luz ambiente
            int new_intensity = map_light_to_intensity(last_sensor_data.light_percentage);
            
            // Aplicar nueva intensidad
            led_set_intensity(new_intensity);
        }
        
        // Actualizar estado global del LED
        last_led_state = led_get_state();
    }
    
    // ========================================================================
    // TAREA 3: Envío periódico de datos al backend
    // ========================================================================
    
    // Verificar si es tiempo de enviar datos
    if ((now - last_data_send_time) >= DATA_SEND_INTERVAL) {
        // Actualizar tiempo de último envío
        last_data_send_time = now;
        
        // Incrementar contador
        data_send_count++;
        
        // Enviar datos HTTP al backend
        String led_command = "";
        HTTPResult result = http_send_sensor_data(
            last_sensor_data.light_percentage,
            last_led_state.current_intensity,
            last_sensor_data.temperature,
            led_command
        );
        
        // Procesar comando de LED recibido
        if (led_command == "OFF") {
            led_set_mode(LED_MODE_OFF);
        } else if (led_command == "AUTO") {
            led_set_mode(LED_MODE_AUTO);
        } else if (led_command == "MANUAL") {
            led_set_mode(LED_MODE_MANUAL);
        }
        
        // Verificar resultado del envío
        if (result == HTTP_SUCCESS) {
            data_send_success_count++;
            if (DEBUG_ENABLED) {
                Serial.println("[ENVIO] Datos enviados exitosamente (#" + String(data_send_success_count) + ")");
            }
        } else {
            if (DEBUG_ENABLED) {
                String error_msg;
                switch (result) {
                    case HTTP_CLIENT_ERROR:
                        error_msg = "Error del cliente (400-499)";
                        break;
                    case HTTP_SERVER_ERROR:
                        error_msg = "Error del servidor (500-599)";
                        break;
                    case HTTP_CONNECTION_ERROR:
                        error_msg = "Error de conexion";
                        break;
                    case HTTP_TIMEOUT:
                        error_msg = "Timeout";
                        break;
                    default:
                        error_msg = "Desconocido";
                }
                Serial.println("[ENVIO] Error enviando datos: " + error_msg);
            }
        }
        
        // Mostrar información de conexión WiFi
        if (DEBUG_ENABLED) {
            WiFiInfo wifi_info = wifi_get_state();
            Serial.println("[WiFi] Clientes conectados: " + String(wifi_info.connected_clients));
        }
    }
    
    // ========================================================================
    // TAREA 4: Información de estado periódica (cada 30 segundos)
    // ========================================================================
    
    // Mostrar estadísticas cada 30 segundos
    static unsigned long last_stats_time = 0;
    if ((now - last_stats_time) >= 30000) {
        last_stats_time = now;
        
        if (DEBUG_ENABLED) {
            unsigned long uptime_seconds = now / 1000;
            unsigned long uptime_minutes = uptime_seconds / 60;
            unsigned long uptime_hours = uptime_minutes / 60;
            
            Serial.println("\n┌─── ESTADÍSTICAS DE SISTEMA ───┐");
            Serial.println("│ Tiempo activo: " + String(uptime_hours) + "h " + 
                          String(uptime_minutes % 60) + "m " + String(uptime_seconds % 60) + "s");
            Serial.println("│ Lecturas sensor: " + String(sensor_read_count));
            Serial.println("│ Envíos totales: " + String(data_send_count));
            Serial.println("│ Envíos exitosos: " + String(data_send_success_count));
            if (data_send_count > 0) {
                int success_rate = (data_send_success_count * 100) / data_send_count;
                Serial.println("│ Tasa de éxito: " + String(success_rate) + "%");
            }
            Serial.println("└─────────────────────────────────┘\n");
        }
    }
    
    // ========================================================================
    // GESTIÓN DE ENERGÍA
    // ========================================================================
    
    // Pequeña pausa para no saturar el CPU
    // (El loop se ejecuta miles de veces por segundo sin esto)
    delay(10);  // 10 ms = deja que otros procesos funcionen
}

// ============================================================================
// NOTAS SOBRE ARQUITECTURA
// ============================================================================

/*
 * VENTAJAS DE ESTE DISEÑO:
 * 
 * 1. MODULARIDAD: Cada componente está en su propio módulo
 *    - sensor_manager.cpp: Solo lógica de sensor
 *    - led_controller.cpp: Solo lógica de LED
 *    - wifi_manager.cpp: Solo lógica de WiFi
 *    - http_client.cpp: Solo comunicación HTTP
 * 
 * 2. DESACOPLAMIENTO: Los módulos no dependen unos de otros
 *    - Puedes reemplazar un módulo sin afectar a otros
 *    - Facilita pruebas unitarias
 * 
 * 3. ESCALABILIDAD: Fácil de agregar nuevos sensores/actuadores
 *    - Solo crear nuevo módulo sensor_X_manager.cpp
 *    - Llamarlo desde loop() con su intervalo
 * 
 * 4. MANTENIBILIDAD: Código bien documentado en español
 *    - Cada función tiene explicación clara
 *    - Fácil de entender para futuros mantenedores
 * 
 * 5. NO BLOQUEANTE: El sistema usa timers, no delay()
 *    - El WiFi sigue funcionando
 *    - Se pueden procesar interrupciones
 *    - Receptivo a múltiples tareas
 * 
 * FUTURAS MEJORAS:
 * - Agregar Real Time Clock (RTC) para timestamps reales
 * - Implementar OTA (Over The Air) updates
 * - Agregar sensor de temperatura real (DHT22)
 * - Implementar machine learning para predicción de luz
 * - Agregar múltiples sensores LDR en diferentes ubicaciones
 * - Persistencia en SPIFFS para datos offline
 */

