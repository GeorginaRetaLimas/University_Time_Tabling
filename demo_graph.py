"""
Script de demostración rápida del uso de grafos
Ejecutar: python demo_graph.py
"""

from backend.graph_model import TimetableGraph, demonstrate_algorithm_steps
from backend.utils import load_professors, load_courses, load_timeslots, prepare_data_for_solver
import json

def main():
    print("=" * 70)
    print("🎓 DEMOSTRACIÓN: Uso de GRAFOS en el Sistema de Horarios")
    print("=" * 70)
    
    # Cargar datos
    print("\n📂 Cargando datos...")
    professors = load_professors('data/professors.json')
    courses = load_courses('data/courses.csv')
    timeslots = load_timeslots('data/timeslots.json')
    
    # Preparar para sept-dec
    period = 'sept-dec'
    profs, crs, slots, groups = prepare_data_for_solver(professors, courses, timeslots, period)
    
    print(f"✅ Datos cargados para período: {period}")
    print(f"   - Profesores: {len(profs)}")
    print(f"   - Cursos: {len(crs)}")
    print(f"   - Bloques: {len(slots)}")
    print(f"   - Grupos: {len(groups)}")
    
    # Crear grafo
    print("\n🔨 Construyendo GRAFOS...")
    graph = TimetableGraph()
    graph.build_from_data(profs, crs, slots, groups)
    graph.build_conflict_graph(profs, slots)
    
    # Estadísticas
    print("\n📊 ESTADÍSTICAS DEL GRAFO:")
    stats = graph.get_graph_stats()
    print(f"   Total de nodos: {stats['total_nodes']}")
    print(f"   Total de aristas: {stats['total_edges']}")
    print(f"   Densidad: {stats['density']:.4f}")
    print(f"\n   Nodos por tipo:")
    print(f"      🧑‍🏫 Profesores: {stats['nodes_by_type']['professors']}")
    print(f"      📚 Cursos: {stats['nodes_by_type']['courses']}")
    print(f"      ⏰ Bloques: {stats['nodes_by_type']['timeslots']}")
    print(f"      👥 Grupos: {stats['nodes_by_type']['groups']}")
    
    print(f"\n   Grafo de Conflictos:")
    print(f"      Nodos: {stats['conflict_graph_nodes']}")
    print(f"      Aristas: {stats['conflict_graph_edges']}")
    
    # Ejemplo de vecindario
    if len(profs) > 0:
        print("\n🔍 EJEMPLO: Vecindario de un profesor")
        neighbor = graph.get_professor_neighborhood(profs[0]['id'])
        print(f"   Profesor: {profs[0]['name']}")
        print(f"   Total conexiones: {neighbor['total_connections']}")
        print(f"   Puede impartir {len(neighbor['can_teach'])} cursos")
        print(f"   Disponible en {len(neighbor['available_slots'])} bloques")
    
    # Matriz 3D
    print("\n🧊 MATRIZ 3D:")
    matrix_info = graph.visualize_matrix_3d(profs, slots, groups)
    print(f"   Dimensiones: [{matrix_info['dimensions']['professors']}]"
          f"[{matrix_info['dimensions']['timeslots']}]"
          f"[{matrix_info['dimensions']['groups']}]")
    print(f"   Total celdas: {matrix_info['total_cells']:,}")
    print(f"   Memoria: {matrix_info['memory_mb']:.2f} MB")
    print(f"\n   Ejemplo de acceso:")
    for idx in matrix_info['sample_indices'][:3]:
        print(f"      {idx['address']}")
    
    # Algoritmo
    print("\n🔄 PASOS DEL ALGORITMO GREEDY:")
    steps = demonstrate_algorithm_steps(profs, crs, slots, groups)
    for step in steps:
        print(f"\n   Paso {step['step']}: {step['name']}")
        print(f"      {step['description']}")
        print(f"      Acción: {step['action']}")
    
    print("\n" + "=" * 70)
    print("✅ Demo completada!")
    print("🌐 Abre http://localhost:5000/algorithm para ver visualización web")
    print("=" * 70)

if __name__ == '__main__':
    main()
