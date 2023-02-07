# CMakeList.txt : CMake project for GameDataReader, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("GameDataReader")

find_package(SDL2 REQUIRED)


# Add source to this project's executable.
add_executable (GameDataReader  "GameDataReader.cpp" "GameDataReader.h")

target_link_libraries(GameDataReader SDL2::SDL2 SDL2::SDL2main)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET GameDataReader PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
