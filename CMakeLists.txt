cmake_minimum_required(VERSION 3.16)
project(OilStickerGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt paths
#set(Qt6_DIR "$ENV{HOME}/Qt/6.10.1/macos/lib/cmake/Qt6")
# find_package will use Qt6_DIR

# Find required Qt6 packages
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui PrintSupport Network)

# Enable AUTOMOC and AUTORCC for Q_OBJECT and .qrc
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

# Resource file
set(RESOURCES
    resources/resources.qrc
)

# Add executable
qt_add_executable(oilstickergui
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}      # <-- include resources here
)

# Link Qt libraries
target_link_libraries(oilstickergui PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui Qt6::PrintSupport Qt6::Network)
