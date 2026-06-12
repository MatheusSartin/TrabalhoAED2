# TrabalhoAED2/core/CMakeLists.txt

# TARGET 1: The Pre-processor (C Program)
# This compiles your standalone .osm to .poly converter.
add_executable(osm_converter LeArqOSM_e_GeraArqPoly.c)
target_link_libraries(osm_converter PRIVATE m)

# TARGET 2: The Algorithm Library (C++)
# We compile parser and dijkstra into a static library (libaed2_core.a).
# This allows both the CLI and the GUI to share this exact same code.
add_library(aed2_core STATIC
    dijkstra.cpp
    parser.cpp
)
# Make sure anything linking to this library can find projects.h
target_include_directories(aed2_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# TARGET 3: Your Terminal Tester (C++)
# This preserves your current cli_main.cpp so you can still test in the terminal.
add_executable(cli_app cli_main.cpp)

# Link the terminal app to the algorithm library
target_link_libraries(cli_app PRIVATE aed2_core)
