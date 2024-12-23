cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# project(cv_cpp)
project(cv_cpp VERSION 1.0 LANGUAGES CXX)

# Find OpenCV package
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

# Create executable
add_executable(cv_cpp main.cpp)
add_executable(cv_read read_data.cpp)
add_executable(cv_basic_operations basic_operations.cpp)
add_executable(cv_draw_data draw_data.cpp)

# Link OpenCV libraries
target_link_libraries(cv_cpp ${OpenCV_LIBS})
target_link_libraries(cv_read ${OpenCV_LIBS})
target_link_libraries(cv_basic_operations ${OpenCV_LIBS})
target_link_libraries(cv_draw_data ${OpenCV_LIBS})

# Include OpenCV headers
# target_include_directories(cv_cpp PRIVATE ${OpenCV_INCLUDE_DIRS})

# Copy Resources folder to the build directory
file(COPY ${CMAKE_SOURCE_DIR}/Resources DESTINATION ${CMAKE_BINARY_DIR}/Debug)