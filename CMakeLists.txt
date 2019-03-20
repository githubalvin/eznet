cmake_minimum_required(VERSION 3.8)

project(eznet)

aux_source_directory(src SRC_DIR)

link_libraries(ws2_32)

add_executable(EXE_EZNET ${SRC_DIR})

