# 🎓 Sistema de Generación de Horarios Universitarios (UTP)

## Manual Técnico - Reporte Detallado

---

## 📋 Tabla de Contenidos

1. [Descripción General del Sistema](#1-descripción-general-del-sistema)
2. [Arquitectura del Sistema](#2-arquitectura-del-sistema)
3. [Estructuras de Datos en C++](#3-estructuras-de-datos-en-c)
4. [Algoritmo Greedy de Asignación](#4-algoritmo-greedy-de-asignación)
5. [Restricciones y Validaciones](#5-restricciones-y-validaciones)
6. [Integración C++ ↔ Python (Cython)](#6-integración-c--python-cython)
7. [Backend Flask y API REST](#7-backend-flask-y-api-rest)
8. [Frontend y Visualización](#8-frontend-y-visualización)
9. [Flujo Completo de Datos](#9-flujo-completo-de-datos)
10. [Compilación y Ejecución](#10-compilación-y-ejecución)

---

## 1. Descripción General del Sistema

El sistema **UTP Scheduler** es una aplicación web para la generación automática de horarios universitarios. Utiliza un **algoritmo Greedy implementado en C++** para resolver el problema de asignación de:

- **Profesores** → **Cursos** → **Bloques Horarios** → **Grupos de Estudiantes**

### Problema a Resolver

El problema de *University Timetabling* es NP-Completo. Este sistema utiliza una aproximación Greedy con heurísticas de priorización para encontrar soluciones factibles en tiempo razonable.

### Stack Tecnológico

| Capa | Tecnología | Propósito |
|------|------------|-----------|
| **Core/Solver** | C++ | Algoritmo de asignación de alta performance |
| **Bridge** | Cython | Wrapper para conectar C++ con Python |
| **Backend** | Flask (Python) | API REST y lógica de negocio |
| **Frontend** | HTML/CSS/JavaScript | Interfaz de usuario |
| **Datos** | JSON/CSV | Almacenamiento de profesores, cursos, horarios |

---

## 2. Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                              FRONTEND (Browser)                              │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │  script.js                                                          │    │
│  │  - Captura selección de período académico                          │    │
│  │  - Envía POST /api/solve                                           │    │
│  │  - Renderiza tabla de horarios con colores por materia             │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼ HTTP POST (JSON)
┌─────────────────────────────────────────────────────────────────────────────┐
│                              BACKEND (Flask)                                 │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │  app.py                                                             │    │
│  │  - Ruta /api/solve recibe período y timeout                        │    │
│  │  - Carga datos: professors.json, timeslots.json, courses.csv       │    │
│  │  - Llama a prepare_data_for_solver()                               │    │
│  │  - Invoca timetable_wrapper.solve_timetable()                      │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │  utils.py                                                           │    │
│  │  - load_professors(), load_courses(), load_timeslots()             │    │
│  │  - get_groups_for_period(): genera grupos por cuatrimestre         │    │
│  │  - prepare_data_for_solver(): filtra datos para el período         │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼ Llamada Python → Cython
┌─────────────────────────────────────────────────────────────────────────────┐
│                           CYTHON WRAPPER                                     │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │  timetable_wrapper.pyx                                              │    │
│  │  - Declara interfaz externa a C++ (cdef extern from)               │    │
│  │  - Función solve_timetable():                                       │    │
│  │    1. Crea instancia SolucionadorHorarios                          │    │
│  │    2. Agrega bloques, profesores, cursos, grupos                   │    │
│  │    3. Llama solver.resolver(timeout)                               │    │
│  │    4. Convierte resultado a diccionarios Python                    │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
                                      │
                                      ▼ Llamada C++
┌─────────────────────────────────────────────────────────────────────────────┐
│                            C++ SOLVER                                        │
│  ┌─────────────────────────────────────────────────────────────────────┐    │
│  │  timetable_solver.cpp / timetable_solver.h                          │    │
│  │  - Clase SolucionadorHorarios                                       │    │
│  │  - Matriz 3D de asignaciones [profesor][bloque][grupo]             │    │
│  │  - Algoritmo Greedy con múltiples estrategias de fallback          │    │
│  │  - Restricciones: conflictos, disponibilidad, consecutividad       │    │
│  └─────────────────────────────────────────────────────────────────────┘    │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## 3. Estructuras de Datos en C++

### 3.1 Archivo: `cpp/timetable_solver.h`

#### Estructura `BloqueHorario`
Representa un slot de tiempo en el horario semanal.

```cpp
struct BloqueHorario {
  int id;           // Identificador único (ej: 1, 101, 201...)
  int dia;          // 1=Lunes, 2=Martes, 3=Miércoles, 4=Jueves, 5=Viernes
  int hora_inicio;  // Hora de inicio (7, 8, 9...)
  int minuto_inicio;// Minuto de inicio (0, 55...)
  int hora_fin;     // Hora de fin
  int minuto_fin;   // Minuto de fin
};
```

**Convención de IDs de Bloques:**
- `1-9`: Lunes (bloques 1-9 del día)
- `101-109`: Martes
- `201-209`: Miércoles
- `301-309`: Jueves
- `401-409`: Viernes

#### Estructura `Profesor`

```cpp
struct Profesor {
  int id;                              // ID único del profesor
  string nombre;                       // Nombre completo
  set<int> horarios_disponibles;       // IDs de bloques donde puede dar clase
  set<string> materias_capacitadas;    // Códigos de cursos que puede impartir
};
```

#### Estructura `Curso`

```cpp
struct Curso {
  int id;               // ID único del curso
  string nombre;        // Nombre del curso
  string codigo;        // Código (ej: "MAT101", "PROG201")
  int creditos;         // Créditos del curso (antes: weekly_hours)
  int cuatrimestre;     // Semestre al que pertenece (1-10)
  bool requiere_profesor; // False para Estadías/cursos especiales
};
```

#### Estructura `Grupo`

```cpp
struct Grupo {
  int id;                    // ID del grupo (ej: 100, 400, 800)
  int cuatrimestre;          // Semestre del grupo
  vector<int> ids_cursos;    // Lista de IDs de cursos que debe tomar
};
```

#### Estructura `SesionClase`

```cpp
struct SesionClase {
  int id;                      // ID único de la sesión
  int id_curso;                // Curso al que pertenece
  int id_grupo;                // Grupo que la toma
  int creditos;                // Créditos del curso padre
  int numero_sesion;           // 1, 2, 3... para múltiples sesiones semanales
  int id_bloque_asignado = -1; // Bloque asignado (-1 = sin asignar)
  int id_profesor_asignado = -1; // Profesor asignado (-1 = sin asignar)
};
```

### 3.2 Clase Principal: `SolucionadorHorarios`

```cpp
class SolucionadorHorarios {
private:
  // Datos del problema
  vector<BloqueHorario> bloques_horarios;
  vector<Profesor> profesores;
  vector<Curso> cursos;
  vector<Grupo> grupos;
  vector<SesionClase> sesiones;

  // ═══════════════════════════════════════════════════════════════
  // MATRIZ 3D DE ASIGNACIONES - Núcleo del algoritmo
  // ═══════════════════════════════════════════════════════════════
  // Dimensiones: [índice_profesor][índice_bloque][índice_grupo]
  // Valor: ID del curso asignado (0 = vacío)
  vector<vector<vector<int>>> matriz_asignaciones;

  // Mapas de conversión ID ↔ Índice (para acceso O(1))
  map<int, int> id_a_indice_prof, id_a_indice_bloque, id_a_indice_grupo;

  // Seguimiento de asignaciones por profesor-grupo
  map<int, map<int, set<int>>> cursos_por_profesor_grupo;

  // Carga horaria por día para distribución equitativa
  map<int, map<int, int>> carga_grupo_dia;

public:
  // Métodos para agregar datos
  void agregarBloqueHorario(...);
  void agregarProfesor(...);
  void agregarCurso(...);
  void agregarGrupo(...);

  // Método principal de resolución
  bool resolver(double tiempo_limite_segundos);

  // Obtener solución
  vector<Asignacion> obtenerSolucion();
};
```

---

## 4. Algoritmo Greedy de Asignación

### 4.1 Flujo General

```
┌─────────────────────────────────────────────────────────────────┐
│                    resolver(timeout)                            │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  1. construirMapasIndices()                                     │
│     - Crea mapas ID → índice para acceso O(1)                  │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  2. inicializarMatriz()                                         │
│     - Crea matriz 3D [profs][bloques][grupos] = 0              │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  3. generarSesiones()                                           │
│     - Para cada grupo → Para cada curso del grupo              │
│     - Calcula num_sesiones = max(1, creditos / 15)             │
│     - Crea SesionClase para cada sesión semanal                │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  4. Ordenar sesiones por prioridad:                             │
│     - Primero: Mayor cantidad de créditos                      │
│     - Segundo: Mismo curso (mantener sesiones juntas)          │
│     - Tercero: Número de sesión ascendente                     │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│  5. Para cada sesión → asignarSesionGreedy()                    │
│     - Intenta con 4 estrategias de fallback                    │
└─────────────────────────────────────────────────────────────────┘
```

### 4.2 Función `generarSesiones()` - Detalle

```cpp
void SolucionadorHorarios::generarSesiones() {
  sesiones.clear();
  int contador_sesiones = 0;

  for (const auto &grupo : grupos) {
    for (int id_curso : grupo.ids_cursos) {
      Curso *curso = obtenerCurso(id_curso);
      if (!curso) continue;

      // ════════════════════════════════════════════════════════
      // CÁLCULO DE SESIONES SEMANALES
      // ════════════════════════════════════════════════════════
      int num_sesiones;
      if (curso->creditos >= 600) {
        // Estadías o cursos especiales (mínimo 2 sesiones)
        num_sesiones = 2;
      } else {
        // Fórmula: cada 15 créditos ≈ 1 hora semanal
        // Ejemplo: 60 créditos → 4 sesiones semanales
        num_sesiones = max(1, curso->creditos / 15);
      }

      // Crear sesiones individuales
      for (int i = 0; i < num_sesiones; i++) {
        SesionClase sesion;
        sesion.id = contador_sesiones++;
        sesion.id_curso = id_curso;
        sesion.id_grupo = grupo.id;
        sesion.creditos = curso->creditos;
        sesion.numero_sesion = i + 1;  // 1, 2, 3...
        sesiones.push_back(sesion);
      }
    }
  }
}
```

### 4.3 Función `asignarSesionGreedy()` - Estrategias

El algoritmo usa **4 estrategias de fallback** para maximizar asignaciones:

```cpp
bool SolucionadorHorarios::asignarSesionGreedy(SesionClase &sesion) {
  // Lambda intentarAsignar(idx_prof, verificar_materia, relajar_consecutividad)

  // ═══════════════════════════════════════════════════════════════
  // ESTRATEGIA 1: ESTRICTA
  // ═══════════════════════════════════════════════════════════════
  // - Verifica que el profesor esté capacitado para la materia
  // - Aplica todas las restricciones de consecutividad
  for (size_t idx_prof = 0; idx_prof < profesores.size(); idx_prof++) {
    if (intentarAsignar(idx_prof, true, false)) return true;
  }

  // ═══════════════════════════════════════════════════════════════
  // ESTRATEGIA 2: RELAJADA (solo para sesiones 2+)
  // ═══════════════════════════════════════════════════════════════
  // - Verifica materia, pero relaja restricción de máximo consecutivas
  if (sesion.numero_sesion > 1) {
    for (size_t idx_prof = 0; idx_prof < profesores.size(); idx_prof++) {
      if (intentarAsignar(idx_prof, true, true)) return true;
    }
  }

  // ═══════════════════════════════════════════════════════════════
  // ESTRATEGIA 3: EMERGENCIA
  // ═══════════════════════════════════════════════════════════════
  // - NO verifica si el profesor puede dar la materia
  // - Cualquier profesor disponible puede cubrir
  for (size_t idx_prof = 0; idx_prof < profesores.size(); idx_prof++) {
    if (intentarAsignar(idx_prof, false, false)) return true;
  }

  // ═══════════════════════════════════════════════════════════════
  // ESTRATEGIA 4: EMERGENCIA TOTAL
  // ═══════════════════════════════════════════════════════════════
  // - NO verifica materia
  // - Relaja todas las restricciones de consecutividad
  for (size_t idx_prof = 0; idx_prof < profesores.size(); idx_prof++) {
    if (intentarAsignar(idx_prof, false, true)) return true;
  }

  return false; // No se pudo asignar
}
```

### 4.4 Ordenamiento Inteligente de Bloques

Dentro de `intentarAsignar()`, los bloques se ordenan con heurísticas:

```cpp
sort(bloques_ordenados.begin(), bloques_ordenados.end(),
     [&](int id_a, int id_b) {
       int carga_a = carga_grupo_dia[sesion.id_grupo][dia_a];
       int carga_b = carga_grupo_dia[sesion.id_grupo][dia_b];
       bool adj_a = esAdyacente(id_a);
       bool adj_b = esAdyacente(id_b);

       // ════════════════════════════════════════════════════════
       // REGLA 1: Días con menos de 2 clases tienen prioridad
       // ════════════════════════════════════════════════════════
       if (carga_a < 2 || carga_b < 2) {
         if (carga_a != carga_b) return carga_a < carga_b;
         if (adj_a != adj_b) return adj_a > adj_b;
       }

       // ════════════════════════════════════════════════════════
       // REGLA 2: Balanceo con tolerancia (diferencia <= 1)
       // ════════════════════════════════════════════════════════
       if (abs(carga_a - carga_b) <= 1) {
         // Preferir adyacencia sobre balance perfecto
         if (adj_a != adj_b) return adj_a > adj_b;
       }

       // ════════════════════════════════════════════════════════
       // REGLA 3: Balanceo estricto si diferencia es grande
       // ════════════════════════════════════════════════════════
       return carga_a < carga_b;
     });
```

---

## 5. Restricciones y Validaciones

### 5.1 Tabla de Restricciones

| Restricción | Función | Descripción |
|-------------|---------|-------------|
| Disponibilidad Profesor | `verificarDisponibilidadProfesor()` | El profesor debe tener el bloque en `horarios_disponibles` |
| Conflicto Profesor | `verificarConflictoProfesor()` | El profesor no puede dar 2 clases en el mismo bloque |
| Conflicto Grupo | `verificarConflictoGrupo()` | El grupo no puede tener 2 clases en el mismo bloque |
| Diversidad Profesor | `verificarDiversidadProfesor()` | Un profesor NO puede dar 2 materias diferentes al mismo grupo |
| Consecutividad | `verificarConsecutividad()` | Todas las sesiones de un curso deben ser del MISMO profesor |
| Máximo Consecutivas | `verificarMaximoConsecutivas()` | Máximo 2 sesiones de la misma materia por día |

### 5.2 Implementación de `verificarConflictoProfesor()`

```cpp
bool SolucionadorHorarios::verificarConflictoProfesor(int idx_prof, int idx_bloque) {
  // Revisar si el profesor ya tiene ALGUNA clase en este bloque
  // (con cualquier grupo)
  for (int idx_grupo = 0; 
       idx_grupo < matriz_asignaciones[idx_prof][idx_bloque].size(); 
       idx_grupo++) {
    if (matriz_asignaciones[idx_prof][idx_bloque][idx_grupo] != 0) {
      return true; // ¡Conflicto!
    }
  }
  return false;
}
```

### 5.3 Implementación de `verificarMaximoConsecutivas()`

```cpp
bool SolucionadorHorarios::verificarMaximoConsecutivas(
    int idx_grupo, int id_curso, int idx_bloque, int idx_prof) {
  
  int dia_actual = bloques_horarios[idx_bloque].dia;
  int sesiones_en_dia = 0;

  // Contar sesiones de este curso en este día
  for (size_t b = 0; b < bloques_horarios.size(); b++) {
    if (bloques_horarios[b].dia == dia_actual &&
        matriz_asignaciones[idx_prof][b][idx_grupo] == id_curso) {
      sesiones_en_dia++;
    }
  }

  // ════════════════════════════════════════════════════════
  // RESTRICCIÓN: Máximo 2 sesiones del mismo curso por día
  // ════════════════════════════════════════════════════════
  return sesiones_en_dia < 2;
}
```

---

## 6. Integración C++ ↔ Python (Cython)

### 6.1 Archivo: `cython/timetable_wrapper.pyx`

Cython actúa como **puente** entre Python y C++:

```python
# distutils: language = c++

from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp cimport bool

# ═══════════════════════════════════════════════════════════════
# DECLARACIÓN DE LA INTERFAZ C++
# ═══════════════════════════════════════════════════════════════
cdef extern from "../cpp/timetable_solver.h":
    cdef cppclass SolucionadorHorarios:
        struct Asignacion:
            int id_grupo
            int id_curso
            int id_profesor
            int id_bloque

        SolucionadorHorarios()
        void agregarBloqueHorario(int id, int dia, int h_inicio, ...)
        void agregarProfesor(int id, string nombre, vector[int] horarios, ...)
        void agregarCurso(int id, string nombre, string codigo, ...)
        void agregarGrupo(int id, int cuatrimestre, vector[int] ids_cursos)
        bool resolver(double tiempo_limite_segundos) except +
        vector[Asignacion] obtenerSolucion()
```

### 6.2 Función `solve_timetable()` - Flujo

```python
def solve_timetable(professors, courses, timeslots, groups, timeout=60.0):
    cdef SolucionadorHorarios solver
    
    # ════════════════════════════════════════════════════════
    # PASO 1: Agregar bloques horarios
    # ════════════════════════════════════════════════════════
    dia_map = {'Lunes': 1, 'Martes': 2, 'Miércoles': 3, 
               'Jueves': 4, 'Viernes': 5}
    for ts in timeslots:
        solver.agregarBloqueHorario(
            ts['id'], dia_map[ts['day']],
            ts['start_hour'], ts['start_minute'],
            ts['end_hour'], ts['end_minute']
        )
    
    # ════════════════════════════════════════════════════════
    # PASO 2: Agregar profesores
    # ════════════════════════════════════════════════════════
    for p in professors:
        codigos = [c.encode('utf-8') for c in p['available_courses']]
        solver.agregarProfesor(
            p['id'], p['name'].encode('utf-8'),
            p['available_timeslots'], codigos
        )
    
    # ════════════════════════════════════════════════════════
    # PASO 3: Agregar cursos
    # ════════════════════════════════════════════════════════
    for c in courses:
        solver.agregarCurso(
            c['id'], c['name'].encode('utf-8'),
            c['code'].encode('utf-8'), c['credits'],
            c['semester'], True
        )
    
    # ════════════════════════════════════════════════════════
    # PASO 4: Agregar grupos
    # ════════════════════════════════════════════════════════
    for g in groups:
        solver.agregarGrupo(g['id'], g['semester'], g['course_ids'])
    
    # ════════════════════════════════════════════════════════
    # PASO 5: Resolver y convertir resultado
    # ════════════════════════════════════════════════════════
    if solver.resolver(timeout):
        solucion = solver.obtenerSolucion()
        
        # Convertir a diccionarios Python
        resultado = []
        for asig in solucion:
            resultado.append({
                'group_id': asig.id_grupo,
                'course_id': asig.id_curso,
                'course_name': course_map[asig.id_curso]['name'],
                'professor_id': asig.id_profesor,
                'professor_name': prof_map[asig.id_profesor],
                'timeslot_id': asig.id_bloque,
                'timeslot_display': timeslot_map[asig.id_bloque]
            })
        return resultado
    return None
```

### 6.3 Compilación del Wrapper

Archivo `cython/setup.py`:

```python
from setuptools import setup
from Cython.Build import cythonize
from setuptools.extension import Extension

extensions = [
    Extension(
        "timetable_wrapper",
        sources=["timetable_wrapper.pyx", "../cpp/timetable_solver.cpp"],
        language="c++",
        extra_compile_args=["-std=c++17"]
    )
]

setup(ext_modules=cythonize(extensions))
```

**Comando de compilación:**
```bash
cd cython
python setup.py build_ext --inplace
```

---

## 7. Backend Flask y API REST

### 7.1 Archivo: `backend/app.py`

#### Endpoint Principal: `/api/solve`

```python
@app.route('/api/solve', methods=['POST'])
def solve():
    data = request.json
    period = data.get('period', 'sept-dec')  # Período académico
    timeout = float(data.get('timeout', 60))
    
    # ════════════════════════════════════════════════════════
    # PASO 1: Cargar datos desde archivos
    # ════════════════════════════════════════════════════════
    professors = load_professors('data/professors.json')
    timeslots = load_timeslots('data/timeslots.json')
    courses = load_courses('data/courses.csv')
    
    # ════════════════════════════════════════════════════════
    # PASO 2: Preparar datos para el solver
    # ════════════════════════════════════════════════════════
    profs, crs, slots, groups = prepare_data_for_solver(
        professors, courses, timeslots, period
    )
    
    # ════════════════════════════════════════════════════════
    # PASO 3: Invocar solver C++ vía Cython
    # ════════════════════════════════════════════════════════
    solution = timetable_wrapper.solve_timetable(
        profs, crs, slots, groups, timeout
    )
    
    if solution:
        return jsonify({'status': 'success', 'solution': solution})
    else:
        return jsonify({'status': 'error', 'message': 'No se encontró solución'})
```

### 7.2 Archivo: `backend/utils.py` - Generación de Grupos

```python
def get_groups_for_period(period_key):
    """
    Genera grupos para el período académico.
    Cada cuatrimestre tiene UN grupo que toma todas las materias.
    """
    if period_key not in PERIODOS:
        return []
    
    groups = []
    period_data = PERIODOS[period_key]
    
    for semester, course_ids in period_data['cuatrimestres'].items():
        # ID del grupo = cuatrimestre * 100
        # Ejemplo: cuatri 8 → grupo 800
        group_id = semester * 100
        
        groups.append({
            'id': group_id,
            'semester': semester,
            'course_ids': course_ids  # Todas las materias del cuatrimestre
        })
    
    return groups
```

### 7.3 Configuración de Períodos: `data/periods.py`

```python
PERIODOS = {
    "sept-dec": {  # Septiembre - Diciembre
        "semanas": 15,
        "cuatrimestres": {
            1: [101, 102, 103, 104, 105, 106, 107],   # Cuatri 1
            4: [401, 402, 403, 404, 405, 406, 407],   # Cuatri 4
            7: [701, 702, 703, 704, 705, 706, 707],   # Cuatri 7
            10: [1001]  # Estancias
        }
    },
    
    "jan-apr": {  # Enero - Abril
        "semanas": 15,
        "cuatrimestres": {
            2: [201, 202, 203, 204, 205, 206, 207],
            5: [501, 502, 503, 504, 505, 506, 507],
            8: [801, 802, 803, 804, 805, 806, 807]
        }
    },
    
    "may-aug": {  # Mayo - Agosto
        "semanas": 15,
        "cuatrimestres": {
            3: [301, 302, 303, 304, 305, 306, 307],
            6: [601],  # Estancias
            9: [901, 902, 903, 904, 905, 906, 907]
        }
    }
}
```

---

## 8. Frontend y Visualización

### 8.1 Archivo: `frontend/static/js/script.js`

#### Flujo de Generación de Horario

```javascript
generateBtn.addEventListener('click', async () => {
    loadingModal.style.display = 'flex';
    
    const period = periodSelect.value;
    const timeout = 60;
    
    // ════════════════════════════════════════════════════════
    // LLAMADA AL API
    // ════════════════════════════════════════════════════════
    const response = await fetch('/api/solve', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ period: period, timeout: timeout })
    });
    
    const data = await response.json();
    
    if (data.status === 'success') {
        renderTimetable(data.solution);
        resultsSection.classList.remove('results-hidden');
    }
});
```

#### Renderizado de Horarios

```javascript
function renderTimetable(solution) {
    // Paleta de 7 colores por materia
    const colorPalette = [
        { bg: '#FFE5E5', border: '#FF6B6B', text: '#C92A2A' },  // Rojo
        { bg: '#E3F2FD', border: '#42A5F5', text: '#1565C0' },  // Azul
        { bg: '#E8F5E9', border: '#66BB6A', text: '#2E7D32' },  // Verde
        // ... más colores
    ];

    // ════════════════════════════════════════════════════════
    // AGRUPAR POR GRUPO ID
    // ════════════════════════════════════════════════════════
    const groups = {};
    solution.forEach(item => {
        if (!groups[item.group_id]) groups[item.group_id] = [];
        groups[item.group_id].push(item);
    });

    // ════════════════════════════════════════════════════════
    // GENERAR TABLA HTML POR CADA GRUPO
    // ════════════════════════════════════════════════════════
    Object.keys(groups).forEach(groupId => {
        const items = groups[groupId];
        
        // Asignar colores únicos por materia
        const uniqueCourses = [...new Set(items.map(i => i.course_id))];
        const courseColorMap = {};
        uniqueCourses.forEach((courseId, index) => {
            courseColorMap[courseId] = colorPalette[index % 7];
        });

        // Generar tabla 5x9 (días x bloques horarios)
        // ...
    });
}
```

### 8.2 Estructura del JSON de Respuesta

```json
{
  "status": "success",
  "solution": [
    {
      "group_id": 100,
      "course_id": 101,
      "course_name": "Matemáticas I",
      "course_code": "MAT101",
      "professor_id": 5,
      "professor_name": "Dr. García",
      "timeslot_id": 1,
      "timeslot_display": "Lunes 7:00-7:55"
    },
    // ... más asignaciones
  ]
}
```

---

## 9. Flujo Completo de Datos

```
┌────────────────────────────────────────────────────────────────────────────┐
│ 1. USUARIO selecciona período "sept-dec" y presiona "Generar Horario"     │
└────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ 2. JAVASCRIPT envía POST /api/solve { period: "sept-dec", timeout: 60 }   │
└────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ 3. FLASK carga archivos:                                                   │
│    - professors.json → Lista de profesores con disponibilidad             │
│    - timeslots.json → 45 bloques horarios (9 bloques × 5 días)            │
│    - courses.csv → Catálogo de cursos con créditos                        │
└────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ 4. UTILS genera grupos dinámicamente:                                      │
│    sept-dec → Cuatrimestres 1, 4, 7, 10                                   │
│    Grupo 100 (cuatri 1) con cursos [101, 102, 103, 104, 105, 106, 107]   │
│    Grupo 400 (cuatri 4) con cursos [401, 402, ...]                        │
│    etc.                                                                    │
└────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ 5. CYTHON recibe datos Python y los convierte a tipos C++:                │
│    - vector<int> horarios_disp                                            │
│    - vector<string> codigos_cursos                                        │
│    - Llama solver.agregarProfesor(), agregarCurso(), etc.                 │
└────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ 6. C++ SOLVER ejecuta algoritmo Greedy:                                    │
│    a) Genera sesiones: 7 cursos × ~4 sesiones = ~28 sesiones por grupo   │
│    b) Ordena por créditos (materias difíciles primero)                    │
│    c) Para cada sesión, intenta asignar con 4 estrategias                 │
│    d) Actualiza matriz 3D y mapas de seguimiento                          │
└────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ 7. C++ devuelve vector<Asignacion> con todas las asignaciones válidas     │
└────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ 8. CYTHON convierte resultado a lista de diccionarios Python              │
│    - Agrega nombres de profesores, cursos, bloques                        │
└────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ 9. FLASK retorna JSON con status y solution                               │
└────────────────────────────────────────────────────────────────────────────┘
                                    │
                                    ▼
┌────────────────────────────────────────────────────────────────────────────┐
│ 10. JAVASCRIPT renderiza tablas de horario por grupo:                      │
│     - Una tabla por grupo (Grupo 100, Grupo 400, etc.)                    │
│     - Colores únicos por materia                                          │
│     - Muestra nombre del curso y profesor en cada celda                   │
└────────────────────────────────────────────────────────────────────────────┘
```

---

## 10. Compilación y Ejecución

### 10.1 Requisitos

```bash
# Python 3.8+
pip install flask cython pandas

# Compilador C++ (g++ o MSVC)
# En Windows: Visual Studio Build Tools
```

### 10.2 Compilar Cython Wrapper

```bash
cd cython
python setup.py build_ext --inplace

# En Windows PowerShell:
# python setup.py build_ext --inplace
```

### 10.3 Ejecutar Servidor

```bash
cd backend
python app.py

# Servidor disponible en http://localhost:5000
```

### 10.4 Estructura de Archivos Requeridos

```
University_Time_Tabling/
├── cpp/
│   ├── timetable_solver.h      # Definiciones de estructuras y clase
│   └── timetable_solver.cpp    # Implementación del algoritmo
├── cython/
│   ├── setup.py                # Configuración de compilación
│   ├── timetable_wrapper.pyx   # Wrapper Cython
│   └── timetable_wrapper.*.pyd # Binario compilado (auto-generado)
├── backend/
│   ├── app.py                  # Servidor Flask
│   └── utils.py                # Utilidades de carga de datos
├── frontend/
│   ├── templates/
│   │   └── index.html          # Página principal
│   └── static/
│       ├── css/style.css       # Estilos
│       └── js/script.js        # Lógica del frontend
├── data/
│   ├── professors.json         # Datos de profesores
│   ├── courses.csv             # Catálogo de cursos
│   ├── timeslots.json          # Bloques horarios
│   └── periods.py              # Configuración de períodos
└── README.md                   # Este documento
```

---

## Anexo A: Complejidad Computacional

| Operación | Complejidad | Notas |
|-----------|-------------|-------|
| Generación de sesiones | O(G × C) | G = grupos, C = cursos por grupo |
| Ordenamiento de sesiones | O(S log S) | S = total de sesiones |
| Asignación por sesión | O(P × B) | P = profesores, B = bloques |
| **Total (peor caso)** | O(S × P × B) | ~28 sesiones × 20 profs × 45 bloques |

Con valores típicos (28 sesiones, 20 profesores, 45 bloques), el algoritmo resuelve en **< 1 segundo**.

---

## Anexo B: Limitaciones Conocidas

1. **Algoritmo Greedy**: No garantiza solución óptima global
2. **Sin aulas**: El sistema no gestiona asignación de salones
3. **Un grupo por cuatrimestre**: No soporta múltiples grupos del mismo semestre
4. **Sin preferencias de horario**: No considera preferencias de profesores más allá de disponibilidad

---

**Autor**: Sistema UTP Scheduler  
**Versión**: 2.0  
**Última actualización**: Diciembre 2024
