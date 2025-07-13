# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/ronaldo/esp/esp-idf/components/bootloader/subproject"
  "/home/ronaldo/esp/RTimu_displacement_tracker/esp32c6_rtdt_app/build/bootloader"
  "/home/ronaldo/esp/RTimu_displacement_tracker/esp32c6_rtdt_app/build/bootloader-prefix"
  "/home/ronaldo/esp/RTimu_displacement_tracker/esp32c6_rtdt_app/build/bootloader-prefix/tmp"
  "/home/ronaldo/esp/RTimu_displacement_tracker/esp32c6_rtdt_app/build/bootloader-prefix/src/bootloader-stamp"
  "/home/ronaldo/esp/RTimu_displacement_tracker/esp32c6_rtdt_app/build/bootloader-prefix/src"
  "/home/ronaldo/esp/RTimu_displacement_tracker/esp32c6_rtdt_app/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/ronaldo/esp/RTimu_displacement_tracker/esp32c6_rtdt_app/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/ronaldo/esp/RTimu_displacement_tracker/esp32c6_rtdt_app/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
