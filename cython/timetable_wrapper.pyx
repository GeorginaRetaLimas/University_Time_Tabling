# distutils: language = c++

from libcpp.vector cimport vector
from libcpp.string cimport string
from libcpp.map cimport map
from libcpp.set cimport set as cset
from libcpp cimport bool

cdef extern from "../cpp/timetable_solver.h":
    cdef cppclass TimetableSolver:
        struct Assignment:
            int group_id
            int course_id
            int professor_id
            int timeslot_id

        TimetableSolver()
        void addTimeSlot(int id, int day, int start_h, int start_m, int end_h, int end_m)
        void addProfessor(int id, string name, vector[int] available_slots, vector[string] course_codes)
        void addCourse(int id, string name, string code, int weekly_hours, int semester, bool requires_professor)
        void addGroup(int id, int semester, vector[int] course_ids)
        void addGroup(int id, int semester, vector[int] course_ids)
        bool solve(double timeout) except +
        vector[Assignment] getSolution()

def solve_timetable(professors, courses, timeslots, groups, timeout=0.0):
    cdef TimetableSolver solver
    
    # 1. Add Timeslots
    for ts in timeslots:
        solver.addTimeSlot(ts['id'], 
                           1 if ts['day'] == 'Lunes' else 2 if ts['day'] == 'Martes' else 3 if ts['day'] == 'Miércoles' else 4 if ts['day'] == 'Jueves' else 5, 
                           ts['start_hour'], ts['start_minute'], 
                           ts['end_hour'], ts['end_minute'])

    # 2. Add Professors
    for p in professors:
        # Ensure available_courses is a list of strings
        codes = [c.encode('utf-8') for c in p['available_courses']]
        solver.addProfessor(p['id'], p['name'].encode('utf-8'), p['available_timeslots'], codes)

    # 3. Add Courses
    for c in courses:
        requires_prof = True
        if c['code'] in [b'EDSM', b'LITIID']:
            requires_prof = False
        solver.addCourse(c['id'], c['name'].encode('utf-8'), c['code'].encode('utf-8'), c['weekly_hours'], c['semester'], requires_prof)

    # 4. Add Groups
    for g in groups:
        solver.addGroup(g['id'], g['semester'], g['course_ids'])

    print(f"Starting C++ Solver with timeout {timeout}s...")
    if solver.solve(timeout):
        print("Solution found!")
        solution = solver.getSolution()
        
        # Create lookups
        prof_map = {p['id']: p['name'] for p in professors}
        course_map = {c['id']: c['name'] for c in courses}
        
        result = []
        for assign in solution:
            # Handle unassigned (0 or -1)
            prof_name = "Sin Asignar"
            if assign.professor_id > 0:
                prof_name = prof_map.get(assign.professor_id, f"Prof {assign.professor_id}")
            elif assign.professor_id == 0:
                prof_name = "N/A" # For courses that don't require professor
                
            course_name = course_map.get(assign.course_id, f"Course {assign.course_id}")
            
            result.append({
                'group_id': assign.group_id,
                'course_id': assign.course_id,
                'course_name': course_name, # Added name
                'professor_id': assign.professor_id,
                'professor_name': prof_name, # Added name
                'timeslot_id': assign.timeslot_id
            })
        return result
    else:
        print("No solution found.")
        return None
