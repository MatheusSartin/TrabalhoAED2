# TrabalhoAED2/gui/CMakeLists.txt

# ── Detecção automática do Qt ─────────────────────────────────────────────────
# Define Qt6_DIR / Qt5_DIR diretamente com o caminho exato do cmake config.
# Garante a resolução confiável entre distribuições e versões do CMake.

if(EXISTS "/usr/bin/qmake6")
    set(QMAKE_EXECUTABLE "/usr/bin/qmake6" CACHE FILEPATH "Caminho para o qmake" FORCE)
elseif(EXISTS "/usr/bin/qmake")
    set(QMAKE_EXECUTABLE "/usr/bin/qmake" CACHE FILEPATH "Caminho para o qmake" FORCE)
else()
    find_program(QMAKE_EXECUTABLE NAMES qmake6 qmake
        HINTS /usr/bin /usr/local/bin /opt/homebrew/bin
        NO_CMAKE_FIND_ROOT_PATH)
endif()

# Constrói lista de candidatos para o diretório cmake do Qt a partir do qmake
if(QMAKE_EXECUTABLE AND NOT QMAKE_EXECUTABLE MATCHES "-NOTFOUND")
    execute_process(
        COMMAND ${QMAKE_EXECUTABLE} -query QT_INSTALL_LIBS
        OUTPUT_VARIABLE _QT_LIBS
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    execute_process(
        COMMAND ${QMAKE_EXECUTABLE} -query QT_INSTALL_PREFIX
        OUTPUT_VARIABLE _QT_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(_QT_LIBS)
        message(STATUS "Qt via qmake: libs=${_QT_LIBS}, prefix=${_QT_PREFIX}")
    endif()
endif()

# Lista de diretórios candidatos onde Qt6Config.cmake pode estar
set(_QT6_CANDIDATES
    "${_QT_LIBS}/cmake/Qt6"
    "/usr/lib/cmake/Qt6"
    "/usr/lib64/cmake/Qt6"
    "/usr/lib/x86_64-linux-gnu/cmake/Qt6"
    "/usr/lib/aarch64-linux-gnu/cmake/Qt6"
    "/opt/homebrew/opt/qt6/lib/cmake/Qt6"
    "/usr/local/opt/qt6/lib/cmake/Qt6"
    "$ENV{HOME}/Qt/6.5.0/gcc_64/lib/cmake/Qt6"
    "$ENV{HOME}/Qt/6.7.0/gcc_64/lib/cmake/Qt6"
)
set(_QT5_CANDIDATES
    "${_QT_LIBS}/cmake/Qt5"
    "/usr/lib/cmake/Qt5"
    "/usr/lib64/cmake/Qt5"
    "/usr/lib/x86_64-linux-gnu/cmake/Qt5"
    "/opt/homebrew/opt/qt5/lib/cmake/Qt5"
    "$ENV{HOME}/Qt/5.15.2/gcc_64/lib/cmake/Qt5"
)

# Seta Qt6_DIR para o primeiro candidato válido encontrado
if(NOT Qt6_DIR OR Qt6_DIR MATCHES "-NOTFOUND")
    foreach(_candidate ${_QT6_CANDIDATES})
        if(EXISTS "${_candidate}/Qt6Config.cmake")
            set(Qt6_DIR "${_candidate}" CACHE PATH "Diretorio cmake do Qt6" FORCE)
            message(STATUS "Qt6_DIR definido como: ${Qt6_DIR}")
            break()
        endif()
    endforeach()
endif()

# Seta Qt5_DIR como fallback
if(NOT Qt5_DIR OR Qt5_DIR MATCHES "-NOTFOUND")
    foreach(_candidate ${_QT5_CANDIDATES})
        if(EXISTS "${_candidate}/Qt5Config.cmake")
            set(Qt5_DIR "${_candidate}" CACHE PATH "Diretorio cmake do Qt5" FORCE)
            message(STATUS "Qt5_DIR definido como: ${Qt5_DIR}")
            break()
        endif()
    endforeach()
endif()

# Também mantém CMAKE_PREFIX_PATH para componentes adicionais
if(Qt6_DIR)
    get_filename_component(_parent1 "${Qt6_DIR}/.." ABSOLUTE)
    get_filename_component(_parent2 "${Qt6_DIR}/../.." ABSOLUTE)
    get_filename_component(_parent3 "${Qt6_DIR}/../../.." ABSOLUTE)
    get_filename_component(_parent4 "${Qt6_DIR}/../../../.." ABSOLUTE)
    list(APPEND CMAKE_PREFIX_PATH "${_parent1}" "${_parent2}" "${_parent3}" "${_parent4}")
endif()
if(Qt5_DIR)
    get_filename_component(_parent1 "${Qt5_DIR}/.." ABSOLUTE)
    get_filename_component(_parent2 "${Qt5_DIR}/../.." ABSOLUTE)
    get_filename_component(_parent3 "${Qt5_DIR}/../../.." ABSOLUTE)
    get_filename_component(_parent4 "${Qt5_DIR}/../../../.." ABSOLUTE)
    list(APPEND CMAKE_PREFIX_PATH "${_parent1}" "${_parent2}" "${_parent3}" "${_parent4}")
endif()
if(CMAKE_PREFIX_PATH)
    list(REMOVE_DUPLICATES CMAKE_PREFIX_PATH)
endif()
if(DEFINED ENV{QT_DIR})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{QT_DIR}")
endif()

# ── Busca Qt6 ou Qt5 ─────────────────────────────────────────────
find_package(Qt6 QUIET COMPONENTS Widgets)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    message(STATUS "Usando Qt6")
else()
    find_package(Qt5 QUIET COMPONENTS Widgets)
    if(Qt5_FOUND)
        set(QT_VERSION_MAJOR 5)
        message(STATUS "Usando Qt5")
    else()
        message(FATAL_ERROR
            "\n===================================================\n"
            "  Erro: Qt não encontrado!\n"
            "  Instale o Qt com um dos comandos abaixo:\n\n"
            "  Arch Linux:   sudo pacman -S qt6-base\n"
            "  Ubuntu/Debian: sudo apt install qt6-base-dev\n"
            "  Fedora:        sudo dnf install qt6-qtbase-devel\n\n"
            "  Ou defina a variável: export QT_DIR=/caminho/para/qt\n"
            "===================================================\n")
    endif()
endif()

# ── Macros do Qt ────────────────────────────────────────────────────
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# ── Executável ────────────────────────────────────────────────────────────────
add_executable(aed2_gui
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(aed2_gui PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    aed2_core
)

# Força o padrão C++20 para compilação e MOC
set_target_properties(aed2_gui PROPERTIES CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)

# Garante o diretório do fonte do GUI no include path
target_include_directories(aed2_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
