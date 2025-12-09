# Lógica del Algoritmo de Horarios

Este documento detalla el funcionamiento interno del sistema de generación de horarios, incluyendo el uso de matrices tridimensionales, grafos de conflictos y el algoritmo Greedy.

## 1. Estructura de Datos: Matrices Tridimensionales

El núcleo del solver C++ utiliza una representación de matriz 3D para gestionar la disponibilidad y asignaciones.

### Dimensiones
La matriz se define como `Horario[Día][Hora][Recurso]`:
- **Día**: Índice del día de la semana (0-4 para Lunes-Viernes).
- **Hora**: Índice del bloque horario (0-13, representando bloques de 55 min).
- **Recurso**: Puede ser un Profesor, un Grupo o un Aula.

### Uso
- **Matriz de Disponibilidad**: `bool disponible[profesor_id][dia][hora]`. Indica si un profesor puede dar clase en ese momento.
- **Matriz de Asignación**: `int asignacion[grupo_id][dia][hora]`. Almacena el ID del curso asignado a un grupo en un bloque específico.

Esta estructura permite verificaciones de conflictos en tiempo constante O(1), lo cual es crucial para el rendimiento.

## 2. Grafos de Conflictos (NeuroNet)

El sistema modela el problema como un **Problema de Coloreado de Grafos** (Graph Coloring Problem).

### Nodos y Aristas
- **Nodos**: Cada nodo representa una *Sesión* de una materia que debe impartirse (ej. "Matemáticas - Grupo A - Sesión 1").
- **Aristas (Conflictos)**: Existe una arista entre dos nodos si no pueden programarse al mismo tiempo.
    - *Conflicto de Profesor*: El mismo profesor imparte ambas sesiones.
    - *Conflicto de Grupo*: El mismo grupo debe asistir a ambas sesiones.

### Simulación
En la visualización (`/algorithm`), se muestra cómo el algoritmo "colorea" este grafo. Cada "color" representa un bloque horario (Día + Hora). Dos nodos conectados no pueden tener el mismo color.

## 3. Algoritmo Greedy (Voraz)

El solver utiliza una estrategia Greedy con heurísticas para asignar horarios.

### Flujo Principal
1.  **Generación de Sesiones**: Se desglosan todas las materias en sesiones individuales necesarias (basado en créditos).
2.  **Ordenamiento (Heurística)**: Las sesiones se ordenan por dificultad:
    - Primero las materias con más restricciones (profesores con poca disponibilidad).
    - Luego las materias con más horas semanales.
    - *Tie-breaker*: Preferir horas tempranas para distribución equitativa.
3.  **Asignación**:
    - Para cada sesión, se buscan los bloques horarios válidos (intersección de disponibilidad de profesor y grupo).
    - Se elige el primer bloque válido disponible.
    - Se actualizan las matrices de ocupación.

### Pases (Passes)
El algoritmo puede realizar múltiples pases para optimizar:
- **Pass 1 (Hard Constraints)**: Asigna respetando estrictamente todas las restricciones.
- **Pass 2 (Optimization)**: Intenta compactar horarios o mejorar preferencias (soft constraints).

## 4. Generación de Vistas

### Vista de Estudiantes
Se genera directamente recorriendo la matriz de asignación por `grupo_id`.

### Vista de Profesores
Se deriva de la solución global. El frontend agrupa las asignaciones por `professor_id`, permitiendo ver la carga académica individual de cada docente.
