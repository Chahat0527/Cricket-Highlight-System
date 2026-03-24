cmake_minimum_required(VERSION 3.10)
project(CricketHighlights)

set(CMAKE_CXX_STANDARD 11)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Add executable
add_executable(cricket_highlights 
    src/CricketAnalyzer.cpp 
    src/main.cpp
)

# Link libraries
target_link_libraries(cricket_highlights ${OpenCV_LIBS})

# For network support on Linux/Mac
if(UNIX)
    target_link_libraries(cricket_highlights pthread)
endif()

# For network support on Windows
if(WIN32)
    target_link_libraries(cricket_highlights ws2_32)
endif()

# Compiler optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(cricket_highlights PRIVATE -O3)
endif()