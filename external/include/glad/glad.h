#ifndef GLAD_H
#define GLAD_H

// Prevenir que Windows incluya gl.h
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI

#include <windows.h>
#include <stddef.h>

#define APIENTRY __stdcall

#ifdef __cplusplus
extern "C" {
#endif

// Evitar conflictos con gl.h
#define __gl_h_

typedef void* (*GLADloadproc)(const char* name);

// OpenGL function pointer typedefs
typedef void (APIENTRY* PFNGLCULLFACEPROC)(unsigned int mode);
typedef void (APIENTRY* PFNGLFRONTFACEPROC)(unsigned int mode);
typedef void (APIENTRY* PFNGLHINTPROC)(unsigned int target, unsigned int mode);
typedef void (APIENTRY* PFNGLLINEWIDTHPROC)(float width);
typedef void (APIENTRY* PFNGLPOINTSIZEPROC)(float size);
typedef void (APIENTRY* PFNGLPOLYGONMODEPROC)(unsigned int face, unsigned int mode);
typedef void (APIENTRY* PFNGLSCISSORPROC)(int x, int y, int width, int height);
typedef void (APIENTRY* PFNGLTEXPARAMETERFPROC)(unsigned int target, unsigned int pname, float param);
typedef void (APIENTRY* PFNGLTEXPARAMETERFVPROC)(unsigned int target, unsigned int pname, const float* params);
typedef void (APIENTRY* PFNGLTEXPARAMETERIPROC)(unsigned int target, unsigned int pname, int param);
typedef void (APIENTRY* PFNGLTEXPARAMETERIVPROC)(unsigned int target, unsigned int pname, const int* params);
typedef void (APIENTRY* PFNGLTEXIMAGE1DPROC)(unsigned int target, int level, int internalformat, int width, int border, unsigned int format, unsigned int type, const void* pixels);
typedef void (APIENTRY* PFNGLTEXIMAGE2DPROC)(unsigned int target, int level, int internalformat, int width, int height, int border, unsigned int format, unsigned int type, const void* pixels);
typedef void (APIENTRY* PFNGLDRAWBUFFERPROC)(unsigned int buf);
typedef void (APIENTRY* PFNGLCLEARPROC)(unsigned int mask);
typedef void (APIENTRY* PFNGLCLEARCOLORPROC)(float red, float green, float blue, float alpha);
typedef void (APIENTRY* PFNGLCLEARSTENCILPROC)(int s);
typedef void (APIENTRY* PFNGLCLEARDEPTHPROC)(double depth);
typedef void (APIENTRY* PFNGLSTENCILMASKPROC)(unsigned int mask);
typedef void (APIENTRY* PFNGLCOLORMASKPROC)(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha);
typedef void (APIENTRY* PFNGLDEPTHMASKPROC)(unsigned char flag);
typedef void (APIENTRY* PFNGLDISABLEPROC)(unsigned int cap);
typedef void (APIENTRY* PFNGLENABLEPROC)(unsigned int cap);
typedef void (APIENTRY* PFNGLFINISHPROC)(void);
typedef void (APIENTRY* PFNGLFLUSHPROC)(void);
typedef void (APIENTRY* PFNGLBLENDFUNCPROC)(unsigned int sfactor, unsigned int dfactor);
typedef void (APIENTRY* PFNGLLOGICOPPROC)(unsigned int opcode);
typedef void (APIENTRY* PFNGLSTENCILFUNCPROC)(unsigned int func, int ref, unsigned int mask);
typedef void (APIENTRY* PFNGLSTENCILOPPROC)(unsigned int fail, unsigned int zfail, unsigned int zpass);
typedef void (APIENTRY* PFNGLDEPTHFUNCPROC)(unsigned int func);
typedef void (APIENTRY* PFNGLPIXELSTOREFPROC)(unsigned int pname, float param);
typedef void (APIENTRY* PFNGLPIXELSTOREIPROC)(unsigned int pname, int param);
typedef void (APIENTRY* PFNGLREADBUFFERPROC)(unsigned int src);
typedef void (APIENTRY* PFNGLREADPIXELSPROC)(int x, int y, int width, int height, unsigned int format, unsigned int type, void* pixels);
typedef void (APIENTRY* PFNGLGETBOOLEANVPROC)(unsigned int pname, unsigned char* data);
typedef void (APIENTRY* PFNGLGETDOUBLEVPROC)(unsigned int pname, double* data);
typedef unsigned int (APIENTRY* PFNGLGETERRORPROC)(void);
typedef void (APIENTRY* PFNGLGETFLOATVPROC)(unsigned int pname, float* data);
typedef void (APIENTRY* PFNGLGETINTEGERVPROC)(unsigned int pname, int* data);
typedef const unsigned char* (APIENTRY* PFNGLGETSTRINGPROC)(unsigned int name);
typedef void (APIENTRY* PFNGLGETTEXIMAGEPROC)(unsigned int target, int level, unsigned int format, unsigned int type, void* pixels);
typedef void (APIENTRY* PFNGLGETTEXPARAMETERFVPROC)(unsigned int target, unsigned int pname, float* params);
typedef void (APIENTRY* PFNGLGETTEXPARAMETERIVPROC)(unsigned int target, unsigned int pname, int* params);
typedef void (APIENTRY* PFNGLGETTEXLEVELPARAMETERFVPROC)(unsigned int target, int level, unsigned int pname, float* params);
typedef void (APIENTRY* PFNGLGETTEXLEVELPARAMETERIVPROC)(unsigned int target, int level, unsigned int pname, int* params);
typedef unsigned char (APIENTRY* PFNGLISENABLEDPROC)(unsigned int cap);
typedef void (APIENTRY* PFNGLDEPTHRANGEPROC)(double n, double f);
typedef void (APIENTRY* PFNGLVIEWPORTPROC)(int x, int y, int width, int height);
typedef void (APIENTRY* PFNGLDRAWARRAYSPROC)(unsigned int mode, int first, int count);
typedef void (APIENTRY* PFNGLDRAWELEMENTSPROC)(unsigned int mode, int count, unsigned int type, const void* indices);
typedef void (APIENTRY* PFNGLGENTEXTURESPROC)(int n, unsigned int* textures);
typedef void (APIENTRY* PFNGLDELETETEXTURESPROC)(int n, const unsigned int* textures);
typedef void (APIENTRY* PFNGLBINDTEXTUREPROC)(unsigned int target, unsigned int texture);
typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC)(int n, unsigned int* arrays);
typedef void (APIENTRY* PFNGLBINDVERTEXARRAYPROC)(unsigned int array);
typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(int n, unsigned int* buffers);
typedef void (APIENTRY* PFNGLBINDBUFFERPROC)(unsigned int target, unsigned int buffer);
typedef void (APIENTRY* PFNGLBUFFERDATAPROC)(unsigned int target, ptrdiff_t size, const void* data, unsigned int usage);
typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC)(unsigned int index, int size, unsigned int type, unsigned char normalized, int stride, const void* pointer);
typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC)(unsigned int index);
typedef unsigned int (APIENTRY* PFNGLCREATESHADERPROC)(unsigned int type);
typedef void (APIENTRY* PFNGLSHADERSOURCEPROC)(unsigned int shader, int count, const char* const* string, const int* length);
typedef void (APIENTRY* PFNGLCOMPILESHADERPROC)(unsigned int shader);
typedef void (APIENTRY* PFNGLGETSHADERIVPROC)(unsigned int shader, unsigned int pname, int* params);
typedef void (APIENTRY* PFNGLGETSHADERINFOLOGPROC)(unsigned int shader, int bufSize, int* length, char* infoLog);
typedef unsigned int (APIENTRY* PFNGLCREATEPROGRAMPROC)(void);
typedef void (APIENTRY* PFNGLATTACHSHADERPROC)(unsigned int program, unsigned int shader);
typedef void (APIENTRY* PFNGLLINKPROGRAMPROC)(unsigned int program);
typedef void (APIENTRY* PFNGLGETPROGRAMIVPROC)(unsigned int program, unsigned int pname, int* params);
typedef void (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC)(unsigned int program, int bufSize, int* length, char* infoLog);
typedef void (APIENTRY* PFNGLUSEPROGRAMPROC)(unsigned int program);
typedef void (APIENTRY* PFNGLDELETESHADERPROC)(unsigned int shader);
typedef void (APIENTRY* PFNGLDELETEPROGRAMPROC)(unsigned int program);
typedef void (APIENTRY* PFNGLUNIFORM1FPROC)(int location, float v0);
typedef void (APIENTRY* PFNGLUNIFORM2FPROC)(int location, float v0, float v1);
typedef void (APIENTRY* PFNGLUNIFORM3FPROC)(int location, float v0, float v1, float v2);
typedef void (APIENTRY* PFNGLUNIFORM4FPROC)(int location, float v0, float v1, float v2, float v3);
typedef void (APIENTRY* PFNGLUNIFORM1IPROC)(int location, int v0);
typedef void (APIENTRY* PFNGLUNIFORM2IPROC)(int location, int v0, int v1);
typedef void (APIENTRY* PFNGLUNIFORM3IPROC)(int location, int v0, int v1, int v2);
typedef void (APIENTRY* PFNGLUNIFORM4IPROC)(int location, int v0, int v1, int v2, int v3);
typedef void (APIENTRY* PFNGLUNIFORM1FVPROC)(int location, int count, const float* value);
typedef void (APIENTRY* PFNGLUNIFORM2FVPROC)(int location, int count, const float* value);
typedef void (APIENTRY* PFNGLUNIFORM3FVPROC)(int location, int count, const float* value);
typedef void (APIENTRY* PFNGLUNIFORM4FVPROC)(int location, int count, const float* value);
typedef void (APIENTRY* PFNGLUNIFORMMATRIX2FVPROC)(int location, int count, unsigned char transpose, const float* value);
typedef void (APIENTRY* PFNGLUNIFORMMATRIX3FVPROC)(int location, int count, unsigned char transpose, const float* value);
typedef void (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC)(int location, int count, unsigned char transpose, const float* value);
typedef int (APIENTRY* PFNGLGETUNIFORMLOCATIONPROC)(unsigned int program, const char* name);
typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTUREPROC)(unsigned int target, unsigned int attachment, unsigned int texture, int level);
typedef void (APIENTRY* PFNGLGENFRAMEBUFFERSPROC)(int n, unsigned int* framebuffers);
typedef void (APIENTRY* PFNGLBINDFRAMEBUFFERPROC)(unsigned int target, unsigned int framebuffer);
typedef void (APIENTRY* PFNGLGENRENDERBUFFERSPROC)(int n, unsigned int* renderbuffers);
typedef void (APIENTRY* PFNGLBINDRENDERBUFFERPROC)(unsigned int target, unsigned int renderbuffer);
typedef void (APIENTRY* PFNGLRENDERBUFFERSTORAGEPROC)(unsigned int target, unsigned int internalformat, int width, int height);
typedef void (APIENTRY* PFNGLFRAMEBUFFERRENDERBUFFERPROC)(unsigned int target, unsigned int attachment, unsigned int renderbuffertarget, unsigned int renderbuffer);
typedef void (APIENTRY* PFNGLFRAMEBUFFERTEXTURE2DPROC)(unsigned int target, unsigned int attachment, unsigned int textarget, unsigned int texture, int level);
typedef unsigned int (APIENTRY* PFNGLCHECKFRAMEBUFFERSTATUSPROC)(unsigned int target);
typedef void (APIENTRY* PFNGLDELETEFRAMEBUFFERSPROC)(int n, const unsigned int* framebuffers);
typedef void (APIENTRY* PFNGLDELETERENDERBUFFERSPROC)(int n, const unsigned int* renderbuffers);
typedef void (APIENTRY* PFNGLDELETEVERTEXARRAYSPROC)(int n, const unsigned int* arrays);
typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC)(int n, const unsigned int* buffers);
typedef void (APIENTRY* PFNGLBLITFRAMEBUFFERPROC)(int srcX0, int srcY0, int srcX1, int srcY1, int dstX0, int dstY0, int dstX1, int dstY1, unsigned int mask, unsigned int filter);

// OpenGL constants
#define GL_FALSE 0
#define GL_TRUE 1
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_POINTS 0x0000
#define GL_LINES 0x0001
#define GL_TRIANGLES 0x0004
#define GL_DEPTH_TEST 0x0B71
#define GL_BLEND 0x0BE2
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_RGBA 0x1908
#define GL_RGB 0x1907
#define GL_UNSIGNED_BYTE 0x1401
#define GL_FLOAT 0x1406
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STATIC_DRAW 0x88E4
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_VERTEX_SHADER 0x8B31
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_INFO_LOG_LENGTH 0x8B84
#define GL_FRAMEBUFFER 0x8D40
#define GL_RENDERBUFFER 0x8D41
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
#define GL_READ_FRAMEBUFFER 0x8CA8
#define GL_DRAW_FRAMEBUFFER 0x8CA9

// Evitar conflictos con gl.h
#ifndef GLAD_NO_PROTOTYPES
#define GLAD_NO_PROTOTYPES
#endif

#ifndef GLAPI
#define GLAPI extern
#endif

// Function declarations
int gladLoadGLLoader(GLADloadproc load);

// OpenGL function pointers
GLAPI PFNGLCULLFACEPROC glad_glCullFace;
GLAPI PFNGLFRONTFACEPROC glad_glFrontFace;
GLAPI PFNGLHINTPROC glad_glHint;
GLAPI PFNGLLINEWIDTHPROC glad_glLineWidth;
GLAPI PFNGLPOINTSIZEPROC glad_glPointSize;
GLAPI PFNGLPOLYGONMODEPROC glad_glPolygonMode;
GLAPI PFNGLSCISSORPROC glad_glScissor;
GLAPI PFNGLTEXPARAMETERFPROC glad_glTexParameterf;
GLAPI PFNGLTEXPARAMETERFVPROC glad_glTexParameterfv;
GLAPI PFNGLTEXPARAMETERIPROC glad_glTexParameteri;
GLAPI PFNGLTEXPARAMETERIVPROC glad_glTexParameteriv;
GLAPI PFNGLTEXIMAGE1DPROC glad_glTexImage1D;
GLAPI PFNGLTEXIMAGE2DPROC glad_glTexImage2D;
GLAPI PFNGLDRAWBUFFERPROC glad_glDrawBuffer;
GLAPI PFNGLCLEARPROC glad_glClear;
GLAPI PFNGLCLEARCOLORPROC glad_glClearColor;
GLAPI PFNGLCLEARSTENCILPROC glad_glClearStencil;
GLAPI PFNGLCLEARDEPTHPROC glad_glClearDepth;
GLAPI PFNGLSTENCILMASKPROC glad_glStencilMask;
GLAPI PFNGLCOLORMASKPROC glad_glColorMask;
GLAPI PFNGLDEPTHMASKPROC glad_glDepthMask;
GLAPI PFNGLDISABLEPROC glad_glDisable;
GLAPI PFNGLENABLEPROC glad_glEnable;
GLAPI PFNGLFINISHPROC glad_glFinish;
GLAPI PFNGLFLUSHPROC glad_glFlush;
GLAPI PFNGLBLENDFUNCPROC glad_glBlendFunc;
GLAPI PFNGLLOGICOPPROC glad_glLogicOp;
GLAPI PFNGLSTENCILFUNCPROC glad_glStencilFunc;
GLAPI PFNGLSTENCILOPPROC glad_glStencilOp;
GLAPI PFNGLDEPTHFUNCPROC glad_glDepthFunc;
GLAPI PFNGLPIXELSTOREFPROC glad_glPixelStoref;
GLAPI PFNGLPIXELSTOREIPROC glad_glPixelStorei;
GLAPI PFNGLREADBUFFERPROC glad_glReadBuffer;
GLAPI PFNGLREADPIXELSPROC glad_glReadPixels;
GLAPI PFNGLGETBOOLEANVPROC glad_glGetBooleanv;
GLAPI PFNGLGETDOUBLEVPROC glad_glGetDoublev;
GLAPI PFNGLGETERRORPROC glad_glGetError;
GLAPI PFNGLGETFLOATVPROC glad_glGetFloatv;
GLAPI PFNGLGETINTEGERVPROC glad_glGetIntegerv;
GLAPI PFNGLGETSTRINGPROC glad_glGetString;
GLAPI PFNGLGETTEXIMAGEPROC glad_glGetTexImage;
GLAPI PFNGLGETTEXPARAMETERFVPROC glad_glGetTexParameterfv;
GLAPI PFNGLGETTEXPARAMETERIVPROC glad_glGetTexParameteriv;
GLAPI PFNGLGETTEXLEVELPARAMETERFVPROC glad_glGetTexLevelParameterfv;
GLAPI PFNGLGETTEXLEVELPARAMETERIVPROC glad_glGetTexLevelParameteriv;
GLAPI PFNGLISENABLEDPROC glad_glIsEnabled;
GLAPI PFNGLDEPTHRANGEPROC glad_glDepthRange;
GLAPI PFNGLVIEWPORTPROC glad_glViewport;
GLAPI PFNGLDRAWARRAYSPROC glad_glDrawArrays;
GLAPI PFNGLDRAWELEMENTSPROC glad_glDrawElements;
GLAPI PFNGLGENTEXTURESPROC glad_glGenTextures;
GLAPI PFNGLDELETETEXTURESPROC glad_glDeleteTextures;
GLAPI PFNGLBINDTEXTUREPROC glad_glBindTexture;
GLAPI PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays;
GLAPI PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray;
GLAPI PFNGLGENBUFFERSPROC glad_glGenBuffers;
GLAPI PFNGLBINDBUFFERPROC glad_glBindBuffer;
GLAPI PFNGLBUFFERDATAPROC glad_glBufferData;
GLAPI PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer;
GLAPI PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray;
GLAPI PFNGLCREATESHADERPROC glad_glCreateShader;
GLAPI PFNGLSHADERSOURCEPROC glad_glShaderSource;
GLAPI PFNGLCOMPILESHADERPROC glad_glCompileShader;
GLAPI PFNGLGETSHADERIVPROC glad_glGetShaderiv;
GLAPI PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog;
GLAPI PFNGLCREATEPROGRAMPROC glad_glCreateProgram;
GLAPI PFNGLATTACHSHADERPROC glad_glAttachShader;
GLAPI PFNGLLINKPROGRAMPROC glad_glLinkProgram;
GLAPI PFNGLGETPROGRAMIVPROC glad_glGetProgramiv;
GLAPI PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog;
GLAPI PFNGLUSEPROGRAMPROC glad_glUseProgram;
GLAPI PFNGLDELETESHADERPROC glad_glDeleteShader;
GLAPI PFNGLDELETEPROGRAMPROC glad_glDeleteProgram;
GLAPI PFNGLUNIFORM1FPROC glad_glUniform1f;
GLAPI PFNGLUNIFORM2FPROC glad_glUniform2f;
GLAPI PFNGLUNIFORM3FPROC glad_glUniform3f;
GLAPI PFNGLUNIFORM4FPROC glad_glUniform4f;
GLAPI PFNGLUNIFORM1IPROC glad_glUniform1i;
GLAPI PFNGLUNIFORM2IPROC glad_glUniform2i;
GLAPI PFNGLUNIFORM3IPROC glad_glUniform3i;
GLAPI PFNGLUNIFORM4IPROC glad_glUniform4i;
GLAPI PFNGLUNIFORM1FVPROC glad_glUniform1fv;
GLAPI PFNGLUNIFORM2FVPROC glad_glUniform2fv;
GLAPI PFNGLUNIFORM3FVPROC glad_glUniform3fv;
GLAPI PFNGLUNIFORM4FVPROC glad_glUniform4fv;
GLAPI PFNGLUNIFORMMATRIX2FVPROC glad_glUniformMatrix2fv;
GLAPI PFNGLUNIFORMMATRIX3FVPROC glad_glUniformMatrix3fv;
GLAPI PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv;
GLAPI PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation;
GLAPI PFNGLFRAMEBUFFERTEXTUREPROC glad_glFramebufferTexture;
GLAPI PFNGLGENFRAMEBUFFERSPROC glad_glGenFramebuffers;
GLAPI PFNGLBINDFRAMEBUFFERPROC glad_glBindFramebuffer;
GLAPI PFNGLGENRENDERBUFFERSPROC glad_glGenRenderbuffers;
GLAPI PFNGLBINDRENDERBUFFERPROC glad_glBindRenderbuffer;
GLAPI PFNGLRENDERBUFFERSTORAGEPROC glad_glRenderbufferStorage;
GLAPI PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer;
GLAPI PFNGLFRAMEBUFFERTEXTURE2DPROC glad_glFramebufferTexture2D;
GLAPI PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus;
GLAPI PFNGLDELETEFRAMEBUFFERSPROC glad_glDeleteFramebuffers;
GLAPI PFNGLDELETERENDERBUFFERSPROC glad_glDeleteRenderbuffers;
GLAPI PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays;
GLAPI PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers;
GLAPI PFNGLBLITFRAMEBUFFERPROC glad_glBlitFramebuffer;

// Convenience macros to wrap function calls
#define glCullFace glad_glCullFace
#define glFrontFace glad_glFrontFace
#define glHint glad_glHint
#define glLineWidth glad_glLineWidth
#define glPointSize glad_glPointSize
#define glPolygonMode glad_glPolygonMode
#define glScissor glad_glScissor
#define glTexParameterf glad_glTexParameterf
#define glTexParameterfv glad_glTexParameterfv
#define glTexParameteri glad_glTexParameteri
#define glTexParameteriv glad_glTexParameteriv
#define glTexImage1D glad_glTexImage1D
#define glTexImage2D glad_glTexImage2D
#define glDrawBuffer glad_glDrawBuffer
#define glClear glad_glClear
#define glClearColor glad_glClearColor
#define glClearStencil glad_glClearStencil
#define glClearDepth glad_glClearDepth
#define glStencilMask glad_glStencilMask
#define glColorMask glad_glColorMask
#define glDepthMask glad_glDepthMask
#define glDisable glad_glDisable
#define glEnable glad_glEnable
#define glFinish glad_glFinish
#define glFlush glad_glFlush
#define glBlendFunc glad_glBlendFunc
#define glLogicOp glad_glLogicOp
#define glStencilFunc glad_glStencilFunc
#define glStencilOp glad_glStencilOp
#define glDepthFunc glad_glDepthFunc
#define glPixelStoref glad_glPixelStoref
#define glPixelStorei glad_glPixelStorei
#define glReadBuffer glad_glReadBuffer
#define glReadPixels glad_glReadPixels
#define glGetBooleanv glad_glGetBooleanv
#define glGetDoublev glad_glGetDoublev
#define glGetError glad_glGetError
#define glGetFloatv glad_glGetFloatv
#define glGetIntegerv glad_glGetIntegerv
#define glGetString glad_glGetString
#define glGetTexImage glad_glGetTexImage
#define glGetTexParameterfv glad_glGetTexParameterfv
#define glGetTexParameteriv glad_glGetTexParameteriv
#define glGetTexLevelParameterfv glad_glGetTexLevelParameterfv
#define glGetTexLevelParameteriv glad_glGetTexLevelParameteriv
#define glIsEnabled glad_glIsEnabled
#define glDepthRange glad_glDepthRange
#define glViewport glad_glViewport
#define glDrawArrays glad_glDrawArrays
#define glDrawElements glad_glDrawElements
#define glGenTextures glad_glGenTextures
#define glDeleteTextures glad_glDeleteTextures
#define glBindTexture glad_glBindTexture
#define glGenVertexArrays glad_glGenVertexArrays
#define glBindVertexArray glad_glBindVertexArray
#define glGenBuffers glad_glGenBuffers
#define glBindBuffer glad_glBindBuffer
#define glBufferData glad_glBufferData
#define glVertexAttribPointer glad_glVertexAttribPointer
#define glEnableVertexAttribArray glad_glEnableVertexAttribArray
#define glCreateShader glad_glCreateShader
#define glShaderSource glad_glShaderSource
#define glCompileShader glad_glCompileShader
#define glGetShaderiv glad_glGetShaderiv
#define glGetShaderInfoLog glad_glGetShaderInfoLog
#define glCreateProgram glad_glCreateProgram
#define glAttachShader glad_glAttachShader
#define glLinkProgram glad_glLinkProgram
#define glGetProgramiv glad_glGetProgramiv
#define glGetProgramInfoLog glad_glGetProgramInfoLog
#define glUseProgram glad_glUseProgram
#define glDeleteShader glad_glDeleteShader
#define glDeleteProgram glad_glDeleteProgram
#define glUniform1f glad_glUniform1f
#define glUniform2f glad_glUniform2f
#define glUniform3f glad_glUniform3f
#define glUniform4f glad_glUniform4f
#define glUniform1i glad_glUniform1i
#define glUniform2i glad_glUniform2i
#define glUniform3i glad_glUniform3i
#define glUniform4i glad_glUniform4i
#define glUniform1fv glad_glUniform1fv
#define glUniform2fv glad_glUniform2fv
#define glUniform3fv glad_glUniform3fv
#define glUniform4fv glad_glUniform4fv
#define glUniformMatrix2fv glad_glUniformMatrix2fv
#define glUniformMatrix3fv glad_glUniformMatrix3fv
#define glUniformMatrix4fv glad_glUniformMatrix4fv
#define glGetUniformLocation glad_glGetUniformLocation
#define glFramebufferTexture glad_glFramebufferTexture
#define glGenFramebuffers glad_glGenFramebuffers
#define glBindFramebuffer glad_glBindFramebuffer
#define glGenRenderbuffers glad_glGenRenderbuffers
#define glBindRenderbuffer glad_glBindRenderbuffer
#define glRenderbufferStorage glad_glRenderbufferStorage
#define glFramebufferRenderbuffer glad_glFramebufferRenderbuffer
#define glFramebufferTexture2D glad_glFramebufferTexture2D
#define glCheckFramebufferStatus glad_glCheckFramebufferStatus
#define glDeleteFramebuffers glad_glDeleteFramebuffers
#define glDeleteRenderbuffers glad_glDeleteRenderbuffers
#define glDeleteVertexArrays glad_glDeleteVertexArrays
#define glDeleteBuffers glad_glDeleteBuffers
#define glBlitFramebuffer glad_glBlitFramebuffer

#ifdef __cplusplus
}
#endif

#endif // GLAD_H 