# Adexa(Adaptive Study Schedule Generator)
A simple Qt Widgets application that generates personalized study schedules based on subjects, their difficulty, importance, and topics. This desktop app helps students organize their study time effectively across multiple days and hours per day.
## 🛠 Build & Run with CMake (VS Code)

Follow these steps if you want to build the project using CMake inside VS Code:

1. Install prerequisites  
   - Install Qt (make sure qmake and CMake can find it).  
   - Install a compiler:  
     - MinGW or MSVC (Windows)  
     - g++/clang (Linux/macOS).  

2. Project setup  
   - Place your main.cpp file in the project folder.  
   - Create a CMakeLists.txt file in the same folder with the build instructions.  

3. Open in VS Code  
   - Open the folder in VS Code.  
   - Install the CMake Tools extension (if not already installed).  

4. Configure CMake  
   - VS Code will detect CMakeLists.txt.  
   - Select the Qt kit/toolchain (for example: MinGW 64-bit with Qt).  
   - Run CMake: Configure to generate build files.  

5. Build the project  
   - Press Ctrl+Shift+B (or use CMake: Build).  
   - This will compile the executable (e.g., StudyScheduleGenerator).  

6. Run the project  
   - Press F5 (or CMake: Run Without Debugging).  
   - The Qt application window will launch.  

7. Done  
   - You now have the Study Schedule Generator running via CMake inside VS Code.
