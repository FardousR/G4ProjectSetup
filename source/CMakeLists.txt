# ----------------------------------------------------------------------------
# 1. Project Setup and CMake Minimums
# ----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(LETrix)

# Geant4 11+ requires C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------------------------------------------------------------
# 2. Find Geant4 Package
# ----------------------------------------------------------------------------
option(WITH_GEANT4_UIVIS "Build LETrix with Geant4 UI and Vis drivers" ON)

if(WITH_GEANT4_UIVIS)
    find_package(Geant4 REQUIRED ui_all vis_all)
else()
    find_package(Geant4 REQUIRED)
endif()

# ----------------------------------------------------------------------------
# 3. Locate Sources and Headers
# ----------------------------------------------------------------------------
# Since CMakeLists.txt is IN the source folder, we look locally
file(GLOB sources CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/include/*.hh)

# ----------------------------------------------------------------------------
# 4. Define the Executable and Link Libraries
# ----------------------------------------------------------------------------
# LETrix.cc is right next to CMakeLists.txt
add_executable(LETrix LETrix.cc ${sources} ${headers})

# Include headers from the local include folder
target_include_directories(LETrix PRIVATE ${PROJECT_SOURCE_DIR}/include)

# Link against Geant4 libraries
target_link_libraries(LETrix PUBLIC ${Geant4_LIBRARIES})

# ----------------------------------------------------------------------------
# 5. Copy Macro Scripts to the Build Directory
# ----------------------------------------------------------------------------
# We tell CMake to look ONE LEVEL UP (..) and into the macros folder
file(COPY
    ${PROJECT_SOURCE_DIR}/../macros/init_vis.mac
    ${PROJECT_SOURCE_DIR}/../macros/run.mac
    ${PROJECT_SOURCE_DIR}/../macros/vis.mac
    DESTINATION ${PROJECT_BINARY_DIR}
)
