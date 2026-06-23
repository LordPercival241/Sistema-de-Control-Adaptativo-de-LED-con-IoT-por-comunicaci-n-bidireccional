# Guía de Configuración del Frontend

## Requisitos

- Navegador web moderno (Google Chrome, Mozilla Firefox, Microsoft Edge, Safari).
- Conexión a la red local establecida por el ESP32.
- Backend en ejecución respondiendo en `http://localhost:5000` (o la dirección IP configurada).

---

## Modos de Ejecución

### Opción 1: Apertura directa en navegador (Modo Local)

1. Navegar a la carpeta `frontend/`.
2. Abrir el archivo `index.html` en el navegador web (haciendo doble clic o arrastrándolo a la ventana del navegador).

### Opción 2: Uso de un servidor HTTP local (Recomendado)

Si dispone de Python instalado en el sistema, puede iniciar un servidor web simple ejecutando:

```bash
cd frontend
python -m http.server 3000
```

Posteriormente, acceda a la dirección: `http://localhost:3000` en su navegador.

---

## Configuración de Conexión

Es posible ajustar la dirección de consulta editando el archivo `js/app.js`:

```javascript
const CONFIG = {
    // Definir la dirección de red según el entorno
    BACKEND_URL: 'http://localhost:5000',
    // ...
};
```

**Ejemplos de configuración:**
- Backend ejecutándose localmente: `http://localhost:5000`
- Backend ejecutándose en una máquina en red cableada/WiFi: `http://192.168.X.X:5000`
- Backend ejecutándose en la laptop conectada a la red AP del ESP32: `http://192.168.4.2:5000`

---

## Características de la Interfaz

### Gráficos Interactivos en Tiempo Real
- **Iluminación vs. Intensidad LED**: Visualiza la relación inversa automatizada del control PID.
- **Historial**: Mapeo temporal de las últimas lecturas.
- **Temperatura**: Monitoreo de la temperatura del microcontrolador.

### Visualización y Descarga
- **Tabla de Telemetría**: Historial de lecturas con marcas de tiempo.
- **Exportación**: Descarga del historial completo en formato plano CSV compatible con hojas de cálculo.

---

## Estado de la Conexión con el Servidor

El frontend realiza consultas periódicas de manera automática:
- **Consulta de telemetría**: Petición GET a `/api/data/latest`.
- **Estadísticas**: Petición GET a `/api/stats`.
- **Indicador Visual**:
  - Activo (Verde): Servidor backend en línea y respondiendo.
  - Inactivo (Rojo): Fallo de conexión con el servidor.

---

## Diseño Responsivo

La interfaz de usuario está adaptada para su correcta visualización en:
- Ordenadores portátiles y de escritorio (Pantalla Completa).
- Tabletas.
- Dispositivos móviles.

---

## Personalización de Estilos

Los colores y variables principales del tema se pueden modificar en `css/styles.css`:

```css
:root {
    --primary-color: #0d6efd;      /* Color de énfasis */
    --warning-color: #ffc107;      /* Sensor de iluminación */
    --danger-color: #dc3545;       /* Sensor de temperatura */
    --info-color: #0dcaf0;         /* Salida de control PWM */
}
```

---

## Diagnóstico y Resolución de Problemas

### Error: No se renderizan los gráficos
1. Abra la consola de desarrollo del navegador pulsando la tecla `F12`.
2. Revise la pestaña de errores (Console) para identificar peticiones fallidas.
3. Asegúrese de que el backend Flask esté en ejecución.

### Error: Fallo de CORS (Cross-Origin Resource Sharing)
1. Confirme la configuración de la variable `BACKEND_URL` en `js/app.js`.
2. Asegúrese de que el origen del cliente web está permitido dentro del parámetro `CORS_ORIGINS` del backend.

---

## Flujo de Datos del Sistema

```
ESP32 (Adquisición de Datos)
   ↓ HTTP POST /api/sensor/data
Backend Flask (Persistencia en Servidor)
   ↓ Escribe en archivo CSV
   ↓ Envía Respuesta JSON (HTTP 200/201)
Frontend (Dashboard de Monitoreo)
   ↓ HTTP GET /api/data/latest
   ↓ Actualiza Gráficos y Tablas
Navegador (Presentación)
   ↓ Renderizado para el usuario final
```
