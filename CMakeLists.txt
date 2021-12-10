# Copyright (C) 2019 Intel Corporation.  All rights reserved.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

cmake_minimum_required(VERSION 3.12)

if (NOT WAMR_BUILD_PLATFORM STREQUAL "windows")
  project (wasm_vm_node)
else()
  project (wasm_vm_node CXX ASM)
  enable_language (ASM_MASM)
endif()

find_package(Threads REQUIRED)

find_package(ament_cmake_ros REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(libstatistics_collector REQUIRED)
find_package(rcl REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(rcl_yaml_param_parser REQUIRED)
find_package(rcpputils REQUIRED)
find_package(rcutils REQUIRED)
find_package(rmw REQUIRED)
find_package(rosgraph_msgs REQUIRED)
find_package(rosidl_runtime_cpp REQUIRED)
find_package(rosidl_typesupport_c REQUIRED)
find_package(rosidl_typesupport_cpp REQUIRED)
find_package(statistics_msgs REQUIRED)
find_package(tracetools REQUIRED)

################  runtime settings  ################
string (TOLOWER ${CMAKE_HOST_SYSTEM_NAME} WAMR_BUILD_PLATFORM)
if (APPLE)
  add_definitions(-DBH_PLATFORM_DARWIN)
endif ()

# Reset default linker flags
set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# WAMR features switch

# Set WAMR_BUILD_TARGET, currently values supported:
# "X86_64", "AMD_64", "X86_32", "AARCH64[sub]", "ARM[sub]", "THUMB[sub]",
# "MIPS", "XTENSA", "RISCV64[sub]", "RISCV32[sub]"
if (NOT DEFINED WAMR_BUILD_TARGET)
  if (CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set (WAMR_BUILD_TARGET "AARCH64")
  elseif (CMAKE_SYSTEM_PROCESSOR STREQUAL "riscv64")
    set (WAMR_BUILD_TARGET "RISCV64")
  elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
    # Build as X86_64 by default in 64-bit platform
    set (WAMR_BUILD_TARGET "X86_64")
  else ()
    # Build as X86_32 by default in 32-bit platform
    set (WAMR_BUILD_TARGET "X86_32")
  endif ()
endif ()


if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release)
endif ()

set (WAMR_BUILD_INTERP 1)
set (WAMR_BUILD_AOT 0)
set (WAMR_BUILD_JIT 0)
set (WAMR_BUILD_LIBC_BUILTIN 1)
set (WAMR_BUILD_DUMP_CALL_STACK 1)

if (NOT MSVC)
  set (WAMR_BUILD_LIBC_WASI 1)
endif ()

if (NOT MSVC)
  # linker flags
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -fPIE")
  if (NOT (CMAKE_C_COMPILER MATCHES ".*clang.*" OR CMAKE_C_COMPILER_ID MATCHES ".*Clang"))
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
  endif ()
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wformat -Wformat-security")
  if (WAMR_BUILD_TARGET MATCHES "X86_.*" OR WAMR_BUILD_TARGET STREQUAL "AMD_64")
    if (NOT (CMAKE_C_COMPILER MATCHES ".*clang.*" OR CMAKE_C_COMPILER_ID MATCHES ".*Clang"))
      set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mindirect-branch-register")
    endif ()
  endif ()
endif ()

find_package(Python3 REQUIRED COMPONENTS Interpreter)


# build out vmlib
# set (WAMR_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
set (WAMR_ROOT_DIR ./wasm-micro-runtime)
include (${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)


add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})

################  application related  ################
include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
include_directories(/opt/ros/foxy/include)
link_directories(/opt/ros/foxy/lib)
include (${SHARED_DIR}/utils/uncommon/shared_uncommon.cmake)


add_executable (wasm_vm_node 
  src/main.cpp 
  src/rcl_common.cpp
  src/create_publisher.cpp
  src/publish.cpp  
${UNCOMMON_SHARED_SOURCE})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
# specific order: dependents before dependencies
ament_target_dependencies(${PROJECT_NAME}
  "ament_index_cpp"
  "libstatistics_collector"
  "rcl"
  "rcl_yaml_param_parser"
  "rcpputils"
  "rcutils"
  "builtin_interfaces"
  "rosgraph_msgs"
  "rosidl_typesupport_cpp"
  "rosidl_runtime_cpp"
  "statistics_msgs"
  "tracetools"
)

ament_package()

if (APPLE)
  target_link_libraries (wasm_vm_node vmlib -lm -ldl -lpthread)
else ()
  target_link_libraries (wasm_vm_node vmlib -lm -ldl -lpthread -lrt)
endif ()
