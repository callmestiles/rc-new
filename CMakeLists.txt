cmake_minimum_required(VERSION 3.16)

project(RC_GUI_NEW VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.5)

qt_add_resources(resources resources.qrc)

qt_add_executable(appRC_GUI_NEW
    PathfindingEngine.h
    PathfindingEngine.cpp
    CarController.h
    CarController.cpp
    main.cpp
    ${resources}
)

qt_add_qml_module(appRC_GUI_NEW
    URI RC_GUI_NEW
    VERSION 1.0
    QML_FILES
        Main.qml
        TopographicalMapView.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appRC_GUI_NEW PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appRC_GUI_NEW
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appRC_GUI_NEW
    PRIVATE Qt6::Quick
)

include(GNUInstallDirs)
install(TARGETS appRC_GUI_NEW
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
