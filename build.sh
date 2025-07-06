#!/bin/bash

if ! cmake --build /home/ryosei/myos/cmake-build-debug --target kernel.elf -j 14; then
  echo "Failed to build kernel."
  exit 1
fi

~/osbook/devenv/run_qemu.sh ~/edk2/Build/MikanLoaderX64/DEBUG_CLANG38/X64/Loader.efi cmake-build-debug/kernel.elf


