import json
import csv
import os
import sys

# Add parent directory to path to import data.periods
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from data.periods import PERIODOS

def load_professors(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        return json.load(f)

def load_timeslots(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        return json.load(f)

def load_courses(filepath):
    """
    Carga cursos desde CSV
    Formato esperado: id, name, code, weekly_hours, semester
    """
    courses = []
    with open(filepath, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            if not row.get('id'):  # Saltar líneas vacías
                continue
            # Convert types
            row['id'] = int(row['id'])
            row['weekly_hours'] = int(row['weekly_hours'])
            row['semester'] = int(row['semester'])
            # IMPORTANT: El wrapper de Cython espera 'credits', así que hacemos un alias
            row['credits'] = row['weekly_hours'] * 15  # 1 hora semanal = 15 créditos
            courses.append(row)
    return courses

def get_groups_for_period(period_key):
    """
    Genera grupos para el período académico.
    Cada cuatrimestre tiene UN grupo que toma todas las materias del cuatrimestre.
    
    Los números en PERIODOS[period_key]['cuatrimestres'][X] son IDs de MATERIAS,
    no IDs de grupos.
    """
    if period_key not in PERIODOS:
        return []
    
    groups = []
    period_data = PERIODOS[period_key]
    
    # Para cada cuatrimestre en este período
    for semester, course_ids in period_data['cuatrimestres'].items():
        # Crear UN grupo por cuatrimestre
        # ID del grupo = cuatrimestre * 100 (ej: cuatri 8 -> grupo 800)
        group_id = semester * 100
        
        groups.append({
            'id': group_id,
            'semester': semester,
            'course_ids': course_ids  # Todas las materias del cuatrimestre
        })
    
    return groups

def prepare_data_for_solver(professors, courses, timeslots, period_key):
    groups = get_groups_for_period(period_key)
    
    # Filtrar cursos relevantes para este período
    relevant_semesters = PERIODOS[period_key]['cuatrimestres'].keys()
    relevant_courses = [c for c in courses if c['semester'] in relevant_semesters]
    
    return professors, relevant_courses, timeslots, groups
