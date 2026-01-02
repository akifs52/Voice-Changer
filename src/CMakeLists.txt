cmake_minimum_required(VERSION 3.16)

project(VoiceChanger VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia)

# Find FFmpeg
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVFORMAT REQUIRED libavformat) 
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED libswresample)

# Find SoundTouch
find_library(SOUNDTOUCH_LIB soundtouch)

# Platform-specific configurations
if(WIN32)
    # Windows-specific configuration
    add_definitions(-DQ_OS_WIN)
    
    # Add Windows libraries
    set(PLATFORM_LIBS user32 mf mfplat mfreadwrite ole32 ksuser)
    
    # Set icon
    set(ICON_RC ${CMAKE_CURRENT_SOURCE_DIR}/img/icon.rc)
    
elseif(APPLE)
    # macOS-specific configuration
    add_definitions(-DQ_OS_MACOS)
    
    # Add macOS frameworks
    set(PLATFORM_LIBS "-framework CoreAudio -framework AudioToolbox -framework Carbon")
    
    # Set icon
    set(MACOSX_BUNDLE_ICON_FILE icon.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/img/icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    
else()
    # Linux-specific configuration
    add_definitions(-DQ_OS_LINUX)
    
    # Find X11 for global hotkeys
    find_package(X11 REQUIRED)
    set(PLATFORM_LIBS ${X11_LIBRARIES})
    
    # Find PulseAudio and ALSA
    pkg_check_modules(PULSE REQUIRED libpulse)
    pkg_check_modules(ALSA REQUIRED alsa)
    list(APPEND PLATFORM_LIBS ${PULSE_LIBRARIES} ${ALSA_LIBRARIES})
endif()

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    audiopipeline.cpp
    circularbuffer.cpp
    deletesoundpack.cpp
    effects.cpp
    hotkeys.cpp
    loadouts.cpp
    psola.cpp
    recorder.cpp
    soundpack.cpp
    voiceeffects.cpp
)

# Header files
set(HEADERS
    mainwindow.h
    audiopipeline.h
    circularbuffer.h
    deletesoundpack.h
    effects.h
    hotkeys.h
    loadouts.h
    psola.h
    recorder.h
    soundpack.h
    voiceeffects.h
)

# UI files
set(UI_FILES
    mainwindow.ui
)

# Resource files
set(RESOURCES
    Soundpack.qrc
)

# Create executable
qt6_add_executable(VoiceChanger
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
    ${RESOURCES}
)

# Platform-specific resources
if(WIN32 AND EXISTS ${ICON_RC})
    target_sources(VoiceChanger PRIVATE ${ICON_RC})
elseif(APPLE AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/img/icon.icns)
    target_sources(VoiceChanger PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/img/icon.icns)
endif()

# Link libraries
target_link_libraries(VoiceChanger PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    ${AVCODEC_LIBRARIES}
    ${AVFORMAT_LIBRARIES}
    ${AVUTIL_LIBRARIES}
    ${SWRESAMPLE_LIBRARIES}
    ${SOUNDTOUCH_LIB}
    ${PLATFORM_LIBS}
)

# Include directories
target_include_directories(VoiceChanger PRIVATE
    ${AVCODEC_INCLUDE_DIRS}
    ${AVFORMAT_INCLUDE_DIRS}
    ${AVUTIL_INCLUDE_DIRS}
    ${SWRESAMPLE_INCLUDE_DIRS}
)

# Compiler-specific options
target_compile_options(VoiceChanger PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
)

# Platform-specific deployment
if(WIN32)
    # Windows deployment
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET VoiceChanger POST_BUILD
            COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:VoiceChanger>
            COMMENT "Deploying Qt libraries")
    endif()
    
elseif(APPLE)
    # macOS deployment
    find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(MACDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET VoiceChanger POST_BUILD
            COMMAND ${MACDEPLOYQT_EXECUTABLE} $<TARGET_BUNDLE_DIR:VoiceChanger>
            COMMENT "Deploying Qt libraries")
    endif()
    
    set_target_properties(VoiceChanger PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
        MACOSX_BUNDLE_BUNDLE_NAME "VoiceChanger"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
        MACOSX_BUNDLE_IDENTIFIER "com.voicechanger.app"
    )
    
else()
    # Linux deployment
    install(TARGETS VoiceChanger
        RUNTIME DESTINATION bin
    )
    
    # Install desktop file
    install(FILES voicechanger.desktop
        DESTINATION share/applications
    )
    
    # Install icon
    install(FILES img/icon.png
        DESTINATION share/icons/hicolor/256x256/apps
        RENAME voicechanger.png
    )
endif()

# Custom targets for development
add_custom_target(run
    COMMAND $<TARGET_FILE:VoiceChanger>
    DEPENDS VoiceChanger
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Running VoiceChanger"
)
