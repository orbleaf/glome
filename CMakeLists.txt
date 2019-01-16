SET(PROJECT_NAME GLOME)

PROJECT(${PROJECT_NAME})
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
#set(CMAKE_VERBOSE_MAKEFILE ON)


set(BUILD_TYPE Release)
#set(BUILD_TYPE RelWithDebInfo)

set(EXECUTABLE_OUTPUT_PATH bin)
set(LIBRARY_OUTPUT_PATH lib)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# optional libraries
find_package(libcgi QUIET)

# flags
add_definitions("-DSTANDALONE_INTERPRETER")
add_definitions("-DENABLE_SSE")
set(CMAKE_CXX_FLAGS
   "${SSE_FLAGS} -O3 -g -std=c++0x -march=native"
#   "${SSE_FLAGS} -O3 -g -std=c++0x -fno-omit-frame-pointer"
)

if (MSVC)
     set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
endif (MSVC)

# Sources files
set(GLOME_SOURCE_FILES
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/asm_streamer.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/il_streamer.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/il1_optimizer.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/il2_optimizer.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/lex.yy.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/lex_proto.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/pkg_encoder.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/pkg_linker.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/scr_generator.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/sem_proto.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/stack_tab.c
  ${PROJECT_SOURCE_DIR}/StackLib/Stack/sym_table.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/midgard.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_framework.c
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_stack.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_aes.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_core.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_crc.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_crypt.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_des.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_mac.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_md5.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_sha1.c
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/src/cr_sha256.c
)



include_directories(
  ${PROJECT_SOURCE_DIR}/StackLib
  ${PROJECT_SOURCE_DIR}/StackLib/Stack
  ${PROJECT_SOURCE_DIR}/StackLib/StackVM
  ${PROJECT_SOURCE_DIR}/StackLib/crypto/inc
) 

# compile main library.
include_directories( ${CSPARSE_INCLUDE_DIR} ${CHOLMOD_INCLUDE_DIR}) 
add_library(libglome ${GLOME_SOURCE_FILES} )

#set_property( TARGET glome APPEND_STRING PROPERTY COMPILE_FLAGS -Wall )


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin") # OSX
    set(BOOST_THREAD_LIBRARY boost_thread-mt)
else()
    set(BOOST_THREAD_LIBRARY boost_thread)
endif()


message("--- compiling glome.")
#add_executable(glome_int ${GLOME_SOURCE_FILES} )
add_executable(glome ${GLOME_SOURCE_FILES} ${PROJECT_SOURCE_DIR}/StackLib/StackVM/vm_interpreter.c)
target_link_libraries(glome ${libcgi_LIBRARIES})

