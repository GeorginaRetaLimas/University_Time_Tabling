#ifndef TIMETABLE_SOLVER_H
#define TIMETABLE_SOLVER_H

#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

struct TimeSlot {
  int id;
  int day; // 1=Mon, 2=Tue, ...
  int start_hour;
  int start_minute;
  int end_hour;
  int end_minute;
};

struct Professor {
  int id;
  string name;
  set<int> available_timeslots;
  set<string> available_courses; // Course codes
};

struct Course {
  int id;
  string name;
  string code;
  int weekly_hours;
  int semester;
  bool requires_professor;
};

struct Group {
  int id;
  int semester;
  // A group needs to take a set of courses
  vector<int> course_ids;
};

// Represents a single class session to be scheduled
struct ClassSession {
  int id;
  int course_id;
  int group_id;
  int duration; // usually 1 hour (55 mins)
  int assigned_timeslot_id = -1;
  int assigned_professor_id = -1;
};

class TimetableSolver {
private:
  vector<TimeSlot> timeslots;
  vector<Professor> professors;
  vector<Course> courses;
  vector<Group> groups;
  vector<ClassSession> sessions;

  // Graph representation
  // Adjacency list: session_id -> list of conflicting session_ids
  vector<vector<int>> conflict_graph;

  // Timeout handling
  std::chrono::steady_clock::time_point start_time;
  double timeout_limit;

  // Helper to find professor by ID
  Professor *getProfessor(int id);

  // Helper to find course by ID
  Course *getCourse(int id);

  // Recursive backtracking solver
  bool solveRecursive(int session_idx, vector<ClassSession> &sessions,
                      const vector<vector<int>> &conflict_graph,
                      const vector<TimeSlot> &timeslots,
                      const vector<Professor> &professors,
                      const vector<Course> &courses);

public:
  TimetableSolver();

  void addTimeSlot(int id, int day, int start_h, int start_m, int end_h,
                   int end_m);
  void addProfessor(int id, string name, const vector<int> &available_slots,
                    const vector<string> &course_codes);
  void addCourse(int id, string name, string code, int weekly_hours,
                 int semester, bool requires_professor);
  void addGroup(int id, int semester, const vector<int> &course_ids);

  // Core logic
  void generateSessions();
  void buildConflictGraph();
  bool solve(double timeout); // Main solver function

  // Output
  struct Assignment {
    int group_id;
    int course_id;
    int professor_id;
    int timeslot_id;
  };

  vector<Assignment> getSolution();
};

#endif
