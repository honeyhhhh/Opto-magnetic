cmake_minimum_required(VERSION 3.23)

project(OE LANGUAGES CXX)

add_definitions(-DWIN32_LEAN_AND_MEAN)
add_compile_options(-DWIN32 -DSTRICT -D_CONSOLE -DPLATFORM_X86)
add_compile_options(/wd4430)
add_compile_options(/wd4819)


# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -m32")
# set(CMAKE_CXX_FLAGS "-m32")


message(STATUS "This is binary dir" ${OE_BINARY_DIR})
message(STATUS "This is Source dir" ${OE_SOURCE_DIR})


# set(CMAKE_C_COMPILER )
# set(CMAKE_CXX_COMPILER )
# 在build目录下，cmake -G "Visual Studio 16 2019" -A Win32 ../   指定编译器vs以及平台工具集类型x64
# 然后编译 cmake --build . --config Release


aux_source_directory(${OE_SOURCE_DIR}/src SRC_LIST)
message(STATUS "source code: " ${SRC_LIST})



set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)


include_directories(${OE_SOURCE_DIR}/include ${OE_SOURCE_DIR}/ndlib/include ${OE_SOURCE_DIR}/ndicapi/include)
link_directories(${OE_SOURCE_DIR}/lib ${OE_SOURCE_DIR}/ndlib/lib32 ${OE_SOURCE_DIR}/ndicapi/lib32)


set(EXECUTABLE_OUTPUT_PATH ${OE_SOURCE_DIR}/target)

# display where CMake searches .cmake file
# set(CMAKE_FIND_DEBUG_MODE ON)
# 导入 pylon
find_package(pylon 7.1.0 REQUIRED)
if(pylon_FOUND)
    message(STATUS "found pylon: " ${pylon_DIR})
else(pylon_FOUND)
    message(FATAL_ERROR "pylon not found!")
endif(pylon_FOUND)

find_package(Threads)
if(Threads_FOUND)
    message(STATUS "found threads")
    # LIST(APPEND ${PROJECT_NAME}_LIBS ${CMAKE_THREAD_LIBS_INIT})
    # message(STATUS ${CMAKE_THREAD_LIBS_INIT})
    message(STATUS ${CMAKE_USE_WIN32_THREADS_INIT})
    # message(STATUS ${CMAKE_USE_PTHREADS_INIT})
    # message(STATUS ${CMAKE_HP_PTHREADS_INIT})

else(Threads_FOUND)
    message(FATAL_ERROR "threads not found!")
endif(Threads_FOUND)




add_executable(OE ${SRC_LIST})

target_link_libraries(OE PUBLIC Threads::Threads wsock32 ws2_32 ndicapi)

# 链接cam第三方库
target_link_libraries(OE PUBLIC pylon::pylon)
# 链接ndi第三方库 oapi.lib ndicapi.lib  extern "C"
target_link_libraries(OE PUBLIC oapi certus_aux)


