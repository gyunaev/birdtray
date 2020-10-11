cmake_minimum_required(VERSION 3.1.0)

if(POLICY CMP0092)
    cmake_policy(SET CMP0092 NEW)
endif()

project(Birdtray CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)
find_package(Qt5Svg CONFIG REQUIRED)
find_package(Qt5LinguistTools CONFIG QUIET)
set(REQUIRED_MODULES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network Qt5::Svg)

set(PLATFORM_SOURCES)
set(PLATFORM_HEADERS)
set(EXECUTABLE_OPTIONS)

option(OPT_THUNDERBIRD_CMDLINE "Default Thunderbird startup command-line" [])
option(OPT_THUNDERBIRD_PROFILE "Default Thunderbird profile path" [])

if(OPT_THUNDERBIRD_CMDLINE)
    message(STATUS "Setting Thunderbird command-line to ${OPT_THUNDERBIRD_CMDLINE}")
    add_definitions(-DOPT_THUNDERBIRD_CMDLINE="${OPT_THUNDERBIRD_CMDLINE}")
endif(OPT_THUNDERBIRD_CMDLINE)

if(OPT_THUNDERBIRD_PROFILE)
    message(STATUS "Setting Thunderbird profile path to ${OPT_THUNDERBIRD_PROFILE}")
    add_definitions(-DOPT_THUNDERBIRD_PROFILE="${OPT_THUNDERBIRD_PROFILE}")
endif(OPT_THUNDERBIRD_PROFILE)

if(NOT Qt5LinguistTools_FOUND)
    message(STATUS "Qt5LinguistTools package not found: Translations will not be available")
endif(NOT Qt5LinguistTools_FOUND)

if(WIN32)
    set(EXECUTABLE_OPTIONS WIN32)
    ENABLE_LANGUAGE(RC)
    add_definitions(-DUNICODE -DNOMINMAX)
    set(PLATFORM_SOURCES src/windowtools_win.cpp src/birdtrayeventfilter.cpp
            src/processhandle.cpp)
    set(PLATFORM_EXE_SOURCES src/res/birdtray.rc)
    set(PLATFORM_HEADERS src/windowtools_win.h src/birdtrayeventfilter.h src/processhandle.h)
    find_package(Qt5WinExtras CONFIG REQUIRED)
    list(APPEND REQUIRED_MODULES Qt5::WinExtras)
else()
    find_package(Qt5X11Extras REQUIRED)
    find_library(X11 X11)
    list(APPEND REQUIRED_MODULES Qt5::X11Extras ${X11})
    set(PLATFORM_SOURCES src/windowtools_x11.cpp)
    set(PLATFORM_HEADERS src/windowtools_x11.h)
endif(WIN32)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
add_definitions(-DQT_DEPRECATED_WARNINGS)
if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
    # AUTO_UIC places generated header files in CMAKE_CURRENT_BINARY_DIR before 3.8.0.
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

set(SOURCES
        src/colorbutton.cpp
        src/dialogaddeditnewemail.cpp
        src/dialogsettings.cpp
        src/dialoglogoutput.cpp
        src/modelaccounttree.cpp
        src/modelnewemails.cpp
        src/morkparser.cpp
        src/setting_newemail.cpp
        src/settings.cpp
        src/trayicon.cpp
        src/unreadmonitor.cpp
        src/utils.cpp
        src/log.cpp
        src/windowtools.cpp
        src/autoupdater.cpp
        src/updatedialog.cpp
        src/updatedownloaddialog.cpp
        src/mailaccountdialog.cpp
        src/birdtrayapp.cpp
        src/dialogaddeditnewemail.ui
        src/dialogsettings.ui
        src/updatedialog.ui
        src/updatedownloaddialog.ui
        src/dialoglogoutput.ui
        src/mailaccountdialog.ui)

set(HEADERS
        src/colorbutton.h
        src/dialogaddeditnewemail.h
        src/dialoglogoutput.h
        src/dialogsettings.h
        src/log.h
        src/modelaccounttree.h
        src/modelnewemails.h
        src/morkparser.h
        src/setting_newemail.h
        src/settings.h
        src/trayicon.h
        src/unreadmonitor.h
        src/utils.h
        src/version.h
        src/windowtools.h
        src/autoupdater.h
        src/updatedialog.h
        src/updatedownloaddialog.h
        src/mailaccountdialog.h
        src/birdtrayapp.h)

set(RESOURCES src/resources.qrc)
source_group("Resources" FILES ${RESOURCES})
qt5_add_resources(RESOURCES_SOURCES ${RESOURCES})
set_source_files_properties(${RESOURCES_SOURCES} PROPERTIES GENERATED ON)

if(Qt5LinguistTools_FOUND)
    set(MAIN_TRANSLATION_FILES
            src/translations/main_en.ts
            src/translations/main_de.ts
            src/translations/main_nl.ts
            src/translations/main_it.ts
            src/translations/main_ru.ts
            src/translations/main_pt.ts
            src/translations/main_es.ts
            src/translations/main_sv.ts
            src/translations/main_tr.ts
            src/translations/main_pl.ts
            )
    set(DYN_TRANSLATION_FILES
            src/translations/dynamic_en.ts
            src/translations/dynamic_de.ts
            src/translations/dynamic_nl.ts
            src/translations/dynamic_it.ts
            src/translations/dynamic_ru.ts
            src/translations/dynamic_pt.ts
            src/translations/dynamic_es.ts
            src/translations/dynamic_sv.ts
            src/translations/main_tr.ts
            src/translations/dynamic_pl.ts
            )

    set_source_files_properties(${MAIN_TRANSLATION_FILES} PROPERTIES OUTPUT_LOCATION
            ${CMAKE_CURRENT_BINARY_DIR}/translations)
    set_source_files_properties(${DYN_TRANSLATION_FILES} PROPERTIES OUTPUT_LOCATION
            ${CMAKE_CURRENT_BINARY_DIR}/translations)

    qt5_create_translation(GEN_TRANSLATION_FILES
            ${MAIN_TRANSLATION_FILES}
            ${SOURCES} ${PLATFORM_SOURCES} ${HEADERS} ${PLATFORM_HEADERS}
            OPTIONS -locations none)
    qt5_add_translation(GEN_DYN_TRANSLATION_FILES ${DYN_TRANSLATION_FILES})
    list(APPEND GEN_TRANSLATION_FILES ${GEN_DYN_TRANSLATION_FILES})
endif(Qt5LinguistTools_FOUND)

add_library(birdtray_lib STATIC
        ${SOURCES} ${PLATFORM_SOURCES} ${RESOURCES_SOURCES} ${GEN_TRANSLATION_FILES}
        ${HEADERS} ${PLATFORM_HEADERS})
target_include_directories(birdtray_lib PUBLIC src)
target_link_libraries(birdtray_lib PUBLIC ${REQUIRED_MODULES})

add_executable(birdtray
        ${EXECUTABLE_OPTIONS} src/main.cpp ${PLATFORM_EXE_SOURCES} ${RESOURCES_SOURCES}
        ${HEADERS} ${PLATFORM_HEADERS})
target_link_libraries(birdtray birdtray_lib)

if(MSVC)
    target_compile_options(birdtray_lib PUBLIC /utf-8)
endif()

option(COMPILER_WARNINGS_AS_ERRORS "Fail the build on compiler warnings" OFF)
if(COMPILER_WARNINGS_AS_ERRORS)
    if(MSVC)
        target_compile_options(birdtray_lib PUBLIC /W4 /WX)
    else()
        target_compile_options(birdtray_lib PUBLIC -Wall -Wextra -Wpedantic -Werror)
    endif()
endif()

# Tests
option(BUILD_WITH_TESTS "Build and add a test target. This requires GTest." OFF)
if(BUILD_WITH_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
if(WIN32)
    if(POLICY CMP0087)
        cmake_policy(SET CMP0087 NEW)
    endif()

    option(DONT_EXECUTE_INSTALLER "Only create the Windows installer but don't execute it" OFF)
    set(INSTALL_ARG --install)
    if (DONT_EXECUTE_INSTALLER)
        set(INSTALL_ARG)
    endif()

    # Find ssl libraries
    find_package(OpenSSL QUIET)
    get_filename_component(SSL_PATH ${OPENSSL_SSL_LIBRARY} DIRECTORY)
    if(SSL_PATH)
        STRING(REGEX REPLACE "/" "\\\\\\\\" SSL_PATH ${SSL_PATH})
    else()
        set(SSL_PATH "C:\\\\Program Files\\\\OpenSSL")
    endif()

    # Create installer
    install(CODE "if(NOT POLICY CMP0087)
            message(FATAL_ERROR \"The install target requires cmake 3.14 or newer\")
        endif()
        execute_process(
        COMMAND cmd /C buildInstaller.bat \"$<TARGET_FILE:birdtray>\" \"${SSL_PATH}\" ${INSTALL_ARG}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/installer
        RESULT_VARIABLE INSTALL_RESULT)
        if(NOT \${INSTALL_RESULT} EQUAL \"0\")
            message(FATAL_ERROR \"Install command failed with exit code \${INSTALL_RESULT}\")
        endif()")
else()
    include(GNUInstallDirs)
    install(TARGETS birdtray RUNTIME DESTINATION bin)
    install(FILES src/res/com.ulduzsoft.Birdtray.desktop
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    install(FILES src/res/com.ulduzsoft.Birdtray.appdata.xml
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
    foreach(size 32 48 64 128)
        install(FILES src/res/icons/${size}/com.ulduzsoft.Birdtray.png
                DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${size}x${size}/apps)
    endforeach()
    install(FILES src/res/birdtray.svg
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
            RENAME com.ulduzsoft.Birdtray.svg)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/translations
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/ulduzsoft/birdtray)
endif()

if(Qt5LinguistTools_FOUND)
    add_custom_target(clean_translations COMMENT "Remove obsolete translation entries")
    foreach(TRANSLATION_FILE ${MAIN_TRANSLATION_FILES})
        get_filename_component(TRANSLATION_NAME ${TRANSLATION_FILE} NAME)
        set(TRANSLATION_LST_FILE
                "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${TRANSLATION_NAME}_lst_file")
        if(NOT EXISTS ${TRANSLATION_LST_FILE})
            get_filename_component(TRANSLATION_NAME ${TRANSLATION_FILE} NAME_WE)
            set(TRANSLATION_LST_FILE
                    "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/${TRANSLATION_NAME}_lst_file")
        endif()
        add_custom_command(TARGET clean_translations
                COMMAND ${Qt5_LUPDATE_EXECUTABLE}
                ARGS -locations none -no-obsolete "@${TRANSLATION_LST_FILE}"
                -ts ${CMAKE_CURRENT_SOURCE_DIR}/${TRANSLATION_FILE}
                DEPENDS ${TRANSLATION_LST_FILE})
    endforeach()
endif()
