cmake_minimum_required(VERSION 3.31)
project(myos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_LINKER "ld.lld")


add_executable(kernel.elf
        kernel/elf.hpp
        kernel/frame_buffer_config.hpp
        kernel/main.cpp
        kernel/graphics.hpp
        kernel/graphics.cpp
        kernel/font.cpp
        kernel/font.hpp)

include_directories(kernel)
include_directories(MikanLoaderPkg)
target_include_directories(kernel.elf PRIVATE
        "$ENV{BASEDIR}/include/c++/v1"
        "$ENV{BASEDIR}/include"
        "$ENV{BASEDIR}/include/freetype2"
        "$ENV{EDK2DIR}/MdePkg/Include"
        "$ENV{EDK2DIR}/MdePkg/Include/X64"
)
target_compile_definitions(kernel.elf PRIVATE
        __ELF__
        _LDBL_EQ_DBL
        _GNU_SOURCE
        _POSIX_TIMERS
        "EFIAPI='__attribute__((ms_abi))'"
)

target_compile_options(kernel.elf PRIVATE
        -O2
        -Wall
        -g
        --target=x86_64-elf
        -ffreestanding
        -mno-red-zone
        -fno-exceptions
        -fno-rtti
)

target_link_options(kernel.elf PRIVATE
        -fuse-ld=lld
        -Wl,--entry=KernelMain
        -Wl,-z,norelro
        -Wl,--image-base,0x100000
        -Wl,--static
        -nostdlib
        -nostartfiles
        -L$ENV{BASEDIR}/lib
)

target_link_libraries(kernel.elf PRIVATE
        -lc
        -lc++
        -lc++abi
)

# Compile and link font data
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/makefont.py -o hankaku.bin ../kernel/hankaku.txt
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

execute_process(
        COMMAND objcopy -I binary -O elf64-x86-64 -B i386:x86-64 hankaku.bin hankaku.o
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_link_libraries(kernel.elf PRIVATE
        ${CMAKE_BINARY_DIR}/hankaku.o
)
