# TrabalhoAED2/gui/CMakeLists.txt

# Find Qt (Works for both Qt5 and Qt6 automatically)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# These three lines tell CMake to handle Qt's special macros (Signals/Slots) and UI files automatically
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Define the GUI executable and its source files
add_executable(aed2_gui
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# Connect the GUI to Qt AND to your backend algorithm library
target_link_libraries(aed2_gui PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    aed2_core
)

# Force the correct C++ standard on the target so MOC compilation uses it
set_target_properties(aed2_gui PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

# Garantir que o diretorio de source do GUI esteja no include path (para o ui_mainwindow.h gerado achar mainwindow.h)
target_include_directories(aed2_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
