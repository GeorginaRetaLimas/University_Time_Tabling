# Generador de Horarios UTP

## 📖 Descripción del Proyecto
Este sistema es una solución integral para la automatización de la generación de horarios académicos en la **Universidad Tecnológica de Puebla (UTP)**. Combina la eficiencia de **C++** para el procesamiento lógico con la flexibilidad de **Python** y **tecnologías web** para la interfaz de usuario.

El objetivo principal es resolver el complejo problema de asignación de recursos (profesores, grupos, materias y tiempos) de manera óptima, respetando estrictas reglas académicas y laborales.

---

## 🎯 ¿Qué Resolvemos?
La planificación manual de horarios enfrenta múltiples desafíos: choques de horarios, profesores asignados fuera de su disponibilidad, y grupos con múltiples profesores para una misma materia.

Nuestro sistema garantiza:
1. **Cero Choques**: Ningún profesor o grupo tiene dos clases al mismo tiempo.
2. **Consistencia Académica**: **Un grupo tiene un SOLO maestro para una sola materia.** No se fragmentan las materias entre múltiples docentes.
3. **Respeto a la Disponibilidad**: Se asignan clases solo en los horarios que los profesores han marcado como disponibles.
4. **Distribución Equilibrada**: Se intenta evitar cargas excesivas en un solo día (máximo 2 horas de la misma materia por día).

---

## 🧠 Algoritmo Utilizado
El núcleo del sistema ("El Cerebro") está construido en **C++ moderno (C++17)** y utiliza un enfoque **Greedy (Voraz) con Heurísticas de Prioridad**.

### Estructura de Datos: La Matriz 3D
Para gestionar las asignaciones de manera eficiente, utilizamos una estructura tridimensional:
```cpp
vector<vector<vector<int>>> matriz_asignaciones;
// Acceso: matriz[id_profesor][id_bloque_tiempo][id_grupo] = id_materia
```
Esto permite verificar conflictos en tiempo constante O(1).

### Estrategia de Resolución
1. **Generación de Sesiones**: Se desglosan las materias en sesiones individuales (bloques de 1 hora) basadas en sus créditos.
2. **Ordenamiento Inteligente**: Las sesiones se ordenan por dificultad de asignación:
   - Primero las materias con más horas semanales (más difíciles de encajar).
   - Luego por orden de sesión para mantener secuencia.
3. **Asignación Voraz (Greedy)**:
   - Para cada sesión, se busca el **primer profesor y bloque horario** que cumpla con TODAS las restricciones.
   - Si falla, se intenta una "Estrategia de Relajación" (Strategy 2) que permite huecos en el horario pero **MANTIENE ESTRICTAMENTE** la regla de un solo profesor por materia.

---

## 📂 Archivos Principales y Estructura

### 1. Núcleo de Procesamiento (`/cpp`)
Aquí reside la lógica pesada.
- **`timetable_solver.h`**: Define las estructuras de datos (`Profesor`, `Curso`, `Grupo`, `Sesion`).
- **`timetable_solver.cpp`**: Implementa la clase `SolucionadorHorarios` y toda la lógica de validación (`verificarConsecutividad`, `verificarDisponibilidad`, etc.).

### 2. Puente de Integración (`/cython`)
Permite que Python hable con C++.
- **`timetable_wrapper.pyx`**: Define la interfaz que Python puede importar. Traduce los objetos de Python a estructuras de C++.
- **`setup.py`**: Script de configuración para compilar el código C++ como un módulo de Python (`.so` o `.pyd`).

### 3. Backend y API (`/backend`)
El servidor web.
- **`app.py`**: Aplicación Flask. Define rutas como `/api/solve` para recibir peticiones del frontend.
- **`data_loader.py`**: Se encarga de leer los CSVs y JSONs y prepararlos para el solver.

### 4. Frontend (`/frontend`)
La interfaz visual.
- **`templates/availability.html`**: La página principal donde se visualizan los horarios generados.
- **`static/`**: Estilos CSS y scripts JS para la interactividad.

---

## ⚙️ Funciones Básicas

### Carga de Datos
El sistema acepta:
- **`courses.csv`**: Catálogo de materias con sus créditos y cuatrimestre.
- **`professors.json`**: Lista de profesores con sus materias capacitadas y disponibilidad horaria.

### Generación
Al pulsar "Generar Horario", el sistema:
1. Compila los datos.
2. Ejecuta el solver C++.
3. Retorna un JSON con todas las asignaciones exitosas.

### Visualización
- Muestra el horario en una cuadrícula semanal.
- Permite filtrar por semestre.
- Muestra detalles (Nombre del Profesor, Materia) en cada celda.

---

## 🚀 Guía de Instalación y Ejecución

### Requisitos
- Python 3.8+
- Compilador C++ (GCC en Linux, MSVC en Windows)

### Pasos
1. **Compilar el Módulo C++**:
   ```bash
   # En Linux
   ./compile.sh
   
   # En Windows
   python cython/setup.py build_ext --build-lib backend
   ```

2. **Ejecutar el Servidor**:
   ```bash
   python backend/app.py
   ```

3. **Usar**:
   Abre tu navegador en `http://localhost:5000`.

---

## 📋 Documentación de Restricciones
El solver valida las siguientes reglas antes de asignar cualquier clase:

| Regla | Descripción |
|-------|-------------|
| **Disponibilidad** | El profesor debe tener el bloque marcado como libre. |
| **No Choques Prof** | El profesor no puede estar dando otra clase a esa hora. |
| **No Choques Grupo** | El grupo no puede tener otra clase a esa hora. |
| **Mismo Profesor** | Si el grupo ya tiene esa materia asignada, DEBE ser el mismo profesor. |
| **Carga Diaria** | Máximo 2 horas de la misma materia por día para evitar fatiga. |

---
**Proyecto de Estructura de Datos - UTP**
