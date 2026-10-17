# Copyright (C) 2005 - 2026 Settlers Freaks <sf-team at siedler25.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# OpenXR-Video-Treiber (Quest-Tabletop). Standardmäßig deaktiviert, damit
# Desktop- und Wrapper-Builds unverändert bleiben.
# Aktivierung: -DRTTR_DRIVER_OPENXR=ON -DOPENXR_INCLUDE_DIR=<sdk>/include
if(NOT RTTR_DRIVER_OPENXR)
    return()
endif()

find_path(OPENXR_INCLUDE_DIR
    NAMES openxr/openxr.h openxr/openxr_platform.h
    HINTS ${OPENXR_SDK} ${OPENXR_SDK}/include
    DOC "Pfad zu den OpenXR-Headern (Khronos- oder Meta-OpenXR-SDK)"
)

if(NOT OPENXR_INCLUDE_DIR)
    message(FATAL_ERROR
        "RTTR_DRIVER_OPENXR=ON, aber keine OpenXR-Header gefunden. "
        "Bitte OPENXR_INCLUDE_DIR setzen (z. B. auf den include-Pfad des "
        "Meta OpenXR SDK) oder -DRTTR_OPENXR_FETCH_SDK=ON nutzen."
    )
endif()

rttr_set_output_dir(RUNTIME ${RTTR_DRIVERDIR}/video)
rttr_set_output_dir(LIBRARY ${RTTR_DRIVERDIR}/video)

add_library(videoOpenXR SHARED ${RTTR_DRIVER_INTERFACE} VideoOpenXR.cpp VideoOpenXR.h)

target_compile_definitions(videoOpenXR PRIVATE RTTR_XR_HAS_OPENXR)
target_include_directories(videoOpenXR PRIVATE ${OPENXR_INCLUDE_DIR})

target_link_libraries(videoOpenXR PRIVATE videodrv s25util::common glad Boost::nowide)

if(TARGET GL)
    # gl4es-Pfad wie im SDL2-Treiber: GL-Calls laufen über libGL.so von gl4es.
    target_link_libraries(videoOpenXR PRIVATE GL)
    target_include_directories(videoOpenXR PRIVATE ${GL4ES_INCLUDE_DIR})
endif()

if(ANDROID)
    # EGL/GLES für die Grafikbindung; dl für den Laufzeit-Loader.
    find_library(ANDROID_EGL_LIB EGL)
    find_library(ANDROID_GLES2_LIB GLESv2)
    find_library(ANDROID_DL_LIB dl)
    find_library(ANDROID_NATIVE_LIB android) # ANativeWindow for the in-app video recording
    target_link_libraries(videoOpenXR PRIVATE ${ANDROID_EGL_LIB} ${ANDROID_GLES2_LIB} ${ANDROID_DL_LIB} ${ANDROID_NATIVE_LIB})
    # SDL2 liefert die Android-Glue (SDL_AndroidGetJNIEnv/Activity) für
    # xrInitializeLoaderKHR. Dasselbe find_package wie im SDL2-Treiber, weil
    # das SDL2::SDL2-Ziel sonst (Abarbeitungsreihenfolge) noch nicht existiert.
    set(SDL2_BUILDING_LIBRARY ON)
    find_package(SDL2 2.0.5)
    if(SDL2_FOUND)
        target_link_libraries(videoOpenXR PRIVATE SDL2::SDL2)
    endif()
    # OpenXR-Loader (vorgebaut, kommt mit der APK); Fallback bleibt der
    # zur Laufzeit per dlopen geladene System-Loader.
    if(TARGET openxr_loader)
        target_link_libraries(videoOpenXR PRIVATE openxr_loader)
    endif()
endif()

enable_warnings(videoOpenXR)

install(TARGETS videoOpenXR
  RUNTIME DESTINATION ${RTTR_DRIVERDIR}/video
  LIBRARY DESTINATION ${RTTR_DRIVERDIR}/video
)
add_dependencies(drivers videoOpenXR)
