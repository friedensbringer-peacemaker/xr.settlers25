// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "VideoOpenXR.h"
#include "driver/VrInput.h"
#include "driver/VrMenuModel.h"
#include "PanelGeometry.h"
#include "RealTable.h"
#include "HqAsset.h"
#include "driver/BuildingHelp.h"
#include "driver/Interface.h" // GetDriverName() mit RTTR_DECL/BOOST_SYMBOL_EXPORT deklariert
#include "driver/VideoDriverLoaderInterface.h"
#include "openglCfg.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// Plattform-Abstraktion: Mit RTTR_XR_HAS_OPENXR wird die echte OpenXR-Pipeline
// kompiliert (Android/Quest). Ohne das Makro entsteht ein Stub, der die
// Treiberschnittstelle erfüllt und den Desktop-Kompiliercheck erlaubt.
// -----------------------------------------------------------------------------
#ifdef RTTR_XR_HAS_OPENXR
// Plattform-/Grafik-API-Makros vor den OpenXR-Headern: aktivieren die
// GLES-Android-Bindings (XrGraphicsBindingOpenGLESAndroidKHR etc.).
#define XR_USE_PLATFORM_ANDROID
#define XR_USE_GRAPHICS_API_OPENGL_ES
#define XR_EXTENSION_PROTOTYPES // Prototypen auch fuer Erweiterungen (xrInitializeLoaderKHR)
#include <type_traits>
#include <jni.h> // vor openxr_platform.h (jobject in den Android-Bindings)
#include <dlfcn.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <glad/glad.h>
#include <SDL_misc.h>
#include <SDL_system.h> // SDL_AndroidGetJNIEnv/SDL_AndroidGetActivity
#include <android/native_window_jni.h> // encoder input surface for the in-app video recording
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#if RTTR_OGL_GL4ES
#    include <gl4esinit.h>
#endif

// OpenXR-Code ist struct-init-lastig; die Aggregat-Initialisierung
// {XR_TYPE_X} loest sonst -Werror=missing-field-initializers aus.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#if RTTR_OGL_GL4ES
// glad (OpenGL-2.0-Compat) deklariert keine Framebuffer-Objekte; die
// Symbole kommen von gl4es/GLES2 und werden hier lokal deklariert.
#    ifndef GL_FRAMEBUFFER
#        define GL_FRAMEBUFFER 0x8D40
#        define GL_COLOR_ATTACHMENT0 0x8CE0
#        define GL_FRAMEBUFFER_COMPLETE 0x8CD5
extern "C" {
void glGenFramebuffers(GLsizei n, GLuint* ids);
void glDeleteFramebuffers(GLsizei n, const GLuint* ids);
void glBindFramebuffer(GLenum target, GLuint framebuffer);
void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
GLenum glCheckFramebufferStatus(GLenum target);
}
#    endif
#endif

namespace {
/// Panel im Raum: Mittelpunkt (Meter, localSpace), Breite; Höhe aus Seitenverhältnis.
/// Eine Panel-Drehung/Brettplatzierung (Stufe 3) ersetzt diese Konstanten.


XrPosef IdentityPose()
{
    XrPosef p{};
    p.orientation.w = 1.f;
    return p;
}
} // namespace

struct VideoOpenXR::XrState
{
    // Der Quest-Lader exportiert die Core-Funktionen als globale Symbole; nach
    // dlopen(RTLD_GLOBAL) sind direkte xr*-Aufrufe möglich.
    void* loaderHandle = nullptr;

    XrInstance instance = XR_NULL_HANDLE;
    XrSession session = XR_NULL_HANDLE;
    XrSystemId systemId = XR_NULL_SYSTEM_ID;
    XrSessionState sessionState = XR_SESSION_STATE_UNKNOWN;
    bool sessionRunning = false;
    XrSpace localSpace = XR_NULL_HANDLE, viewSpace = XR_NULL_HANDLE;
    XrTime predictedDisplayTime = 0;
    bool shouldRender = false;
    float panelWidth = 2.0f, panelZ = -1.8f;
    bool initialDistanceApplied = false;
    XrVector3f panelOrigin{};
    float panelYaw = 0.f, panelPitch = 0.f, panelHeight = 0.f;
    int presentation = -1;
    bool startPoseApplied = false;
    int panelControl = -1;
    vrInput::StickTurbo stickTurbo;
    vrInput::PanAccumulator pan;
    vrPanel::Pose panelPose() const
    { return {{panelOrigin.x,panelOrigin.y,panelOrigin.z},panelYaw,panelZ,panelPitch,panelHeight}; }
    unsigned recenterSeen = 0;
    vrMenuModel::DoublePulse confirmPulse;
    // VR menu navigation (P2): A selects, B closes, right stick moves the focus with hold-to-repeat.
    vrMenuModel::StickRepeat menuStick;
    bool menuLatch[4] = {false, false, false, false}; // button held across a menu open/close: ignore until released
    bool prevMenuNav = false;
    bool hadFocus = false, filterValid = false;
    int pointerHand = 1;
    float smoothU = 0.5f, smoothV = 0.5f;
    unsigned long clickLockUntil = 0, panelSaveAt = 0;
    vrInput::ButtonGesture buttonGestures[4];
    bool inputArmed=false;
    XrVector3f toWorld(float x, float y, float z) const
    {
        const auto p=panelPose().toWorld({x,y,z});
        return {p.x,p.y,p.z};
    }
    XrVector3f toLocal(XrVector3f v, bool point) const
    {
        const auto p=point ? panelPose().toLocal({v.x,v.y,v.z})
                           : panelPose().directionToLocal({v.x,v.y,v.z});
        return {p.x,p.y,p.z};
    }
    unsigned long lastPanelUpdate = 0;
    bool prevResetPanel = false;
    vrInput::ZoomRepeat zoomRepeat;
    vrInput::MapGrab mapGrab;
    Position grabLastPos{0, 0};
    float handDepth = 0.f; // pointer hand distance along the panel normal, towards the panel positive
    bool movingFrame=false, previousFrameGrip=false;
    vrPanel::TranslationDrag frameDrag;
    bool pointerValid = false, pointerOnPanel = false;
    XrVector3f pointerOrigin{}, pointerEnd{};

    struct Headquarters { float u,v,width; };
    std::vector<Headquarters> headquarters;
    bool hqAssetAttempted=false;
    GLuint hqVertices=0,hqIndices=0,hqTexture=0;
    GLsizei hqIndexCount=0;
    struct Eye
    {
        XrSwapchain swapchain = XR_NULL_HANDLE;
        GLuint framebuffer = 0;
        GLuint nativeDepth = 0;
        std::vector<XrSwapchainImageOpenGLESKHR> images;
        int32_t width = 0, height = 0;
    };
    Eye eyes[2];
    bool passthroughSupported=false, passthroughRunning=false, passthroughRequested=false;
    int passthroughStatus=0;
    XrPassthroughFB passthrough=XR_NULL_HANDLE;
    XrPassthroughLayerFB passthroughLayer=XR_NULL_HANDLE;
    PFN_xrCreatePassthroughFB createPassthrough=nullptr;
    PFN_xrDestroyPassthroughFB destroyPassthrough=nullptr;
    PFN_xrPassthroughStartFB startPassthrough=nullptr;
    PFN_xrPassthroughPauseFB pausePassthrough=nullptr;
    PFN_xrCreatePassthroughLayerFB createPassthroughLayer=nullptr;
    PFN_xrDestroyPassthroughLayerFB destroyPassthroughLayer=nullptr;
    PFN_xrPassthroughLayerResumeFB resumePassthroughLayer=nullptr;
    PFN_xrPassthroughLayerPauseFB pausePassthroughLayer=nullptr;
    PFN_xrPassthroughLayerSetStyleFB setPassthroughStyle=nullptr;
    int passthroughOpacityApplied=-1;
    XrTime recenterChangeTime=0;
    // VR-022: real table from the Quest scene model (room scan).
    bool sceneSupported=false, scenePermission=false, scenePermissionRequested=false;
    bool tableQueryPending=false, tableFound=false, tableWidthApplied=false, realTableWasActive=false;
    int tableStatus=0; // 0 unsupported, 1 permission missing, 2 searching, 3 no table found, 4 table found
    unsigned long nextSceneCheck=0;
    unsigned tableRecenterSeen=0;
    XrAsyncRequestIdFB tableQuery=0;
    vrTable::Placement tablePlacement{};
    PFN_xrQuerySpacesFB querySpaces=nullptr;
    PFN_xrRetrieveSpaceQueryResultsFB retrieveSpaceQueryResults=nullptr;
    PFN_xrGetSpaceSemanticLabelsFB getSpaceSemanticLabels=nullptr;
    PFN_xrGetSpaceBoundingBox2DFB getSpaceBoundingBox2D=nullptr;
    // In-app video recording of the flat game image (own shared GL context, so gl4es state stays untouched).
    bool recording=false, recordFailed=false;
    EGLConfig recConfig=nullptr;
    EGLContext recContext=EGL_NO_CONTEXT;
    EGLSurface recSurface=EGL_NO_SURFACE;
    ANativeWindow* recWindow=nullptr;
    int recWidth=0, recHeight=0;
    unsigned recProgram=0;
    std::chrono::steady_clock::time_point recLastFrame{};
    Eye panel; // Full-resolution compositor texture, independent of the eye buffers.
    bool stereo = true;
    bool cylinderSupported = false;
    uint32_t maxPanelWidth = 0, maxPanelHeight = 0;
    GLuint colorProgram = 0;
    bool colorProgramAttempted = false;

    // EGL-Kontext (surfaceless); gl4es hängt sich an diesen Kontext.
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLContext eglContext = EGL_NO_CONTEXT;
    EGLConfig eglConfig = nullptr;

    // Globaler Referenz auf die Activity für den Loader-Init.
    jobject appContext = nullptr;

    // Eingabe: Aktionen und Pfade pro Hand.
    XrActionSet actionSet = XR_NULL_HANDLE;
    XrAction aimPoseAction = XR_NULL_HANDLE, triggerAction = XR_NULL_HANDLE, squeezeAction = XR_NULL_HANDLE,
             stickAction = XR_NULL_HANDLE, aButtonAction = XR_NULL_HANDLE, bButtonAction = XR_NULL_HANDLE, resetPanelAction = XR_NULL_HANDLE, speedAction = XR_NULL_HANDLE, menuAction = XR_NULL_HANDLE, hapticAction = XR_NULL_HANDLE;
    XrSpace aimSpaces[2] = {XR_NULL_HANDLE, XR_NULL_HANDLE};
    XrPath handPaths[2] = {XR_NULL_PATH, XR_NULL_PATH};
    bool prevTrigger[2] = {false, false}, prevSqueeze[2] = {false, false}, prevA[2] = {false, false};
    float stickX = 0.f, stickY = 0.f;   ///< Gemittelt über beide Hände (dominant: rechts)
    unsigned long lastStickKeyRepeat = 0;
};

#else
struct VideoOpenXR::XrState
{
    // Stub: hält keinen Zustand.
};
#endif

VideoOpenXR::VideoOpenXR(VideoDriverLoaderInterface* CallBack) : VideoDriver(CallBack), xr_(new XrState) {}

VideoOpenXR::~VideoOpenXR()
{
    DestroyScreen();
}

const char* VideoOpenXR::GetName() const
{
    return GetDriverName();
}

const char* GetDriverName()
{
    return "(OpenXR) Panel/Tabletop via OpenXR (Quest)";
}

IVideoDriver* CreateVideoInstance(VideoDriverLoaderInterface* CallBack)
{
    return new VideoOpenXR(CallBack);
}

void FreeVideoInstance(IVideoDriver* driver)
{
    delete driver;
}

bool VideoOpenXR::Initialize()
{
#ifdef RTTR_XR_HAS_OPENXR
    // RTLD_GLOBAL: Core-xr*-Symbole werden prozessweit sichtbar, danach sind
    // die in openxr.h deklarierten Funktionen direkt aufrufbar.
    xr_->loaderHandle = dlopen("libopenxr_loader.so", RTLD_NOW | RTLD_GLOBAL);
    if(!xr_->loaderHandle)
        xr_->loaderHandle = dlopen("libopenxr_loader.so.1", RTLD_NOW | RTLD_GLOBAL);
    if(!xr_->loaderHandle)
    {
        ShowErrorMessage("OpenXR", "OpenXR-Loader (libopenxr_loader.so) nicht gefunden.");
        return false;
    }
#endif
    return true;
}

#ifdef RTTR_XR_HAS_OPENXR
namespace {
// --- VR-022: Quest scene model (room scan) ------------------------------------------------------
/// Calls a static helper of org.s25rttr.sdl.SDLActivity (see SDLActivity.java).
bool CallSceneHelper(const char* name, bool request)
{
    auto* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    auto activity = static_cast<jobject>(SDL_AndroidGetActivity());
    if(!env || !activity) return false;
    bool result = false;
    jclass cls = env->GetObjectClass(activity); // app class; FindClass would use the system class loader here
    if(jmethodID method = env->GetStaticMethodID(cls, name, request ? "()V" : "()Z"))
    {
        if(request) env->CallStaticVoidMethod(cls, method);
        else result = env->CallStaticBooleanMethod(cls, method) == JNI_TRUE;
    }
    if(env->ExceptionCheck()) { env->ExceptionClear(); result = false; }
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(activity);
    return result;
}

bool StartTableQuery(PFN_xrQuerySpacesFB querySpaces, XrSession session, XrAsyncRequestIdFB& request)
{
    XrSpaceStorageLocationFilterInfoFB storage{XR_TYPE_SPACE_STORAGE_LOCATION_FILTER_INFO_FB};
    storage.location = XR_SPACE_STORAGE_LOCATION_LOCAL_FB;
    XrSpaceComponentFilterInfoFB filter{XR_TYPE_SPACE_COMPONENT_FILTER_INFO_FB};
    filter.next = &storage;
    filter.componentType = XR_SPACE_COMPONENT_TYPE_SEMANTIC_LABELS_FB;
    XrSpaceQueryInfoFB info{XR_TYPE_SPACE_QUERY_INFO_FB};
    info.queryAction = XR_SPACE_QUERY_ACTION_LOAD_FB;
    info.maxResultCount = 128;
    info.filter = reinterpret_cast<const XrSpaceFilterInfoBaseHeaderFB*>(&filter);
    const XrResult result = querySpaces(session, reinterpret_cast<const XrSpaceQueryInfoBaseHeaderFB*>(&info), &request);
    std::fprintf(stderr, "[videoOpenXR] Scene query started: %d\n", static_cast<int>(result));
    return XR_SUCCEEDED(result);
}

std::string SemanticLabels(PFN_xrGetSpaceSemanticLabelsFB getLabels, XrSession session, XrSpace space)
{
    XrSemanticLabelsSupportInfoFB support{XR_TYPE_SEMANTIC_LABELS_SUPPORT_INFO_FB};
    support.flags = XR_SEMANTIC_LABELS_SUPPORT_MULTIPLE_SEMANTIC_LABELS_BIT_FB
                    | XR_SEMANTIC_LABELS_SUPPORT_ACCEPT_DESK_TO_TABLE_MIGRATION_BIT_FB;
    support.recognizedLabels = "TABLE,COUCH,FLOOR,CEILING,WALL_FACE,DOOR_FRAME,WINDOW_FRAME,OTHER";
    for(const bool withSupport : {true, false})
    {
        XrSemanticLabelsFB labels{XR_TYPE_SEMANTIC_LABELS_FB};
        labels.next = withSupport ? &support : nullptr;
        if(XR_FAILED(getLabels(session, space, &labels)) || labels.bufferCountOutput == 0) continue;
        std::string buffer(labels.bufferCountOutput, '\0');
        labels.bufferCapacityInput = labels.bufferCountOutput;
        labels.buffer = buffer.data();
        if(XR_FAILED(getLabels(session, space, &labels))) continue;
        buffer.resize(std::strlen(buffer.c_str()));
        return buffer;
    }
    return {};
}

/// Evaluates query results and picks the largest table within reach. Unused spaces are destroyed.
bool PickTable(XrSession session, XrSpace localSpace, XrSpace viewSpace, XrTime time, XrAsyncRequestIdFB request,
               PFN_xrRetrieveSpaceQueryResultsFB retrieve, PFN_xrGetSpaceSemanticLabelsFB getLabels,
               PFN_xrGetSpaceBoundingBox2DFB getBounds, float aspect, vrTable::Placement& best)
{
    XrSpaceQueryResultsFB results{XR_TYPE_SPACE_QUERY_RESULTS_FB};
    if(XR_FAILED(retrieve(session, request, &results)) || results.resultCountOutput == 0) return false;
    std::vector<XrSpaceQueryResultFB> items(results.resultCountOutput, XrSpaceQueryResultFB{});
    results.resultCapacityInput = static_cast<uint32_t>(items.size());
    results.results = items.data();
    if(XR_FAILED(retrieve(session, request, &results))) return false;
    XrSpaceLocation head{XR_TYPE_SPACE_LOCATION};
    const bool headValid = time > 0 && XR_SUCCEEDED(xrLocateSpace(viewSpace, localSpace, time, &head))
                           && (head.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT);
    const vrPanel::Vec3 headPos = headValid ? vrPanel::Vec3{head.pose.position.x, head.pose.position.y, head.pose.position.z}
                                            : vrPanel::Vec3{0.f, 1.2f, 0.f};
    bool found = false;
    for(const auto& item : items)
    {
        const std::string labels = SemanticLabels(getLabels, session, item.space);
        const bool table = labels.find("TABLE") != std::string::npos || labels.find("DESK") != std::string::npos;
        XrRect2Df rect{};
        const bool hasRect = XR_SUCCEEDED(getBounds(session, item.space, &rect));
        XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
        const bool located = time > 0 && XR_SUCCEEDED(xrLocateSpace(item.space, localSpace, time, &location))
                             && (location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
                             && (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT);
        std::fprintf(stderr, "[videoOpenXR] Scene entity '%s' rect=%d (%.2f x %.2f) located=%d y=%.2f\n", labels.c_str(),
                     hasRect, rect.extent.width, rect.extent.height, located, location.pose.position.y);
        if(!table || !hasRect || !located)
            continue;
        const auto& p = location.pose;
        const auto candidate = vrTable::fit({p.position.x, p.position.y, p.position.z},
                                            {p.orientation.x, p.orientation.y, p.orientation.z, p.orientation.w},
                                            rect.offset.x, rect.offset.y, rect.extent.width, rect.extent.height,
                                            headPos, aspect, 2.4f);
        const float dx = candidate.center.x - headPos.x, dz = candidate.center.z - headPos.z;
        if(dx * dx + dz * dz > 3.f * 3.f) continue; // out of reach
        if(!found || candidate.area > best.area) { best = candidate; found = true; }
    }
    // Only the pose is kept; the anchors themselves are not needed any more.
    for(const auto& item : items) xrDestroySpace(item.space);
    return found;
}

GLuint CreateColorProgram()
{
    const char* vertex = "#version 120\nvoid main(){gl_Position=ftransform();gl_TexCoord[0]=gl_MultiTexCoord0;}";
    const char* fragment = "#version 120\nuniform sampler2D image; uniform float brightness; uniform float contrast; uniform float sharpness; uniform vec2 texel;"
      "void main(){vec4 c=texture2D(image,gl_TexCoord[0].xy);"
      "vec2 uv=gl_TexCoord[0].xy; vec3 n=texture2D(image,uv+vec2(texel.x,0.0)).rgb;"
      "vec3 s=texture2D(image,uv-vec2(texel.x,0.0)).rgb;"
      "vec3 e=texture2D(image,uv+vec2(0.0,texel.y)).rgb;"
      "vec3 w=texture2D(image,uv-vec2(0.0,texel.y)).rgb;"
      "vec3 filtered=c.rgb+sharpness*(c.rgb-(n+s+e+w)*0.25);"
      "c.rgb=clamp(filtered,min(c.rgb,min(min(n,s),min(e,w))),max(c.rgb,max(max(n,s),max(e,w))));"
      "gl_FragColor=vec4(clamp(((c.rgb-vec3(0.5))*contrast+vec3(0.5))*brightness,0.0,1.0),c.a);}";
    GLuint shaders[2] = {glCreateShader(GL_VERTEX_SHADER), glCreateShader(GL_FRAGMENT_SHADER)};
    const char* sources[2] = {vertex, fragment};
    GLuint program = glCreateProgram();
    bool valid = true;
    for(int i = 0; i < 2; ++i)
    {
        glShaderSource(shaders[i], 1, &sources[i], nullptr);
        glCompileShader(shaders[i]);
        GLint ok = GL_FALSE;
        glGetShaderiv(shaders[i], GL_COMPILE_STATUS, &ok);
        if(!ok)
        {
            char log[2048]{};
            glGetShaderInfoLog(shaders[i], sizeof(log), nullptr, log);
            std::fprintf(stderr, "[videoOpenXR] Color shader failed: %s\n", log);
            valid = false;
        }
        glAttachShader(program, shaders[i]);
    }
    glLinkProgram(program);
    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    for(GLuint shader : shaders) glDeleteShader(shader);
    if(!valid || !linked)
    {
        char log[2048]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[videoOpenXR] Color program unavailable, using original image: %s\n", log);
        glDeleteProgram(program);
        return 0;
    }
    std::fprintf(stderr, "[videoOpenXR] Color adjustment shader ready\n");
    return program;
}

void CheckXr(XrResult result, const char* what)
{
    if(XR_SUCCEEDED(result))
        return;
    char buffer[XR_MAX_RESULT_STRING_SIZE];
    if(xrResultToString(XR_NULL_HANDLE, result, buffer) == XR_SUCCESS)
        std::fprintf(stderr, "[videoOpenXR] %s fehlgeschlagen: %s\n", what, buffer);
    else
        std::fprintf(stderr, "[videoOpenXR] %s fehlgeschlagen: %d\n", what, result);
    throw std::runtime_error(what);
}
} // namespace
#endif

bool VideoOpenXR::CreateScreen(const std::string& /*title*/, VideoMode newSize, DisplayMode displayMode)
{
    displayMode_ = displayMode;
#ifdef RTTR_XR_HAS_OPENXR
    // Die Android-Oberfläche liefert u. U. ein kleines Hochformat-Fenster
    // (500x800 Portrait). Wir zeichnen offscreen in unseren eigenen Backbuffer
    // und wählen selbst eine landschaftliche Auflösung für das VR-Panel.
    const auto modes = ListVideoModes();
    if(std::find(modes.begin(), modes.end(), newSize) == modes.end())
        newSize = VideoMode(1920, 1200);
#endif
    SetNewSize(newSize, Extent(newSize.width, newSize.height));

#ifdef RTTR_XR_HAS_OPENXR
    // --- 0) Android: Loader mit VM/Kontext initialisieren ---------------------
    // Der Khronos-Loader verlangt auf Android xrInitializeLoaderKHR, sonst
    // schlägt xrCreateInstance fehl ("xrInitializeLoaderKHR was not
    // successfully called"). VM/Kontext kommen über die SDL-Android-Glue.
    {
        static bool loaderInitialized = false;
        if(!loaderInitialized)
        {
            JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
            jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
            if(!env || !activity)
            {
                ShowErrorMessage("OpenXR", "SDL-Android-Kontext (JNIEnv/Activity) nicht verfügbar.");
                return false;
            }
            JavaVM* jvm = nullptr;
            if(env->GetJavaVM(&jvm) != JNI_OK)
            {
                ShowErrorMessage("OpenXR", "JavaVM konnte nicht ermittelt werden.");
                return false;
            }
            xr_->appContext = env->NewGlobalRef(activity);
            // Der Loader exportiert xrInitializeLoaderKHR nicht als globales
            // Symbol (Khronos-Exportmap); Abruf mit NULL-Instanz wie in der
            // Spezifikation für loaderseitige Funktionen vorgesehen.
            PFN_xrVoidFunction initFn = nullptr;
            CheckXr(xrGetInstanceProcAddr(XR_NULL_HANDLE, "xrInitializeLoaderKHR", &initFn), "xrGetInstanceProcAddr(xrInitializeLoaderKHR)");
            auto initLoader = reinterpret_cast<PFN_xrInitializeLoaderKHR>(initFn);
            XrLoaderInitInfoAndroidKHR loaderInit{XR_TYPE_LOADER_INIT_INFO_ANDROID_KHR};
            loaderInit.applicationVM = jvm;
            loaderInit.applicationContext = xr_->appContext;
            CheckXr(initLoader(reinterpret_cast<XrLoaderInitInfoBaseHeaderKHR*>(&loaderInit)), "xrInitializeLoaderKHR");
            loaderInitialized = true;
        }
    }

    // --- 1) EGL-Kontext (surfaceless, ES2) für gl4es -------------------------
    xr_->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(xr_->eglDisplay == EGL_NO_DISPLAY)
        return false;
    EGLint major, minor;
    if(!eglInitialize(xr_->eglDisplay, &major, &minor))
        return false;
    const EGLint configAttribs[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                                    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                                    EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
                                    EGL_NONE};
    EGLint numConfigs = 0;
    if(!eglChooseConfig(xr_->eglDisplay, configAttribs, &xr_->eglConfig, 1, &numConfigs) || numConfigs < 1)
        return false;
    const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    xr_->eglContext = eglCreateContext(xr_->eglDisplay, xr_->eglConfig, EGL_NO_CONTEXT, contextAttribs);
    if(xr_->eglContext == EGL_NO_CONTEXT)
        return false;
    // 1x1-PBuffer als vollständiges Ziel, damit der Kontext current werden kann.
    const EGLint pbufferAttribs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
    EGLSurface pbuffer = eglCreatePbufferSurface(xr_->eglDisplay, xr_->eglConfig, pbufferAttribs);
    if(pbuffer == EGL_NO_SURFACE || !eglMakeCurrent(xr_->eglDisplay, pbuffer, pbuffer, xr_->eglContext))
        return false;
#if RTTR_OGL_GL4ES
    // glad ist in diesem Modul als eigene Kopie gelinkt; ohne Load bleiben
    // seine Funktionszeiger (glGenTextures etc.) null -> Crash beim ersten GL-Aufruf.
    if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(gl4es_GetProcAddress)))
        return false;
#endif

    // --- 2) XrInstance (GLES-Extension zwingend) ------------------------------
    std::vector<const char*> extensions{XR_KHR_OPENGL_ES_ENABLE_EXTENSION_NAME};
    uint32_t extensionCount = 0;
    CheckXr(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr), "enumerate extensions");
    std::vector<XrExtensionProperties> available(extensionCount, XrExtensionProperties{XR_TYPE_EXTENSION_PROPERTIES});
    CheckXr(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, available.data()), "enumerate extensions");
    xr_->cylinderSupported = std::any_of(available.begin(), available.end(), [](const auto& extension) {
        return std::strcmp(extension.extensionName, XR_KHR_COMPOSITION_LAYER_CYLINDER_EXTENSION_NAME) == 0;
    });
    xr_->passthroughSupported=std::any_of(available.begin(),available.end(),[](const auto& extension) {
        return std::strcmp(extension.extensionName,XR_FB_PASSTHROUGH_EXTENSION_NAME)==0;
    });
    if(xr_->passthroughSupported) extensions.push_back(XR_FB_PASSTHROUGH_EXTENSION_NAME);
    if(xr_->cylinderSupported) extensions.push_back(XR_KHR_COMPOSITION_LAYER_CYLINDER_EXTENSION_NAME);
    // Scene model for the real table (VR-022): all four extensions are needed together.
    const char* sceneExtensions[] = {XR_FB_SPATIAL_ENTITY_EXTENSION_NAME, XR_FB_SPATIAL_ENTITY_QUERY_EXTENSION_NAME,
                                     XR_FB_SPATIAL_ENTITY_STORAGE_EXTENSION_NAME, XR_FB_SCENE_EXTENSION_NAME};
    xr_->sceneSupported = std::all_of(std::begin(sceneExtensions), std::end(sceneExtensions), [&](const char* name) {
        return std::any_of(available.begin(), available.end(),
                           [&](const auto& extension) { return std::strcmp(extension.extensionName, name) == 0; });
    });
    if(xr_->sceneSupported) extensions.insert(extensions.end(), std::begin(sceneExtensions), std::end(sceneExtensions));
    std::fprintf(stderr, "[videoOpenXR] Cylinder layer supported: %d\n", xr_->cylinderSupported);
    XrInstanceCreateInfo instanceInfo{XR_TYPE_INSTANCE_CREATE_INFO};
    std::strcpy(instanceInfo.applicationInfo.applicationName, "RTTR-Quest");
    instanceInfo.applicationInfo.applicationVersion = 1;
    instanceInfo.applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
    std::strcpy(instanceInfo.applicationInfo.engineName, "Return to the Roots");
    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    instanceInfo.enabledExtensionNames = extensions.data();
    CheckXr(xrCreateInstance(&instanceInfo, &xr_->instance), "xrCreateInstance");

    // --- 3) System, Session, Referenzraum ------------------------------------
    XrSystemGetInfo systemInfo{XR_TYPE_SYSTEM_GET_INFO};
    systemInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    CheckXr(xrGetSystem(xr_->instance, &systemInfo, &xr_->systemId), "xrGetSystem");

    XrSystemProperties properties{XR_TYPE_SYSTEM_PROPERTIES};
    XrSystemPassthroughPropertiesFB passthroughProperties{XR_TYPE_SYSTEM_PASSTHROUGH_PROPERTIES_FB};
    if(xr_->passthroughSupported) properties.next=&passthroughProperties;
    CheckXr(xrGetSystemProperties(xr_->instance, xr_->systemId, &properties), "xrGetSystemProperties");
    xr_->passthroughSupported=xr_->passthroughSupported && passthroughProperties.supportsPassthrough;
    if(xr_->passthroughSupported) {
        const auto load=[&](const char* name,auto& fn) {
            PFN_xrVoidFunction pointer=nullptr;
            if(XR_FAILED(xrGetInstanceProcAddr(xr_->instance,name,&pointer))) return false;
            fn=reinterpret_cast<std::decay_t<decltype(fn)>>(pointer);return fn!=nullptr;
        };
        xr_->passthroughSupported=load("xrCreatePassthroughFB",xr_->createPassthrough)
          && load("xrDestroyPassthroughFB",xr_->destroyPassthrough)
          && load("xrPassthroughStartFB",xr_->startPassthrough)
          && load("xrPassthroughPauseFB",xr_->pausePassthrough)
          && load("xrCreatePassthroughLayerFB",xr_->createPassthroughLayer)
          && load("xrDestroyPassthroughLayerFB",xr_->destroyPassthroughLayer)
          && load("xrPassthroughLayerResumeFB",xr_->resumePassthroughLayer)
          && load("xrPassthroughLayerPauseFB",xr_->pausePassthroughLayer)
          && load("xrPassthroughLayerSetStyleFB",xr_->setPassthroughStyle);
    }
    if(xr_->sceneSupported) {
        const auto load=[&](const char* name,auto& fn) {
            PFN_xrVoidFunction pointer=nullptr;
            if(XR_FAILED(xrGetInstanceProcAddr(xr_->instance,name,&pointer))) return false;
            fn=reinterpret_cast<std::decay_t<decltype(fn)>>(pointer);return fn!=nullptr;
        };
        xr_->sceneSupported=load("xrQuerySpacesFB",xr_->querySpaces)
          && load("xrRetrieveSpaceQueryResultsFB",xr_->retrieveSpaceQueryResults)
          && load("xrGetSpaceSemanticLabelsFB",xr_->getSpaceSemanticLabels)
          && load("xrGetSpaceBoundingBox2DFB",xr_->getSpaceBoundingBox2D);
    }
    xr_->tableStatus=xr_->sceneSupported ? 1 : 0;
    std::fprintf(stderr,"[videoOpenXR] Scene model (real table) available: %d\n",xr_->sceneSupported);
    xr_->passthroughStatus=xr_->passthroughSupported ? 1 : 0;
    std::fprintf(stderr,"[videoOpenXR] Passthrough available: %d\n",xr_->passthroughSupported);
    GLint textureLimit = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &textureLimit);
    xr_->maxPanelWidth = std::min(properties.graphicsProperties.maxSwapchainImageWidth, static_cast<uint32_t>(textureLimit));
    xr_->maxPanelHeight = std::min(properties.graphicsProperties.maxSwapchainImageHeight, static_cast<uint32_t>(textureLimit));
    if(newSize.width > xr_->maxPanelWidth || newSize.height > xr_->maxPanelHeight)
    {
        newSize = VideoMode(1920, 1200);
        SetNewSize(newSize, Extent(newSize.width, newSize.height));
    }

    // Vom Meta-Runtime erzwungen: GraphicsRequirements vor xrCreateSession abrufen.
    // Erweiterungsfunktion -> dynamisch per xrGetInstanceProcAddr holen.
    PFN_xrVoidFunction gfxFn = nullptr;
    CheckXr(xrGetInstanceProcAddr(xr_->instance, "xrGetOpenGLESGraphicsRequirementsKHR", &gfxFn),
            "xrGetInstanceProcAddr(xrGetOpenGLESGraphicsRequirementsKHR)");
    auto getGfxReq = reinterpret_cast<PFN_xrGetOpenGLESGraphicsRequirementsKHR>(gfxFn);
    XrGraphicsRequirementsOpenGLESKHR gfxReq{XR_TYPE_GRAPHICS_REQUIREMENTS_OPENGL_ES_KHR};
    CheckXr(getGfxReq(xr_->instance, xr_->systemId, &gfxReq), "xrGetOpenGLESGraphicsRequirementsKHR");

    XrGraphicsBindingOpenGLESAndroidKHR graphicsBinding{XR_TYPE_GRAPHICS_BINDING_OPENGL_ES_ANDROID_KHR};
    graphicsBinding.display = xr_->eglDisplay;
    graphicsBinding.config = xr_->eglConfig;
    graphicsBinding.context = xr_->eglContext;
    XrSessionCreateInfo sessionInfo{XR_TYPE_SESSION_CREATE_INFO};
    sessionInfo.systemId = xr_->systemId;
    sessionInfo.next = &graphicsBinding;
    CheckXr(xrCreateSession(xr_->instance, &sessionInfo, &xr_->session), "xrCreateSession");

    XrReferenceSpaceCreateInfo spaceInfo{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
    spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_LOCAL;
    spaceInfo.poseInReferenceSpace = IdentityPose();
    CheckXr(xrCreateReferenceSpace(xr_->session, &spaceInfo, &xr_->localSpace), "xrCreateReferenceSpace");

    spaceInfo.referenceSpaceType = XR_REFERENCE_SPACE_TYPE_VIEW;
    CheckXr(xrCreateReferenceSpace(xr_->session, &spaceInfo, &xr_->viewSpace), "xrCreateReferenceSpace(view)");

    // --- 4) Stereo-Swapchains mit empfohlener Auflösung -----------------------
    uint32_t viewCount = 0;
    CheckXr(xrEnumerateViewConfigurationViews(xr_->instance, xr_->systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, 0, &viewCount, nullptr),
            "xrEnumerateViewConfigurationViews(count)");
    std::vector<XrViewConfigurationView> views(viewCount, XrViewConfigurationView{XR_TYPE_VIEW_CONFIGURATION_VIEW});
    CheckXr(xrEnumerateViewConfigurationViews(xr_->instance, xr_->systemId, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, viewCount, &viewCount, views.data()),
            "xrEnumerateViewConfigurationViews");
    xr_->stereo = viewCount >= 2;
    for(uint32_t i = 0; i < 3; i++)
    {
        const bool isPanel = i == 2;
        auto& eye = isPanel ? xr_->panel : xr_->eyes[i];
        eye.width = isPanel ? newSize.width : static_cast<int32_t>(views.at(i).recommendedImageRectWidth);
        eye.height = isPanel ? newSize.height : static_cast<int32_t>(views.at(i).recommendedImageRectHeight);
        XrSwapchainCreateInfo swapInfo{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapInfo.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        swapInfo.format = GL_RGBA8;
        swapInfo.width = eye.width;
        swapInfo.height = eye.height;
        swapInfo.faceCount = 1;
        swapInfo.arraySize = 1;
        swapInfo.mipCount = 1;
        swapInfo.sampleCount = 1;
        CheckXr(xrCreateSwapchain(xr_->session, &swapInfo, &eye.swapchain), "xrCreateSwapchain");
        uint32_t imageCount = 0;
        CheckXr(xrEnumerateSwapchainImages(eye.swapchain, 0, &imageCount, nullptr), "xrEnumerateSwapchainImages(count)");
        eye.images.resize(imageCount, XrSwapchainImageOpenGLESKHR{XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR});
        CheckXr(xrEnumerateSwapchainImages(eye.swapchain, imageCount, &imageCount,
                                           reinterpret_cast<XrSwapchainImageBaseHeader*>(eye.images.data())),
                "xrEnumerateSwapchainImages");
    }

    // --- 5) Eingabe-Aktionen: Strahl, Trigger, Griff, Stick, A-Knopf ----------
    xrStringToPath(xr_->instance, "/user/hand/left", &xr_->handPaths[0]);
    xrStringToPath(xr_->instance, "/user/hand/right", &xr_->handPaths[1]);

    XrActionSetCreateInfo setInfo{XR_TYPE_ACTION_SET_CREATE_INFO};
    std::strcpy(setInfo.actionSetName, "rttr_input");
    std::strcpy(setInfo.localizedActionSetName, "RTTR Eingabe");
    setInfo.priority = 0;
    CheckXr(xrCreateActionSet(xr_->instance, &setInfo, &xr_->actionSet), "xrCreateActionSet");

    auto createAction = [&](const char* name, const char* localized, XrActionType type, XrAction& action) {
        XrActionCreateInfo actionInfo{XR_TYPE_ACTION_CREATE_INFO};
        actionInfo.actionType = type;
        std::strcpy(actionInfo.actionName, name);
        std::strcpy(actionInfo.localizedActionName, localized);
        actionInfo.countSubactionPaths = 2;
        actionInfo.subactionPaths = xr_->handPaths;
        CheckXr(xrCreateAction(xr_->actionSet, &actionInfo, &action), name);
    };
    createAction("aim_pose", "Zeigen", XR_ACTION_TYPE_POSE_INPUT, xr_->aimPoseAction);
    createAction("trigger", "Auswählen", XR_ACTION_TYPE_BOOLEAN_INPUT, xr_->triggerAction);
    createAction("squeeze", "Kontext", XR_ACTION_TYPE_BOOLEAN_INPUT, xr_->squeezeAction);
    createAction("thumbstick", "Verschieben", XR_ACTION_TYPE_VECTOR2F_INPUT, xr_->stickAction);
    createAction("button_b", "Game menu", XR_ACTION_TYPE_BOOLEAN_INPUT, xr_->bButtonAction);
    createAction("button_a", "Zoom / build", XR_ACTION_TYPE_BOOLEAN_INPUT, xr_->aButtonAction);
    createAction("cycle_speed", "Spielgeschwindigkeit", XR_ACTION_TYPE_BOOLEAN_INPUT, xr_->speedAction);
    createAction("haptic", "Click feedback", XR_ACTION_TYPE_VIBRATION_OUTPUT, xr_->hapticAction);
    createAction("vr_menu", "VR-Schnellmenü", XR_ACTION_TYPE_BOOLEAN_INPUT, xr_->menuAction);
    createAction("reset_panel", "Panel zurücksetzen", XR_ACTION_TYPE_BOOLEAN_INPUT, xr_->resetPanelAction);

    for(int hand = 0; hand < 2; hand++)
    {
        XrActionSpaceCreateInfo actionSpaceInfo{XR_TYPE_ACTION_SPACE_CREATE_INFO};
        actionSpaceInfo.action = xr_->aimPoseAction;
        actionSpaceInfo.subactionPath = xr_->handPaths[hand];
        actionSpaceInfo.poseInActionSpace = IdentityPose();
        CheckXr(xrCreateActionSpace(xr_->session, &actionSpaceInfo, &xr_->aimSpaces[hand]), "xrCreateActionSpace");
    }

    // Bindings: Quest-(Plus)-, Quest-2-Controller und generischer Fallback.
    auto suggest = [&](const char* profile, const char* triggerPath, const char* squeezePath, const char* stickPath,
                       const char* aPath) {
        XrPath profilePath;
        if(xrStringToPath(xr_->instance, profile, &profilePath) != XR_SUCCESS)
            return;
        const struct
        {
            XrAction action;
            const char* path;
        } bindings[] = {
          {xr_->hapticAction, "/output/haptic"},
          {xr_->aimPoseAction, "/input/aim/pose"},
          {xr_->triggerAction, triggerPath},
          {xr_->squeezeAction, squeezePath},
          {xr_->stickAction, stickPath},
          {xr_->aButtonAction, aPath},
          {xr_->bButtonAction, aPath ? "/input/b/click" : nullptr},
          {xr_->speedAction, aPath ? "/input/y/click" : nullptr},
          {xr_->menuAction, aPath ? "/input/menu/click" : nullptr},
          {xr_->resetPanelAction, stickPath ? "/input/thumbstick/click" : nullptr},
        };
        std::vector<XrActionSuggestedBinding> suggested;
        for(int hand = 0; hand < 2; hand++)
        {
            const char* handName = hand == 0 ? "left" : "right";
            char fullPath[XR_MAX_PATH_LENGTH];
            for(const auto& b : bindings)
            {
                if((b.action == xr_->bButtonAction && hand != 1) || !b.path || ((b.action == xr_->speedAction || b.action == xr_->menuAction) && hand != 0))
                    continue; // Aktion hat auf diesem Profil kein Pendant
                XrActionSuggestedBinding binding{};
                binding.action = b.action;
                const char* component = (hand == 0 && b.action == xr_->aButtonAction) ? "/input/x/click" : b.path;
                std::snprintf(fullPath, sizeof(fullPath), "/user/hand/%s%s", handName, component);
                xrStringToPath(xr_->instance, fullPath, &binding.binding);
                suggested.push_back(binding);
            }
        }
        XrInteractionProfileSuggestedBinding suggestInfo{XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING};
        suggestInfo.interactionProfile = profilePath;
        suggestInfo.suggestedBindings = suggested.data();
        suggestInfo.countSuggestedBindings = static_cast<uint32_t>(suggested.size());
        CheckXr(xrSuggestInteractionProfileBindings(xr_->instance, &suggestInfo), "xrSuggestInteractionProfileBindings");
    };
    suggest("/interaction_profiles/oculus/touch_controller", "/input/trigger/value", "/input/squeeze/value",
            "/input/thumbstick", "/input/a/click");
    suggest("/interaction_profiles/khr/simple_controller", "/input/select/click", "/input/menu/click", nullptr,
            nullptr);
    XrSessionActionSetsAttachInfo attachInfo{XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO};
    attachInfo.countActionSets = 1;
    attachInfo.actionSets = &xr_->actionSet;
    CheckXr(xrAttachSessionActionSets(xr_->session, &attachInfo), "xrAttachSessionActionSets");
#endif // RTTR_XR_HAS_OPENXR

    if(!CreateOffscreenBackbuffer(Extent(newSize.width, newSize.height)))
        return false;

    initialized = true;
    return true;
}

bool VideoOpenXR::CreateOffscreenBackbuffer(Extent size)
{
#ifdef RTTR_XR_HAS_OPENXR
    // Allocate before replacing the current framebuffer, so a failed resize is recoverable.
    GLuint texture = 0, framebuffer = 0;
    GLint previousTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindTexture(GL_TEXTURE_2D, previousTexture == static_cast<GLint>(backbufferColor_) ? 0 : previousTexture);
    if(!complete)
    {
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &texture);
        glBindFramebuffer(GL_FRAMEBUFFER, backbufferFbo_);
        return false;
    }
    DestroyOffscreenBackbuffer();
    backbufferColor_ = texture;
    backbufferFbo_ = framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, backbufferFbo_);
    return true;
#else
    (void)size;
    return true; // Stub ohne GL
#endif
}

void VideoOpenXR::DestroyOffscreenBackbuffer()
{
#ifdef RTTR_XR_HAS_OPENXR
    if(backbufferFbo_)
        glDeleteFramebuffers(1, &backbufferFbo_);
    if(backbufferColor_)
        glDeleteTextures(1, &backbufferColor_);
#endif
    backbufferFbo_ = 0;
    backbufferColor_ = 0;
}

bool VideoOpenXR::ResizeScreen(VideoMode newSize, DisplayMode displayMode)
{
    const auto modes = ListVideoModes();
    if(std::find(modes.begin(), modes.end(), newSize) == modes.end())
        return false;
    if(newSize == GetWindowSize())
    {
        displayMode_ = displayMode;
        return true;
    }
#ifdef RTTR_XR_HAS_OPENXR
    if(!xr_->session) return false;
    XrState::Eye replacement;
    replacement.width = newSize.width;
    replacement.height = newSize.height;
    try
    {
        XrSwapchainCreateInfo info{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        info.usageFlags = XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        info.format = GL_RGBA8;
        info.width = newSize.width;
        info.height = newSize.height;
        info.faceCount = info.arraySize = info.mipCount = info.sampleCount = 1;
        CheckXr(xrCreateSwapchain(xr_->session, &info, &replacement.swapchain), "resize panel swapchain");
        uint32_t count = 0;
        CheckXr(xrEnumerateSwapchainImages(replacement.swapchain, 0, &count, nullptr), "resize image count");
        replacement.images.resize(count, XrSwapchainImageOpenGLESKHR{XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_ES_KHR});
        CheckXr(xrEnumerateSwapchainImages(replacement.swapchain, count, &count,
                  reinterpret_cast<XrSwapchainImageBaseHeader*>(replacement.images.data())), "resize images");
        if(!CreateOffscreenBackbuffer(Extent(newSize.width, newSize.height)))
            throw std::runtime_error("resize framebuffer");
    } catch(const std::exception& error)
    {
        if(replacement.swapchain) xrDestroySwapchain(replacement.swapchain);
        std::fprintf(stderr, "[videoOpenXR] Resolution unchanged: %s\n", error.what());
        return false;
    }
    glFinish();
    if(xr_->panel.framebuffer) glDeleteFramebuffers(1, &xr_->panel.framebuffer);
    if(xr_->panel.swapchain) xrDestroySwapchain(xr_->panel.swapchain);
    xr_->panel = std::move(replacement);
#else
    if(!CreateOffscreenBackbuffer(Extent(newSize.width, newSize.height))) return false;
#endif
    displayMode_ = displayMode;
    SetNewSize(newSize, Extent(newSize.width, newSize.height));
    return true;
}

void VideoOpenXR::DestroyScreen()
{
#ifdef RTTR_XR_HAS_OPENXR
    if(xr_->passthroughLayer && xr_->destroyPassthroughLayer) xr_->destroyPassthroughLayer(xr_->passthroughLayer);
    if(xr_->passthrough && xr_->destroyPassthrough) xr_->destroyPassthrough(xr_->passthrough);
    xr_->passthroughLayer=XR_NULL_HANDLE;xr_->passthrough=XR_NULL_HANDLE;
    xr_->passthroughRunning=false;xr_->passthroughRequested=false;xr_->passthroughOpacityApplied=-1;
    if(xr_->hqVertices) glDeleteBuffers(1,&xr_->hqVertices);
    if(xr_->hqIndices) glDeleteBuffers(1,&xr_->hqIndices);
    if(xr_->hqTexture) glDeleteTextures(1,&xr_->hqTexture);
    xr_->hqVertices=xr_->hqIndices=xr_->hqTexture=0;xr_->hqIndexCount=0;xr_->hqAssetAttempted=false;
    if(xr_->colorProgram) glDeleteProgram(xr_->colorProgram);
    xr_->colorProgram = 0;
    xr_->colorProgramAttempted = false;
    if(xr_->recording) StopRecording(); // finalises the MP4 when the game closes
    if(xr_->recContext != EGL_NO_CONTEXT) { eglDestroyContext(xr_->eglDisplay, xr_->recContext); xr_->recContext = EGL_NO_CONTEXT; }
    xr_->recProgram = 0;
    if(xr_->session)
    {
        if(xr_->sessionRunning)
            xrRequestExitSession(xr_->session);
        for(auto* target : {&xr_->eyes[0], &xr_->eyes[1], &xr_->panel})
        {
            auto& eye = *target;
            if(eye.nativeDepth) {
                const auto del=reinterpret_cast<void (*)(GLsizei,const GLuint*)>(eglGetProcAddress("glDeleteRenderbuffers"));
                if(del) del(1,&eye.nativeDepth);
            }
            if(eye.framebuffer)
                glDeleteFramebuffers(1, &eye.framebuffer);
            if(eye.swapchain)
                xrDestroySwapchain(eye.swapchain);
            eye = VideoOpenXR::XrState::Eye{};
        }
        for(auto& space : xr_->aimSpaces)
        {
            if(space)
                xrDestroySpace(space);
            space = XR_NULL_HANDLE;
        }
        if(xr_->viewSpace) xrDestroySpace(xr_->viewSpace);
        xr_->viewSpace = XR_NULL_HANDLE;
        if(xr_->localSpace)
            xrDestroySpace(xr_->localSpace);
        xrDestroySession(xr_->session);
        xr_->session = XR_NULL_HANDLE;
    }
    if(xr_->actionSet)
        xrDestroyActionSet(xr_->actionSet);
    if(xr_->instance)
        xrDestroyInstance(xr_->instance);
    DestroyOffscreenBackbuffer();
    if(xr_->eglDisplay != EGL_NO_DISPLAY)
    {
        eglMakeCurrent(xr_->eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if(xr_->eglContext != EGL_NO_CONTEXT)
            eglDestroyContext(xr_->eglDisplay, xr_->eglContext);
        eglTerminate(xr_->eglDisplay);
    }
    if(xr_->loaderHandle)
        dlclose(xr_->loaderHandle);
    if(xr_->appContext)
    {
        if(JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv()))
            env->DeleteGlobalRef(xr_->appContext);
        xr_->appContext = nullptr;
    }
    *xr_ = VideoOpenXR::XrState{};
#endif
    DestroyOffscreenBackbuffer();
    initialized = false;
    sessionRunning_ = false;
    frameActive_ = false;
}

bool VideoOpenXR::BeginFrame()
{
#ifdef RTTR_XR_HAS_OPENXR
    if(!xr_->session || !xr_->sessionRunning)
        return false;
    XrFrameWaitInfo waitInfo{XR_TYPE_FRAME_WAIT_INFO};
    XrFrameState frameState{XR_TYPE_FRAME_STATE};
    if(xrWaitFrame(xr_->session, &waitInfo, &frameState) != XR_SUCCESS)
        return false;
    xr_->predictedDisplayTime = frameState.predictedDisplayTime;
    xr_->shouldRender = frameState.shouldRender == XR_TRUE;
    static unsigned frameNo = 0;
    if(++frameNo % 72 == 0)
        std::fprintf(stderr, "[videoOpenXR] Frame %u läuft (sessionRunning=%d)\n", frameNo, xr_->sessionRunning ? 1 : 0);
    XrFrameBeginInfo beginInfo{XR_TYPE_FRAME_BEGIN_INFO};
    return xrBeginFrame(xr_->session, &beginInfo) == XR_SUCCESS;
#else
    return sessionRunning_;
#endif
}

bool VideoOpenXR::SwapBuffers()
{
#ifdef RTTR_XR_HAS_OPENXR
    auto headquarters=std::move(xr_->headquarters);
    xr_->headquarters.clear();
#endif
    if(!BeginFrame())
        return true; // Kein Fokus: Frame schlucken, nicht blockieren.

#ifdef RTTR_XR_HAS_OPENXR
    // Backbuffer-Textur auf jedes Auge blitten (Fixed-Function-Quad über gl4es)
    // und als Projektionsschicht mit echten Kopfposen einreichen.
    uint32_t viewCount = xr_->stereo ? 2 : 1;
    XrCompositionLayerProjectionView projectionViews[2]{};
    uint32_t validViews = 0;
    XrView eyeViews[2] = {{XR_TYPE_VIEW}, {XR_TYPE_VIEW}};
    XrViewLocateInfo viewInfo{XR_TYPE_VIEW_LOCATE_INFO};
    viewInfo.viewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    viewInfo.displayTime = xr_->predictedDisplayTime;
    viewInfo.space = xr_->localSpace;
    uint32_t locatedViews = 0;
    XrViewState viewState{XR_TYPE_VIEW_STATE};
    const bool viewsValid = XR_SUCCEEDED(xrLocateViews(xr_->session, &viewInfo, &viewState, viewCount, &locatedViews, eyeViews))
      && locatedViews == viewCount
      && (viewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT)
      && (viewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT);

    // Runtime textures use native GLES names, not gl4es' virtual texture IDs.
    using AttachTexture = void (*)(GLenum, GLenum, GLenum, GLuint, GLint);
    static const auto attachTexture = reinterpret_cast<AttachTexture>(eglGetProcAddress("glFramebufferTexture2D"));
    using CheckFramebuffer = GLenum (*)(GLenum);
    static const auto checkFramebuffer = reinterpret_cast<CheckFramebuffer>(eglGetProcAddress("glCheckFramebufferStatus"));
    const VrSettings vr = CallBack->GetVrSettings();
    if(!xr_->initialDistanceApplied)
    {
        xr_->panelZ = -vr.restoredDistance();
        if(vr.rememberPanel)
        {
            // Only the size is restored; position and yaw come from the start centering.
            xr_->panelWidth = vr.panelWidthMm / 1000.f;
        }
        xr_->initialDistanceApplied = true;
    }
    const float panelArc = xr_->cylinderSupported ? vrPanel::angle(vr.viewCurvature()) : 0.f;
    GLint previousProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousProgram);
    glUseProgram(0);
    GLint matrixMode = GL_MODELVIEW;
    glGetIntegerv(GL_MATRIX_MODE, &matrixMode);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glActiveTexture(GL_TEXTURE0);
    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_FOG);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glColor4f(1.f, 1.f, 1.f, 1.f);

    // Copy the game once at panel resolution. No perspective resampling into
    // the eye buffers; OpenXR places this texture directly in the headset.
    bool panelReady = false;
    // Menu toggle for the in-app recording; a failed start is not retried until the toggle is switched off.
    if(vr.recordVideo && !xr_->recording && !xr_->recordFailed && xr_->panel.width > 0)
        StartRecording(xr_->panel.width, xr_->panel.height);
    else if(!vr.recordVideo)
    {
        if(xr_->recording) StopRecording();
        xr_->recordFailed = false;
    }
    if(xr_->shouldRender && viewsValid && attachTexture && checkFramebuffer)
    {
        auto& panel = xr_->panel;
        uint32_t imageIndex = 0;
        XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        CheckXr(xrAcquireSwapchainImage(panel.swapchain, &acquireInfo, &imageIndex), "acquire panel");
        XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        waitInfo.timeout = XR_INFINITE_DURATION;
        CheckXr(xrWaitSwapchainImage(panel.swapchain, &waitInfo), "wait panel");
        if(!panel.framebuffer)
            glGenFramebuffers(1, &panel.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, panel.framebuffer);
        attachTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, panel.images[imageIndex].image, 0);
        const GLenum status = checkFramebuffer(GL_FRAMEBUFFER);
        if(status == GL_FRAMEBUFFER_COMPLETE)
        {
            glViewport(0, 0, panel.width, panel.height);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, 1, 0, 1, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, backbufferColor_);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            const bool adjustColor = vr.brightness != 100 || vr.contrast != 100 || vr.sharpness != 0;
            if(adjustColor && !xr_->colorProgramAttempted)
            {
                xr_->colorProgramAttempted = true;
                xr_->colorProgram = CreateColorProgram();
            }
            if(adjustColor && xr_->colorProgram)
            {
                glUseProgram(xr_->colorProgram);
                glUniform1i(glGetUniformLocation(xr_->colorProgram, "image"), 0);
                glUniform1f(glGetUniformLocation(xr_->colorProgram, "sharpness"), std::max(-2, std::min(2, vr.sharpness)) * 0.4f);
                glUniform2f(glGetUniformLocation(xr_->colorProgram, "texel"), 1.f / xr_->panel.width, 1.f / xr_->panel.height);
                glUniform1f(glGetUniformLocation(xr_->colorProgram, "brightness"), std::max(50, std::min(150, vr.brightness)) / 100.f);
                glUniform1f(glGetUniformLocation(xr_->colorProgram, "contrast"), std::max(50, std::min(150, vr.contrast)) / 100.f);
            }
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(0, 0);
            glTexCoord2f(1, 0); glVertex2f(1, 0);
            glTexCoord2f(1, 1); glVertex2f(1, 1);
            glTexCoord2f(0, 1); glVertex2f(0, 1);
            glEnd();
            glUseProgram(0);
            glDisable(GL_TEXTURE_2D);
            glFlush();
            panelReady = true;
            if(xr_->recording) EncodeRecordingFrame(panel.images[imageIndex].image);
        } else
            std::fprintf(stderr, "[videoOpenXR] Panel framebuffer incomplete: 0x%x\n", status);
        XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        CheckXr(xrReleaseSwapchainImage(panel.swapchain, &releaseInfo), "release panel");
    }

    for(uint32_t i = 0; xr_->shouldRender && viewsValid && attachTexture && checkFramebuffer && i < viewCount; i++)
    {
        auto& eye = xr_->eyes[i];
        if(!eye.swapchain)
            continue;
        uint32_t imageIndex = 0;
        XrSwapchainImageAcquireInfo acquireInfo{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
        if(xrAcquireSwapchainImage(eye.swapchain, &acquireInfo, &imageIndex) != XR_SUCCESS)
            continue;
        XrSwapchainImageWaitInfo waitInfo{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO};
        waitInfo.timeout = XR_INFINITE_DURATION;
        CheckXr(xrWaitSwapchainImage(eye.swapchain, &waitInfo), "xrWaitSwapchainImage");

        if(!eye.framebuffer)
            glGenFramebuffers(1, &eye.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, eye.framebuffer);
        attachTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, eye.images[imageIndex].image, 0);
        if(!headquarters.empty() && !eye.nativeDepth) {
            const auto gen=reinterpret_cast<void (*)(GLsizei,GLuint*)>(eglGetProcAddress("glGenRenderbuffers"));
            const auto bind=reinterpret_cast<void (*)(GLenum,GLuint)>(eglGetProcAddress("glBindRenderbuffer"));
            const auto storage=reinterpret_cast<void (*)(GLenum,GLenum,GLsizei,GLsizei)>(eglGetProcAddress("glRenderbufferStorage"));
            const auto attach=reinterpret_cast<void (*)(GLenum,GLenum,GLenum,GLuint)>(eglGetProcAddress("glFramebufferRenderbuffer"));
            if(gen && bind && storage && attach) {
                constexpr GLenum renderbuffer=0x8D41,depthAttachment=0x8D00,depth16=0x81A5;
                gen(1,&eye.nativeDepth);bind(renderbuffer,eye.nativeDepth);
                storage(renderbuffer,depth16,eye.width,eye.height);
                attach(GL_FRAMEBUFFER,depthAttachment,renderbuffer,eye.nativeDepth);
                bind(renderbuffer,0);
            }
        }

        const GLenum status = checkFramebuffer(GL_FRAMEBUFFER);
        if(status != GL_FRAMEBUFFER_COMPLETE)
        {
            std::fprintf(stderr, "[videoOpenXR] Eye %u framebuffer incomplete: 0x%x\n", i, status);
            XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
            xrReleaseSwapchainImage(eye.swapchain, &releaseInfo);
            continue;
        }
        glViewport(0, 0, eye.width, eye.height);
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glDepthMask(GL_TRUE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // The panel and input use the same LOCAL-space plane, rendered from each eye.
        const auto& view = eyeViews[i];
        constexpr float nearZ = 0.05f;
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(std::tan(view.fov.angleLeft) * nearZ, std::tan(view.fov.angleRight) * nearZ,
                  std::tan(view.fov.angleDown) * nearZ, std::tan(view.fov.angleUp) * nearZ, nearZ, 100.f);
        const auto& q = view.pose.orientation;
        // Column-major inverse rotation (transpose of the eye's orientation).
        const GLfloat inverseRotation[16] = {
          1-2*(q.y*q.y+q.z*q.z), 2*(q.x*q.y-q.z*q.w), 2*(q.x*q.z+q.y*q.w), 0,
          2*(q.x*q.y+q.z*q.w), 1-2*(q.x*q.x+q.z*q.z), 2*(q.y*q.z-q.x*q.w), 0,
          2*(q.x*q.z-q.y*q.w), 2*(q.y*q.z+q.x*q.w), 1-2*(q.x*q.x+q.y*q.y), 0,
          0, 0, 0, 1};
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(inverseRotation);
        glTranslatef(-view.pose.position.x, -view.pose.position.y, -view.pose.position.z);
        glDisable(GL_TEXTURE_2D);
        if(vr.headquarters3d && vr.presentation==1 && vr.frameMoveAvailable && !vr.hideLoadingPointer && !headquarters.empty()) {
            if(xr_->hqIndexCount) {
                glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
                glEnable(GL_TEXTURE_2D);glBindTexture(GL_TEXTURE_2D,xr_->hqTexture);
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);glColor4f(1,1,1,1);
                glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);
                glBindBuffer(GL_ARRAY_BUFFER,xr_->hqVertices);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,xr_->hqIndices);
                glEnableClientState(GL_VERTEX_ARRAY);glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                glDisableClientState(GL_COLOR_ARRAY);glDisableClientState(GL_NORMAL_ARRAY);
                glVertexPointer(3,GL_FLOAT,5*sizeof(float),nullptr);
                glTexCoordPointer(2,GL_FLOAT,5*sizeof(float),reinterpret_cast<const void*>(3*sizeof(float)));
                for(const auto& h:headquarters) {
                    const float scale=h.width*xr_->panelWidth;
                    const vrPanel::Vec3 base{(h.u-.5f)*xr_->panelWidth,(.5f-h.v)*xr_->panelWidth*GetRenderSize().y/GetRenderSize().x,xr_->panelZ};
                    const auto pose=xr_->panelPose();const auto o=pose.toWorld(base);
                    const auto x=pose.toWorld({base.x+scale,base.y,base.z}),y=pose.toWorld({base.x,base.y+scale,base.z}),z=pose.toWorld({base.x,base.y,base.z+scale});
                    const GLfloat transform[]={x.x-o.x,x.y-o.y,x.z-o.z,0,y.x-o.x,y.y-o.y,y.z-o.z,0,z.x-o.x,z.y-o.y,z.z-o.z,0,o.x,o.y,o.z,1};
                    glPushMatrix();glMultMatrixf(transform);
                    glDrawElements(GL_TRIANGLES,xr_->hqIndexCount,GL_UNSIGNED_INT,nullptr);glPopMatrix();
                }
                glBindBuffer(GL_ARRAY_BUFFER,0);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
                glPopClientAttrib();glDisable(GL_TEXTURE_2D);glDisable(GL_DEPTH_TEST);

            }
        }
        if(xr_->recording)
        {
            // Recording indicator: red dot at the panel's top-left corner. Drawn in the projection layer only,
            // so it is visible in the headset but not part of the recorded game image.
            const float aspect = static_cast<float>(GetRenderSize().y) / std::max(1u, GetRenderSize().x);
            const auto dot = xr_->toWorld(-xr_->panelWidth * 0.5f + 0.05f, xr_->panelWidth * aspect * 0.5f - 0.05f,
                                          xr_->panelZ + 0.01f);
            const auto& o = view.pose.orientation;
            const XrVector3f right{1-2*(o.y*o.y+o.z*o.z), 2*(o.x*o.y+o.z*o.w), 2*(o.x*o.z-o.y*o.w)};
            const XrVector3f up{2*(o.x*o.y-o.z*o.w), 1-2*(o.x*o.x+o.z*o.z), 2*(o.y*o.z+o.x*o.w)};
            const float r = 0.018f;
            glColor3f(0.92f, 0.12f, 0.12f);
            glBegin(GL_TRIANGLE_FAN);
            for(int i = 0; i < 16; ++i)
            {
                const float angle = i * 6.283185307f / 16.f, x = std::cos(angle) * r, y = std::sin(angle) * r;
                glVertex3f(dot.x + right.x * x + up.x * y, dot.y + right.y * x + up.y * y, dot.z + right.z * x + up.z * y);
            }
            glEnd();
        }
        const vrMenuModel::PointerVisibility pointerVisibility{vr.rayVisible, vr.markerVisible};
        if(xr_->pointerValid && !vr.hideLoadingPointer && !vr.screenshotMode)
        {
            const GLfloat colors[][3] = {{1.f, 1.f, 1.f}, {0.85f, 0.85f, 0.82f}, {0.22f, 0.22f, 0.22f}, {1.f, 0.82f, 0.3f}};
            const auto& color = colors[std::max(0, std::min(3, vr.rayColor))];
            glColor3f(color[0], color[1], color[2]);
            const auto& a = xr_->pointerOrigin;
            const auto& b = xr_->pointerEnd;
            const XrVector3f beam{b.x-a.x, b.y-a.y, b.z-a.z};
            const XrVector3f towardEye{view.pose.position.x-a.x, view.pose.position.y-a.y, view.pose.position.z-a.z};
            XrVector3f side{beam.y*towardEye.z-beam.z*towardEye.y, beam.z*towardEye.x-beam.x*towardEye.z,
                           beam.x*towardEye.y-beam.y*towardEye.x};
            const float length = std::sqrt(side.x*side.x+side.y*side.y+side.z*side.z);
            const float widths[] = {0.001f, 0.003f, 0.006f};
            const float halfWidth = widths[std::max(0, std::min(2, vr.rayWidth))] * 0.5f;
            if(length > 1e-6f && pointerVisibility.showRay(vr.menuOpen))
            {
                side.x *= halfWidth/length; side.y *= halfWidth/length; side.z *= halfWidth/length;
                // A ribbon has reliable thickness even on GLES devices limited to 1-pixel lines.
                glBegin(GL_QUADS);
                glVertex3f(a.x-side.x, a.y-side.y, a.z-side.z);
                glVertex3f(a.x+side.x, a.y+side.y, a.z+side.z);
                glVertex3f(b.x+side.x, b.y+side.y, b.z+side.z);
                glVertex3f(b.x-side.x, b.y-side.y, b.z-side.z);
                glEnd();
            }
            if(xr_->pointerOnPanel && pointerVisibility.showMarker(vr.menuOpen))
            {
                const float sizes[] = {0.004f, 0.007f, 0.012f};
                const float radius = sizes[std::max(0, std::min(2, vr.markerSize))];
                const XrVector3f right{1-2*(q.y*q.y+q.z*q.z), 2*(q.x*q.y+q.z*q.w), 2*(q.x*q.z-q.y*q.w)};
                const XrVector3f up{2*(q.x*q.y-q.z*q.w), 1-2*(q.x*q.x+q.z*q.z), 2*(q.y*q.z+q.x*q.w)};
                const auto vertex = [&](float x, float y) {
                    glVertex3f(b.x + radius*(right.x*x+up.x*y), b.y + radius*(right.y*x+up.y*y),
                               b.z + radius*(right.z*x+up.z*y));
                };
                if(vr.markerShape == 2)
                {
                    glBegin(GL_QUADS);
                    vertex(-1.f,-0.18f); vertex(1.f,-0.18f); vertex(1.f,0.18f); vertex(-1.f,0.18f);
                    vertex(-0.18f,-1.f); vertex(0.18f,-1.f); vertex(0.18f,1.f); vertex(-0.18f,1.f);
                    glEnd();
                } else
                {
                    const float inner = vr.markerShape == 1 ? 0.65f : 0.f;
                    glBegin(GL_TRIANGLE_STRIP);
                    for(int segment = 0; segment <= 32; ++segment)
                    {
                        const float angle = segment * 6.283185307f / 32.f;
                        const float x = std::cos(angle), y = std::sin(angle);
                        vertex(x, y); vertex(inner*x, inner*y);
                    }
                    glEnd();
                }
            }
        }
        glFlush();

        XrSwapchainImageReleaseInfo releaseInfo{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
        xrReleaseSwapchainImage(eye.swapchain, &releaseInfo);

        auto& pv = projectionViews[validViews++];
        pv = XrCompositionLayerProjectionView{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW};
        pv.pose = view.pose;
        pv.fov = view.fov;
        pv.subImage.swapchain = eye.swapchain;
        pv.subImage.imageRect.extent = {eye.width, eye.height};
    }

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glUseProgram(previousProgram);
    glMatrixMode(matrixMode);
    glBindFramebuffer(GL_FRAMEBUFFER, backbufferFbo_);

    // Keep the layer alive until xrEndFrame has consumed its pointer.
    XrCompositionLayerProjection projectionLayer{XR_TYPE_COMPOSITION_LAYER_PROJECTION};
    XrCompositionLayerQuad panelLayer{XR_TYPE_COMPOSITION_LAYER_QUAD};
    XrCompositionLayerCylinderKHR cylinderLayer{XR_TYPE_COMPOSITION_LAYER_CYLINDER_KHR};
    std::vector<XrCompositionLayerBaseHeader*> layers;
    UpdatePassthrough(vr.passthrough,vr.passthroughPercent);
    XrCompositionLayerPassthroughFB passthroughLayer{XR_TYPE_COMPOSITION_LAYER_PASSTHROUGH_FB};
    if(xr_->passthroughRunning && xr_->shouldRender) {
        passthroughLayer.layerHandle=xr_->passthroughLayer;
        passthroughLayer.flags=XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
        layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&passthroughLayer));
    }
    if(panelReady)
    {
        panelLayer.space = xr_->localSpace;
        panelLayer.eyeVisibility = XR_EYE_VISIBILITY_BOTH;
        panelLayer.pose = IdentityPose();
        panelLayer.pose.position = xr_->toWorld(0.f, 0.f, xr_->panelZ);
        const auto orientation = xr_->panelPose().orientation();
        panelLayer.pose.orientation = {orientation.x, orientation.y, orientation.z, orientation.w};
        panelLayer.size = {xr_->panelWidth, xr_->panelWidth * GetRenderSize().y / GetRenderSize().x};
        panelLayer.subImage.swapchain = xr_->panel.swapchain;
        panelLayer.subImage.imageRect.extent = {xr_->panel.width, xr_->panel.height};
        if(panelArc > 0.f)
        {
            cylinderLayer.space = panelLayer.space;
            cylinderLayer.eyeVisibility = panelLayer.eyeVisibility;
            cylinderLayer.subImage = panelLayer.subImage;
            cylinderLayer.radius = xr_->panelWidth / panelArc;
            cylinderLayer.centralAngle = panelArc;
            cylinderLayer.aspectRatio = static_cast<float>(GetRenderSize().x) / GetRenderSize().y;
            cylinderLayer.pose = panelLayer.pose;
            cylinderLayer.pose.position = xr_->toWorld(0.f, 0.f, xr_->panelZ + cylinderLayer.radius);
            layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&cylinderLayer));
        } else
            layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&panelLayer));
    }
    if(validViews == viewCount)
    {
        projectionLayer.layerFlags = XR_COMPOSITION_LAYER_BLEND_TEXTURE_SOURCE_ALPHA_BIT;
        projectionLayer.space = xr_->localSpace;
        projectionLayer.viewCount = validViews;
        projectionLayer.views = projectionViews;
        layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&projectionLayer));
    }

    XrFrameEndInfo endInfo{XR_TYPE_FRAME_END_INFO};
    endInfo.displayTime = xr_->predictedDisplayTime;
    endInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE; // TODO(Stufe 3): ALPHA_BLEND für Passthrough
    endInfo.layerCount = static_cast<uint32_t>(layers.size());
    endInfo.layers = layers.data();
    frameActive_ = false;
    const XrResult result = xrEndFrame(xr_->session, &endInfo);
    static unsigned submittedFrames = 0;
    if(XR_FAILED(result) || ++submittedFrames % 300 == 0)
        std::fprintf(stderr, "[videoOpenXR] Submit quad: result=%d views=%u shouldRender=%d GL=0x%x\n",
                     result, validViews, xr_->shouldRender ? 1 : 0, glGetError());
    return XR_SUCCEEDED(result);
#else
    frameActive_ = false;
    return true;
#endif
}

bool VideoOpenXR::MessageLoop()
{
#ifdef RTTR_XR_HAS_OPENXR
    // 1) Offscreen-Backbuffer als Zeichnungsziel sicherstellen: RTTRs Zeichnung
    //    läuft unverändert in FBO statt 0 (surfaceless wäre unvollständig).
    if(backbufferFbo_)
        glBindFramebuffer(GL_FRAMEBUFFER, backbufferFbo_);

    // 2) Xr-Events: Zustandsautomat der Session.
    if(xr_->instance && xr_->session)
    {
        XrEventDataBuffer event{XR_TYPE_EVENT_DATA_BUFFER};
        while(xrPollEvent(xr_->instance, &event) == XR_SUCCESS)
        {
            switch(event.type)
            {
                case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING: {
                    const auto* change=reinterpret_cast<const XrEventDataReferenceSpaceChangePending*>(&event);
                    if(change->referenceSpaceType==XR_REFERENCE_SPACE_TYPE_LOCAL) {
                        xr_->recenterChangeTime=change->changeTime;
                        // The table pose is stored in LOCAL coordinates and becomes stale: search again.
                        xr_->tableFound=false;xr_->tableWidthApplied=false;xr_->nextSceneCheck=0;
                    }
                    break;
                }
                case XR_TYPE_EVENT_DATA_SPACE_QUERY_RESULTS_AVAILABLE_FB: {
                    const auto* ready=reinterpret_cast<const XrEventDataSpaceQueryResultsAvailableFB*>(&event);
                    if(ready->requestId!=xr_->tableQuery) break;
                    const auto size=GetRenderSize();
                    vrTable::Placement placement{};
                    if(PickTable(xr_->session,xr_->localSpace,xr_->viewSpace,xr_->predictedDisplayTime,ready->requestId,
                                 xr_->retrieveSpaceQueryResults,xr_->getSpaceSemanticLabels,xr_->getSpaceBoundingBox2D,
                                 static_cast<float>(size.x)/std::max(1u,size.y),placement)) {
                        xr_->tablePlacement=placement;xr_->tableFound=true;xr_->tableWidthApplied=false;xr_->tableStatus=4;
                        std::fprintf(stderr,"[videoOpenXR] Real table: centre %.2f %.2f %.2f, width %.2f m, area %.2f m2\n",
                                     placement.center.x,placement.center.y,placement.center.z,placement.width,placement.area);
                    }
                    break;
                }
                case XR_TYPE_EVENT_DATA_SPACE_QUERY_COMPLETE_FB: {
                    const auto* done=reinterpret_cast<const XrEventDataSpaceQueryCompleteFB*>(&event);
                    if(done->requestId!=xr_->tableQuery) break;
                    xr_->tableQueryPending=false;
                    if(!xr_->tableFound) {
                        // No scan yet or no table in it: try again later (the user may scan the room meanwhile).
                        xr_->tableStatus=3;xr_->nextSceneCheck=GetTickCount()+10000;
                        std::fprintf(stderr,"[videoOpenXR] Scene query complete (%d), no table found\n",static_cast<int>(done->result));
                    }
                    break;
                }
                case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
                {
                    auto* stateChanged = reinterpret_cast<XrEventDataSessionStateChanged*>(&event);
                    xr_->sessionState = stateChanged->state;
                    std::fprintf(stderr, "[videoOpenXR] Session-State: %d\n", static_cast<int>(stateChanged->state));
                    if(stateChanged->state == XR_SESSION_STATE_READY)
                    {
                        XrSessionBeginInfo beginInfo{XR_TYPE_SESSION_BEGIN_INFO};
                        beginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                        if(xrBeginSession(xr_->session, &beginInfo) == XR_SUCCESS)
                            xr_->sessionRunning = true;
                    } else if(stateChanged->state == XR_SESSION_STATE_STOPPING)
                    {
                        xrEndSession(xr_->session);
                        xr_->sessionRunning = false;
                    } else if(stateChanged->state == XR_SESSION_STATE_LOSS_PENDING ||
                              stateChanged->state == XR_SESSION_STATE_EXITING)
                    {
                        xr_->sessionRunning = false;
                    }
                    break;
                }
                default: break;
            }
            event = XrEventDataBuffer{XR_TYPE_EVENT_DATA_BUFFER};
        }
    }

    const VrSettings vr = CallBack->GetVrSettings();
    if(!xr_->initialDistanceApplied)
    {
        xr_->panelZ = -vr.restoredDistance();
        if(vr.rememberPanel)
        {
            // Only the size is restored; position and yaw come from the start centering.
            xr_->panelWidth = vr.panelWidthMm / 1000.f;
        }
        xr_->initialDistanceApplied = true;
    }
    const auto panelNow = GetTickCount();
    const float panelDt = xr_->lastPanelUpdate == 0 ? 0.f
      : std::min(0.05f, static_cast<float>(panelNow - xr_->lastPanelUpdate) / 1000.f);
    xr_->lastPanelUpdate = panelNow;
    xr_->pointerValid = false;
    xr_->pointerOnPanel = false;
    // VR-022: look for a real table once the option is on; ask for the scene permission only then.
    if(vr.realTable && xr_->sceneSupported && xr_->sessionRunning && !xr_->tableQueryPending && !xr_->tableFound
       && panelNow >= xr_->nextSceneCheck)
    {
        xr_->nextSceneCheck = panelNow + 3000;
        xr_->scenePermission = xr_->scenePermission || CallSceneHelper("hasScenePermission", false);
        if(!xr_->scenePermission)
        {
            xr_->tableStatus = 1;
            if(!xr_->scenePermissionRequested) { CallSceneHelper("requestScenePermission", true); xr_->scenePermissionRequested = true; }
        } else if(StartTableQuery(xr_->querySpaces, xr_->session, xr_->tableQuery))
        {
            xr_->tableQueryPending = true;
            xr_->tableStatus = 2;
        }
    }
    const auto finishFrameMove = [&] {
        if(!xr_->movingFrame) return;
        xr_->movingFrame=false;xr_->filterValid=false;
        CallBack->SaveVrPanel(static_cast<int>(xr_->panelWidth*1000),static_cast<int>(xr_->panelOrigin.x*1000),
          static_cast<int>(xr_->panelOrigin.y*1000),static_cast<int>(xr_->panelOrigin.z*1000),
          static_cast<int>(xr_->panelYaw*1000),static_cast<int>(-xr_->panelZ*1000),xr_->presentation);
    };
    const auto releaseInput = [&] {
        finishFrameMove();xr_->previousFrameGrip=false;
        const auto oldPos=mouse_xy.pos; mouse_xy.pos=Position(-10000,-10000);
        if(mouse_xy.ldown) { mouse_xy.ldown = false; CallBack->Msg_LeftUp(mouse_xy); }
        if(mouse_xy.rdown) { mouse_xy.rdown = false; CallBack->Msg_RightUp(mouse_xy); }
        mouse_xy.pos=oldPos;
        for(int h=0; h<2; ++h) xr_->prevTrigger[h] = xr_->prevSqueeze[h] = xr_->prevA[h] = false;
        for(auto& gesture:xr_->buttonGestures) gesture={};
        xr_->mapGrab={}; // a pending tap is dropped, never turned into a late click
        xr_->inputArmed=false;
        xr_->filterValid = false;
        xr_->stickTurbo={}; xr_->pan={}; xr_->zoomRepeat={};
        xr_->menuStick={}; for(auto& latch:xr_->menuLatch) latch=false;
    };
    if(vr.hideLoadingPointer) { releaseInput();return true; }
    // On a real table the board lies flat on its surface; otherwise the simulated table/screen settings apply.
    const bool realTableActive = vr.presentation == 1 && vr.realTable && xr_->tableFound;
    const float targetPitch = realTableActive ? -1.5707963f : vr.viewPitch();
    const float targetHeight = realTableActive ? 0.f : vr.viewHeight();
    // Leaving the real table (option off, canvas, table lost): place the board in front of the user again,
    // otherwise it would stay at the table position with the simulated tilt.
    if(xr_->realTableWasActive && !realTableActive) xr_->startPoseApplied = false;
    xr_->realTableWasActive = realTableActive;
    if(xr_->presentation != vr.presentation || xr_->panelPitch != targetPitch || xr_->panelHeight != targetHeight)
    {
        releaseInput(); // Changing the surface must not turn a held trigger into a game click.
        if(xr_->presentation != vr.presentation) xr_->panelZ = -vr.restoredDistance();
        xr_->presentation = vr.presentation;
        xr_->panelPitch = targetPitch;
        xr_->panelHeight = targetHeight;
    }
    if(xr_->panelControl != vr.panelControl) { releaseInput(); xr_->panelControl=vr.panelControl; }
    const bool focused = xr_->sessionState == XR_SESSION_STATE_FOCUSED;
    if(focused != xr_->hadFocus)
    {
        if(!focused) releaseInput();
        CallBack->VrFocus(focused);
        xr_->hadFocus = focused;
    }
    const int requestedHand = vr.leftHanded ? 0 : 1;
    if(requestedHand != xr_->pointerHand) { releaseInput(); xr_->pointerHand = requestedHand; }
    const auto recenter = [&](bool fullReset=false) {
        XrSpaceLocation head{XR_TYPE_SPACE_LOCATION};
        if(xr_->predictedDisplayTime <= 0 || XR_FAILED(xrLocateSpace(xr_->viewSpace, xr_->localSpace,
              xr_->predictedDisplayTime, &head)) ||
           !(head.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT) ||
           !(head.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)) return false;
        finishFrameMove();
        if(fullReset) xr_->panelWidth=2.f;
        const auto& q = head.pose.orientation;
        xr_->panelYaw = std::atan2(2.f*(q.x*q.z+q.w*q.y), 1.f-2.f*(q.x*q.x+q.y*q.y));
        xr_->panelOrigin = head.pose.position;
        xr_->panelZ = -(fullReset ? vr.viewStartDistance() : vr.restoredDistance());
        xr_->filterValid = false;
        xr_->panelSaveAt = panelNow + 500;
        return true;
    };
    // App start always places the panel in front of the user's gaze, regardless of the last saved
    // position. This must not wait for input focus, otherwise the old pose is visible first.
    if(xr_->session && xr_->sessionRunning)
    {
        const bool systemReset=xr_->recenterChangeTime>0 && xr_->predictedDisplayTime>=xr_->recenterChangeTime;
        if(systemReset || vr.recenterRequest != xr_->recenterSeen || !xr_->startPoseApplied)
        {
            // Retry while the headset does not yet provide a valid tracked pose.
            if(recenter(systemReset || vr.recenterRequest != xr_->recenterSeen)) {
                xr_->startPoseApplied=true;xr_->recenterSeen=vr.recenterRequest;
                if(systemReset) xr_->recenterChangeTime=0;
            }
        }
    }
    // A menu button armed itself ("Beenden": click again to confirm): short double pulse on the pointer hand.
    if(const int confirmPulse = xr_->confirmPulse.poll(vr.hapticConfirmRequest, panelNow))
    {
        if(vr.haptics && xr_->session && xr_->hapticAction != XR_NULL_HANDLE)
        {
            XrHapticActionInfo info{XR_TYPE_HAPTIC_ACTION_INFO};
            info.action = xr_->hapticAction; info.subactionPath = xr_->handPaths[xr_->pointerHand];
            XrHapticVibration vibration{XR_TYPE_HAPTIC_VIBRATION};
            vibration.duration = 40000000; vibration.frequency = XR_FREQUENCY_UNSPECIFIED;
            vibration.amplitude = confirmPulse == 1 ? 0.5f : 0.6f;
            xrApplyHapticFeedback(xr_->session, &info, reinterpret_cast<const XrHapticBaseHeader*>(&vibration));
        }
    }
    if(vr.realTable && vr.recenterRequest != xr_->tableRecenterSeen)
    {
        // "Centre" on the real table means: look for the table again (it may have moved).
        xr_->tableRecenterSeen = vr.recenterRequest;
        if(!xr_->tableQueryPending) { xr_->tableFound = false; xr_->tableWidthApplied = false; xr_->nextSceneCheck = 0; }
    }
    if(realTableActive && xr_->tableFound)
    {
        if(!xr_->tableWidthApplied) { xr_->panelWidth = xr_->tablePlacement.width; xr_->tableWidthApplied = true; }
        // Keep the board centre on the table for any panel distance; runs after recentering so it wins.
        xr_->panelYaw = xr_->tablePlacement.yaw;
        const auto origin = vrTable::originFor(xr_->tablePlacement.center, xr_->panelYaw, xr_->panelZ);
        xr_->panelOrigin = {origin.x, origin.y, origin.z};
    }
    // 3) Eingabe: Aktionen synchronisieren, Strahl → Maus, Knöpfe → Klicks.
    if(xr_->session && xr_->sessionRunning && xr_->actionSet && xr_->sessionState == XR_SESSION_STATE_FOCUSED)
    {
        XrActiveActionSet activeSet{xr_->actionSet, XR_NULL_PATH};
        XrActionsSyncInfo syncInfo{XR_TYPE_ACTIONS_SYNC_INFO};
        syncInfo.countActiveActionSets = 1;
        syncInfo.activeActionSets = &activeSet;
        if(xrSyncActions(xr_->session, &syncInfo) == XR_SUCCESS)
        {
            // A held button must be released after focus/hand changes before it can act again.
            if(!xr_->inputArmed)
            {
                const XrAction neutralActions[]={xr_->triggerAction,xr_->squeezeAction,xr_->aButtonAction,
                    xr_->bButtonAction,xr_->speedAction,xr_->menuAction,xr_->resetPanelAction};
                for(auto action:neutralActions) for(int h=0;h<2;++h)
                {
                    XrActionStateGetInfo info{XR_TYPE_ACTION_STATE_GET_INFO}; info.action=action; info.subactionPath=xr_->handPaths[h];
                    XrActionStateBoolean state{XR_TYPE_ACTION_STATE_BOOLEAN};
                    if(XR_SUCCEEDED(xrGetActionStateBoolean(xr_->session,&info,&state)) && state.isActive && state.currentState) return true;
                }
                xr_->inputArmed=true;
            }
            XrActionStateGetInfo menuInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            menuInfo.action = xr_->menuAction;
            menuInfo.subactionPath = xr_->handPaths[0];
            XrActionStateBoolean menuState{XR_TYPE_ACTION_STATE_BOOLEAN};
            if(XR_SUCCEEDED(xrGetActionStateBoolean(xr_->session, &menuInfo, &menuState))
               && menuState.isActive && menuState.changedSinceLastSync && menuState.currentState)
                CallBack->Msg_KeyDown(KeyEvent(KeyType::VrQuickMenu));
            // Reset works even when the controller does not point at the panel.
            XrActionStateGetInfo resetInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            resetInfo.action = xr_->resetPanelAction;
            resetInfo.subactionPath = xr_->handPaths[0];
            XrActionStateBoolean resetState{XR_TYPE_ACTION_STATE_BOOLEAN};
            const bool resetPressed = XR_SUCCEEDED(xrGetActionStateBoolean(xr_->session, &resetInfo, &resetState))
              && resetState.isActive && resetState.currentState;
            XrActionStateGetInfo gripInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            gripInfo.action=xr_->squeezeAction; gripInfo.subactionPath=xr_->handPaths[1];
            XrActionStateBoolean gripState{XR_TYPE_ACTION_STATE_BOOLEAN};
            const bool gripHeld=XR_SUCCEEDED(xrGetActionStateBoolean(xr_->session,&gripInfo,&gripState)) && gripState.isActive && gripState.currentState;
            gripInfo.subactionPath=xr_->handPaths[0];
            XrActionStateBoolean leftGrip{XR_TYPE_ACTION_STATE_BOOLEAN};
            const bool zoomHeld=XR_SUCCEEDED(xrGetActionStateBoolean(xr_->session,&gripInfo,&leftGrip)) && leftGrip.isActive && leftGrip.currentState;
            const bool panelUnlocked=!zoomHeld && !gripHeld && !xr_->movingFrame && vr.panelControl==0;
            if(panelUnlocked && resetPressed && !xr_->prevResetPanel)
            {
                xr_->panelWidth = 2.0f;
                recenter(true);
                std::fprintf(stderr, "[videoOpenXR] Panel reset: width=2.0 distance=%.1f\n", -xr_->panelZ);
            }
            xr_->prevResetPanel = resetPressed;

            // Left stick adjusts the panel independently of the map controls.
            XrActionStateGetInfo panelStickInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            panelStickInfo.action = xr_->stickAction;
            panelStickInfo.subactionPath = xr_->handPaths[0];
            XrActionStateVector2f panelStick{XR_TYPE_ACTION_STATE_VECTOR2F};
            const bool leftStickActive=XR_SUCCEEDED(xrGetActionStateVector2f(xr_->session,&panelStickInfo,&panelStick)) && panelStick.isActive;
            const auto leftAxes=vrInput::mapAxes(panelStick.currentState.x,panelStick.currentState.y,vr.invertLeftX,vr.invertLeftY,vr.swapLeftStickAxes);
            const int zoomStep=xr_->zoomRepeat.sample(panelStick.currentState.y*(vr.invertLeftY ? -1.f : 1.f),zoomHeld && leftStickActive && !resetPressed && xr_->inputArmed && !xr_->movingFrame,panelNow);
            if(zoomStep) { KeyEvent key(KeyType::VrZoom);key.c=zoomStep>0 ? 1 : 2;CallBack->Msg_KeyDown(key); }
            if(panelUnlocked && !resetPressed && leftStickActive)
            {
                const auto axis = [](float value) {
                    constexpr float deadzone = 0.25f;
                    return std::fabs(value) <= deadzone ? 0.f
                      : std::copysign((std::fabs(value)-deadzone)/(1.f-deadzone), value);
                };
                const float oldWidth = xr_->panelWidth, oldZ = xr_->panelZ;
                const float widthAxis = leftAxes.x;
                const float distanceAxis = leftAxes.y;
                xr_->panelWidth = std::max(1.0f, std::min(3.2f, xr_->panelWidth + axis(widthAxis)*panelDt));
                // Stick forward pushes the panel away; back pulls it closer.
                xr_->panelZ = std::max(-3.5f, std::min(-0.7f, xr_->panelZ - axis(distanceAxis)*panelDt));
                // Limit apparent horizontal size to roughly 31–84 degrees at
                // the reference origin. Physical head movement remains unrestricted.
                const float distance = -xr_->panelZ;
                if(axis(widthAxis)!=0 || axis(distanceAxis)!=0)
                    xr_->panelWidth = std::max(0.55f*distance, std::min(1.8f*distance, xr_->panelWidth));
                if(oldWidth != xr_->panelWidth || oldZ != xr_->panelZ) xr_->panelSaveAt = panelNow + 700;
            }
            // Dominante Hand: rechts (Quest). Linker Griff bleibt für Stufe 3 (Brett greifen).
            if(xr_->panelSaveAt && panelNow >= xr_->panelSaveAt)
            {
                if(vr.rememberPanel || vr.rememberDistance) CallBack->SaveVrPanel(static_cast<int>(xr_->panelWidth*1000),
                    static_cast<int>(xr_->panelOrigin.x*1000), static_cast<int>(xr_->panelOrigin.y*1000),
                    static_cast<int>(xr_->panelOrigin.z*1000), static_cast<int>(xr_->panelYaw*1000), static_cast<int>(-xr_->panelZ*1000), xr_->presentation);
                xr_->panelSaveAt = 0;
            }
            const int hand = xr_->pointerHand;
            XrActionStateGetInfo triggerInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            triggerInfo.action = xr_->triggerAction; triggerInfo.subactionPath = xr_->handPaths[hand];
            XrActionStateBoolean triggerNow{XR_TYPE_ACTION_STATE_BOOLEAN};
            const bool triggerPressed = XR_SUCCEEDED(xrGetActionStateBoolean(xr_->session, &triggerInfo, &triggerNow))
              && triggerNow.isActive && triggerNow.currentState;
            if(triggerPressed && !xr_->prevTrigger[hand]) xr_->clickLockUntil = panelNow + 85;


            // --- Strahl → Mausposition über Panel-Ebene ----------------------
            XrActionStateGetInfo poseInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            poseInfo.action = xr_->aimPoseAction;
            poseInfo.subactionPath = xr_->handPaths[hand];
            XrActionStatePose poseState{XR_TYPE_ACTION_STATE_POSE};
            xrGetActionStatePose(xr_->session, &poseInfo, &poseState);
            XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
            if(poseState.isActive && xr_->predictedDisplayTime > 0 && xrLocateSpace(xr_->aimSpaces[hand], xr_->localSpace, xr_->predictedDisplayTime, &location) == XR_SUCCESS &&
               (location.locationFlags & XR_SPACE_LOCATION_POSITION_VALID_BIT)
               && (location.locationFlags & XR_SPACE_LOCATION_ORIENTATION_VALID_BIT))
            {
                const XrVector3f& o = location.pose.position;
                const XrQuaternionf& q = location.pose.orientation;
                // Richtung aus der Quaternion: -Z rotiert.
                XrVector3f dir{2.f * (q.x * q.z + q.w * q.y), 2.f * (q.y * q.z - q.w * q.x),
                               1.f - 2.f * (q.x * q.x + q.y * q.y)};
                dir.x = -dir.x;
                dir.y = -dir.y;
                dir.z = -dir.z;
                xr_->pointerValid = true;
                xr_->pointerOrigin = o;
                xr_->pointerEnd = {o.x+1.f*dir.x, o.y+1.f*dir.y, o.z+1.f*dir.z};
                if(xr_->movingFrame) {
                    if(!gripHeld || !vr.frameMove || !vr.frameMoveAvailable) { releaseInput();return true; }
                    const auto pos=xr_->frameDrag.translated({o.x,o.y,o.z},{dir.x,dir.y,dir.z});
                    xr_->panelOrigin={pos.x,pos.y,pos.z};
                    const auto end=xr_->frameDrag.target({o.x,o.y,o.z},{dir.x,dir.y,dir.z});
                    xr_->pointerEnd={end.x,end.y,end.z};
                    // Translation only. Width, distance parameter, yaw, pitch and curvature stay unchanged.
                    return true;
                }
                vrPanel::Hit hit;
                const float aspect = static_cast<float>(GetRenderSize().y) / GetRenderSize().x;
                const float arc = xr_->cylinderSupported ? vrPanel::angle(vr.viewCurvature()) : 0.f;
                const auto localO = xr_->toLocal(o, true), localD = xr_->toLocal(dir, false);
                xr_->handDepth = -localO.z;
                if(vrPanel::intersect({localO.x, localO.y, localO.z},
                                      {localD.x, localD.y, localD.z}, xr_->panelWidth, xr_->panelWidth * aspect,
                                      xr_->panelZ, arc, hit))
                {
                    if(hand==1 && gripHeld && !xr_->previousFrameGrip && vr.frameMove && vr.frameMoveAvailable
                       && !triggerPressed && !mouse_xy.ldown && !mouse_xy.rdown && vrPanel::moveFrameHit(hit.u,hit.v)) {
                        const auto anchor=xr_->panelPose().toWorld(hit.point);
                        xr_->frameDrag.begin({xr_->panelOrigin.x,xr_->panelOrigin.y,xr_->panelOrigin.z},anchor,{o.x,o.y,o.z});
                        xr_->movingFrame=true;xr_->filterValid=false;xr_->previousFrameGrip=true;
                        xr_->pointerEnd={anchor.x,anchor.y,anchor.z};return true;
                    }
                    xr_->pointerOnPanel = true;
                    if(!xr_->filterValid) { xr_->smoothU = hit.u; xr_->smoothV = hit.v; }
                    else if(!vr.clickStabilization || !triggerPressed || panelNow >= xr_->clickLockUntil)
                    {
                        const float alpha = vrInput::smoothingAlpha(vr.smoothing,panelDt);
                        xr_->smoothU += alpha*(hit.u-xr_->smoothU);
                        xr_->smoothV += alpha*(hit.v-xr_->smoothV);
                    }
                    xr_->filterValid = true;
                    hit.u = xr_->smoothU; hit.v = xr_->smoothV;
                    float px = (hit.u-0.5f)*xr_->panelWidth, pz = xr_->panelZ;
                    if(arc > 0.f) { const float radius=xr_->panelWidth/arc, theta=(hit.u-0.5f)*arc;
                        px=radius*std::sin(theta); pz+=radius*(1.f-std::cos(theta)); }
                    xr_->pointerEnd = xr_->toWorld(px, (0.5f-hit.v)*xr_->panelWidth*aspect, pz);
                    xr_->pointerEnd.x -= dir.x*0.002f; xr_->pointerEnd.y -= dir.y*0.002f; xr_->pointerEnd.z -= dir.z*0.002f;
                    const VideoMode wnSize = GetWindowSize();
                    // Pointer offset X/Y (duty P3) shifts only the game cursor, not the ray; inside VR menus none.
                    const float gameU = vr.vrMenuFocus ? hit.u : vrMenuModel::ptroff::apply(hit.u, vrMenuModel::ptroff::fromQuarters(vr.ptrOffsetX));
                    const float gameV = vr.vrMenuFocus ? hit.v : vrMenuModel::ptroff::apply(hit.v, vrMenuModel::ptroff::fromQuarters(vr.ptrOffsetY));
                    const Position screenPos(std::min(static_cast<int>(wnSize.width) - 1, static_cast<int>(gameU * wnSize.width)),
                                             std::min(static_cast<int>(wnSize.height) - 1, static_cast<int>(gameV * wnSize.height)));
                    const Position newPos = getGuiScale().screenToView(screenPos);
                    if(newPos != mouse_xy.pos)
                    {
                        mouse_xy.pos = newPos;
                        CallBack->Msg_MouseMove(mouse_xy);
                    }
                }
            }

            if(!xr_->pointerValid && xr_->movingFrame) { releaseInput();return true; }
            xr_->previousFrameGrip=gripHeld;
            if(!xr_->pointerOnPanel) xr_->filterValid = false;
            auto buttonState = [&](XrAction action, bool& previous, auto onDown, auto onUp) {
                XrActionStateGetInfo getInfo{XR_TYPE_ACTION_STATE_GET_INFO};
                getInfo.action = action;
                getInfo.subactionPath = xr_->handPaths[hand];
                XrActionStateBoolean state{XR_TYPE_ACTION_STATE_BOOLEAN};
                if(xrGetActionStateBoolean(xr_->session, &getInfo, &state) == XR_SUCCESS && state.isActive)
                {
                    const bool pressed = state.currentState == XR_TRUE && xr_->pointerOnPanel;
                    if(pressed && !previous)
                        onDown();
                    else if(!pressed && previous)
                        onUp();
                    previous = pressed;
                } else if(previous)
                {
                    onUp();
                    previous = false;
                }
            };
            // Trigger = Linksklick (Auswählen, Befehle; Doppelklick erkennt der
            // WindowManager zeit-/positionsbasiert selbst).
            const auto pulse = [&](float amplitude) {
                if(!vr.haptics) return;
                XrHapticActionInfo info{XR_TYPE_HAPTIC_ACTION_INFO};
                info.action=xr_->hapticAction; info.subactionPath=xr_->handPaths[hand];
                XrHapticVibration vibration{XR_TYPE_HAPTIC_VIBRATION};
                vibration.duration=18000000; vibration.frequency=XR_FREQUENCY_UNSPECIFIED; vibration.amplitude=amplitude;
                xrApplyHapticFeedback(xr_->session, &info, reinterpret_cast<const XrHapticBaseHeader*>(&vibration));
            };
            buttonState(xr_->triggerAction, xr_->prevTrigger[hand], [&] {
                // Optional map grab: defer the click until release, so moving the hand can pan/zoom instead.
                if(vr.triggerGrab && CallBack->VrMapAt(mouse_xy))
                {
                    xr_->mapGrab.begin(mouse_xy.pos.x, mouse_xy.pos.y, xr_->handDepth);
                    xr_->grabLastPos = mouse_xy.pos;
                    pulse(0.25f);
                    return;
                }
                mouse_xy.ldown = true;
                CallBack->Msg_LeftDown(mouse_xy);
                pulse(0.25f);
            }, [&] {
                if(xr_->mapGrab.engaged())
                {
                    if(xr_->mapGrab.end())
                    {
                        // Tap: click where the trigger was pressed.
                        const auto current=mouse_xy.pos;
                        mouse_xy.pos=Position(xr_->mapGrab.startX, xr_->mapGrab.startY);
                        mouse_xy.ldown = true; CallBack->Msg_LeftDown(mouse_xy);
                        mouse_xy.ldown = false; CallBack->Msg_LeftUp(mouse_xy);
                        mouse_xy.pos=current;
                    }
                    return;
                }
                mouse_xy.ldown = false;
                CallBack->Msg_LeftUp(mouse_xy);
            });
            if(xr_->mapGrab.engaged())
            {
                const int zoom=xr_->mapGrab.sample(mouse_xy.pos.x, mouse_xy.pos.y, xr_->handDepth);
                if(xr_->mapGrab.active && mouse_xy.pos != xr_->grabLastPos)
                {
                    MouseCoords from=mouse_xy; from.pos=xr_->grabLastPos;
                    CallBack->VrDrag(from, mouse_xy);
                }
                xr_->grabLastPos = mouse_xy.pos;
                if(zoom) { KeyEvent key(KeyType::VrZoom); key.c=zoom>0 ? 1 : 2; CallBack->Msg_KeyDown(key); pulse(0.15f); }
            }
            // Griff = Rechtsklick (Abbrechen/Abreißen).
            buttonState(xr_->squeezeAction, xr_->prevSqueeze[hand], [&] {
                if(hand==0) return; // Left grip is reserved for stick zoom.
                mouse_xy.rdown = true;
                CallBack->Msg_RightDown(mouse_xy);
            }, [&] {
                if(mouse_xy.rdown) { mouse_xy.rdown=false;CallBack->Msg_RightUp(mouse_xy); }
            });
            // Physical X/Y/A/B remain independently configurable in either pointer-hand mode.
            const XrAction actions[4] = {xr_->aButtonAction, xr_->speedAction, xr_->aButtonAction, xr_->bButtonAction};
            const bool menuNav = vr.vrMenuFocus;
            const bool menuNavStarted = menuNav && !xr_->prevMenuNav;
            xr_->prevMenuNav = menuNav;
            for(int i=0; i<4; ++i)
            {
                XrActionStateGetInfo info{XR_TYPE_ACTION_STATE_GET_INFO}; info.action=actions[i];
                info.subactionPath=xr_->handPaths[i<2?0:1];
                XrActionStateBoolean state{XR_TYPE_ACTION_STATE_BOOLEAN};
                const bool down=XR_SUCCEEDED(xrGetActionStateBoolean(xr_->session,&info,&state)) && state.isActive && state.currentState;
                // VR menu on top: physical A selects the focus, B closes the window; their configured game actions
                // are suspended and stay suspended until the button is released after the menu closed.
                if(i>=2 && (menuNav || xr_->menuLatch[i]))
                {
                    if(menuNavStarted) xr_->menuLatch[i]=down; // held while the menu opened: wait for release
                    else if(menuNav && down && !xr_->menuLatch[i])
                    {
                        CallBack->Msg_KeyDown(KeyEvent(i==2 ? KeyType::VrMenuSelect : KeyType::VrMenuBack));
                        pulse(0.25f);
                    }
                    xr_->menuLatch[i]=down;
                    xr_->buttonGestures[i]={};
                    continue;
                }
                const auto dispatch = [&](int action) {
                    if(!action || (action==2 && !xr_->pointerOnPanel)) return;
                    if(action==15) { recenter(true); return; }
                    KeyEvent key(KeyType::VrAction); key.c=action; CallBack->Msg_KeyDown(key);
                };
                dispatch(xr_->buttonGestures[i].sample(down,state.isActive,panelNow,vr.buttons[i],vr.longButtons[i]));
            }

            // Right stick pans the game directly. Lists retain their slower repeat,
            // and modal/UI windows are filtered by the game-side callback.
            XrActionStateGetInfo stickGetInfo{XR_TYPE_ACTION_STATE_GET_INFO};
            stickGetInfo.action=xr_->stickAction; stickGetInfo.subactionPath=xr_->handPaths[1];
            XrActionStateVector2f stickState{XR_TYPE_ACTION_STATE_VECTOR2F};
            if(XR_SUCCEEDED(xrGetActionStateVector2f(xr_->session,&stickGetInfo,&stickState)) && stickState.isActive)
            {
                const auto axes=vrInput::mapAxes(stickState.currentState.x,stickState.currentState.y,vr.invertRightX,vr.invertRightY,vr.swapRightStickAxes);
                const float x=axes.x, y=axes.y;
                const bool tapped=xr_->stickTurbo.sample(x,y,panelNow);
                XrActionStateGetInfo clickInfo{XR_TYPE_ACTION_STATE_GET_INFO};
                clickInfo.action=xr_->resetPanelAction; clickInfo.subactionPath=xr_->handPaths[1];
                XrActionStateBoolean click{XR_TYPE_ACTION_STATE_BOOLEAN};
                const bool pressed=XR_SUCCEEDED(xrGetActionStateBoolean(xr_->session,&clickInfo,&click)) && click.isActive && click.currentState;
                const bool turbo=((vr.mapBoost==1 || vr.mapBoost==3) && pressed) || ((vr.mapBoost==2 || vr.mapBoost==3) && tapped);
                const bool repeat=panelNow-xr_->lastStickKeyRepeat>=150;
                const bool list=vr.smartScroll && xr_->pointerOnPanel && std::fabs(y)>.25f
                  && CallBack->VrScroll(mouse_xy,y>0,repeat);
                if(list) { if(repeat) xr_->lastStickKeyRepeat=panelNow; xr_->pan={}; }
                else if(menuNav)
                {
                    // Raw axes: menu directions follow the stick, independent of the map inversion options.
                    xr_->pan={};
                    if(const int dir=xr_->menuStick.sample(stickState.currentState.x,stickState.currentState.y,panelNow))
                    { KeyEvent key(KeyType::VrMenuMove); key.c=static_cast<char32_t>(dir); CallBack->Msg_KeyDown(key); }
                }
                else {
                    int dx=0,dy=0;
                    xr_->pan.sample(x,y,panelDt,vr.mapSpeed,turbo,dx,dy);
                    if(dx || dy) CallBack->VrPan(dx,dy);
                }
            } else { xr_->stickTurbo={}; xr_->pan={}; xr_->menuStick={}; }
            if(!menuNav) xr_->menuStick={};

        } else releaseInput();
    }
#else
    // Stub: keine Ereignisquelle.
#endif
    return true;
}

unsigned long VideoOpenXR::GetTickCount() const
{
    return static_cast<unsigned long>(
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
        .count());
}

OpenGL_Loader_Proc VideoOpenXR::GetLoaderFunction() const
{
#if defined(RTTR_XR_HAS_OPENXR) && RTTR_OGL_GL4ES
    return gl4es_GetProcAddress;
#else
    return nullptr; // Stub
#endif
}

GuiScaleRange VideoOpenXR::getGuiScaleRange() const
{
    auto range = VideoDriver::getGuiScaleRange();
#ifdef RTTR_XR_HAS_OPENXR
    range.recommendedPercent = std::min(200u, range.maxPercent);
#endif
    return range;
}

std::vector<VideoMode> VideoOpenXR::ListVideoModes() const
{
    // These are game-panel resolutions, not the per-eye swapchain dimensions.
    std::vector<VideoMode> modes{VideoMode(1920, 1200), VideoMode(1600, 1000), VideoMode(1280, 800),
                                 VideoMode(1920, 1080), VideoMode(2560, 1080), VideoMode(3440, 1440),
                                 VideoMode(3840, 1080), VideoMode(3840, 2160), VideoMode(3840, 2400),
                                 VideoMode(5120, 2160), VideoMode(5760, 2400)};
#ifdef RTTR_XR_HAS_OPENXR
    if(xr_->maxPanelWidth && xr_->maxPanelHeight)
        modes.erase(std::remove_if(modes.begin(), modes.end(), [&](const auto& mode) {
            return mode.width > xr_->maxPanelWidth || mode.height > xr_->maxPanelHeight;
        }), modes.end());
#endif
    return modes;
}

void VideoOpenXR::SetMousePos(Position pos)
{
    mouse_xy.pos = pos;
}

KeyEvent VideoOpenXR::GetModKeyState() const
{
    // Keine Modifier auf Quest-Controllern; Tastatur ggf. später via
    // Companion-App. Touch-/Controller-Pfade brauchen keine Modifier.
    return KeyEvent{};
}

void* VideoOpenXR::GetMapPointer() const
{
#ifdef RTTR_XR_HAS_OPENXR
    return xr_->eglDisplay;
#else
    return nullptr;
#endif
}

void VideoOpenXR::ShowErrorMessage(const std::string& title, const std::string& message)
{
    // TODO(Stufe 3): __android_log_print; bis dahin stderr.
    std::fprintf(stderr, "[videoOpenXR] %s: %s\n", title.c_str(), message.c_str());
}

#ifdef RTTR_XR_HAS_OPENXR
#    pragma GCC diagnostic pop
#endif

void VideoOpenXR::UpdatePassthrough(bool enabled,int percent)
{
#ifdef RTTR_XR_HAS_OPENXR
    if(!xr_->passthroughSupported) return;
    percent=std::clamp(percent,0,100)/10*10;
    enabled=enabled && percent>0;
    if(enabled!=xr_->passthroughRequested) {
        xr_->passthroughRequested=enabled;
        if(!enabled) {
            if(xr_->passthroughLayer) xr_->pausePassthroughLayer(xr_->passthroughLayer);
            if(xr_->passthrough) xr_->pausePassthrough(xr_->passthrough);
            xr_->passthroughRunning=false;xr_->passthroughStatus=1;
        } else {
            XrResult result=XR_SUCCESS;
            if(!xr_->passthrough) {
                XrPassthroughCreateInfoFB info{};info.type=XR_TYPE_PASSTHROUGH_CREATE_INFO_FB;
                result=xr_->createPassthrough(xr_->session,&info,&xr_->passthrough);
            }
            if(XR_SUCCEEDED(result) && !xr_->passthroughLayer) {
                XrPassthroughLayerCreateInfoFB info{};info.type=XR_TYPE_PASSTHROUGH_LAYER_CREATE_INFO_FB;
                info.passthrough=xr_->passthrough;info.purpose=XR_PASSTHROUGH_LAYER_PURPOSE_RECONSTRUCTION_FB;
                result=xr_->createPassthroughLayer(xr_->session,&info,&xr_->passthroughLayer);
            }
            if(XR_SUCCEEDED(result)) result=xr_->startPassthrough(xr_->passthrough);
            if(XR_SUCCEEDED(result)) result=xr_->resumePassthroughLayer(xr_->passthroughLayer);
            xr_->passthroughRunning=XR_SUCCEEDED(result);
            xr_->passthroughStatus=xr_->passthroughRunning ? 2 : 3;
            xr_->passthroughOpacityApplied=-1;
            if(XR_FAILED(result) && xr_->passthrough) xr_->pausePassthrough(xr_->passthrough);
            std::fprintf(stderr,"[videoOpenXR] Passthrough start: %d\n",result);
        }
    }
    if(xr_->passthroughRunning && percent!=xr_->passthroughOpacityApplied) {
        XrPassthroughStyleFB style{};style.type=XR_TYPE_PASSTHROUGH_STYLE_FB;
        style.textureOpacityFactor=percent/100.f;style.edgeColor={0,0,0,0};
        const auto result=xr_->setPassthroughStyle(xr_->passthroughLayer,&style);
        if(XR_SUCCEEDED(result)) xr_->passthroughOpacityApplied=percent;
        else {
            xr_->pausePassthroughLayer(xr_->passthroughLayer);xr_->pausePassthrough(xr_->passthrough);
            xr_->passthroughRunning=false;xr_->passthroughStatus=3;
        }
    }
#else
    (void)enabled;(void)percent;
#endif
}

bool VideoOpenXR::OpenBuildingHelp(int site)
{
#ifdef RTTR_XR_HAS_OPENXR
    const char* url=vrHelp::website(site);
    return *url && SDL_OpenURL(url)==0;
#else
    (void)site;return false;
#endif
}

#ifdef RTTR_XR_HAS_OPENXR
namespace {
/// Native GLES2 entry points for the recording context (gl* in this module resolve to gl4es).
struct NativeGl
{
    GLuint (*CreateShader)(GLenum) = nullptr;
    void (*ShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*) = nullptr;
    void (*CompileShader)(GLuint) = nullptr;
    GLuint (*CreateProgram)() = nullptr;
    void (*AttachShader)(GLuint, GLuint) = nullptr;
    void (*BindAttribLocation)(GLuint, GLuint, const GLchar*) = nullptr;
    void (*LinkProgram)(GLuint) = nullptr;
    void (*GetProgramiv)(GLuint, GLenum, GLint*) = nullptr;
    void (*UseProgram)(GLuint) = nullptr;
    GLint (*GetUniformLocation)(GLuint, const GLchar*) = nullptr;
    void (*Uniform1i)(GLint, GLint) = nullptr;
    void (*ActiveTexture)(GLenum) = nullptr;
    void (*BindTexture)(GLenum, GLuint) = nullptr;
    void (*TexParameteri)(GLenum, GLenum, GLint) = nullptr;
    void (*BindFramebuffer)(GLenum, GLuint) = nullptr;
    void (*Viewport)(GLint, GLint, GLsizei, GLsizei) = nullptr;
    void (*EnableVertexAttribArray)(GLuint) = nullptr;
    void (*VertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) = nullptr;
    void (*DrawArrays)(GLenum, GLint, GLsizei) = nullptr;
    void (*Finish)() = nullptr;
    EGLBoolean (*PresentationTime)(EGLDisplay, EGLSurface, EGLnsecsANDROID) = nullptr;
    bool loaded = false;

    bool Load()
    {
        if(loaded) return true;
        bool ok = true;
        const auto get = [&](const char* name, auto& fn) {
            fn = reinterpret_cast<std::decay_t<decltype(fn)>>(eglGetProcAddress(name));
            ok = ok && fn != nullptr;
        };
        get("glCreateShader", CreateShader); get("glShaderSource", ShaderSource); get("glCompileShader", CompileShader);
        get("glCreateProgram", CreateProgram); get("glAttachShader", AttachShader);
        get("glBindAttribLocation", BindAttribLocation); get("glLinkProgram", LinkProgram);
        get("glGetProgramiv", GetProgramiv); get("glUseProgram", UseProgram);
        get("glGetUniformLocation", GetUniformLocation); get("glUniform1i", Uniform1i);
        get("glActiveTexture", ActiveTexture); get("glBindTexture", BindTexture); get("glTexParameteri", TexParameteri);
        get("glBindFramebuffer", BindFramebuffer); get("glViewport", Viewport);
        get("glEnableVertexAttribArray", EnableVertexAttribArray); get("glVertexAttribPointer", VertexAttribPointer);
        get("glDrawArrays", DrawArrays); get("glFinish", Finish);
        get("eglPresentationTimeANDROID", PresentationTime);
        loaded = ok;
        return ok;
    }
} nativeGl;

/// Calls a static helper of org.s25rttr.sdl.SDLActivity with the given signature.
template<typename F>
bool WithActivityClass(F&& call)
{
    auto* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    auto activity = static_cast<jobject>(SDL_AndroidGetActivity());
    if(!env || !activity) return false;
    jclass cls = env->GetObjectClass(activity);
    const bool ok = call(env, cls);
    if(env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(activity);
    return ok;
}
} // namespace
#endif

void VideoOpenXR::StartRecording(int panelWidth, int panelHeight)
{
#ifdef RTTR_XR_HAS_OPENXR
    auto& xr = *xr_;
    xr.recordFailed = true; // cleared on success
    if(!nativeGl.Load()) { std::fprintf(stderr, "[videoOpenXR] Recording: native GLES entry points missing\n"); return; }
    // Encoder size: game image scaled to at most 1920 px wide, even dimensions (H.264 requirement).
    const float scale = std::min(1.f, 1920.f / std::max(1, panelWidth));
    xr.recWidth = static_cast<int>(panelWidth * scale) & ~1;
    xr.recHeight = static_cast<int>(panelHeight * scale) & ~1;
    if(!xr.recConfig)
    {
        const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                                  EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_RECORDABLE_ANDROID, 1,
                                  EGL_NONE};
        EGLint count = 0;
        if(!eglChooseConfig(xr.eglDisplay, attribs, &xr.recConfig, 1, &count) || count < 1) { xr.recConfig = nullptr; return; }
    }
    if(xr.recContext == EGL_NO_CONTEXT)
    {
        const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        // Shared with the game context so the panel swapchain textures can be sampled directly.
        xr.recContext = eglCreateContext(xr.eglDisplay, xr.recConfig, xr.eglContext, contextAttribs);
        if(xr.recContext == EGL_NO_CONTEXT) return;
    }
    ANativeWindow* window = nullptr;
    WithActivityClass([&](JNIEnv* env, jclass cls) {
        jmethodID start = env->GetStaticMethodID(cls, "recorderStart", "(III)Landroid/view/Surface;");
        if(!start) return false;
        jobject surface = env->CallStaticObjectMethod(cls, start, xr.recWidth, xr.recHeight, 30);
        if(!surface || env->ExceptionCheck()) return false;
        window = ANativeWindow_fromSurface(env, surface);
        env->DeleteLocalRef(surface);
        return window != nullptr;
    });
    if(!window) { std::fprintf(stderr, "[videoOpenXR] Recording: encoder could not be started\n"); return; }
    xr.recWindow = window;
    xr.recSurface = eglCreateWindowSurface(xr.eglDisplay, xr.recConfig, window, nullptr);
    if(xr.recSurface == EGL_NO_SURFACE) { StopRecording(); xr.recordFailed = true; return; }
    xr.recording = true;
    xr.recordFailed = false;
    xr.recLastFrame = {};
    std::fprintf(stderr, "[videoOpenXR] Recording started %dx%d\n", xr.recWidth, xr.recHeight);
#else
    (void)panelWidth; (void)panelHeight;
#endif
}

void VideoOpenXR::StopRecording()
{
#ifdef RTTR_XR_HAS_OPENXR
    auto& xr = *xr_;
    // Stop rendering into the encoder before it is finalised on the Java side.
    if(xr.recSurface != EGL_NO_SURFACE) { eglDestroySurface(xr.eglDisplay, xr.recSurface); xr.recSurface = EGL_NO_SURFACE; }
    if(xr.recWindow) { ANativeWindow_release(xr.recWindow); xr.recWindow = nullptr; }
    WithActivityClass([](JNIEnv* env, jclass cls) {
        jmethodID stop = env->GetStaticMethodID(cls, "recorderStop", "()V");
        if(stop) env->CallStaticVoidMethod(cls, stop);
        return stop != nullptr;
    });
    xr.recording = false;
#endif
}

void VideoOpenXR::EncodeRecordingFrame(unsigned nativeTexture)
{
#ifdef RTTR_XR_HAS_OPENXR
    auto& xr = *xr_;
    const auto now = std::chrono::steady_clock::now();
    if(now - xr.recLastFrame < std::chrono::milliseconds(33)) return; // ~30 fps
    xr.recLastFrame = now;
    nativeGl.Finish(); // the panel texture was just written by the game context
    const EGLSurface draw = eglGetCurrentSurface(EGL_DRAW), read = eglGetCurrentSurface(EGL_READ);
    const EGLContext context = eglGetCurrentContext();
    if(!eglMakeCurrent(xr.eglDisplay, xr.recSurface, xr.recSurface, xr.recContext))
    {
        StopRecording();
        xr.recordFailed = true;
        return;
    }
    auto& gl = nativeGl;
    if(!xr.recProgram)
    {
        const char* vs = "attribute vec2 p; varying vec2 uv; void main(){ uv = p * 0.5 + 0.5; gl_Position = vec4(p, 0.0, 1.0); }";
        const char* fs = "precision mediump float; varying vec2 uv; uniform sampler2D image; void main(){ gl_FragColor = texture2D(image, uv); }";
        const GLuint v = gl.CreateShader(GL_VERTEX_SHADER), f = gl.CreateShader(GL_FRAGMENT_SHADER);
        gl.ShaderSource(v, 1, &vs, nullptr); gl.CompileShader(v);
        gl.ShaderSource(f, 1, &fs, nullptr); gl.CompileShader(f);
        const GLuint program = gl.CreateProgram();
        gl.AttachShader(program, v); gl.AttachShader(program, f);
        gl.BindAttribLocation(program, 0, "p");
        gl.LinkProgram(program);
        GLint linked = 0;
        gl.GetProgramiv(program, GL_LINK_STATUS, &linked);
        xr.recProgram = linked ? program : 0;
    }
    if(xr.recProgram)
    {
        static const GLfloat quad[] = {-1.f, -1.f, 1.f, -1.f, -1.f, 1.f, 1.f, 1.f};
        gl.BindFramebuffer(GL_FRAMEBUFFER, 0);
        gl.Viewport(0, 0, xr.recWidth, xr.recHeight);
        gl.UseProgram(xr.recProgram);
        gl.Uniform1i(gl.GetUniformLocation(xr.recProgram, "image"), 0);
        gl.ActiveTexture(GL_TEXTURE0);
        gl.BindTexture(GL_TEXTURE_2D, nativeTexture);
        gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl.EnableVertexAttribArray(0);
        gl.VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, quad);
        gl.DrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        gl.PresentationTime(xr.eglDisplay, xr.recSurface,
                            std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count());
        eglSwapBuffers(xr.eglDisplay, xr.recSurface);
    }
    eglMakeCurrent(xr.eglDisplay, draw, read, context);
#else
    (void)nativeTexture;
#endif
}

int VideoOpenXR::VrRecordStatus() const
{
#ifdef RTTR_XR_HAS_OPENXR
    return xr_->recording ? 2 : xr_->recordFailed ? 3 : 1;
#else
    return 0;
#endif
}

bool VideoOpenXR::VrPassthroughAvailable() const
{
#ifdef RTTR_XR_HAS_OPENXR
    return xr_->passthroughSupported;
#else
    return false;
#endif
}

int VideoOpenXR::VrTableStatus() const
{
#ifdef RTTR_XR_HAS_OPENXR
    return xr_->tableStatus;
#else
    return 0;
#endif
}

bool VideoOpenXR::DrawVrHeadquarters(Position point,unsigned color)
{
    (void)color;
#ifdef RTTR_XR_HAS_OPENXR
    const auto vr=CallBack->GetVrSettings();
    if(!vr.headquarters3d || vr.presentation!=1 || !vr.frameMoveAvailable || vr.hideLoadingPointer || !xr_->sessionRunning) return false;
    GLfloat model[16],projection[16];
    glGetFloatv(GL_MODELVIEW_MATRIX,model);glGetFloatv(GL_PROJECTION_MATRIX,projection);
    const auto p=vrHq::project(point.x,point.y,model,projection);
    const auto right=vrHq::project(point.x+65,point.y,model,projection);
    const float width=std::fabs(right[0]-p[0]);
    const float margin=width*2.1f*GetRenderSize().x/GetRenderSize().y;
    if(width<=0 || p[0]-width<0 || p[0]+width>1 || p[1]-margin<0 || p[1]+margin>1) return false;
    glPushAttrib(GL_TEXTURE_BIT);glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
            if(!xr_->hqAssetAttempted) {
                xr_->hqAssetAttempted=true;vrHq::Asset asset;
                if(asset.load("/sdcard/RTTR/vr-assets/headquarters.xrhq")) {
                    glGenBuffers(1,&xr_->hqVertices);glBindBuffer(GL_ARRAY_BUFFER,xr_->hqVertices);
                    glBufferData(GL_ARRAY_BUFFER,asset.vertices.size()*sizeof(float),asset.vertices.data(),GL_STATIC_DRAW);
                    glGenBuffers(1,&xr_->hqIndices);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,xr_->hqIndices);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER,asset.indices.size()*sizeof(uint32_t),asset.indices.data(),GL_STATIC_DRAW);
                    glGenTextures(1,&xr_->hqTexture);glBindTexture(GL_TEXTURE_2D,xr_->hqTexture);
                    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,asset.width,asset.height,0,GL_RGBA,GL_UNSIGNED_BYTE,asset.pixels.data());
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
                    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
                    xr_->hqIndexCount=static_cast<GLsizei>(asset.indices.size());
                    glBindBuffer(GL_ARRAY_BUFFER,0);glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
                    std::fprintf(stderr,"[videoOpenXR] External HQ loaded: %d triangles\n",xr_->hqIndexCount/3);
                } else std::fprintf(stderr,"[videoOpenXR] External HQ absent/invalid: original sprite retained\n");
            }

    glPopClientAttrib();glPopAttrib();
    if(!xr_->hqIndexCount) return false;
    xr_->headquarters.push_back({p[0],p[1],width});
    return true;
#else
    (void)point;return false;
#endif
}
