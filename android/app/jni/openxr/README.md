# OpenXR headers and loader

The headers in `include/openxr` originate from Khronos OpenXR-SDK commit
`f2448a8797c8` (2026-09-02). Preserve their copyright and SPDX license notices.
The Apache-2.0 license text is in [Apache-2.0](../../../LICENSES/Apache-2.0.txt).

The APK needs its own ARM64 OpenXR loader. `scripts/build-quest.py` builds it
from the pinned `third_party/openxr-sdk-source` submodule and copies the output
to `android/app/src/main/jniLibs/arm64-v8a/libopenxr_loader.so` before Gradle runs.
That generated binary is intentionally not versioned. The driver links the loader
and initializes it with the Android application context.
