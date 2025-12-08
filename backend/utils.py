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
    courses = []
    with open(filepath, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f)
        for row in reader:
            # Convert types
            row['id'] = int(row['id'])
            row['weekly_hours'] = int(row['weekly_hours'])
            row['semester'] = int(row['semester'])
            courses.append(row)
    return courses

def get_groups_for_period(period_key):
    if period_key not in PERIODOS:
        return []
    
    groups = []
    period_data = PERIODOS[period_key]
    
    # Iterate over semesters in this period
    for semester, group_ids in period_data['cuatrimestres'].items():
        for gid in group_ids:
            groups.append({
                'id': gid,
                'semester': semester,
                'course_ids': [] # Will be populated based on courses matching this semester
            })
    return groups

def prepare_data_for_solver(professors, courses, timeslots, period_key):
    groups = get_groups_for_period(period_key)
    
    # Assign courses to groups based on semester
    # A group takes ALL courses for its semester
    # Optimization: Filter courses that are actually relevant for this period
    
    relevant_semesters = PERIODOS[period_key]['cuatrimestres'].keys()
    relevant_courses = [c for c in courses if c['semester'] in relevant_semesters]
    
    for group in groups:
        group_courses = [c['id'] for c in relevant_courses if c['semester'] == group['semester']]
        group['course_ids'] = group_courses
        
    return professors, relevant_courses, timeslots, groups
