#include "timetable_solver.h"
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>

TimetableSolver::TimetableSolver() {}

void TimetableSolver::addTimeSlot(int id, int day, int start_h, int start_m,
                                  int end_h, int end_m) {
  timeslots.push_back({id, day, start_h, start_m, end_h, end_m});
}

void TimetableSolver::addProfessor(int id, string name,
                                   const vector<int> &available_slots,
                                   const vector<string> &course_codes) {
  set<int> slots(available_slots.begin(), available_slots.end());
  set<string> courses(course_codes.begin(), course_codes.end());
  professors.push_back({id, name, slots, courses});
}

void TimetableSolver::addCourse(int id, string name, string code,
                                int weekly_hours, int semester,
                                bool requires_professor) {
  courses.push_back(
      {id, name, code, weekly_hours, semester, requires_professor});
}

void TimetableSolver::addGroup(int id, int semester,
                               const vector<int> &course_ids) {
  groups.push_back({id, semester, course_ids});
}

Professor *TimetableSolver::getProfessor(int id) {
  for (auto &p : professors) {
    if (p.id == id)
      return &p;
  }
  return nullptr;
}

Course *TimetableSolver::getCourse(int id) {
  for (auto &c : courses) {
    if (c.id == id)
      return &c;
  }
  return nullptr;
}

void TimetableSolver::generateSessions() {
  sessions.clear();
  int session_counter = 0;

  for (const auto &group : groups) {
    for (int course_id : group.course_ids) {
      Course *course = getCourse(course_id);
      if (!course)
        continue;

      // Create one session per weekly hour
      for (int i = 0; i < course->weekly_hours; ++i) {
        ClassSession session;
        session.id = session_counter++;
        session.course_id = course_id;
        session.group_id = group.id;
        session.duration = 1; // Assuming 1 hour slots for now
        sessions.push_back(session);
      }
    }
  }
  cout << "Generated " << sessions.size() << " sessions." << endl;
}

void TimetableSolver::buildConflictGraph() {
  int n = sessions.size();
  conflict_graph.assign(n, vector<int>());

  for (int i = 0; i < n; ++i) {
    for (int j = i + 1; j < n; ++j) {
      // Conflict 1: Same Group (cannot be in two places at once)
      if (sessions[i].group_id == sessions[j].group_id) {
        conflict_graph[i].push_back(j);
        conflict_graph[j].push_back(i);
        continue;
      }

      // Conflict 2: Same Professor (handled dynamically during assignment,
      // as professors are not assigned yet. But if we pre-assigned, we'd add
      // edges here.) Since we assign professors AND timeslots, the graph mainly
      // models "Cannot happen at same time" constraints.
    }
  }
}

// Helper to check if a timeslot is valid for a session given a professor
bool isValid(const ClassSession &session, int timeslot_id, int professor_id,
             const vector<ClassSession> &current_sessions,
             const vector<TimeSlot> &all_timeslots,
             const vector<Professor> &all_professors) {

  // 1. Professor Availability
  const Professor *prof = nullptr;
  for (const auto &p : all_professors) {
    if (p.id == professor_id) {
      prof = &p;
      break;
    }
  }
  if (!prof || prof->available_timeslots.find(timeslot_id) ==
                   prof->available_timeslots.end()) {
    return false;
  }

  // 2. Professor Overlap
  for (const auto &s : current_sessions) {
    if (s.id != session.id && s.assigned_timeslot_id == timeslot_id &&
        s.assigned_professor_id == professor_id) {
      return false;
    }
  }

  // 3. Group Overlap (Already handled by graph coloring, but good to double
  // check or if graph is incomplete) The graph coloring ensures that connected
  // nodes (same group) have different colors (timeslots). So we just need to
  // ensure the timeslot index matches the color.

  return true;
}

// Backtracking solver with MRV (Minimum Remaining Values)
bool TimetableSolver::solveRecursive(int session_idx,
                                     vector<ClassSession> &sessions,
                                     const vector<vector<int>> &conflict_graph,
                                     const vector<TimeSlot> &timeslots,
                                     const vector<Professor> &professors,
                                     const vector<Course> &courses) {

  // Check timeout
  if (timeout_limit > 0) {
    auto current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = current_time - start_time;
    if (elapsed.count() > timeout_limit) {
      cout << "Timeout reached after " << elapsed.count() << " seconds."
           << endl;
      return false; // Timeout reached
    }
  }

  if (session_idx % 100 == 0) {
    cout << "Solving session " << session_idx << "/" << sessions.size() << endl;
  }

  if (session_idx >= sessions.size()) {
    return true; // All assigned
  }

  ClassSession &current_session = sessions[session_idx];

  // Find course info
  const Course *course = nullptr;
  for (const auto &c : courses) {
    if (c.id == current_session.course_id) {
      course = &c;
      break;
    }
  }

  if (!course)
    return false; // Should not happen

  // If course does NOT require a professor (e.g. Estadia)
  if (!course->requires_professor) {
    // Just find a valid timeslot
    for (const auto &ts : timeslots) {
      // Check hard constraints with neighbors in conflict graph
      bool conflict_with_neighbor = false;
      for (int neighbor_id : conflict_graph[current_session.id]) {
        if (sessions[neighbor_id].assigned_timeslot_id == ts.id) {
          conflict_with_neighbor = true;
          break;
        }
      }
      if (conflict_with_neighbor)
        continue;

      // Assign without professor
      current_session.assigned_timeslot_id = ts.id;
      current_session.assigned_professor_id =
          0; // 0 or -1 indicates no professor

      if (solveRecursive(session_idx + 1, sessions, conflict_graph, timeslots,
                         professors, courses)) {
        return true;
      }

      // Backtrack
      current_session.assigned_timeslot_id = -1;
      current_session.assigned_professor_id = -1;
    }
    return false;
  }

  // Normal case: Requires Professor
  string course_code = course->code;
  vector<int> eligible_professors;
  for (const auto &p : professors) {
    if (p.available_courses.count(course_code)) {
      eligible_professors.push_back(p.id);
    }
  }

  if (eligible_professors.empty()) {
    return false;
  }

  // Try each eligible professor
  for (int prof_id : eligible_professors) {
    // Check availability
    const Professor *p_ptr = nullptr;
    for (const auto &p : professors) {
      if (p.id == prof_id) {
        p_ptr = &p;
        break;
      }
    }

    // Try timeslots available for this professor
    for (const auto &ts : timeslots) {
      // Check if professor is available at this time
      if (p_ptr->available_timeslots.find(ts.id) ==
          p_ptr->available_timeslots.end()) {
        continue;
      }

      // Check hard constraints with neighbors (Group conflict)
      bool conflict_with_neighbor = false;
      for (int neighbor_id : conflict_graph[current_session.id]) {
        if (sessions[neighbor_id].assigned_timeslot_id == ts.id) {
          conflict_with_neighbor = true;
          break;
        }
      }
      if (conflict_with_neighbor)
        continue;

      // Check Professor Overlap (is this professor already booked at this
      // time?)
      bool prof_busy = false;
      for (int i = 0; i < session_idx; ++i) {
        if (sessions[i].assigned_professor_id == prof_id &&
            sessions[i].assigned_timeslot_id == ts.id) {
          prof_busy = true;
          break;
        }
      }
      if (prof_busy)
        continue;

      // Assign
      current_session.assigned_timeslot_id = ts.id;
      current_session.assigned_professor_id = prof_id;

      if (solveRecursive(session_idx + 1, sessions, conflict_graph, timeslots,
                         professors, courses)) {
        return true;
      }

      // Backtrack
      current_session.assigned_timeslot_id = -1;
      current_session.assigned_professor_id = -1;
    }
  }

  return false;
}

bool TimetableSolver::solve(double timeout_seconds) {
  this->timeout_limit = timeout_seconds;
  this->start_time = std::chrono::steady_clock::now();
  generateSessions();
  buildConflictGraph();

  // Sort sessions by degree (heuristic) to fail fast
  // But we need to keep track of original IDs if we sort.
  // For now, let's just solve linearly.

  return solveRecursive(0, sessions, conflict_graph, timeslots, professors,
                        courses);
}

vector<TimetableSolver::Assignment> TimetableSolver::getSolution() {
  vector<Assignment> result;
  for (const auto &s : sessions) {
    result.push_back({s.group_id, s.course_id, s.assigned_professor_id,
                      s.assigned_timeslot_id});
  }
  return result;
}
