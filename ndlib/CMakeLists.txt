cmake_minimum_required(VERSION 3.23)
project(ADDLIB)
set(LIB_SRC certus_aux.c)

add_definitions(-DWIN32_LEAN_AND_MEAN)
add_definitions(-DWIN32 -DSTRICT -D_CONSOLE -DPLATFORM_X86)

include_directories(${ADDLIB_SOURCE_DIR}/include)
link_directories(${ADDLIB_SOURCE_DIR}/lib32)



add_library(certus_aux_shared SHARED ${LIB_SRC})
add_library(certus_aux_static STATIC ${LIB_SRC})

target_link_libraries(certus_aux_shared oapi)
target_link_libraries(certus_aux_static oapi)


set_target_properties(certus_aux_static PROPERTIES OUTPUT_NAME "certus_aux")
set_target_properties(certus_aux_shared PROPERTIES OUTPUT_NAME "certus_aux")






