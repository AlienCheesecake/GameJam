set(LIB
  mmedia)

set(SRCS
AssetManager.cc
Animator.cc
MusicPlayer.cc
MusicStackOfQueues.cc
)

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
add_library(${LIB} ${SRCS})
target_compile_features(${LIB} PRIVATE cxx_std_20)
target_include_directories(${LIB} PRIVATE "${CMAKE_SOURCE_DIR}/include/${LIB}")
message(STATUS "${LIB} include : ${CMAKE_SOURCE_DIR}/include/${LIB}")
target_link_libraries(${LIB} PUBLIC sfml-system sfml-network sfml-graphics sfml-window sfml-audio)