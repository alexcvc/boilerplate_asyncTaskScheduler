cmake_minimum_required(VERSION 3.15)
project(stack
   VERSION 1.0.0
   DESCRIPTION "C library for testing handler usage"
   LANGUAGES CXX C
)

add_library(stack STATIC cstack.c cstack.h
)

add_library(stack::stack ALIAS stack)

target_include_directories(stack PUBLIC
   $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
   $<INSTALL_INTERFACE:include>
)