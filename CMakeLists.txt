cmake_minimum_required(VERSION 3.16)
project(OilStickerGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Qt6_DIR "$ENV{HOME}/Qt/6.10.1/macos/lib/cmake/Qt6")

# Qt Universal
set(CMAKE_PREFIX_PATH "$ENV{HOME}/Qt/6.10.1/lib/cmake")

# Find required Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui PrintSupport)

# Enable AUTOMOC for Q_OBJECT classes
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(include)

# Source files
set(SOURCES
    main.cpp
    src/OilLabelGUI.cpp
    src/LabelPreview.cpp
)

# Headers
set(HEADERS
    include/OilLabelGUI.hpp
    include/LabelPreview.hpp
)

# Add executable
qt_add_executable(oilstickergui
    ${SOURCES}
    ${HEADERS}
)

# Link Qt libraries
target_link_libraries(oilstickergui PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::PrintSupport)
