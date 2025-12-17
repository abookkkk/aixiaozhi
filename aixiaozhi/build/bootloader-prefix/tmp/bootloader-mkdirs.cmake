# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Embed/Espressif/frameworks/esp-idf-v5.3.1/components/bootloader/subproject"
  "D:/Code/ESP32_Projects/Studay_Projects/xiaozhi/aixiaozhi/build/bootloader"
  "D:/Code/ESP32_Projects/Studay_Projects/xiaozhi/aixiaozhi/build/bootloader-prefix"
  "D:/Code/ESP32_Projects/Studay_Projects/xiaozhi/aixiaozhi/build/bootloader-prefix/tmp"
  "D:/Code/ESP32_Projects/Studay_Projects/xiaozhi/aixiaozhi/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Code/ESP32_Projects/Studay_Projects/xiaozhi/aixiaozhi/build/bootloader-prefix/src"
  "D:/Code/ESP32_Projects/Studay_Projects/xiaozhi/aixiaozhi/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Code/ESP32_Projects/Studay_Projects/xiaozhi/aixiaozhi/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Code/ESP32_Projects/Studay_Projects/xiaozhi/aixiaozhi/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
