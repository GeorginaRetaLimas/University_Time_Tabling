# Guía de Ejecución para Windows

Sigue estos pasos para compilar y ejecutar el proyecto en un entorno Windows (10/11).

## 1. Requisitos Previos

Asegúrate de tener instalados:
- **Python 3.8+** (Asegúrate de marcar "Add Python to PATH" durante la instalación)
- **Visual Studio Build Tools** (Necesario para compilar C++)
  - Descarga desde: [Visual Studio Downloads](https://visualstudio.microsoft.com/visual-cpp-build-tools/)
  - Durante la instalación, selecciona la carga de trabajo: **"Desarrollo para el escritorio con C++"**

Para verificar Python en PowerShell o CMD:
```powershell
python --version
pip --version
```

## 2. Instalación de Dependencias

Abre una terminal (PowerShell o CMD) en la carpeta raíz del proyecto e instala las librerías:

```powershell
pip install -r requirements.txt
```

## 3. Compilación del Núcleo C++ (Cython)

El sistema utiliza un núcleo en C++ que debe ser compilado.

```powershell
# Ir al directorio cython
cd cython

# Compilar el módulo
python setup.py build_ext --inplace

# Volver a la raíz
cd ..
```

Si la compilación es exitosa, se generará un archivo `.pyd` (por ejemplo `timetable_wrapper.cp310-win_amd64.pyd`).

**Nota:** Si recibes errores de "vcvarsall.bat" o compilador no encontrado, asegúrate de haber instalado correctamente las **Visual Studio Build Tools**.

## 4. Ejecución del Servidor

Inicia el servidor Flask.

```powershell
# Ir al directorio backend
cd backend

# Ejecutar la aplicación
python app.py
```

Verás un mensaje como:
`Running on http://127.0.0.1:5000`

## 5. Uso de la Aplicación

1. Abre tu navegador web.
2. Ve a: [http://localhost:5000](http://localhost:5000)
3. Selecciona el período y genera el horario.
