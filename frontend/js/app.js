/*
 * ============================================================================
 * ARCHIVO: js/app.js
 * PROPÓSITO: Lógica principal del frontend para el sistema PID Daylight Harvesting
 *
 * Gestiona:
 * - Conexión con el backend
 * - Actualización de datos en tiempo real
 * - Gráficos interactivos del PID
 * - Interfaz del usuario
 * ============================================================================
 */

// ============================================================================
// CONFIGURACIÓN GLOBAL
// ============================================================================

const CONFIG = {
    // URL del backend (detectada dinámicamente o fallback a IP de la laptop)
    BACKEND_URL: window.location.hostname ? `${window.location.protocol}//${window.location.hostname}:5000` : 'http://192.168.4.2:5000',
    
    // Intervalo de actualización por defecto (segundos)
    DEFAULT_REFRESH_INTERVAL: 1,
    
    // Número máximo de puntos en gráficos
    MAX_CHART_POINTS: 20,
    
    // Colores para gráficos
    COLORS: {
        primary: '#0d6efd',
        success: '#198754',
        warning: '#ffc107',
        danger: '#dc3545',
        info: '#0dcaf0'
    }
};

// ============================================================================
// VARIABLES GLOBALES
// ============================================================================

// Datos históricos
let sensorDataHistory = [];

// Gráficos Chart.js
let chartPid = null;

// ============================================================================
// GESTIÓN DE TEMAS (Claro/Oscuro)
// ============================================================================

/**
 * Inicializa el sistema de temas
 * Lee preferencia de localStorage o usa preferencia del sistema
 */
function initTheme() {
    const savedTheme = localStorage.getItem('theme');
    const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
    const isDarkTheme = savedTheme === 'dark' || (!savedTheme && prefersDark);
    
    if (isDarkTheme) {
        setTheme('dark');
    } else {
        setTheme('light');
    }
    
    // Redibujar gráficos después de aplicar tema
    setTimeout(() => {
        if (chartPid) chartPid.resize();
    }, 50);
}

/**
 * Cambia entre temas claro y oscuro
 * @param {string} theme - 'light' o 'dark'
 */
function setTheme(theme) {
    const body = document.body;
    const isLight = theme === 'light';
    
    if (isLight) {
        body.classList.add('light-theme');
    } else {
        body.classList.remove('light-theme');
    }
    
    localStorage.setItem('theme', theme);
    updateThemeIcon();
}

/**
 * Alterna entre tema claro y oscuro
 */
function toggleTheme() {
    const currentTheme = localStorage.getItem('theme') || 'light';
    const newTheme = currentTheme === 'light' ? 'dark' : 'light';
    setTheme(newTheme);
    
    // Redibujar gráficos después del cambio de tema
    setTimeout(() => {
        if (chartPid) chartPid.resize();
    }, 100);
}

/**
 * Actualiza el icono del botón de tema
 */
function updateThemeIcon() {
    const themeIcon = document.getElementById('theme-icon');
    if (themeIcon) {
        const isLight = document.body.classList.contains('light-theme');
        themeIcon.textContent = isLight ? 'Dia' : 'Noche';
    }
}

// Intervalo de actualización automática
let autoRefreshInterval = null;

// ============================================================================
// INICIALIZACIÓN
// ============================================================================

document.addEventListener('DOMContentLoaded', function() {
    console.log('Inicializando aplicación...');
    
    // Inicializar tema (claro/oscuro)
    initTheme();
    
    // Inicializar gráficos
    initializeCharts();
    
    // Configurar event listeners
    setupEventListeners();
    
    // Cargar datos iniciales
    refreshData();
    
    // Iniciar actualización automática
    startAutoRefresh();
    
    console.log('Aplicación inicializada');
});

// ============================================================================
// FUNCIÓN: Inicializar Gráficos
// ============================================================================

function initializeCharts() {
    // Gráfico de Control PID
    const ctxPid = document.getElementById('chart-pid').getContext('2d');
    chartPid = new Chart(ctxPid, {
        type: 'line',
        data: {
            labels: [],
            datasets: [
                {
                    label: 'Setpoint (Lux)',
                    data: [],
                    borderColor: CONFIG.COLORS.danger,
                    borderWidth: 2,
                    borderDash: [5, 5], // Línea punteada para el setpoint
                    fill: false,
                    pointRadius: 0,
                    tension: 0,
                    yAxisID: 'y'
                },
                {
                    label: 'Lux Reales',
                    data: [],
                    borderColor: CONFIG.COLORS.warning,
                    backgroundColor: 'rgba(255, 193, 7, 0.1)',
                    borderWidth: 2,
                    fill: true,
                    tension: 0.1,
                    yAxisID: 'y'
                },
                {
                    label: 'PWM Output (0-255)',
                    data: [],
                    borderColor: CONFIG.COLORS.info,
                    backgroundColor: 'rgba(13, 202, 240, 0.1)',
                    borderWidth: 2,
                    fill: true,
                    tension: 0.1,
                    yAxisID: 'y1'
                }
            ]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            interaction: {
                mode: 'index',
                intersect: false
            },
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                }
            },
            scales: {
                y: {
                    title: {
                        display: true,
                        text: 'Iluminación (Lux)'
                    },
                    position: 'left',
                    min: 0,
                    suggestedMax: 1000 // Sugerido, se autoescala si sube más
                },
                y1: {
                    title: {
                        display: true,
                        text: 'PWM (0-255)'
                    },
                    position: 'right',
                    min: 0,
                    max: 255,
                    grid: {
                        drawOnChartArea: false // Evitar líneas de grid superpuestas
                    }
                }
            }
        }
    });
}

// ============================================================================
// FUNCIÓN: Configurar Event Listeners
// ============================================================================

function setupEventListeners() {
    // Botón actualizar
    const btnRefresh = document.getElementById('btn-refresh');
    if (btnRefresh) {
        btnRefresh.addEventListener('click', function() {
            this.textContent = 'Actualizando...';
            this.disabled = true;
            
            refreshData().then(() => {
                this.textContent = 'ACTUALIZAR DATOS';
                this.disabled = false;
            });
        });
    }
    
    // Botón exportar
    const btnExport = document.getElementById('btn-export');
    if (btnExport) {
        btnExport.addEventListener('click', exportToCSV);
    }
    
    // Botón cambiar tema
    const themeToggle = document.getElementById('theme-toggle');
    if (themeToggle) {
        themeToggle.addEventListener('click', function() {
            toggleTheme();
            log('Tema cambiado');
        });
    }
    
    // Controles LED
    const btnLedAuto = document.getElementById('btn-led-auto');
    const btnLedOff = document.getElementById('btn-led-off');
    if (btnLedAuto) btnLedAuto.addEventListener('click', () => setLedMode('AUTO'));
    if (btnLedOff) btnLedOff.addEventListener('click', () => setLedMode('OFF'));
    
    // Checkbox actualización automática
    const autoRefresh = document.getElementById('auto-refresh');
    if (autoRefresh) {
        autoRefresh.addEventListener('change', function() {
            if (this.checked) {
                startAutoRefresh();
                log('Actualización automática habilitada');
            } else {
                stopAutoRefresh();
                log('Actualización automática deshabilitada');
            }
        });
    }
    
    // Intervalo de actualización
    const refreshInterval = document.getElementById('refresh-interval');
    if (refreshInterval) {
        refreshInterval.addEventListener('change', function() {
            const interval = parseInt(this.value);
            if (interval >= 1 && interval <= 60) {
                const autoRefresh = document.getElementById('auto-refresh');
                if (autoRefresh && autoRefresh.checked) {
                    stopAutoRefresh();
                    startAutoRefresh();
                    log(`Intervalo de actualización: ${interval}s`);
                }
            }
        });
    }
}

// ============================================================================
// FUNCIÓN: Actualizar Datos
// ============================================================================

async function refreshData() {
    try {
        // Paso 1: Obtener últimos datos
        const response = await fetch(`${CONFIG.BACKEND_URL}/api/data/latest?limit=${CONFIG.MAX_CHART_POINTS}`);
        
        if (!response.ok) {
            throw new Error(`Error HTTP: ${response.status}`);
        }
        
        const result = await response.json();
        
        if (!result.success) {
            throw new Error(result.error);
        }
        
        // Guardar datos históricos
        sensorDataHistory = result.data;
        
        if (sensorDataHistory.length === 0) {
            log('Advertencia: Sin datos disponibles');
            updateConnectionStatus(false);
            return;
        }
        
        // Paso 2: Obtener el último registro
        const latestData = sensorDataHistory[sensorDataHistory.length - 1];
        
        // Paso 3: Actualizar métricas en pantalla
        updateMetrics(latestData);
        
        // Paso 4: Actualizar gráficos
        updateCharts(sensorDataHistory);
        
        // Paso 5: Actualizar tabla de datos
        updateDataTable(sensorDataHistory);
        
        // Paso 6: Obtener estadísticas
        await updateStatistics();
        
        // Actualizar estado de conexión basado en la edad del último dato
        const lastDataTime = new Date(latestData.timestamp);
        const currentTime = new Date();
        const diffSeconds = (currentTime - lastDataTime) / 1000;
        
        if (isNaN(diffSeconds) || diffSeconds > 15) {
            updateConnectionStatus(false);
            // Opcional: mostrar un log advirtiendo
            // log(`Desconectado: último dato hace ${Math.round(diffSeconds)}s`);
        } else {
            updateConnectionStatus(true);
        }
        log(`Exito: Datos actualizados (${sensorDataHistory.length} registros)`);
        
    } catch (error) {
        console.error('Error actualizando datos:', error);
        log(`Error: ${error.message}`);
        updateConnectionStatus(false);
    }
}

// ============================================================================
// FUNCIÓN: Actualizar Métricas
// ============================================================================

function updateMetrics(data) {
    // Setpoint
    const setpointValue = data.setpoint.toFixed(1);
    document.getElementById('setpoint-value').textContent = setpointValue;
    
    // Lux reales
    const luxValue = data.lux.toFixed(1);
    document.getElementById('lux-value').textContent = luxValue;
    
    // Calcular porcentaje de lux respecto al setpoint para la barra (max 100%)
    let luxPercent = (data.lux / data.setpoint) * 100;
    if (luxPercent > 100) luxPercent = 100;
    if (isNaN(luxPercent)) luxPercent = 0;
    document.getElementById('lux-bar').style.width = luxPercent + '%';
    
    // Determinar estado de la luz
    const error = Math.abs(data.setpoint - data.lux);
    const setpointStatus = document.getElementById('setpoint-status');
    if (error <= 20) {
        setpointStatus.textContent = 'En rango (±20)';
        setpointStatus.className = 'text-success';
    } else if (data.lux < data.setpoint) {
        setpointStatus.textContent = `Muy oscuro (-${error.toFixed(1)})`;
        setpointStatus.className = 'text-warning';
    } else {
        setpointStatus.textContent = `Muy brillante (+${error.toFixed(1)})`;
        setpointStatus.className = 'text-info';
    }
    
    // PWM Output
    const pwmPercent = Math.round((data.pwm_output / 255) * 100);
    document.getElementById('pwm-value').textContent = data.pwm_output;
    document.getElementById('pwm-bar').style.width = pwmPercent + '%';
}

// ============================================================================
// FUNCIÓN: Actualizar Gráficos
// ============================================================================

function updateCharts(dataArray) {
    // Extraer timestampos para etiquetas
    const labels = dataArray.map(d => {
        try {
            const date = new Date(d.timestamp);
            return date.toLocaleTimeString('es-ES');
        } catch (e) {
            return d.timestamp.substring(11, 19);
        }
    });
    
    // Extraer valores
    const setpointValues = dataArray.map(d => d.setpoint);
    const luxValues = dataArray.map(d => d.lux);
    const pwmValues = dataArray.map(d => d.pwm_output);
    
    // Actualizar gráfico PID
    chartPid.data.labels = labels;
    chartPid.data.datasets[0].data = setpointValues;
    chartPid.data.datasets[1].data = luxValues;
    chartPid.data.datasets[2].data = pwmValues;
    chartPid.update();
}

// ============================================================================
// FUNCIÓN: Actualizar Tabla de Datos
// ============================================================================

function updateDataTable(dataArray) {
    const tableBody = document.getElementById('data-table-body');
    if (tableBody) {
        tableBody.innerHTML = '';  // Limpiar tabla
        
        // Mostrar datos en orden inverso (más recientes primero)
        // Usamos una copia del array para no mutar el original
        [...dataArray].reverse().forEach(data => {
            const row = document.createElement('tr');
            
            // Formatear timestamp
            let timestamp = data.timestamp;
            try {
                const date = new Date(timestamp);
                timestamp = date.toLocaleString('es-ES');
            } catch (e) {
                // Si hay error, usar el timestamp como está
            }
            
            row.innerHTML = `
                <td><small>${timestamp}</small></td>
                <td><strong>${data.setpoint.toFixed(1)}</strong></td>
                <td><strong>${data.lux.toFixed(1)}</strong></td>
                <td><strong>${data.pwm_output}</strong></td>
            `;
            
            tableBody.appendChild(row);
        });
    }
}

// ============================================================================
// FUNCIÓN: Obtener Estadísticas
// ============================================================================

async function updateStatistics() {
    try {
        const response = await fetch(`${CONFIG.BACKEND_URL}/api/stats`);
        const result = await response.json();
        
        if (!result.success) {
            throw new Error(result.error);
        }
        
        const stats = result.statistics;
        
        // Actualizar elementos de estadísticas
        const statTotal = document.getElementById('stat-total');
        if (statTotal) {
            statTotal.textContent = stats.total_records;
        }
        
        const statAvgLux = document.getElementById('stat-avg-lux');
        if (statAvgLux) {
            statAvgLux.textContent = `${stats.avg_lux.toFixed(1)} Lux`;
        }
        
        const statAvgPwm = document.getElementById('stat-avg-pwm');
        if (statAvgPwm) {
            statAvgPwm.textContent = stats.avg_pwm_output.toFixed(0);
        }
        
    } catch (error) {
        console.error('Error obteniendo estadísticas:', error);
    }
}

// ============================================================================
// FUNCIÓN: Actualizar Estado de Conexión
// ============================================================================

function updateConnectionStatus(connected) {
    const statusElement = document.getElementById('connection-status');
    if (!statusElement) return;
    
    if (connected) {
        statusElement.textContent = 'ESTADO: CONECTADO';
        statusElement.className = 'status-badge connected';
    } else {
        statusElement.textContent = 'ESTADO: DESCONECTADO';
        statusElement.className = 'status-badge disconnected';
    }
}

// ============================================================================
// FUNCIÓN: Iniciar Actualización Automática
// ============================================================================

function startAutoRefresh() {
    // Detener intervalo anterior si existe
    stopAutoRefresh();
    
    // Obtener intervalo configurado
    const refreshInterval = document.getElementById('refresh-interval');
    const interval = refreshInterval ? parseInt(refreshInterval.value) : CONFIG.DEFAULT_REFRESH_INTERVAL;
    
    // Crear nuevo intervalo (convertir segundos a milisegundos)
    autoRefreshInterval = setInterval(refreshData, interval * 1000);
    
    console.log(`Actualización automática iniciada (${interval}s)`);
}

// ============================================================================
// FUNCIÓN: Detener Actualización Automática
// ============================================================================

function stopAutoRefresh() {
    if (autoRefreshInterval) {
        clearInterval(autoRefreshInterval);
        autoRefreshInterval = null;
    }
}

// ============================================================================
// FUNCIÓN: Exportar a CSV
// ============================================================================

function exportToCSV() {
    if (sensorDataHistory.length === 0) {
        alert('No hay datos para exportar');
        return;
    }
    
    // Paso 1: Crear encabezados CSV
    let csvContent = 'timestamp,setpoint,lux,pwm_output\n';
    
    // Paso 2: Agregar datos
    sensorDataHistory.forEach(data => {
        csvContent += `${data.timestamp},${data.setpoint},${data.lux},${data.pwm_output}\n`;
    });
    
    // Paso 3: Crear blob
    const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
    
    // Paso 4: Crear enlace y descargar
    const link = document.createElement('a');
    const url = URL.createObjectURL(blob);
    
    link.href = url;
    link.download = `pid_sensor_data_${new Date().toISOString().slice(0, 10)}.csv`;
    link.click();
    
    log('Archivo CSV descargado');
}

// ============================================================================
// FUNCIÓN: Controlar Modo LED
// ============================================================================

async function setLedMode(mode) {
    try {
        const response = await fetch(`${CONFIG.BACKEND_URL}/api/led/control`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ mode: mode })
        });
        
        const result = await response.json();
        
        if (!result.success) {
            throw new Error(result.error);
        }
        
        // Actualizar UI
        const btnAuto = document.getElementById('btn-led-auto');
        const btnOff = document.getElementById('btn-led-off');
        
        if (mode === 'AUTO') {
            btnAuto.classList.add('active');
            btnOff.classList.remove('active');
            log('Comando enviado: Modo Auto');
        } else if (mode === 'OFF') {
            btnOff.classList.add('active');
            btnAuto.classList.remove('active');
            log('Comando enviado: Apagar LED');
        }
        
    } catch (error) {
        console.error('Error enviando comando LED:', error);
        log(`Error: No se pudo cambiar el modo a ${mode}`);
    }
}

// ============================================================================
// FUNCIÓN: Logging
// ============================================================================

function log(message) {
    const logElement = document.getElementById('log-text');
    const timestamp = new Date().toLocaleTimeString('es-ES');
    logElement.textContent = `[${timestamp}] ${message}`;
}
