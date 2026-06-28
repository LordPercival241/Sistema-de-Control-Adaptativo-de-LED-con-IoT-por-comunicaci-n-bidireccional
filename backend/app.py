# ============================================================================
# ARCHIVO: app.py
# PROPÓSITO: Aplicación principal Flask
#
# Punto de entrada del backend - configura Flask y todos los módulos
# ============================================================================

from flask import Flask, jsonify, send_from_directory
from flask_cors import CORS
import config
import os
from routes.sensor_routes import sensor_bp

# Ruta absoluta a la carpeta del frontend
FRONTEND_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'frontend')

# ============================================================================
# CREAR APLICACIÓN FLASK
# ============================================================================

def create_app():
    """
    FUNCIÓN: create_app()
    PROPÓSITO: Factory function para crear la aplicación Flask
    
    RETORNA: Instancia de Flask configurada
    
    NOTAS:
    - Factory pattern permite crear múltiples instancias de la app
    - Útil para testing
    - Centraliza toda la configuración
    """
    
    # Crear instancia de Flask que también sirve el frontend
    app = Flask(__name__, static_folder=FRONTEND_DIR, static_url_path='')
    
    # Configurar CORS (Cross-Origin Resource Sharing)
    # Permite que el frontend acceda a la API desde localhost
    CORS(app, resources={
        r"/api/*": {
            "origins": config.CORS_ORIGINS,
            "methods": ["GET", "POST", "OPTIONS"],
            "allow_headers": ["Content-Type"]
        }
    })
    
    # ========================================================================
    # REGISTRAR BLUEPRINTS (módulos de rutas)
    # ========================================================================
    
    # Blueprint de rutas de sensores
    app.register_blueprint(sensor_bp)
    
    # ========================================================================
    # RUTAS GENERALES
    # ========================================================================
    
    @app.route('/', methods=['GET'])
    def index():
        """
        ENDPOINT: GET /
        PROPÓSITO: Servir el dashboard del frontend
        """
        return send_from_directory(FRONTEND_DIR, 'index.html')
    
    # ========================================================================
    # MANEJO DE ERRORES GLOBAL
    # ========================================================================
    
    @app.errorhandler(404)
    def not_found(error):
        """
        MANEJADOR: Error 404 - Ruta no encontrada
        """
        return jsonify({
            'success': False,
            'error': 'Ruta no encontrada',
            'path': error.description
        }), 404
    
    @app.errorhandler(500)
    def server_error(error):
        """
        MANEJADOR: Error 500 - Error interno del servidor
        """
        return jsonify({
            'success': False,
            'error': 'Error interno del servidor'
        }), 500
    
    # ========================================================================
    # LOGGING Y DEBUG
    # ========================================================================
    
    if config.DEBUG:
        print("\n" + "="*60)
        print("BACKEND EN MODO DEBUG")
        print("="*60)
        print(f"Host: {config.HOST}")
        print(f"Puerto: {config.PORT}")
        print(f"Archivo CSV: {config.CSV_FILE_PATH}")
        print(f"CORS Orígenes: {config.CORS_ORIGINS}")
        print("="*60 + "\n")
    
    return app

# ============================================================================
# PUNTO DE ENTRADA
# ============================================================================

if __name__ == '__main__':
    """
    SECCIÓN: Punto de entrada del programa
    
    Nota: No usamos app.run() directamente en desarrollo
    Mejor usar: flask --app app run
    O en producción: gunicorn -w 4 app:create_app()
    """
    
    # Crear aplicación
    app = create_app()
    
    # Ejecutar servidor de desarrollo
    # ADVERTENCIA: No usar en producción
    app.run(
        host=config.HOST,
        port=config.PORT,
        debug=config.DEBUG,
        use_reloader=False  # Desactivar recarga automática en loop
    )

