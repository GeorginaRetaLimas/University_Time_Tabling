import json
import os

def update_professors():
    file_path = 'data/professors.json'
    
    with open(file_path, 'r', encoding='utf-8') as f:
        professors = json.load(f)
    
    # Generar todos los slots posibles (aprox de id 1 a 500, asumiendo estructura conocida)
    # O mejor, simplemente asignamos una lista generosa de IDs.
    # Basado en timeslots.json, los IDs van por dia y hora.
    # Asumiremos que los IDs 1-500 cubren toda la semana laboral.
    # Para ser "creíble", podemos dar turnos: Matutino (1-200) o Vespertino (200-400) o Mixto.
    # Pero el usuario dijo "aumenta sus horas... para que sea posible generar".
    # Así que seremos generosos: Turno completo 7am - 9pm para la mayoría.
    
    # IDs típicos en el sistema actual parecen ser:
    # Lunes: 1-15
    # Martes: 101-115
    # ...
    # Vamos a generar una lista de slots disponibles basada en lo que hemos visto en logs.
    # Mejor aún, leemos timeslots.json para saber qué IDs existen.
    
    with open('data/timeslots.json', 'r', encoding='utf-8') as f:
        timeslots = json.load(f)
        all_slot_ids = [t['id'] for t in timeslots]
        
    print(f"Total de slots de tiempo encontrados: {len(all_slot_ids)}")
    
    for prof in professors:
        # Asignar TODOS los slots como disponibles para máxima flexibilidad
        # El usuario pidió "aumenta sus horas... que sea posible generar"
        prof['available_timeslots'] = all_slot_ids
        
    with open(file_path, 'w', encoding='utf-8') as f:
        json.dump(professors, f, indent=2, ensure_ascii=False)
        
    print(f"Actualizados {len(professors)} profesores con disponibilidad completa.")

if __name__ == "__main__":
    update_professors()
