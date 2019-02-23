cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(minivitatv)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -nostdlib")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_executable(shell
  shell.c
)

target_link_libraries(shell
  taiHEN_stub
)

vita_create_self(minivitatv.suprx shell CONFIG exports.yml UNSAFE)

add_executable(kernel
  kernel.c
)

target_link_libraries(kernel
  SceIofilemgrForDriver_stub
  SceSysclibForDriver_stub
  SceThreadmgrForDriver_stub
  taihenForKernel_stub
  taihenModuleUtils_stub
  SceSysmemForDriver_stub
  SceSysrootForDriver_stub
  SceProcessmgrForDriver_stub
)

vita_create_self(minivitatv.skprx kernel CONFIG exports.yml UNSAFE)
