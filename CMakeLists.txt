# Check cmake version meets our requirements
cmake_minimum_required(VERSION 3.3)

# Declare project, which will configure compiler for us
project(TKReconstruct)

#Finds ROOT and Falaise packages and defines cmake variables
find_package(Falaise REQUIRED)
find_package(ROOT    REQUIRED)

include_directories(include ${Bayeux_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cpp" "src/dicts/*.cpp")
add_library(TKReconstruct SHARED ${SOURCES} )

target_link_libraries(TKReconstruct PUBLIC Falaise::Falaise ${ROOT_LIBRARIES} ROOT::Geom)  

install(TARGETS TKReconstruct DESTINATION ${CMAKE_INSTALL_PLUGINDIR})

configure_file("TKReconstructPipeline.conf.in" "TKReconstructPipeline.conf" @ONLY)
