/*
 *  OpenGL.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 31.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#if defined(__APPLE_CC__) && (defined(IPHONE) || defined(IPHONE_SIMULATOR))
// iOS
#define GLES 1
#define NEEDS_GL_FUNCTION_POINTERS 0
#include <OpenGLES/ES1/gl.h>

#elif defined(__APPLE_CC__)
// Mac OS X
#define GLES 0
#define NEEDS_GL_FUNCTION_POINTERS 0
#include <OpenGL/gl.h>

#elif defined(ANDROID_NDK)
// Android
#define GLES 1
#define NEEDS_GL_FUNCTION_POINTERS 0
#include <GLES/gl.h>
#include <android/log.h>

#else
// Generic (i.e. Windows)
#define GLES 0
#define NEEDS_GL_FUNCTION_POINTERS 1
#include <SDL_opengl.h>

#endif /* Platform */



#if NEEDS_GL_FUNCTION_POINTERS

#ifdef PROC_POINTERS_NOT_EXTERN
#define GLPROCPOINTER
#else
#define GLPROCPOINTER extern
#endif

// Buffers
GLPROCPOINTER PFNGLGENBUFFERSPROC glGenBuffers;
GLPROCPOINTER PFNGLDELETEBUFFERSPROC glDeleteBuffers;
GLPROCPOINTER PFNGLBINDBUFFERPROC glBindBuffer;
GLPROCPOINTER PFNGLBUFFERDATAPROC glBufferData;
GLPROCPOINTER PFNGLBUFFERSUBDATAPROC glBufferSubData;

#endif /* NEEDS_GL_FUNCTION_POINTERS */

#ifdef ANDROID_NDK
#define checkGLError() do { GLenum error = glGetError(); if (error != GL_NO_ERROR) __android_log_print(ANDROID_LOG_WARN, "librobosim.so", "OpenGL Error %x in %s:%d", error, __FILE__, __LINE__); } while(false)
#else
#define checkGLError() do { GLenum error = glGetError(); if (error != GL_NO_ERROR) printf("%s:%u: glError %x\n", __FILE__, __LINE__, error); } while(false)
#endif