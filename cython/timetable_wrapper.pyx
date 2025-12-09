# distutils: language = c++

from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.map cimport map
from libcpp.set cimport set as cset
from libcpp cimport bool

cdef extern from "../cpp/timetable_solver.h":
    cdef cppclass SolucionadorHorarios:
        struct Asignacion:
            int id_grupo
            int id_curso
            int id_profesor
            int id_bloque

        SolucionadorHorarios()
        void agregarBloqueHorario(int id, int dia, int h_inicio, int m_inicio, int h_fin, int m_fin)
        void agregarProfesor(int id, string nombre, vector[int] horarios_disp, vector[string] codigos_cursos)
        void agregarCurso(int id, string nombre, string codigo, int creditos, int cuatrimestre, bool req_profesor)
        void agregarGrupo(int id, int cuatrimestre, vector[int] ids_cursos)
        bool resolver(double tiempo_limite_segundos) except +
        vector[Asignacion] obtenerSolucion()

def solve_timetable(professors, courses, timeslots, groups, timeout=60.0):
    """
    Resuelve el problema de horarios usando el solver greedy en C++
    
    Args:
        professors: Lista de profesores con disponibilidad
        courses: Lista de cursos con créditos
        timeslots: Lista de bloques horarios
        groups: Lista de grupos de estudiantes
        timeout: Tiempo límite en segundos (default: 60)
    
    Returns:
        Lista de asignaciones o None si no hay solución
    """
    cdef SolucionadorHorarios solver
    
    # 1. Agregar bloques horarios
    print("⏰ Agregando bloques horarios...")
    for ts in timeslots:
        # Convertir día a número
        dia_map = {
            'Lunes': 1,
            'Martes': 2,
            'Miércoles': 3,
            'Jueves': 4,
            'Viernes': 5
        }
        dia_num = dia_map.get(ts['day'], 1)
        
        solver.agregarBloqueHorario(
            ts['id'],
            dia_num,
            ts['start_hour'], ts['start_minute'],
            ts['end_hour'], ts['end_minute']
        )
    
    # 2. Agregar profesores
    print("👨‍🏫 Agregando profesores...")
    for p in professors:
        # Convertir códigos de cursos a bytes, EXCLUYENDO Estadías
        codigos = [
            c.encode('utf-8') 
            for c in p['available_courses'] 
            if c not in ['EDSM', 'LITIID']
        ]
        solver.agregarProfesor(
            p['id'],
            p['name'].encode('utf-8'),
            p['available_timeslots'],
            codigos
        )
    
    # Agregar Profesor Virtual para Estadías
    virtual_prof_id = 99999
    virtual_prof_name = b"Estadia"
    # Disponibilidad total (todos los bloques del 1 al 500)
    virtual_timeslots = list(range(1, 500)) 
    # Materias que puede dar
    virtual_courses = [b"EDSM", b"LITIID"]
    
    solver.agregarProfesor(
        virtual_prof_id,
        virtual_prof_name,
        virtual_timeslots,
        virtual_courses
    )
    
    # 3. Agregar cursos
    print("📚 Agregando cursos...")
    for c in courses:
        semestre = c.get('semester', c.get('cuatrimestre', 1))
        
        # EXCLUIR SEMESTRES 6 y 10 (Estadías)
        if semestre in [6, 10]:
            continue

        # Determinar si requiere profesor
        requires_prof = True
        creditos = c.get('credits', c.get('weekly_hours', 0))
        
        solver.agregarCurso(
            c['id'],
            c['name'].encode('utf-8'),
            c['code'].encode('utf-8'),
            creditos,
            semestre,
            requires_prof
        )
    
    # 4. Agregar grupos
    print("👥 Agregando grupos...")
    for g in groups:
        solver.agregarGrupo(
            g['id'],
            g['semester'],
            g['course_ids']
        )
    
    # 5. Resolver
    print(f"\n🚀 Iniciando solver C++ (timeout: {timeout}s)...")
    if solver.resolver(timeout):
        print("✅ ¡Solución encontrada!")
        solucion = solver.obtenerSolucion()
        
        # Crear mapas de búsqueda
        prof_map = {p['id']: p['name'] for p in professors}
        course_map = {c['id']: {'name': c['name'], 'code': c['code']} for c in courses}
        timeslot_map = {ts['id']: ts['display'] for ts in timeslots}
        
        resultado = []
        for asig in solucion:
            # Manejar profesores no asignados
            nombre_prof = "Sin Asignar"
            if asig.id_profesor == 99999:
                nombre_prof = "" # No mostrar nombre para Estadía
            elif asig.id_profesor > 0:
                nombre_prof = prof_map.get(asig.id_profesor, f"Profesor {asig.id_profesor}")
            elif asig.id_profesor == 0:
                nombre_prof = "N/A"  # Cursos sin profesor
            
            info_curso = course_map.get(asig.id_curso, {'name': f"Curso {asig.id_curso}", 'code': '???'})
            display_bloque = timeslot_map.get(asig.id_bloque, f"Bloque {asig.id_bloque}")
            
            resultado.append({
                'group_id': asig.id_grupo,
                'course_id': asig.id_curso,
                'course_name': info_curso['name'],
                'course_code': info_curso['code'],
                'professor_id': asig.id_profesor,
                'professor_name': nombre_prof,
                'timeslot_id': asig.id_bloque,
                'timeslot_display': display_bloque
            })
        
        print(f"📊 Total de asignaciones: {len(resultado)}")
        return resultado
    else:
        print("❌ No se encontró solución completa")
        return None
