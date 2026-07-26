cmake_minimum_required(VERSION 3.14)

project(LJXController LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Gui)

add_executable(LJXController
  main.cpp
  resources.qrc
  controller.h controller.cpp
  logger.cpp logger.h
)
target_link_libraries(LJXController Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Gui)

# set icon for app exe
if(WIN32)
    target_sources(LJXController PRIVATE "${CMAKE_SOURCE_DIR}/app_icon.rc")
endif()

# add logging spdlog library
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/spdlog")
message("${CMAKE_CURRENT_SOURCE_DIR}/spdlog")
target_link_libraries(LJXController spdlog)

include_directories ("${CMAKE_CURRENT_SOURCE_DIR}/LJXLib/include")
target_link_directories(LJXController PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/LJXLib/x64")
target_link_libraries(LJXController  "${CMAKE_CURRENT_SOURCE_DIR}/LJXLib/x64/LJX8_IF.lib")

# copy connection_config.json from project directory to build dir
file(GLOB CONFIG_FILES "${CMAKE_SOURCE_DIR}/config/*")
set(DEST_DIR "${CMAKE_BINARY_DIR}/config")
add_custom_command(
    TARGET LJXController POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${DEST_DIR}
    COMMENT "Creating destination directory"
)

foreach(SOURCE_FILE ${CONFIG_FILES})
    get_filename_component(FILE_NAME ${SOURCE_FILE} NAME)
    add_custom_command(
        TARGET LJXController POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE_FILE} ${DEST_DIR}/${FILE_NAME}
        COMMENT "Copying ${FILE_NAME} to executable directory"
        VERBATIM
    )
endforeach()

include(GNUInstallDirs)
install(TARGETS LJXController
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
add_subdirectory(LJXConfiguration)