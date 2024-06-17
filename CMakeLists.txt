# 指定cmake的最低版本
cmake_minimum_required(VERSION 3.20.0)
# 工程名称,支持的语言
PROJECT(RatTracing C CXX)

# 开启OpenMP功能
find_package(OpenMP REQUIRED)

# 可执行文件的名称、相关的源文件
ADD_EXECUTABLE(main main.cpp "rtw_stb_image.h"  "camera.h" "perlin.h" "quad.h" "constant_medium.h" "onb.h" "pdf.h")

# 编译时需要开启OpenMP支持
target_link_libraries(main
  PUBLIC
    OpenMP::OpenMP_CXX
  )