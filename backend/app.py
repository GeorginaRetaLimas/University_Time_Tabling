from flask import Flask, render_template, request, jsonify, send_file
import os
import json
import pandas as pd
from utils import load_professors, load_timeslots, load_courses, prepare_data_for_solver
import sys
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
    
    # Load data (either from upload or default)
    # For now, we use default files in data/
    professors = load_professors(os.path.join(DATA_DIR, 'professors.json'))
    timeslots = load_timeslots(os.path.join(DATA_DIR, 'timeslots.json'))
    courses = load_courses(os.path.join(DATA_DIR, 'courses.csv'))
    
    # Prepare data
    profs, crs, slots, groups = prepare_data_for_solver(professors, courses, timeslots, period)
    
    timeout = float(data.get('timeout', 60)) # Default 60 seconds
    
    # Solve
    try:
        solution = timetable_wrapper.solve_timetable(profs, crs, slots, groups, timeout)
        if solution:
            return jsonify({'status': 'success', 'solution': solution})
        else:
            return jsonify({'status': 'error', 'message': 'No solution found'}), 400
    except Exception as e:
        return jsonify({'status': 'error', 'message': str(e)}), 500

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

if __name__ == '__main__':
    app.run(debug=True, port=5000)
