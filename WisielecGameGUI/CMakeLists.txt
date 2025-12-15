cmake_minimum_required(VERSION 3.19)
project(WisielecGameGUI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)

qt_standard_project_setup()

qt_add_executable(WisielecGameGUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    startscreen.h startscreen.cpp
    waitingroomscreen.h waitingroomscreen.cpp
)

target_link_libraries(WisielecGameGUI
    PRIVATE
        Qt::Core
        Qt::Widgets
)
target_link_libraries(WisielecGameGUI PRIVATE Qt6::Core)

include(GNUInstallDirs)

install(TARGETS WisielecGameGUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET WisielecGameGUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
