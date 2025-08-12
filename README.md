# Adexa(Adaptive Study Schedule Generator)
A simple Qt Widgets application that generates personalized study schedules based on subjects, their difficulty, importance, and topics. This desktop app helps students organize their study time effectively across multiple days and hours per day.

## Features

- **Add Subjects** with:
  - Name
  - Difficulty (1–10)
  - Importance (1–10)
  - List of topics (one per line)
  
- **Schedule Generation**
  - Allocates study hours proportionally based on difficulty, importance, and number of topics
  - Cyclic repetition of topics if needed
  - Time distributed across all available days and hours per day
  - Limits maximum continuous study slot per topic to 2 hours
  
- **Interactive UI**
  - Add, remove, and edit subjects
  - Set total study days and hours per day
  - View generated schedule in a table with day-wise subject, topic, and time slots
  - Save generated schedule as a CSV file for external use

## UI Overview

- **Schedule Settings**: Configure number of days and daily study hours.
- **Subjects Table**: Manage the list of subjects and their parameters.
- **Generated Schedule**: Displays the detailed study plan per day.
- **Buttons**:
  - Add Subject
  - Remove Selected Subject
  - Generate Schedule
  - Save CSV
  - Clear Schedule

## How It Works

1. **Input Subjects**: Enter subjects with their difficulty, importance, and topics.
2. **Set Schedule Parameters**: Specify total study days and hours per day.
3. **Generate Schedule**: The app calculates weights for each subject and distributes available time proportionally.
4. **View & Save**: The generated plan is displayed and can be saved in CSV format.

## Dependencies

- Qt 5 or later (Widgets module)
- Standard C++17 (or later) compatible compiler

## Build Instructions

1. Ensure Qt development environment is set up (Qt Creator or qmake + compiler).
2. Place the provided `main.cpp` in your Qt project.
3. Build and run the project.
4. The app window will open allowing you to add subjects and generate schedules.

## Code Highlights

- **Subject class**: Holds subject data and topic list.
- **ScheduleGenerator**: Core logic that assigns study hours based on weights.
- **AddSubjectDialog**: Modal dialog to input subject details.
- **MainWindow**: Main UI handling subject management and schedule display.
- Time formatting converts decimal hours into human-readable "Xh Ym" format.
- Schedule generation allows cyclic topic assignment and respects max 2-hour chunks per task.

## Future Improvements

- Support editing existing subjects.
- Add user profile saving/loading.
- Include break suggestions and study reminders.
- Add prioritization based on deadlines or exam dates.
