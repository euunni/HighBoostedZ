cmake_minimum_required(VERSION 3.10)
project(CMSAnalysis)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ROOT package
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree TreePlayer)
include(${ROOT_USE_FILE})

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add library for the framework
add_library(AnalysisLib SHARED
  src/NtupleReader.cc
  src/Analyzer.cc
  src/Muon.cc
)

# Link ROOT libraries to the framework library
target_link_libraries(AnalysisLib ${ROOT_LIBRARIES})

# Add the executable
add_executable(Analyzer src/main.cc)

# Link the executable with the framework library
target_link_libraries(Analyzer AnalysisLib ${ROOT_LIBRARIES}) 