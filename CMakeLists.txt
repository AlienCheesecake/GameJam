cmake_minimum_required(VERSION 3.28.0)
project(gamejam VERSION 0.0.0)


add_subdirectory(apps)
message(STATUS "Collected apps: ${APPLIST}")

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)

foreach(APP ${APPLIST})
  target_link_libraries(${APP} PRIVATE sfml-system sfml-network sfml-graphics sfml-window)
endforeach(APP ${APPLIST})
