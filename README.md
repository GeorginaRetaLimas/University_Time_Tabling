# University Timetabling System

Sistema de generación de horarios universitarios optimizado con C++ y Python.

## Requisitos Previos

### General
- Python 3.8 o superior
- Navegador Web Moderno

### Linux (Ubuntu/Debian)
- Compilador GCC/G++: `sudo apt install build-essential`
- Python Dev: `sudo apt install python3-dev`

### Windows
- **Visual Studio Build Tools**: Descargar e instalar con la carga de trabajo "Desarrollo para el escritorio con C++".
- O alternativamente **MinGW-w64**.

## Instalación y Ejecución

### 🐧 Linux

1.  **Instalar dependencias de Python:**
    ```bash
    pip install -r requirements.txt
    ```

2.  **Compilar el módulo C++:**
    Da permisos de ejecución y corre el script de compilación:
    ```bash
    chmod +x compile.sh
    ./compile.sh
    ```

3.  **Ejecutar la aplicación:**
    ```bash
    cd backend
    python3 app.py
    ```

4.  **Acceder:**
    Abre [http://localhost:5000](http://localhost:5000) en tu navegador.

### 🪟 Windows

1.  **Instalar dependencias:**
    Abre CMD o PowerShell y ejecuta:
    ```cmd
    pip install -r requirements.txt
    ```

2.  **Compilar extensión:**
    Ejecuta el siguiente comando para construir el módulo de Cython (asegúrate de tener las Build Tools instaladas):
    ```cmd
    python cython/setup.py build_ext --build-lib backend
    ```
    *Nota: Si usas MinGW, añade `--compiler=mingw32` al final del comando.*

3.  **Ejecutar la aplicación:**
    ```cmd
    cd backend
    python app.py
    ```

4.  **Acceder:**
    Abre [http://localhost:5000](http://localhost:5000) en tu navegador.

## Estructura del Proyecto
- `cpp/`: Código fuente C++ (algoritmo de optimización).
- `cython/`: Wrapper para conectar C++ con Python.
- `backend/`: Servidor Flask y lógica de negocio.
- `frontend/`: Interfaz web (HTML, CSS, JS).
- `data/`: Archivos de datos (JSON, CSV).

## Solución de Problemas
- **Error "Python.h not found"**: Asegúrate de tener instalado `python3-dev` (Linux) o que las rutas de Python estén en el PATH (Windows).
- **Error de compilación C++**: Verifica que tu compilador soporte C++17.
