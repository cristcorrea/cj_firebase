# The following lines of boilerplate have to be in your project's CMakeLists
# in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(smart_grow)

set(COMPONENTS_DIR components)
set(EXTRA_COMPONENT_DIRS  "${COMPONENTS_DIR}/DHT"
                          "${COMPONENTS_DIR}/BLUFI"
                          "${COMPONENTS_DIR}/SOIL"
                          "${COMPONENTS_DIR}/RTC"
                          "${COMPONENTS_DIR}/POMP"
                          "${COMPONENTS_DIR}/STORAGE"
                          "${COMPONENTS_DIR}/BH1750"
                          "${COMPONENTS_DIR}/HEADER"
                          "${COMPONENTS_DIR}/HTTP"
                          "${COMPONENTS_DIR}/CJSON")