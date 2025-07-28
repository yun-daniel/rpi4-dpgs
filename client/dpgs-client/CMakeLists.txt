cmake_minimum_required(VERSION 3.19)
project(Final LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)
qt_standard_project_setup()
qt6_add_resources(APP_RESOURCES resource.qrc)

message(STATUS "Configuring for Windows")

set(GSTREAMER_DIR "C:/Program Files/gstreamer/1.0/msvc_x86_64")
include_directories(
    "${GSTREAMER_DIR}/include/gstreamer-1.0"
    "${GSTREAMER_DIR}/include/glib-2.0"
    "${GSTREAMER_DIR}/lib/glib-2.0/include"
)
link_directories("${GSTREAMER_DIR}/lib")

find_package(OpenSSL REQUIRED)

set(GSTREAMER_LIBS
    gstreamer-1.0
    gstapp-1.0
    gobject-2.0
    glib-2.0
    gio-2.0
)

qt_add_executable(Final
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    tls_helper.c tls_helper.h
    parkingmapwidget.cpp parkingmapwidget.h parkingmapwidget.ui
    page1st.cpp page1st.h page1st.ui
    page2nd.cpp page2nd.h page2nd.ui
    cctvstreamthread.cpp cctvstreamthread.h

    ${APP_RESOURCES}
    slotdata.h
)

target_link_libraries(Final
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Network
        ${GSTREAMER_LIBS}
        OpenSSL::SSL
        OpenSSL::Crypto
)
