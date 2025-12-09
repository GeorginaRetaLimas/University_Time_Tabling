# 🎯 NUEVA FUNCIONALIDAD: Visualización de Algoritmo y Grafos

## ¿Qué se agregó?

Se implementó una **demostración completa del algoritmo y estructura de grafos** utilizada en el sistema de horarios.

## Archivos Creados/Modificados

### 1. **`backend/graph_model.py`** (NUEVO)
Módulo completo para representación explícita con grafos usando **NetworkX**:

- **Clase `TimetableGraph`**: Modela el problema usando grafos
  - **Nodos**: Profesores, Cursos, Bloques Horarios, Grupos
  - **Aristas**: Relaciones (puede_impartir, disponibilidad, debe_tomar)
  
- **Grafo de Conflictos**: 
  - Nodos: (Profesor, Bloque)
  - Aristas: conflictos (no pueden ocurrir simultáneamente)
  
- **Grafo de Asignaciones**: Grafo dirigido de la solución

- **Visualización de Matriz 3D**: Muestra estructura `matriz[P][T][G]`

- **Demostración del Algoritmo**: 7 pasos del proceso Greedy

### 2. **`frontend/templates/algorithm.html`** (NUEVO)
Página web interactiva que muestra:

#### 📊 Sección 1: Representación con Grafos
- Estadísticas del grafo (nodos, aristas, densidad)
- Visualización de tipos de nodos con colores
- Tipos de relaciones (aristas)
- **Grafo de Conflictos** explícito
- Ejemplo de vecindario de un profesor

#### 🧊 Sección 2: Matriz 3D
- Representación visual en código
- Dimensiones [Profesores][Bloques][Grupos]
- Memoria utilizada
- Ejemplos de acceso O(1)

#### 🔄 Sección 3: Pasos del Algoritmo Greedy
Demostración paso a paso:
1. **Inicialización**: Crear matriz 3D
2. **Generar Sesiones**: Convertir créditos a sesiones
3. **Ordenar**: Por prioridad (créditos, curso, #sesión)
4. **Asignación Greedy**: 4 estrategias de fallback
5. **Ordenar Bloques (LCV)**: Least Constraining Value
6. **Validar Restricciones**: 5 verificaciones
7. **Asignar en Matriz**: Actualización O(1)

### 3. **`backend/app.py`** (MODIFICADO)
Agregadas 2 nuevas rutas:
- `GET /algorithm`: Página de visualización
- `POST /api/graph/structure`: API que retorna datos del grafo

### 4. **`frontend/templates/layout.html`** (MODIFICADO)
- Agregado menú "Algoritmo" en la navegación

### 5. **`requirements.txt`** (MODIFICADO)
- Agregada dependencia: `networkx`

## 🎨 Características de la Visualización

### Interactiva
- Selector de período académico
- Análisis en tiempo real al presionar "Analizar Estructura"

### Código con Sintaxis
- Matriz 3D con sintaxis highlighting
- Ejemplos de acceso a la matriz

### Estadísticas en Tiempo Real
- Nodos totales, aristas, profesores, cursos, bloques, grupos
- Grafo de conflictos (nodos y aristas)
- Memoria utilizada por la matriz 3D

### Colores por Tipo
- 🔵 Profesores (azul)
- 🟢 Cursos (verde)
- 🟠 Bloques Horarios (naranja)
- 🟣 Grupos (púrpura)

## 📚 USO DE GRAFOS EN EL SISTEMA

### Grafos Explícitos (NUEVO)
Ahora el sistema usa **3 tipos de grafos** explícitos:

1. **Grafo Principal** (`self.graph`)
   - Modela todas las entidades y relaciones
   - Permite consultas de vecindad
   
2. **Grafo de Conflictos** (`self.conflict_graph`)
   - Modela restricciones del problema
   - Útil para detectar incompatibilidades
   
3. **Grafo de Asignaciones** (`self.assignment_graph`)
   - Grafo dirigido con la solución
   - Cada asignación es un nodo central

### Operaciones de Grafo
- `get_professor_neighborhood()`: Vecinos de un profesor
- `build_conflict_graph()`: Construcción de restricciones
- `export_for_visualization()`: Conversión a JSON para frontend

## 🚀 Cómo Usar

1. **Iniciar el servidor Flask**:
   ```bash
   cd backend
   python app.py
   ```

2. **Abrir en navegador**:
   ```
   http://localhost:5000/algorithm
   ```

3. **Interactuar**:
   - Seleccionar período (Mayo-Agosto, Sept-Dic, Ene-Abr)
   - Presionar "Analizar Estructura"
   - Ver grafos, matriz 3D y pasos del algoritmo

## 🎓 Propósito Académico

Esta visualización es perfecta para:
- ✅ Demostrar **uso explícito de grafos** en el proyecto
- ✅ Explicar la **estructura de datos (matriz 3D)**
- ✅ Mostrar el **algoritmo Greedy paso a paso**
- ✅ Validar restricciones usando **grafo de conflictos**
- ✅ Presentaciones y documentación del proyecto

## 💡 Conceptos Demostrados

- **Teoría de Grafos**: Nodos, aristas, vecindarios, conflictos
- **Algoritmos Greedy**: Heurísticas, priorización, fallback
- **Estructuras de Datos**: Matriz 3D con acceso O(1)
- **CSP (Constraint Satisfaction)**: Modelado de restricciones
- **Complejidad Computacional**: Análisis de espacio y tiempo
