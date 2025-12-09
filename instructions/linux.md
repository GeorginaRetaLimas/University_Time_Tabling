# Guía de Ejecución para Linux

Sigue estos pasos para compilar y ejecutar el proyecto en un entorno Linux (Ubuntu/Debian/Fedora/etc.).

## 1. Requisitos Previos

Asegúrate de tener instalados:
- **Python 3.8+**
- **pip** (gestor de paquetes de Python)
- **Compilador C++** (g++)

```bash
# Verificar versiones
python3 --version
g++ --version
```

Si no tienes g++, instálalo:
```bash
# Ubuntu/Debian
sudo apt update
sudo apt install build-essential python3-dev

# Fedora
sudo dnf groupinstall "Development Tools" "Development Libraries"
```

## 2. Configuración del Entorno (Virtual Environment)

Es **altamente recomendable** usar un entorno virtual para evitar conflictos con el sistema.

```bash
# 1. Crear el entorno virtual (llamado 'venv')
python3 -m venv venv

# 2. Activar el entorno
source venv/bin/activate

# Verás que tu terminal ahora muestra (venv) al principio.
```

## 3. Instalación de Dependencias

Con el entorno activado, instala las librerías:

```bash
pip install -r requirements.txt
```

## 4. Compilación del Núcleo C++ (Cython)

El sistema utiliza un núcleo en C++ para el algoritmo de asignación.

```bash
# Ir al directorio cython
cd cython

# Compilar el módulo
python3 setup.py build_ext --inplace

# Volver a la raíz
cd ..
```

Deberías ver un archivo generado como `timetable_wrapper.cpython-3x-x86_64-linux-gnu.so`.

## 5. Ejecución del Servidor

```bash
# Ir al directorio backend
cd backend

# Ejecutar la aplicación
python3 app.py
```

Verás un mensaje indicando que el servidor está corriendo, por ejemplo:
`Running on http://127.0.0.1:5000`

## 5. Uso de la Aplicación

1. Abre tu navegador web (Chrome, Firefox, etc.).
2. Ve a: [http://localhost:5000](http://localhost:5000)
3. Selecciona el período académico y haz clic en "Generar Horario".
