# Sistema de Generación de Horarios Universitarios UTP

Sistema optimizado de generación de horarios con **algoritmo greedy** y **matrices 3D** en C++.

## 🚀 Características

- **Motor C++ de Alto Rendimiento**: Algoritmo greedy con matrices tridimensionales
- **Restricciones Inteligentes**:
  - ✅ Disponibilidad de profesores
  - ✅ Unicidad (profesor y grupo)
  - ✅ Diversidad (un profesor no da 2 materias al mismo grupo)
  - ✅ Sesiones consecutivas (máximo 2 seguidas)
  - ✅ Priorización por carga horaria (créditos)
- **Interfaz Web**: Frontend HTML/CSS/JS con visualización de horarios
- **API REST**: Backend Flask con endpoints JSON

---

## 📋 Requisitos Previos

### General
- Python 3.8+ (recomendado 3.11)
- Navegador Web Moderno
- Compilador C++ compatible con C++17

### Windows
- **Visual Studio Build Tools** (con carga de trabajo "Desarrollo para el escritorio con C++")
- O **MinGW-w64**

### Linux (Ubuntu/Debian)
```bash
sudo apt install build-essential python3-dev
```

---

## 🔧 Instalación y Ejecución

### 🪟 Windows

1. **Instalar dependencias de Python**:
   ```cmd
   pip install -r requirements.txt
   ```

2. **Compilar el módulo C++**:
   ```cmd
   python cython/setup.py build_ext --build-lib backend
   ```
   > **Nota**: Si usas MinGW, agrega `--compiler=mingw32` al final del comando.

3. **Ejecutar la aplicación**:
   ```cmd
   cd backend
   python app.py
   ```

4. **Acceder**:
   Abre [http://localhost:5000](http://localhost:5000) en tu navegador.

### 🐧 Linux

1. **Instalar dependencias**:
   ```bash
   pip install -r requirements.txt
   ```

2. **Compilar el módulo C++**:
   ```bash
   chmod +x compile.sh
   ./compile.sh
   ```

3. **Ejecutar**:
   ```bash
   cd backend
   python3 app.py
   ```

4. **Acceder**:
   Abre [http://localhost:5000](http://localhost:5000) en tu navegador.

---

## 📁 Estructura del Proyecto

```
University_Time_Tabling/
├── cpp/                          # Motor C++ (algoritmo greedy)
│   ├── timetable_solver.h        # Definiciones de clases
│   └── timetable_solver.cpp      # Implementación del solver
├── cython/                       # Interfaz Python-C++
│   ├── timetable_wrapper.pyx     # Wrapper de Cython
│   └── setup.py                  # Script de compilación
├── backend/                      # Servidor Flask
│   ├── app.py                    # API REST
│   └── utils.py                  # Utilidades (carga de datos)
├── frontend/                     # Interfaz web
│   ├── templates/                # HTML
│   └── static/                   # CSS, JS, imágenes
└── data/                         # Datos del problema
    ├── courses_full.csv          # Catálogo de materias
    ├── professors.json           # Profesores y disponibilidad
    ├── timeslots.json            # Bloques horarios
    └── periods.py                # Configuración de períodos académicos
```

---

## 🧮 Algoritmo Greedy con Matrices 3D

### Estructura de Datos Principal

```cpp
// Matriz 3D: [profesores][bloques_horarios][grupos] = id_curso
vector<vector<vector<int>>> matriz_asignaciones;
```

### Flujo del Algoritmo

1. **Generación de Sesiones**: Calcular sesiones desde créditos (`creditos / 15`)
2. **Ordenamiento**: Priorizar por créditos (descendente)
3. **Asignación Greedy**: Para cada sesión:
   - Buscar profesor capacitado
   - Buscar bloque disponible
   - Validar todas las restricciones
   - Asignar en matriz 3D
4. **Resultado**: Lista de asignaciones válidas

### Restricciones Implementadas

| Restricción | Descripción |
|-------------|-------------|
| **Disponibilidad** | Solo asignar en horarios disponibles del profesor |
| **Unicidad Profesor** | Un profesor no puede dar 2 clases simultáneas |
| **Unicidad Grupo** | Un grupo no puede tener 2 materias simultáneas |
| **Diversidad** | Un profesor NO puede dar 2 materias diferentes al mismo grupo |
| **Consecutividad** | Sesiones de una materia deben ser consecutivas |
| **Máximo 2 Consecutivas** | No más de 2 sesiones seguidas de la misma materia |

---

## 📊 Formato de Datos

### courses_full.csv
```csv
id,name,code,credits
101,INGLÉS I,ING1,75
102,DESARROLLO HUMANO Y VALORES,DHV,60
...
```

### professors.json
```json
[
  {
    "id": 1,
    "name": "Myriam Ornelas (ITI)",
    "available_timeslots": [2, 3, 4, 5, ...],
    "available_courses": ["LEAD", "PE", "CI", ...]
  },
  ...
]
```

### timeslots.json
```json
[
  {
    "id": 1,
    "day": "Lunes",
    "start_hour": 7,
    "start_minute": 0,
    "end_hour": 7,
    "end_minute": 55,
    "display": "Lunes 07:00-07:55"
  },
  ...
]
```

---

## 🔍 Solución de Problemas

### Error de Compilación

**Windows**: Asegúrate de tener instalado Visual Studio Build Tools con componente C++.

**Linux**: Verifica que tengas `python3-dev`:
```bash
sudo apt install python3-dev
```

### Error "Python.h not found"

Instala los archivos de desarrollo de Python:
- **Windows**: Reinstala Python marcando "Include development headers"
- **Linux**: `sudo apt install python3-dev`

### Error en tiempo de ejecución

Verifica que el módulo `.pyd` o `.so` esté en la carpeta `backend/`:
```bash
ls backend/*.pyd   # Windows
ls backend/*.so    # Linux
```

---

## 📝 Uso de la API

### Endpoint Principal: `/api/solve`

**Request**:
```json
{
  "period": "sept-dec",
  "timeout": 60
}
```

**Response**:
```json
{
  "status": "success",
  "assignments": [
    {
      "group_id": 101,
      "course_id": 101,
      "course_name": "INGLÉS I",
      "course_code": "ING1",
      "professor_id": 1,
      "professor_name": "Myriam Ornelas (ITI)",
      "timeslot_id": 2,
      "timeslot_display": "Lunes 07:55-08:50"
    },
    ...
  ]
}
```

---

## 🎯 Períodos Académicos

El sistema soporta 3 períodos por año:

- **sept-dec**: Septiembre-Diciembre (Cuatrimestres 1, 4, 7, 10)
- **jan-apr**: Enero-Abril (Cuatrimestres 2, 5, 8)
- **may-aug**: Mayo-Agosto (Cuatrimestres 3, 6, 9)

---

## 👨‍💻 Desarrollo

El código está completamente en **español** para facilitar el mantenimiento:

- Variables en español (`id_profesor`, `id_grupo`, `creditos`)
- Comentarios en español
- Mensajes de consola en español con emojis

---

## 📄 Licencia

Este proyecto es para uso académico de la Universidad Tecnológica de Puebla.

---

## 🤝 Contribución

Para contribuir al proyecto:

1. Haz fork del repositorio
2. Crea una rama para tu feature (`git checkout -b feature/nueva-restriccion`)
3. Commit tus cambios (`git commit -am 'Agregar nueva restricción'`)
4. Push a la rama (`git push origin feature/nueva-restriccion`)
5. Crea un Pull Request

---

**Desarrollado con ❤️ para la Universidad Tecnológica de Puebla**
