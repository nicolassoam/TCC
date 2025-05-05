cmake_minimum_required(VERSION 3.10)
enable_language(CUDA)
find_package(CUDAToolkit REQUIRED)

project(TCC_FRQ)
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} --expt-relaxed-constexpr")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CUDA_ARCHITECTURES 89)
include_directories(include)

set (PROJECT_DIR ${PROJECT_SOURCE_DIR})
set (DATA ${PROJECT_DIR}/data)
set (PROCESSED ${DATA}/processed)
set (RESULT ${PROJECT_DIR}/result)
set (GPU_ON 1)
set (DEBUG 1)
configure_file(${PROJECT_DIR}/config.h.in ${PROJECT_DIR}/include/config.h)
# add_subdirectory(src/GPU)
# add_subdirectory(src/GPU/gpu_module)
# add_subdirectory(util)

add_executable(main "src/main.cu")
target_sources(main PRIVATE "util/util.cpp")


#add_executable(tree "src/tree.cpp")
target_include_directories(main PRIVATE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
set_target_properties(main PROPERTIES CUDA_ARCHITECTURES "89")

add_compile_options(-prec-div=true -prec-sqrt=true -std=c++20 -Ofast -ltbb -fPIC -march=native -Wall -Wpedantic -Wextra)

