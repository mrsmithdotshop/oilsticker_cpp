cmake_minimum_required(VERSION 3.16)
project(OilStickerGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Qt Setup ---------------------------------------------------------------
# Optional: set Qt path if not in CMAKE_PREFIX_PATH
# set(Qt6_DIR "C://Qt//6.10.1//mingw_64//lib//cmake//Qt6")

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui PrintSupport Network)

# Enable Qt auto-processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- Sources, headers, resources -------------------------------------------
set(SOURCES
    main.cpp
    src/OilLabelGUI.cpp
    src/LabelPreview.cpp
)

set(HEADERS
    include/OilLabelGUI.hpp
    include/LabelPreview.hpp
)

set(RESOURCES
    resources/resources.qrc
)

if(WIN32)
    set(WINDOWS_ICON resources/app_icon.rc)
endif()

# --- Executable -------------------------------------------------------------
qt_add_executable(oilstickergui
    WIN32
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
    ${WINDOWS_ICON}
)

target_include_directories(oilstickergui PRIVATE include)

target_link_libraries(oilstickergui PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::PrintSupport
    Qt6::Network
)

# --- Install rules ----------------------------------------------------------
# Executable
install(TARGETS oilstickergui RUNTIME DESTINATION bin)

# Run windeployqt on installed exe
install(CODE "
    message(STATUS \"Running windeployqt on installed exe...\")
    message(STATUS \"CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}\")
    execute_process(
        COMMAND \"C:/Qt/6.10.1/mingw_64/bin/windeployqt.exe\"
                --no-translations
                --no-system-d3d-compiler
                --no-opengl-sw
                --no-quick
                --no-opengl
                --no-qml
                --dir \"dist/bin\"
                \"dist/bin/oilstickergui.exe\"
        RESULT_VARIABLE res
    )
")

# --- Optional: CPack packaging ---------------------------------------------
include(CPack)