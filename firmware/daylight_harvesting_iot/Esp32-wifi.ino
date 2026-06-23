/*
 * Sistema de Control PID - Daylight Harvesting (Respuesta Rápida) + WiFi y HTTP
 * Plataforma: ESP32 (Estándar 38 pines)
 * Actuador: Tira LED vía Módulo MOSFET IRF520
 * 
 * NOTA: Para subir este código, necesitas instalar la librería ArduinoJson si decides usarla,
 * pero en esta versión construimos el JSON como String directamente para mantenerlo simple.
 */

#include <WiFi.h>
#include <HTTPClient.h>

// ---------------- Configuraciones de Hardware ----------------
#define PIN_SENSOR_LDR 34   
#define PIN_ACTUADOR_LED 16 

// ---------------- Configuraciones WiFi y Backend (Access Point) ----------------
const char* WIFI_SSID = "Esp32-Tópicos IV";
const char* WIFI_PASSWORD = "12345678";

// La laptop se conectará a esta red y la IP asignada será 192.168.4.2
const char* BACKEND_URL = "http://192.168.4.2:5000/api/sensor/data";

// --- Configuración AP Estática ---
IPAddress local_ip(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// ---------------- Parámetros de Control ----------------
const double SETPOINT_LUX = 400.0; 
const double TOLERANCIA_LUX = 20.0; // Banda muerta de +- 20 Lux

// --- NUEVAS CONSTANTES PID (Más rápidas) ---
double Kp = 0.25;  // Aumentado para reaccionar más rápido a las sombras/luces
double Ki = 0.08;  // Aumentado para alcanzar el Setpoint en menos tiempo
double Kd = 0.0;   // Se mantiene en 0 para evitar ruido

// Variables de estado del PID
double integral = 0;
double error_previo = 0;
unsigned long tiempo_anterior = 0;
const int tiempo_muestreo_ms = 100; 

// Variables para envío HTTP
unsigned long tiempo_ultimo_envio = 0;
const int intervalo_envio_ms = 1000; // Enviar datos cada 1 segundo (1000 ms)

// Estado del PWM y Sensor
double accion_control = 0; 
int pwm_final = 0;
double lux_actuales = 0;

// ---------------- Configuraciones PWM ----------------
const int frecuenciaPWM = 5000; 
const int resolucionPWM = 8;    

bool logica_invertida = false; 
bool led_apagado_remoto = false; // Variable global para control web

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  ledcAttach(PIN_ACTUADOR_LED, frecuenciaPWM, resolucionPWM);
  tiempo_anterior = millis();
  
  Serial.println("Iniciando Sistema Daylight Harvesting (Modo Rápido)...");

  // Iniciar WiFi en modo Access Point
  Serial.print("Iniciando MODO ACCESS POINT: ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.println("\nRed Wi-Fi Creada Exitosamente!");
  Serial.print("Dirección IP del ESP32 (Servidor): ");
  Serial.println(WiFi.softAPIP());
  Serial.println("ESPERANDO QUE LA LAPTOP SE CONECTE...");
}

void loop() {
  unsigned long tiempo_actual = millis();
  double dt = (double)(tiempo_actual - tiempo_anterior) / 1000.0; 

  if (dt >= ((double)tiempo_muestreo_ms / 1000.0)) {
    
    // --- 1. LECTURA Y ACONDICIONAMIENTO ---
    int valorADC = analogRead(PIN_SENSOR_LDR);
    double Vout = valorADC * (3.3 / 4095.0);
    if (Vout < 0.0) Vout = 0.001; 
    
    // --- 2. LINEALIZACIÓN (Informe N°2) ---
    double numerador = 0.5 - (Vout / 20.0);
    double denominador = 0.5 + (Vout / 20.0);
    if (denominador == 0) denominador = 0.0001; 
    
    double R_ldr = 10.0 * (numerador / denominador);
    double ln_R = log(R_ldr); 
    double ln_E = (2.984 - ln_R) / 0.154;
    lux_actuales = exp(ln_E);
    
    // --- 3. CÁLCULO DEL ERROR Y BANDA MUERTA ---
    double error = SETPOINT_LUX - lux_actuales;
    
    // Si la luz está dentro de la tolerancia, congelamos el error para no parpadear
    if (abs(error) <= TOLERANCIA_LUX) {
      error = 0;
    }
    
    // --- 4. ALGORITMO PID ---
    double P = Kp * error;                     
    integral += (error * dt);                  
    
    // Anti-Windup mejorado
    if (integral > 255.0 / Ki) integral = 255.0 / Ki;
    if (integral < 0) integral = 0;
    
    double I = Ki * integral;                  
    double D = Kd * (error - error_previo) / dt; 
    
    accion_control = P + I + D;
    
    // --- 5. SATURACIÓN FINAL ---
    if (accion_control > 255) accion_control = 255;
    if (accion_control < 0) accion_control = 0;
    
    pwm_final = (int)accion_control;
    
    // --- 6. ACTUACIÓN ---
    // Si desde la web se ordenó apagar, forzamos la salida a 0
    if (led_apagado_remoto) {
      pwm_final = 0;
      integral = 0; // Reseteamos la integral para evitar que se acumule error mientras está apagado
    }

    if (logica_invertida) {
      ledcWrite(PIN_ACTUADOR_LED, 255 - pwm_final); 
    } else {
      ledcWrite(PIN_ACTUADOR_LED, pwm_final);       
    }
    
    // --- 7. ACTUALIZACIÓN DE ESTADOS ---
    error_previo = error;
    tiempo_anterior = tiempo_actual;
    
    // --- 8. MONITOREO ---
    Serial.print("Setpoint:"); Serial.print(SETPOINT_LUX); Serial.print(",");
    Serial.print("Lux_Reales:"); Serial.print(lux_actuales); Serial.print(",");
    Serial.print("PWM_Output:"); Serial.println(pwm_final);
  }

  // --- 9. ENVÍO DE DATOS AL BACKEND ---
  if (tiempo_actual - tiempo_ultimo_envio >= intervalo_envio_ms) {
    HTTPClient http;
    http.begin(BACKEND_URL);
    http.addHeader("Content-Type", "application/json");

    // Construcción del JSON
    // Se omite timestamp para que el backend lo asigne automáticamente usando str(datetime.now())
    String json_data = "{";
    json_data += "\"setpoint\":" + String(SETPOINT_LUX, 1) + ",";
    json_data += "\"lux\":" + String(lux_actuales, 2) + ",";
    json_data += "\"pwm_output\":" + String(pwm_final) + ",";
    json_data += "\"device_id\":\"ESP32_PID\"";
    json_data += "}";

    int http_response_code = http.POST(json_data);

    if (http_response_code > 0) {
      String response = http.getString();
      
      // Analizar la respuesta del backend buscando la orden del LED
      if (response.indexOf("\"led_command\":\"OFF\"") >= 0 || response.indexOf("\"led_command\": \"OFF\"") >= 0) {
        led_apagado_remoto = true;
      } else if (response.indexOf("\"led_command\":\"AUTO\"") >= 0 || response.indexOf("\"led_command\": \"AUTO\"") >= 0) {
        led_apagado_remoto = false;
      }
      
    } else {
      Serial.print("[HTTP] Error en envío: ");
      Serial.println(http_response_code);
    }
    
    http.end();
    tiempo_ultimo_envio = tiempo_actual;
  }
}
