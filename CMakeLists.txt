cmake_minimum_required(VERSION 2.8)
project (ParsaBot)
find_package( OpenCV REQUIRED )

file(GLOB_RECURSE ParsaBot_SOURCES "src/*.cpp")
file(GLOB_RECURSE ParsaBot_HEADERS "src/*.h")

set (ParsaBot_INCLUDE_DIRS "")
foreach (_headerFile ${ParsaBot_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND ParsaBot_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES ParsaBot_INCLUDE_DIRS)

include_directories(${ParsaBot_INCLUDE_DIRS})
add_executable (ParsaBot ${ParsaBot_SOURCES})
target_link_libraries( ParsaBot ${OpenCV_LIBS} )
