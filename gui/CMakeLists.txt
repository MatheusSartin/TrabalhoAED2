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
