cmake_minimum_required(VERSION 3.0)

project(LDrawConverter)

file(GLOB SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cxx)
add_executable(LDrawConverter ${SRC_FILES})

#you may need to change this path to your FBX SDK path
set(FBX_SDK_PATH "C:/Program Files/Autodesk/FBX/FBX SDK/2020.0.1")

add_subdirectory(vendor/portable-file-dialogs portableFD)

target_include_directories(LDrawConverter PRIVATE ${FBX_SDK_PATH}/include ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm/ ${CMAKE_CURRENT_SOURCE_DIR}/vendor/portable-file-dialogs/)
target_link_directories(LDrawConverter PRIVATE ${FBX_SDK_PATH}/lib/x64/debug/)
target_link_libraries(LDrawConverter PRIVATE libfbxsdk-md.lib libxml2-md.lib zlib-md.lib INTERFACE)