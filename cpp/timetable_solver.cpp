#include "timetable_solver.h"
#include <algorithm>
#include <chrono>
#include <iostream>

using namespace std;

// ============================================
// Constructor
// ============================================

SolucionadorHorarios::SolucionadorHorarios() {}

// ============================================
// Métodos para Agregar Datos
// ============================================

void SolucionadorHorarios::agregarBloqueHorario(int id, int dia, int h_inicio,
                                                 int m_inicio, int h_fin,
                                                 int m_fin) {
  bloques_horarios.push_back({id, dia, h_inicio, m_inicio, h_fin, m_fin});
}

void SolucionadorHorarios::agregarProfesor(int id, string nombre,
                                            const vector<int> &horarios_disp,
                                            const vector<string> &codigos_cursos) {
  set<int> horarios(horarios_disp.begin(), horarios_disp.end());
  set<string> materias(codigos_cursos.begin(), codigos_cursos.end());
  profesores.push_back({id, nombre, horarios, materias});
}

void SolucionadorHorarios::agregarCurso(int id, string nombre, string codigo,
                                         int creditos, int cuatrimestre,
                                         bool req_profesor) {
  cursos.push_back({id, nombre, codigo, creditos, cuatrimestre, req_profesor});
}

void SolucionadorHorarios::agregarGrupo(int id, int cuatrimestre,
                                         const vector<int> &ids_cursos) {
  grupos.push_back({id, cuatrimestre, ids_cursos});
}

// ============================================
// Métodos Helper
// ============================================

Profesor *SolucionadorHorarios::obtenerProfesor(int id) {
  for (auto &p : profesores) {
    if (p.id == id)
      return &p;
  }
  return nullptr;
}

Curso *SolucionadorHorarios::obtenerCurso(int id) {
  for (auto &c : cursos) {
    if (c.id == id)
      return &c;
  }
  return nullptr;
}

// ============================================
// Construcción de Índices y Matriz
// ============================================

void SolucionadorHorarios::construirMapasIndices() {
  // Profesores
  for (size_t i = 0; i < profesores.size(); i++) {
    id_a_indice_prof[profesores[i].id] = i;
    indice_a_id_prof[i] = profesores[i].id;
  }

  // Bloques horarios
  for (size_t i = 0; i < bloques_horarios.size(); i++) {
    id_a_indice_bloque[bloques_horarios[i].id] = i;
    indice_a_id_bloque[i] = bloques_horarios[i].id;
  }

  // Grupos
  for (size_t i = 0; i < grupos.size(); i++) {
    id_a_indice_grupo[grupos[i].id] = i;
    indice_a_id_grupo[i] = grupos[i].id;
  }
}

void SolucionadorHorarios::inicializarMatriz() {
  int num_profs = profesores.size();
  int num_bloques = bloques_horarios.size();
  int num_grupos = grupos.size();

  // Inicializar matriz 3D con ceros
  matriz_asignaciones.assign(
      num_profs, vector<vector<int>>(num_bloques, vector<int>(num_grupos, 0)));
}

// ============================================
// Generación de Sesiones desde Créditos
// ============================================

void SolucionadorHorarios::generarSesiones() {
  sesiones.clear();
  int contador_sesiones = 0;

  for (const auto &grupo : grupos) {
    for (int id_curso : grupo.ids_cursos) {
      Curso *curso = obtenerCurso(id_curso);
      if (!curso)
        continue;

      // Calcular número de sesiones basado en créditos
      int num_sesiones;
      if (curso->creditos >= 600) {
        // Estadías o cursos especiales
        num_sesiones = 2;
      } else {
        // Fórmula: cada 15 créditos ≈ 1 hora semanal
        num_sesiones = max(1, curso->creditos / 15);
      }

      // Crear sesiones individuales
      for (int i = 0; i < num_sesiones; i++) {
        SesionClase sesion;
        sesion.id = contador_sesiones++;
        sesion.id_curso = id_curso;
        sesion.id_grupo = grupo.id;
        sesion.creditos = curso->creditos;
        sesion.numero_sesion = i + 1;
        sesion.id_bloque_asignado = -1;
        sesion.id_profesor_asignado = -1;
        sesiones.push_back(sesion);
      }
    }
  }

  cout << "📚 Generadas " << sesiones.size() << " sesiones de clase." << endl;
}

// ============================================
// Métodos de Validación de Restricciones
// ============================================

int SolucionadorHorarios::obtenerSiguienteBloque(int idx_bloque_actual) {
  if (idx_bloque_actual < 0 || idx_bloque_actual >= bloques_horarios.size())
    return -1;

  BloqueHorario &actual = bloques_horarios[idx_bloque_actual];

  // Buscar el siguiente bloque consecutivo
  for (size_t i = 0; i < bloques_horarios.size(); i++) {
    BloqueHorario &candidato = bloques_horarios[i];

    // Mismo día, hora de inicio = hora de fin del actual
    if (candidato.dia == actual.dia && 
        candidato.hora_inicio == actual.hora_fin &&
        candidato.minuto_inicio == actual.minuto_fin) {
      return i;
    }
  }

  return -1; // No hay siguiente bloque consecutivo
}

bool SolucionadorHorarios::sonBloquesConsecutivos(int idx1, int idx2) {
  return obtenerSiguienteBloque(idx1) == idx2;
}

bool SolucionadorHorarios::verificarDisponibilidadProfesor(int idx_prof,
                                                            int idx_bloque) {
  int id_bloque = indice_a_id_bloque[idx_bloque];
  const Profesor &prof = profesores[idx_prof];
  return prof.horarios_disponibles.count(id_bloque) > 0;
}

bool SolucionadorHorarios::verificarConflictoProfesor(int idx_prof,
                                                       int idx_bloque) {
  // Verificar si el profesor ya tiene alguna clase asignada en ese bloque
  for (int idx_grupo = 0; idx_grupo < matriz_asignaciones[idx_prof][idx_bloque].size();
       idx_grupo++) {
    if (matriz_asignaciones[idx_prof][idx_bloque][idx_grupo] != 0) {
      return true; // Conflicto: ya tiene clase
    }
  }
  return false;
}

bool SolucionadorHorarios::verificarConflictoGrupo(int idx_grupo,
                                                    int idx_bloque) {
  // Verificar si el grupo ya tiene alguna clase asignada en ese bloque
  for (int idx_prof = 0; idx_prof < matriz_asignaciones.size(); idx_prof++) {
    if (matriz_asignaciones[idx_prof][idx_bloque][idx_grupo] != 0) {
      return true; // Conflicto: el grupo ya tiene clase
    }
  }
  return false;
}

bool SolucionadorHorarios::verificarDiversidadProfesor(int id_prof,
                                                        int id_grupo,
                                                        int id_curso) {
  // Restricción: Un profesor NO puede dar 2 materias diferentes al mismo grupo
  if (cursos_por_profesor_grupo[id_grupo][id_prof].size() > 0) {
    // El profesor ya tiene asignaciones con este grupo
    for (int curso_previo : cursos_por_profesor_grupo[id_grupo][id_prof]) {
      if (curso_previo != id_curso) {
        // El profesor ya da una materia DIFERENTE a este grupo
        return false;
      }
    }
  }
  return true;
}

bool SolucionadorHorarios::verificarConsecutividad(const SesionClase &sesion,
                                                    int idx_prof,
                                                    int idx_bloque) {
  // NUEVA LÓGICA: Las sesiones NO tienen que ser todas consecutivas
  // Solo deben ser impartidas por el MISMO profesor
  // Y se distribuyen respet and el máximo de 2 consecutivas
  
  // Si es la primera sesión, OK
  if (sesion.numero_sesion == 1) {
    return true;
  }
  
  int idx_grupo = id_a_indice_grupo[sesion.id_grupo];
  
  // Buscar si ya hay sesiones previas de esta materia asignadas
  bool hay_sesiones_previas = false;
  int prof_anterior = -1;
  
  for (size_t p = 0; p < matriz_asignaciones.size(); p++) {
    for (size_t b = 0; b < matriz_asignaciones[p].size(); b++) {
      if (matriz_asignaciones[p][b][idx_grupo] == sesion.id_curso) {
        hay_sesiones_previas = true;
        prof_anterior = p;
        break;
      }
    }
    if (hay_sesiones_previas) break;
  }
  
  // Si hay sesiones previas, debe ser el MISMO profesor
  if (hay_sesiones_previas && prof_anterior != idx_prof) {
    return false; // Diferente profesor, no permitido
  }
  
  return true; // OK: mismo profesor o no hay sesiones previas
}

bool SolucionadorHorarios::verificarMaximoConsecutivas(int idx_grupo,
                                                        int id_curso,
                                                        int idx_bloque,
                                                        int idx_prof) {
  // NUEVA RESTRICCIÓN:
  // 1. Máximo 2 sesiones de la misma materia en el mismo DÍA
  // 2. Si hay 2 sesiones en el mismo día, deben ser consecutivas
  
  BloqueHorario& bloque_actual = bloques_horarios[idx_bloque];
  int dia_actual = bloque_actual.dia;
  
  // Contar cuántas sesiones de este curso ya hay en este día
  int sesiones_en_dia = 0;
  int idx_primera_sesion_dia = -1;
  int idx_ultima_sesion_dia = -1;
  
  for (size_t b = 0; b < bloques_horarios.size(); b++) {
    if (bloques_horarios[b].dia == dia_actual &&
        matriz_asignaciones[idx_prof][b][idx_grupo] == id_curso) {
      sesiones_en_dia++;
      if (idx_primera_sesion_dia == -1) {
        idx_primera_sesion_dia = b;
      }
      idx_ultima_sesion_dia = b;
    }
  }
  
  // Si ya hay 2 sesiones en este día, NO permitir más
  if (sesiones_en_dia >= 2) {
    return false;
  }
  
  // Si ya hay 1 sesión en este día, la nueva debe ser consecutiva
  if (sesiones_en_dia == 1) {
    // Verificar que el bloque actual sea consecutivo a la sesión existente
    bool es_consecutivo = 
      (obtenerSiguienteBloque(idx_ultima_sesion_dia) == idx_bloque) ||
      (obtenerSiguienteBloque(idx_bloque) == idx_primera_sesion_dia);
    
    if (!es_consecutivo) {
      return false; // No consecutivo, no permitido
    }
  }
  
  // Validación adicional: contar consecutivas en este punto
  int consecutivas = 1;
  
  // Contar hacia atrás
  int idx_anterior = idx_bloque;
  while (consecutivas < 3) {
    int idx_prev = -1;
    for (size_t i = 0; i < bloques_horarios.size(); i++) {
      if (obtenerSiguienteBloque(i) == idx_anterior) {
        idx_prev = i;
        break;
      }
    }
    
    if (idx_prev == -1) break;
    
    if (matriz_asignaciones[idx_prof][idx_prev][idx_grupo] == id_curso) {
      consecutivas++;
      idx_anterior = idx_prev;
    } else {
      break;
    }
  }
  
  // Contar hacia adelante
  int idx_siguiente = obtenerSiguienteBloque(idx_bloque);
  while (idx_siguiente != -1 && consecutivas < 3) {
    if (matriz_asignaciones[idx_prof][idx_siguiente][idx_grupo] == id_curso) {
      consecutivas++;
      idx_siguiente = obtenerSiguienteBloque(idx_siguiente);
    } else {
      break;
    }
  }
  
  return consecutivas <= 2;
}

// ============================================
// Asignación Greedy de Sesión
// ============================================

bool SolucionadorHorarios::asignarSesionGreedy(SesionClase &sesion) {
  Curso *curso = obtenerCurso(sesion.id_curso);
  if (!curso)
    return false;

  int idx_grupo = id_a_indice_grupo[sesion.id_grupo];

  // Buscar profesores capacitados para esta materia
  for (size_t idx_prof = 0; idx_prof < profesores.size(); idx_prof++) {
    const Profesor &prof = profesores[idx_prof];

    // ¿Puede dar esta materia?
    if (prof.materias_capacitadas.find(curso->codigo) ==
        prof.materias_capacitadas.end()) {
      continue;
    }

    // Restricción: Diversidad de profesor por grupo
    if (!verificarDiversidadProfesor(prof.id, sesion.id_grupo, sesion.id_curso)) {
      continue;
    }

    // Probar cada bloque horario disponible del profesor
    for (int id_bloque : prof.horarios_disponibles) {
      if (id_a_indice_bloque.find(id_bloque) == id_a_indice_bloque.end()) {
        continue;
      }

      int idx_bloque = id_a_indice_bloque[id_bloque];

      // Restricción 1: Disponibilidad
      if (!verificarDisponibilidadProfesor(idx_prof, idx_bloque)) {
        continue;
      }

      // Restricción 2: Unicidad de profesor (no dos clases simultáneas)
      if (verificarConflictoProfesor(idx_prof, idx_bloque)) {
        continue;
      }

      // Restricción 3: Unicidad de grupo (no dos materias simultáneas)
      if (verificarConflictoGrupo(idx_grupo, idx_bloque)) {
        continue;
      }

      // Restricción 4: Consecutividad de sesiones
      if (!verificarConsecutividad(sesion, idx_prof, idx_bloque)) {
        continue;
      }

      // Restricción 5: Máximo 2 sesiones consecutivas
      if (!verificarMaximoConsecutivas(idx_grupo, sesion.id_curso, idx_bloque, idx_prof)) {
        continue;
      }

      // ¡Asignación válida!
      matriz_asignaciones[idx_prof][idx_bloque][idx_grupo] = sesion.id_curso;

      // Registrar en seguimiento de diversidad
      cursos_por_profesor_grupo[sesion.id_grupo][prof.id].insert(sesion.id_curso);

      // Actualizar sesión
      sesion.id_bloque_asignado = id_bloque;
      sesion.id_profesor_asignado = prof.id;

      return true;
    }
  }

  return false; // No se pudo asignar
}

// ============================================
// Solver Greedy Principal
// ============================================

bool SolucionadorHorarios::resolverGreedy() {
  cout << "🔧 Construyendo mapas de índices..." << endl;
  construirMapasIndices();

  cout << "🧱 Inicializando matriz 3D..." << endl;
  inicializarMatriz();

  cout << "📝 Generando sesiones desde créditos..." << endl;
  generarSesiones();

  // Ordenar sesiones por prioridad
  cout << "📊 Ordenando sesiones (créditos desc, sesión asc)..." << endl;
  sort(sesiones.begin(), sesiones.end(),
       [](const SesionClase &a, const SesionClase &b) {
         // Prioridad 1: Más créditos primero
         if (a.creditos != b.creditos)
           return a.creditos > b.creditos;

         // Prioridad 2: Mismo curso (para mantener sesiones juntas)
         if (a.id_curso != b.id_curso)
           return a.id_curso < b.id_curso;

         // Prioridad 3: Número de sesión ascendente (sesión 1, luego 2, etc.)
         return a.numero_sesion < b.numero_sesion;
       });

  // Asignar sesiones de forma greedy
  cout << "🚀 Iniciando asignación greedy..." << endl;
  int asignadas = 0;
  int total = sesiones.size();

  for (auto &sesion : sesiones) {
    if (asignarSesionGreedy(sesion)) {
      asignadas++;
    } else {
      Curso *curso = obtenerCurso(sesion.id_curso);
      cout << "⚠️  No asignada: " << (curso ? curso->codigo : "???") 
           << " (Sesión " << sesion.numero_sesion << ") → Grupo "
           << sesion.id_grupo << endl;
    }
  }

  cout << "\n✅ Asignadas: " << asignadas << "/" << total << endl;
  double tasa = (total > 0) ? (100.0 * asignadas / total) : 0.0;
  cout << "📊 Tasa de éxito: " << tasa << "%" << endl;

  return asignadas > 0;
}

// ============================================
// Método Principal: Resolver
// ============================================

bool SolucionadorHorarios::resolver(double tiempo_limite_segundos) {
  try {
    tiempo_inicio = chrono::steady_clock::now();
    limite_tiempo = tiempo_limite_segundos;

    cout << "\n========================================" << endl;
    cout << "🎓 SOLUCIONADOR DE HORARIOS UTP" << endl;
    cout << "========================================" << endl;
    cout << "Profesores: " << profesores.size() << endl;
    cout << "Bloques horarios: " << bloques_horarios.size() << endl;
    cout << "Grupos: " << grupos.size() << endl;
    cout << "Cursos: " << cursos.size() << endl;
    cout << "========================================\n" << endl;

    bool exito = resolverGreedy();

    auto tiempo_fin = chrono::steady_clock::now();
    chrono::duration<double> transcurrido = tiempo_fin - tiempo_inicio;
    cout << "\n⏱️  Tiempo de ejecución: " << transcurrido.count() << " segundos"
         << endl;
    cout << "========================================\n" << endl;

    return exito;

  } catch (const exception &e) {
    cout << "❌ Error en resolver(): " << e.what() << endl;
    return false;
  } catch (...) {
    cout << "❌ Error desconocido en resolver()" << endl;
    return false;
  }
}

// ============================================
// Obtener Solución
// ============================================

vector<SolucionadorHorarios::Asignacion>
SolucionadorHorarios::obtenerSolucion() {
  vector<Asignacion> resultado;

  for (const auto &sesion : sesiones) {
    if (sesion.id_bloque_asignado != -1 && sesion.id_profesor_asignado != -1) {
      resultado.push_back({sesion.id_grupo, sesion.id_curso,
                           sesion.id_profesor_asignado,
                           sesion.id_bloque_asignado});
    }
  }

  return resultado;
}
