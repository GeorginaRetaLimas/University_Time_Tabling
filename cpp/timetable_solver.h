#ifndef TIMETABLE_SOLVER_H
#define TIMETABLE_SOLVER_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

// ============================================
// Estructuras de Datos (en español)
// ============================================

struct BloqueHorario {
  int id;
  int dia; // 1=Lun, 2=Mar, 3=Mié, 4=Jue, 5=Vie
  int hora_inicio;
  int minuto_inicio;
  int hora_fin;
  int minuto_fin;
};

struct Profesor {
  int id;
  string nombre;
  set<int> horarios_disponibles;    // IDs de bloques horarios disponibles
  set<string> materias_capacitadas; // Códigos de cursos que puede impartir
};

struct Curso {
  int id;
  string nombre;
  string codigo;
  int creditos;           // Antes: weekly_hours
  int cuatrimestre;       // Antes: semester
  bool requiere_profesor; // Para casos especiales como Estadías
};

struct Grupo {
  int id;
  int cuatrimestre;
  vector<int> ids_cursos; // Cursos que debe tomar este grupo
};

struct SesionClase {
  int id;
  int id_curso;
  int id_grupo;
  int creditos;
  int numero_sesion; // 1, 2, 3... para sesiones de la misma materia
  int id_bloque_asignado = -1;
  int id_profesor_asignado = -1;
};

// ============================================
// Clase Principal del Solver
// ============================================

class SolucionadorHorarios {
private:
  // Datos del problema
  vector<BloqueHorario> bloques_horarios;
  vector<Profesor> profesores;
  vector<Curso> cursos;
  vector<Grupo> grupos;
  vector<SesionClase> sesiones;

  // Matriz 3D de asignaciones: [índice_prof][índice_bloque][índice_grupo] =
  // id_curso
  vector<vector<vector<int>>> matriz_asignaciones;

  // Mapas de conversión ID ↔ Índice
  map<int, int> id_a_indice_prof, id_a_indice_bloque, id_a_indice_grupo;
  map<int, int> indice_a_id_prof, indice_a_id_bloque, indice_a_id_grupo;

  // Seguimiento: qué materias da cada profesor a cada grupo
  // [id_grupo][id_profesor] = {set de id_curso}
  map<int, map<int, set<int>>> cursos_por_profesor_grupo;

  // Seguimiento: carga horaria por día para cada grupo (para distribución
  // equitativa) [id_grupo][dia] = cantidad_sesiones
  map<int, map<int, int>> carga_grupo_dia;

  // Control de tiempo
  chrono::steady_clock::time_point tiempo_inicio;
  double limite_tiempo;

  // Métodos privados (helpers)
  Profesor *obtenerProfesor(int id);
  Curso *obtenerCurso(int id);

  void construirMapasIndices();
  void inicializarMatriz();
  void generarSesiones();

  int obtenerSiguienteBloque(int idx_bloque_actual);
  bool sonBloquesConsecutivos(int idx_bloque1, int idx_bloque2);

  bool verificarDisponibilidadProfesor(int idx_prof, int idx_bloque);
  bool verificarConflictoProfesor(int idx_prof, int idx_bloque);
  bool verificarConflictoGrupo(int idx_grupo, int idx_bloque);
  bool verificarDiversidadProfesor(int id_prof, int id_grupo, int id_curso);
  bool verificarConsecutividad(const SesionClase &sesion, int idx_prof,
                               int idx_bloque);
  bool verificarMaximoConsecutivas(int idx_grupo, int id_curso, int idx_bloque,
                                   int idx_prof);

  bool asignarSesionGreedy(SesionClase &sesion);
  bool resolverGreedy();

public:
  SolucionadorHorarios();

  // Métodos para agregar datos
  void agregarBloqueHorario(int id, int dia, int h_inicio, int m_inicio,
                            int h_fin, int m_fin);
  void agregarProfesor(int id, string nombre, const vector<int> &horarios_disp,
                       const vector<string> &codigos_cursos);
  void agregarCurso(int id, string nombre, string codigo, int creditos,
                    int cuatrimestre, bool req_profesor);
  void agregarGrupo(int id, int cuatrimestre, const vector<int> &ids_cursos);

  // Método principal de resolución
  bool resolver(double tiempo_limite_segundos);

  // Obtener solución
  struct Asignacion {
    int id_grupo;
    int id_curso;
    int id_profesor;
    int id_bloque;
  };

  vector<Asignacion> obtenerSolucion();
};

#endif
