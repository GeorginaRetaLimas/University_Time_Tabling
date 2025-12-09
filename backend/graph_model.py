"""
Representación del Sistema de Horarios usando GRAFOS
=====================================================
Este módulo demuestra cómo el problema de horarios puede modelarse explícitamente usando grafos.
"""

import json
from typing import Dict, List, Set, Tuple
import networkx as nx


class TimetableGraph:
    """
    Modelo de Grafos para el Problema de Horarios Universitarios
    
    NODOS:
    - Profesores
    - Cursos
    - Bloques Horarios
    - Grupos
    
    ARISTAS:
    - Profesor → Curso (puede impartir)
    - Profesor → Bloque (disponibilidad)
    - Curso → Grupo (debe tomarlo)
    - Curso → Bloque (asignación)
    """
    
    def __init__(self):
        self.graph = nx.Graph()
        self.conflict_graph = nx.Graph()  # Grafo de conflictos
        self.assignment_graph = nx.DiGraph()  # Grafo dirigido de asignaciones
        
    def build_from_data(self, professors, courses, timeslots, groups):
        """Construye el grafo desde los datos del sistema"""
        
        # ==================================================
        # NODOS: Agregar todos los elementos del sistema
        # ==================================================
        
        # Profesores
        for prof in professors:
            self.graph.add_node(
                f"P{prof['id']}", 
                type='professor',
                name=prof['name'],
                data=prof
            )
        
        # Cursos
        for course in courses:
            self.graph.add_node(
                f"C{course['id']}", 
                type='course',
                name=course['name'],
                code=course['code'],
                data=course
            )
        
        # Bloques horarios
        for slot in timeslots:
            self.graph.add_node(
                f"T{slot['id']}", 
                type='timeslot',
                day=slot['day'],
                time=f"{slot['start_hour']}:{slot['start_minute']:02d}",
                data=slot
            )
        
        # Grupos
        for group in groups:
            self.graph.add_node(
                f"G{group['id']}", 
                type='group',
                semester=group['semester'],
                data=group
            )
        
        # ==================================================
        # ARISTAS: Relaciones entre entidades
        # ==================================================
        
        # Profesor → Curso (capacidad de enseñar)
        for prof in professors:
            for course_code in prof['available_courses']:
                matching_courses = [c for c in courses if c['code'] == course_code]
                for course in matching_courses:
                    self.graph.add_edge(
                        f"P{prof['id']}", 
                        f"C{course['id']}", 
                        relation='can_teach'
                    )
        
        # Profesor → Bloque (disponibilidad)
        for prof in professors:
            for slot_id in prof['available_timeslots']:
                self.graph.add_edge(
                    f"P{prof['id']}", 
                    f"T{slot_id}", 
                    relation='available'
                )
        
        # Grupo → Curso (debe tomar)
        for group in groups:
            for course_id in group['course_ids']:
                self.graph.add_edge(
                    f"G{group['id']}", 
                    f"C{course_id}", 
                    relation='must_take'
                )
        
    def build_conflict_graph(self, professors, timeslots):
        """
        Construye un GRAFO DE CONFLICTOS
        - Cada nodo es una combinación (Profesor, Bloque)
        - Arista entre nodos si NO pueden ocurrir simultáneamente
        """
        
        # Crear nodos para cada combinación válida (Profesor, Bloque)
        for prof in professors:
            for slot_id in prof['available_timeslots']:
                node_id = f"P{prof['id']}_T{slot_id}"
                self.conflict_graph.add_node(
                    node_id,
                    professor_id=prof['id'],
                    timeslot_id=slot_id
                )
        
        # Agregar aristas de conflicto
        nodes = list(self.conflict_graph.nodes())
        for i, node1 in enumerate(nodes):
            for node2 in nodes[i+1:]:
                p1, t1 = node1.split('_')
                p2, t2 = node2.split('_')
                
                # Conflicto si: mismo profesor O mismo bloque
                if p1 == p2 or t1 == t2:
                    self.conflict_graph.add_edge(node1, node2, type='conflict')
    
    def add_assignment(self, professor_id, course_id, timeslot_id, group_id):
        """Agrega una asignación al grafo de asignaciones"""
        assignment_id = f"A_{professor_id}_{course_id}_{timeslot_id}_{group_id}"
        
        self.assignment_graph.add_node(
            assignment_id,
            professor_id=professor_id,
            course_id=course_id,
            timeslot_id=timeslot_id,
            group_id=group_id
        )
        
        # Aristas dirigidas
        self.assignment_graph.add_edge(f"P{professor_id}", assignment_id, type='teaches')
        self.assignment_graph.add_edge(assignment_id, f"C{course_id}", type='of_course')
        self.assignment_graph.add_edge(assignment_id, f"T{timeslot_id}", type='at_time')
        self.assignment_graph.add_edge(assignment_id, f"G{group_id}", type='to_group')
    
    def get_graph_stats(self) -> Dict:
        """Estadísticas del grafo"""
        return {
            'total_nodes': self.graph.number_of_nodes(),
            'total_edges': self.graph.number_of_edges(),
            'nodes_by_type': {
                'professors': len([n for n, d in self.graph.nodes(data=True) if d.get('type') == 'professor']),
                'courses': len([n for n, d in self.graph.nodes(data=True) if d.get('type') == 'course']),
                'timeslots': len([n for n, d in self.graph.nodes(data=True) if d.get('type') == 'timeslot']),
                'groups': len([n for n, d in self.graph.nodes(data=True) if d.get('type') == 'group'])
            },
            'density': nx.density(self.graph),
            'conflict_graph_nodes': self.conflict_graph.number_of_nodes(),
            'conflict_graph_edges': self.conflict_graph.number_of_edges()
        }
    
    def get_professor_neighborhood(self, professor_id: int) -> Dict:
        """Obtiene el vecindario de un profesor en el grafo"""
        node_id = f"P{professor_id}"
        if node_id not in self.graph:
            return {}
        
        neighbors = list(self.graph.neighbors(node_id))
        
        return {
            'professor': node_id,
            'total_connections': len(neighbors),
            'can_teach': [n for n in neighbors if self.graph.nodes[n].get('type') == 'course'],
            'available_slots': [n for n in neighbors if self.graph.nodes[n].get('type') == 'timeslot']
        }
    
    def visualize_matrix_3d(self, professors, timeslots, groups) -> Dict:
        """
        Genera una representación de la matriz 3D
        matriz[profesor][timeslot][grupo] = course_id
        """
        matrix_structure = {
            'dimensions': {
                'professors': len(professors),
                'timeslots': len(timeslots),
                'groups': len(groups)
            },
            'total_cells': len(professors) * len(timeslots) * len(groups),
            'memory_mb': (len(professors) * len(timeslots) * len(groups) * 4) / (1024 * 1024),  # 4 bytes por int
            'sample_indices': []
        }
        
        # Ejemplos de índices
        for p_idx in range(min(3, len(professors))):
            for t_idx in range(min(3, len(timeslots))):
                for g_idx in range(min(2, len(groups))):
                    matrix_structure['sample_indices'].append({
                        'professor_idx': p_idx,
                        'timeslot_idx': t_idx,
                        'group_idx': g_idx,
                        'address': f"matriz[{p_idx}][{t_idx}][{g_idx}]"
                    })
        
        return matrix_structure
    
    def export_for_visualization(self) -> Dict:
        """Exporta el grafo en formato JSON para visualización web"""
        nodes = []
        edges = []
        
        for node, data in self.graph.nodes(data=True):
            nodes.append({
                'id': node,
                'type': data.get('type', 'unknown'),
                'label': data.get('name', data.get('code', node))
            })
        
        for source, target, data in self.graph.edges(data=True):
            edges.append({
                'source': source,
                'target': target,
                'relation': data.get('relation', 'connected')
            })
        
        return {
            'nodes': nodes,
            'edges': edges,
            'stats': self.get_graph_stats()
        }


def demonstrate_algorithm_steps(professors, courses, timeslots, groups) -> List[Dict]:
    """
    Demuestra los pasos del algoritmo Greedy
    Retorna una lista de pasos con el estado en cada iteración
    """
    
    steps = []
    
    # Paso 1: Inicialización
    steps.append({
        'step': 1,
        'name': 'Inicialización',
        'description': 'Crear matriz 3D y mapas de índices',
        'action': 'matriz[P][T][G] = 0 (vacío)',
        'state': {
            'professors': len(professors),
            'timeslots': len(timeslots),
            'groups': len(groups),
            'sessions_to_assign': 0
        }
    })
    
    # Paso 2: Generar sesiones
    total_sessions = 0
    for group in groups:
        for course_id in group['course_ids']:
            course = next((c for c in courses if c['id'] == course_id), None)
            if course:
                credits = course.get('credits', 60)
                num_sessions = max(1, credits // 15)
                total_sessions += num_sessions
    
    steps.append({
        'step': 2,
        'name': 'Generar Sesiones',
        'description': f'Crear {total_sessions} sesiones desde créditos de cursos',
        'action': 'sesiones = max(1, creditos / 15)',
        'state': {
            'total_sessions': total_sessions,
            'formula': 'num_sesiones = max(1, creditos / 15)'
        }
    })
    
    # Paso 3: Ordenar por prioridad
    steps.append({
        'step': 3,
        'name': 'Ordenar Sesiones',
        'description': 'Ordenar por: 1) Mayor créditos, 2) Mismo curso, 3) Número sesión',
        'action': 'sort(sesiones, key=lambda s: (-s.creditos, s.id_curso, s.numero))',
        'state': {
            'priority': 'Cursos pesados primero (más créditos)'
        }
    })
    
    # Paso 4: Asignar con Greedy
    steps.append({
        'step': 4,
        'name': 'Asignación Greedy',
        'description': 'Para cada sesión, buscar mejor (profesor, bloque) disponible',
        'action': 'Intentar 4 estrategias: Estricta → Relajada → Emergencia → Emergencia Total',
        'state': {
            'strategies': [
                '1. Estricta: Verifica materia + restricciones completas',
                '2. Relajada: Permite más consecutivas',
                '3. Emergencia: Ignora capacitación profesor',
                '4. Emergencia Total: Relaja todo'
            ]
        }
    })
    
    # Paso 5: Ordenar bloques
    steps.append({
        'step': 5,
        'name': 'Ordenar Bloques (LCV)',
        'description': 'Least Constraining Value: preferir bloques que dejan más opciones',
        'action': 'sort(bloques, key=lambda b: (carga_dia[b], -adyacencia[b]))',
        'state': {
            'heuristic': 'Balancear días + preferir bloques consecutivos'
        }
    })
    
    # Paso 6: Validar restricciones
    steps.append({
        'step': 6,
        'name': 'Validar Restricciones',
        'description': 'Verificar que no haya conflictos',
        'action': 'Verificar: disponibilidad, conflictos profesor/grupo, consecutividad',
        'state': {
            'checks': [
                '✓ Profesor disponible en bloque',
                '✓ Profesor no tiene otra clase',
                '✓ Grupo no tiene otra clase',
                '✓ Mismo profesor para todas las sesiones de un curso',
                '✓ Máximo 2 sesiones consecutivas del mismo curso'
            ]
        }
    })
    
    # Paso 7: Asignar en matriz
    steps.append({
        'step': 7,
        'name': 'Asignar en Matriz 3D',
        'description': 'Actualizar matriz con la asignación',
        'action': 'matriz[idx_profesor][idx_bloque][idx_grupo] = id_curso',
        'state': {
            'complexity': 'O(1) - acceso directo a matriz',
            'update': 'Actualizar estructuras auxiliares de seguimiento'
        }
    })
    
    return steps
