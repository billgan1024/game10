cmake_minimum_required(VERSION 4.1)
include(FetchContent)

macro(fetch NAME)
   FetchContent_Declare(${NAME} ${ARGN})
   FetchContent_MakeAvailable(${NAME})
endmacro()


# 4244, 4267, 4305, 4838: conversion warnings
# 4865: https://learn.microsoft.com/en-us/cpp/build/reference/zc-enumtypes?view=msvc-170
add_compile_options(/std:c++latest /Zc:preprocessor /wd4244 /wd4267 /wd4305 /wd4838 /wd4865)
set(SLANG_VERSION "2025.24.1")
fetch(slang URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-x86_64.zip)
fetch(DirectXTK GIT_REPOSITORY https://github.com/microsoft/DirectXTK GIT_TAG da4c330)
fetch(tinygltf GIT_REPOSITORY https://github.com/syoyo/tinygltf.git GIT_TAG 37250b3)

link_libraries(d3d11 d3dcompiler DirectXTK tinygltf)

add_executable(game 
# WIN32
main.cpp)

set(slang ${slang_SOURCE_DIR}/bin/slangc.exe)
file(READ shaders.hlsl shaders)
# https://stackoverflow.com/questions/72098319/cmake-string-replace-removes-semi-colon
string(REGEX REPLACE "(vec[234]) ([a-zA-Z][a-zA-Z0-9]+)([,)])" "\\1 \\2 : \\2\\3" shaders "${shaders}")
file(WRITE out/shaders.hlsl "${shaders}")
file(COPY_FILE shared.h out/shared.h)

string(REGEX MATCHALL "[vp]s[a-zA-Z]+" matches "${shaders}")

# use slang because fxc doesn't accept ConstantBuffer syntax

foreach(match ${matches})
   string(SUBSTRING ${match} 0 2 stage)
   add_custom_command(TARGET game POST_BUILD
      COMMAND ${slang} -matrix-layout-row-major -profile ${stage}_5_0 out/shaders.hlsl -entry ${match} -o out/${match}.hlsl
      COMMAND ${slang} -matrix-layout-row-major -profile ${stage}_5_0 out/shaders.hlsl -entry ${match} -o out/${match}.dxbc 
      COMMAND ${slang} -matrix-layout-row-major -profile ${stage}_5_0 out/shaders.hlsl -entry ${match} -o out/${match}.dxbc-asm
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
   )
endforeach()

set(spritefont ${DirectXTK_SOURCE_DIR}/MakeSpriteFont/bin/Debug/MakeSpriteFont.exe)
add_custom_command(TARGET game POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:game>/game.exe ${CMAKE_SOURCE_DIR}
   COMMAND msbuild ${DirectXTK_SOURCE_DIR}/MakeSpriteFont/MakeSpriteFont.csproj
   COMMAND ${spritefont} Inter inter.spritefont
   COMMAND ${spritefont} "Fira Code" fira.spritefont
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
