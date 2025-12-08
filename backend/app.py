from flask import Flask, render_template, request, jsonify, send_file
import os
import json
import pandas as pd
from utils import load_professors, load_timeslots, load_courses, prepare_data_for_solver
import sys
import os
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
import timetable_wrapper

app = Flask(__name__, template_folder='../frontend/templates', static_folder='../frontend/static')

DATA_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '../data'))

from data.periods import PERIODOS

@app.route('/')
def index():
    return render_template('index.html', periods=PERIODOS)

@app.route('/api/solve', methods=['POST'])
def solve():
    data = request.json
    period = data.get('period', 'sept-dec')
    
    # Cargar datos
    professors = load_professors(os.path.join(DATA_DIR, 'professors.json'))
    timeslots = load_timeslots(os.path.join(DATA_DIR, 'timeslots.json'))
    courses = load_courses(os.path.join(DATA_DIR, 'courses.csv'))
    
    # Preparar datos
    profs, crs, slots, groups = prepare_data_for_solver(professors, courses, timeslots, period)
    
    timeout = float(data.get('timeout', 60))  # Tiempo límite en segundos
    
    # Resolver usando el solver C++ (greedy con matrices 3D)
    try:
        solution = timetable_wrapper.solve_timetable(profs, crs, slots, groups, timeout)
        if solution:
            # Frontend espera 'solution', no 'assignments'
            return jsonify({'status': 'success', 'solution': solution})
        else:
            return jsonify({'status': 'error', 'message': 'No se encontró solución'}), 400
    except Exception as e:
        import traceback
        error_details = traceback.format_exc()
        print("❌ Error en solver:")
        print(error_details)
        return jsonify({'status': 'error', 'message': str(e), 'details': error_details}), 500

@app.route('/professors')
def professors_page():
    profs = load_professors(os.path.join(DATA_DIR, 'professors.json'))
    return render_template('professors.html', professors=profs)

@app.route('/availability')
def availability_page():
    profs = load_professors(os.path.join(DATA_DIR, 'professors.json'))
    slots = load_timeslots(os.path.join(DATA_DIR, 'timeslots.json'))
    return render_template('availability.html', professors=profs, timeslots=slots)

@app.route('/timetable')
def timetable_page():
    return render_template('timetable.html')

@app.route('/api/professors', methods=['GET'])
def get_professors():
    profs = load_professors(os.path.join(DATA_DIR, 'professors.json'))
    return jsonify(profs)

@app.route('/api/courses', methods=['GET'])
def get_courses():
    courses = load_courses(os.path.join(DATA_DIR, 'courses.csv'))
    return jsonify(courses)

@app.route('/api/professors/save', methods=['POST'])
def save_professor():
    data = request.json
    prof_id = data.get('id')
    name = data.get('name')
    
    file_path = os.path.join(DATA_DIR, 'professors.json')
    profs = load_professors(file_path)
    
    if prof_id:
        # Edit existing
        for p in profs:
            if p['id'] == prof_id:
                p['name'] = name
                break
    else:
        # Create new (simple ID generation)
        new_id = max([p['id'] for p in profs]) + 1 if profs else 1
        profs.append({
            "id": new_id,
            "name": name,
            "available_timeslots": [],
            "available_courses": []
        })
    
    with open(file_path, 'w', encoding='utf-8') as f:
        json.dump(profs, f, indent=2, ensure_ascii=False)
        
    return jsonify({'status': 'success'})

@app.route('/api/professors/availability', methods=['POST'])
def save_availability():
    data = request.json
    prof_id = data.get('id')
    slots = data.get('available_timeslots')
    
    file_path = os.path.join(DATA_DIR, 'professors.json')
    profs = load_professors(file_path)
    
    for p in profs:
        if p['id'] == prof_id:
            p['available_timeslots'] = slots
            break
            
    with open(file_path, 'w', encoding='utf-8') as f:
        json.dump(profs, f, indent=2, ensure_ascii=False)
        
    return jsonify({'status': 'success'})

@app.route('/api/professors/subjects', methods=['POST'])
def save_subjects():
    data = request.json
    prof_id = data.get('id')
    courses = data.get('available_courses')
    
    file_path = os.path.join(DATA_DIR, 'professors.json')
    profs = load_professors(file_path)
    
    for p in profs:
        if p['id'] == prof_id:
            p['available_courses'] = courses
            break
            
    with open(file_path, 'w', encoding='utf-8') as f:
        json.dump(profs, f, indent=2, ensure_ascii=False)
        
    return jsonify({'status': 'success'})

@app.route('/api/timeslots', methods=['GET'])
def get_timeslots():
    slots = load_timeslots(os.path.join(DATA_DIR, 'timeslots.json'))
    return jsonify(slots)

if __name__ == '__main__':
    app.run(debug=True, port=5000)
