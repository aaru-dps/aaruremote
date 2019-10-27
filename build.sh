#!/usr/bin/env bash
cmake .
make
if [ -f $DEVKITPRO/wii.cmake ]; then
  rm -R ./*/CMakeFiles CMakeCache.txt CMakeFiles ./*/Makefile Makefile ./*/cmake_install.cmake cmake_install.cmake
  . $DEVKITPRO/ppcvars.sh
  cmake . -DCMAKE_TOOLCHAIN_FILE=$DEVKITPRO/wii.cmake
  make
fi

# Clean
rm -R ./*/CMakeFiles CMakeCache.txt CMakeFiles ./*/Makefile Makefile ./*/cmake_install.cmake cmake_install.cmake
