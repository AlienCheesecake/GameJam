cmake_minimum_required(VERSION 3.28.0)
project(gamejam VERSION 0.0.0)

add_subdirectory(libs)
message(STATUS "Collected libs: ${LIBLIST}")

add_subdirectory(apps)
message(STATUS "Collected apps: ${APPLIST}")

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)

foreach(APP ${APPLIST})
  target_compile_features(${APP} PRIVATE cxx_std_20)
  target_link_libraries(${APP} PRIVATE sfml-system sfml-network sfml-graphics sfml-window sfml-audio ${LIBLIBST})
  target_include_directories(${APP} PRIVATE include)
endforeach(APP ${APPLIST})
