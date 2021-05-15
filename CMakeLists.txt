# Requires cmake v3.20 for Boost 1.75.0
cmake_minimum_required(VERSION 3.20)
project(jdock LANGUAGES CXX)

# Enforce C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Create the target and add source files
add_executable(${PROJECT_NAME}
  src/array.cpp
  src/io_service_pool.cpp
  src/main.cpp
  src/random_forest.cpp
  src/random_forest_y.cpp
  src/residue.cpp
  src/safe_counter.cpp
  src/stopwatch.cpp
  src/atom.cpp
  src/ligand.cpp
  src/pka.cpp
  src/random_forest_x.cpp
  src/receptor.cpp
  src/result.cpp
  src/scoring_function.cpp
)

# Use posix thread lib if the system doesn't provide the thread functions
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

# Allow running without .dll or .so
set(Boost_USE_STATIC_LIBS TRUE)
find_package(Boost REQUIRED COMPONENTS program_options)

# Set include path for the target only
target_include_directories(${PROJECT_NAME} PRIVATE
  ${Boost_INCLUDE_DIRS}
)

# Set lib path for the target only
target_link_libraries(${PROJECT_NAME}
  Threads::Threads
  Boost::program_options
)

# Enable cmake --install to copy the binary to system dir
install(
  TARGETS ${PROJECT_NAME}
)