#ifndef __IRR_BUILD_CONFIG_OPTIONS_H_INCLUDED__
#define __IRR_BUILD_CONFIG_OPTIONS_H_INCLUDED__

/* #undef _IRR_DEBUG */

// architecture
/* #undef __IRR_COMPILE_WITH_ARM_SIMD_ */

// OS
#define _IRR_PLATFORM_WINDOWS_
/* #undef _IRR_PLATFORM_LINUX_ */
/* #undef _IRR_PLATFORM_OSX_ */

// graphics API backend
#define _IRR_COMPILE_WITH_OPENGL_
/* #undef _IRR_COMPILE_WITH_VULKAN_ */

// extra config
#define __IRR_FAST_MATH

#endif //__IRR_BUILD_CONFIG_OPTIONS_H_INCLUDED__
