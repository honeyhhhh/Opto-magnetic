cmake_minimum_required(VERSION 3.23)

project(OE)

add_definitions(-DWIN32_LEAN_AND_MEAN)

message(STATUS "This is binary dir" ${OE_BINARY_DIR})
message(STATUS "This is Source dir" ${OE_SOURCE_DIR})


# set(CMAKE_C_COMPILER )
# set(CMAKE_CXX_COMPILER )
# 在build目录下，cmake -G "Visual Studio 16 2019 Win64" ../   指定编译器vs以及平台工具集类型x64
# 然后编译 cmake --build . --config Release


aux_source_directory(${OE_SOURCE_DIR}/src SRC_LIST)
message(STATUS "source code: " ${SRC_LIST})



set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)


include_directories(${OE_SOURCE_DIR}/include)
link_directories(${OE_SOURCE_DIR}/lib)


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



add_executable(OE ${SRC_LIST})

# 链接cam第三方库
target_link_libraries(OE PUBLIC pylon::pylon)
# 链接ndi第三方库 oapi.lib ndicapi.lib 及其头文件
target_link_directories(OE PUBLIC ${OE_SOURCE_DIR}/ndlib/msvc ${OE_SOURCE_DIR}/ndicapi/lib)
target_link_libraries(OE PUBLIC oapi ndicapi)
target_include_directories(OE PUBLIC ${OE_SOURCE_DIR}/ndlib/include ${OE_SOURCE_DIR}/ndicapi/include)

