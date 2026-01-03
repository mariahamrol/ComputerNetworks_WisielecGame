# cmake_minimum_required(VERSION 3.19)
# project(WisielecGameGUI LANGUAGES CXX)

# find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
# find_package(Qt6 REQUIRED COMPONENTS Core)

# qt_standard_project_setup()

# qt_add_executable(WisielecGameGUI
#     WIN32 MACOSX_BUNDLE
#     main.cpp
#     mainwindow.cpp
#     mainwindow.h
#     mainwindow.ui
#     startscreen.h startscreen.cpp
#     lobbyscreen.h lobbyscreen.cpp
#     gamecontroller.h gamecontroller.cpp
#     waitingroomscreen.h waitingroomscreen.cpp
#     gamescreen.h gamescreen.cpp
# )

# target_include_directories(WisielecGameGUI PRIVATE
#     ${CMAKE_CURRENT_SOURCE_DIR}
#     ${CMAKE_CURRENT_SOURCE_DIR}/..
# )

# target_link_libraries(WisielecGameGUI
#     PRIVATE
#         Qt::Core
#         Qt::Widgets
# )

# target_link_libraries(WisielecGameGUI PRIVATE Qt6::Core)

# include(GNUInstallDirs)

# install(TARGETS WisielecGameGUI
#     BUNDLE  DESTINATION .
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
# )

# qt_generate_deploy_app_script(
#     TARGET WisielecGameGUI
#     OUTPUT_SCRIPT deploy_script
#     NO_UNSUPPORTED_PLATFORM_ERROR
# )
# install(SCRIPT ${deploy_script})
cmake_minimum_required(VERSION 3.16)

project(WisielecGameGUI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(WisielecGameGUI
    main.cpp
    mainwindow.cpp
    startscreen.cpp
    waitingroomscreen.cpp
    gamescreen.cpp
    gameendscreen.cpp
    lobbyscreen.cpp
    gamecontroller.cpp
    hangman.cpp
    adminloginscreen.cpp
    adminlobbyscreen.cpp
    ../client/ClientConnection.cpp
    ../include/net.cpp
)

target_include_directories(WisielecGameGUI
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(WisielecGameGUI
    PRIVATE
        Qt6::Widgets
)

set_target_properties(WisielecGameGUI PROPERTIES
    WIN32_EXECUTABLE TRUE
)
