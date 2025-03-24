#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>

static void* get_proc_address(const char* name);

#ifdef _WIN32
#include <windows.h>
static HMODULE libgl;

static void* get_proc_address(const char* name) {
    void* result = (void*)wglGetProcAddress(name);
    if (result) return result;
    if (libgl == NULL) {
        libgl = LoadLibraryA("opengl32.dll");
    }
    return (void*)GetProcAddress(libgl, name);
}
#else
#include <dlfcn.h>
static void* libgl;

static void* get_proc_address(const char* name) {
    void* result = NULL;
    if (libgl == NULL) {
        libgl = dlopen("libGL.so.1", RTLD_LAZY);
    }
    if (libgl) {
        result = dlsym(libgl, name);
    }
    return result;
}
#endif

int gladLoadGLLoader(GLADloadproc load) {
    static int initialized = 0;
    if (initialized) return 1;
    
    if (load == NULL) {
        load = (GLADloadproc)get_proc_address;
    }
    
    // Load OpenGL functions
    void* (*fp)(const char*) = load;
    
    // Core OpenGL 3.3 functions
    // Version: 1.0
    glad_glCullFace = (PFNGLCULLFACEPROC)fp("glCullFace");
    glad_glFrontFace = (PFNGLFRONTFACEPROC)fp("glFrontFace");
    glad_glHint = (PFNGLHINTPROC)fp("glHint");
    glad_glLineWidth = (PFNGLLINEWIDTHPROC)fp("glLineWidth");
    glad_glPointSize = (PFNGLPOINTSIZEPROC)fp("glPointSize");
    glad_glPolygonMode = (PFNGLPOLYGONMODEPROC)fp("glPolygonMode");
    glad_glScissor = (PFNGLSCISSORPROC)fp("glScissor");
    glad_glTexParameterf = (PFNGLTEXPARAMETERFPROC)fp("glTexParameterf");
    glad_glTexParameterfv = (PFNGLTEXPARAMETERFVPROC)fp("glTexParameterfv");
    glad_glTexParameteri = (PFNGLTEXPARAMETERIPROC)fp("glTexParameteri");
    glad_glTexParameteriv = (PFNGLTEXPARAMETERIVPROC)fp("glTexParameteriv");
    glad_glTexImage1D = (PFNGLTEXIMAGE1DPROC)fp("glTexImage1D");
    glad_glTexImage2D = (PFNGLTEXIMAGE2DPROC)fp("glTexImage2D");
    glad_glDrawBuffer = (PFNGLDRAWBUFFERPROC)fp("glDrawBuffer");
    glad_glClear = (PFNGLCLEARPROC)fp("glClear");
    glad_glClearColor = (PFNGLCLEARCOLORPROC)fp("glClearColor");
    glad_glClearStencil = (PFNGLCLEARSTENCILPROC)fp("glClearStencil");
    glad_glClearDepth = (PFNGLCLEARDEPTHPROC)fp("glClearDepth");
    glad_glStencilMask = (PFNGLSTENCILMASKPROC)fp("glStencilMask");
    glad_glColorMask = (PFNGLCOLORMASKPROC)fp("glColorMask");
    glad_glDepthMask = (PFNGLDEPTHMASKPROC)fp("glDepthMask");
    glad_glDisable = (PFNGLDISABLEPROC)fp("glDisable");
    glad_glEnable = (PFNGLENABLEPROC)fp("glEnable");
    glad_glFinish = (PFNGLFINISHPROC)fp("glFinish");
    glad_glFlush = (PFNGLFLUSHPROC)fp("glFlush");
    glad_glBlendFunc = (PFNGLBLENDFUNCPROC)fp("glBlendFunc");
    glad_glLogicOp = (PFNGLLOGICOPPROC)fp("glLogicOp");
    glad_glStencilFunc = (PFNGLSTENCILFUNCPROC)fp("glStencilFunc");
    glad_glStencilOp = (PFNGLSTENCILOPPROC)fp("glStencilOp");
    glad_glDepthFunc = (PFNGLDEPTHFUNCPROC)fp("glDepthFunc");
    glad_glPixelStoref = (PFNGLPIXELSTOREFPROC)fp("glPixelStoref");
    glad_glPixelStorei = (PFNGLPIXELSTOREIPROC)fp("glPixelStorei");
    glad_glReadBuffer = (PFNGLREADBUFFERPROC)fp("glReadBuffer");
    glad_glReadPixels = (PFNGLREADPIXELSPROC)fp("glReadPixels");
    glad_glGetBooleanv = (PFNGLGETBOOLEANVPROC)fp("glGetBooleanv");
    glad_glGetDoublev = (PFNGLGETDOUBLEVPROC)fp("glGetDoublev");
    glad_glGetError = (PFNGLGETERRORPROC)fp("glGetError");
    glad_glGetFloatv = (PFNGLGETFLOATVPROC)fp("glGetFloatv");
    glad_glGetIntegerv = (PFNGLGETINTEGERVPROC)fp("glGetIntegerv");
    glad_glGetString = (PFNGLGETSTRINGPROC)fp("glGetString");
    glad_glGetTexImage = (PFNGLGETTEXIMAGEPROC)fp("glGetTexImage");
    glad_glGetTexParameterfv = (PFNGLGETTEXPARAMETERFVPROC)fp("glGetTexParameterfv");
    glad_glGetTexParameteriv = (PFNGLGETTEXPARAMETERIVPROC)fp("glGetTexParameteriv");
    glad_glGetTexLevelParameterfv = (PFNGLGETTEXLEVELPARAMETERFVPROC)fp("glGetTexLevelParameterfv");
    glad_glGetTexLevelParameteriv = (PFNGLGETTEXLEVELPARAMETERIVPROC)fp("glGetTexLevelParameteriv");
    glad_glIsEnabled = (PFNGLISENABLEDPROC)fp("glIsEnabled");
    glad_glDepthRange = (PFNGLDEPTHRANGEPROC)fp("glDepthRange");
    glad_glViewport = (PFNGLVIEWPORTPROC)fp("glViewport");
    
    // Version: 1.1
    glad_glDrawArrays = (PFNGLDRAWARRAYSPROC)fp("glDrawArrays");
    glad_glDrawElements = (PFNGLDRAWELEMENTSPROC)fp("glDrawElements");
    glad_glGenTextures = (PFNGLGENTEXTURESPROC)fp("glGenTextures");
    glad_glDeleteTextures = (PFNGLDELETETEXTURESPROC)fp("glDeleteTextures");
    glad_glBindTexture = (PFNGLBINDTEXTUREPROC)fp("glBindTexture");
    
    // Version: 3.0
    glad_glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)fp("glGenVertexArrays");
    glad_glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)fp("glBindVertexArray");
    glad_glGenBuffers = (PFNGLGENBUFFERSPROC)fp("glGenBuffers");
    glad_glBindBuffer = (PFNGLBINDBUFFERPROC)fp("glBindBuffer");
    glad_glBufferData = (PFNGLBUFFERDATAPROC)fp("glBufferData");
    glad_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)fp("glVertexAttribPointer");
    glad_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)fp("glEnableVertexAttribArray");
    glad_glCreateShader = (PFNGLCREATESHADERPROC)fp("glCreateShader");
    glad_glShaderSource = (PFNGLSHADERSOURCEPROC)fp("glShaderSource");
    glad_glCompileShader = (PFNGLCOMPILESHADERPROC)fp("glCompileShader");
    glad_glGetShaderiv = (PFNGLGETSHADERIVPROC)fp("glGetShaderiv");
    glad_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)fp("glGetShaderInfoLog");
    glad_glCreateProgram = (PFNGLCREATEPROGRAMPROC)fp("glCreateProgram");
    glad_glAttachShader = (PFNGLATTACHSHADERPROC)fp("glAttachShader");
    glad_glLinkProgram = (PFNGLLINKPROGRAMPROC)fp("glLinkProgram");
    glad_glGetProgramiv = (PFNGLGETPROGRAMIVPROC)fp("glGetProgramiv");
    glad_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)fp("glGetProgramInfoLog");
    glad_glUseProgram = (PFNGLUSEPROGRAMPROC)fp("glUseProgram");
    glad_glDeleteShader = (PFNGLDELETESHADERPROC)fp("glDeleteShader");
    glad_glDeleteProgram = (PFNGLDELETEPROGRAMPROC)fp("glDeleteProgram");
    glad_glUniform1f = (PFNGLUNIFORM1FPROC)fp("glUniform1f");
    glad_glUniform2f = (PFNGLUNIFORM2FPROC)fp("glUniform2f");
    glad_glUniform3f = (PFNGLUNIFORM3FPROC)fp("glUniform3f");
    glad_glUniform4f = (PFNGLUNIFORM4FPROC)fp("glUniform4f");
    glad_glUniform1i = (PFNGLUNIFORM1IPROC)fp("glUniform1i");
    glad_glUniform2i = (PFNGLUNIFORM2IPROC)fp("glUniform2i");
    glad_glUniform3i = (PFNGLUNIFORM3IPROC)fp("glUniform3i");
    glad_glUniform4i = (PFNGLUNIFORM4IPROC)fp("glUniform4i");
    glad_glUniform1fv = (PFNGLUNIFORM1FVPROC)fp("glUniform1fv");
    glad_glUniform2fv = (PFNGLUNIFORM2FVPROC)fp("glUniform2fv");
    glad_glUniform3fv = (PFNGLUNIFORM3FVPROC)fp("glUniform3fv");
    glad_glUniform4fv = (PFNGLUNIFORM4FVPROC)fp("glUniform4fv");
    glad_glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)fp("glUniformMatrix2fv");
    glad_glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)fp("glUniformMatrix3fv");
    glad_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)fp("glUniformMatrix4fv");
    glad_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)fp("glGetUniformLocation");
    
    // Version: 3.2
    glad_glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)fp("glFramebufferTexture");
    glad_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)fp("glGenFramebuffers");
    glad_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)fp("glBindFramebuffer");
    glad_glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)fp("glGenRenderbuffers");
    glad_glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)fp("glBindRenderbuffer");
    glad_glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)fp("glRenderbufferStorage");
    glad_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)fp("glFramebufferRenderbuffer");
    glad_glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)fp("glFramebufferTexture2D");
    glad_glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)fp("glCheckFramebufferStatus");
    glad_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)fp("glDeleteFramebuffers");
    glad_glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)fp("glDeleteRenderbuffers");
    glad_glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)fp("glDeleteVertexArrays");
    glad_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)fp("glDeleteBuffers");
    glad_glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)fp("glBlitFramebuffer");
    
    // Check if all required functions were loaded
    if (glad_glClear == NULL ||
        glad_glClearColor == NULL ||
        glad_glGenTextures == NULL ||
        glad_glBindTexture == NULL ||
        glad_glGenFramebuffers == NULL ||
        glad_glBindFramebuffer == NULL ||
        glad_glGenBuffers == NULL ||
        glad_glBindBuffer == NULL ||
        glad_glBufferData == NULL ||
        glad_glGenVertexArrays == NULL ||
        glad_glBindVertexArray == NULL) {
        return 0;
    }
    
    initialized = 1;
    return 1;
}

// OpenGL function definitions
PFNGLCULLFACEPROC glad_glCullFace = NULL;
PFNGLFRONTFACEPROC glad_glFrontFace = NULL;
PFNGLHINTPROC glad_glHint = NULL;
PFNGLLINEWIDTHPROC glad_glLineWidth = NULL;
PFNGLPOINTSIZEPROC glad_glPointSize = NULL;
PFNGLPOLYGONMODEPROC glad_glPolygonMode = NULL;
PFNGLSCISSORPROC glad_glScissor = NULL;
PFNGLTEXPARAMETERFPROC glad_glTexParameterf = NULL;
PFNGLTEXPARAMETERFVPROC glad_glTexParameterfv = NULL;
PFNGLTEXPARAMETERIPROC glad_glTexParameteri = NULL;
PFNGLTEXPARAMETERIVPROC glad_glTexParameteriv = NULL;
PFNGLTEXIMAGE1DPROC glad_glTexImage1D = NULL;
PFNGLTEXIMAGE2DPROC glad_glTexImage2D = NULL;
PFNGLDRAWBUFFERPROC glad_glDrawBuffer = NULL;
PFNGLCLEARPROC glad_glClear = NULL;
PFNGLCLEARCOLORPROC glad_glClearColor = NULL;
PFNGLCLEARSTENCILPROC glad_glClearStencil = NULL;
PFNGLCLEARDEPTHPROC glad_glClearDepth = NULL;
PFNGLSTENCILMASKPROC glad_glStencilMask = NULL;
PFNGLCOLORMASKPROC glad_glColorMask = NULL;
PFNGLDEPTHMASKPROC glad_glDepthMask = NULL;
PFNGLDISABLEPROC glad_glDisable = NULL;
PFNGLENABLEPROC glad_glEnable = NULL;
PFNGLFINISHPROC glad_glFinish = NULL;
PFNGLFLUSHPROC glad_glFlush = NULL;
PFNGLBLENDFUNCPROC glad_glBlendFunc = NULL;
PFNGLLOGICOPPROC glad_glLogicOp = NULL;
PFNGLSTENCILFUNCPROC glad_glStencilFunc = NULL;
PFNGLSTENCILOPPROC glad_glStencilOp = NULL;
PFNGLDEPTHFUNCPROC glad_glDepthFunc = NULL;
PFNGLPIXELSTOREFPROC glad_glPixelStoref = NULL;
PFNGLPIXELSTOREIPROC glad_glPixelStorei = NULL;
PFNGLREADBUFFERPROC glad_glReadBuffer = NULL;
PFNGLREADPIXELSPROC glad_glReadPixels = NULL;
PFNGLGETBOOLEANVPROC glad_glGetBooleanv = NULL;
PFNGLGETDOUBLEVPROC glad_glGetDoublev = NULL;
PFNGLGETERRORPROC glad_glGetError = NULL;
PFNGLGETFLOATVPROC glad_glGetFloatv = NULL;
PFNGLGETINTEGERVPROC glad_glGetIntegerv = NULL;
PFNGLGETSTRINGPROC glad_glGetString = NULL;
PFNGLGETTEXIMAGEPROC glad_glGetTexImage = NULL;
PFNGLGETTEXPARAMETERFVPROC glad_glGetTexParameterfv = NULL;
PFNGLGETTEXPARAMETERIVPROC glad_glGetTexParameteriv = NULL;
PFNGLGETTEXLEVELPARAMETERFVPROC glad_glGetTexLevelParameterfv = NULL;
PFNGLGETTEXLEVELPARAMETERIVPROC glad_glGetTexLevelParameteriv = NULL;
PFNGLISENABLEDPROC glad_glIsEnabled = NULL;
PFNGLDEPTHRANGEPROC glad_glDepthRange = NULL;
PFNGLVIEWPORTPROC glad_glViewport = NULL;
PFNGLDRAWARRAYSPROC glad_glDrawArrays = NULL;
PFNGLDRAWELEMENTSPROC glad_glDrawElements = NULL;
PFNGLGENTEXTURESPROC glad_glGenTextures = NULL;
PFNGLDELETETEXTURESPROC glad_glDeleteTextures = NULL;
PFNGLBINDTEXTUREPROC glad_glBindTexture = NULL;
PFNGLGENVERTEXARRAYSPROC glad_glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glad_glBindVertexArray = NULL;
PFNGLGENBUFFERSPROC glad_glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glad_glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glad_glBufferData = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glad_glVertexAttribPointer = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glad_glEnableVertexAttribArray = NULL;
PFNGLCREATESHADERPROC glad_glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glad_glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glad_glCompileShader = NULL;
PFNGLGETSHADERIVPROC glad_glGetShaderiv = NULL;
PFNGLGETSHADERINFOLOGPROC glad_glGetShaderInfoLog = NULL;
PFNGLCREATEPROGRAMPROC glad_glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glad_glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glad_glLinkProgram = NULL;
PFNGLGETPROGRAMIVPROC glad_glGetProgramiv = NULL;
PFNGLGETPROGRAMINFOLOGPROC glad_glGetProgramInfoLog = NULL;
PFNGLUSEPROGRAMPROC glad_glUseProgram = NULL;
PFNGLDELETESHADERPROC glad_glDeleteShader = NULL;
PFNGLDELETEPROGRAMPROC glad_glDeleteProgram = NULL;
PFNGLUNIFORM1FPROC glad_glUniform1f = NULL;
PFNGLUNIFORM2FPROC glad_glUniform2f = NULL;
PFNGLUNIFORM3FPROC glad_glUniform3f = NULL;
PFNGLUNIFORM4FPROC glad_glUniform4f = NULL;
PFNGLUNIFORM1IPROC glad_glUniform1i = NULL;
PFNGLUNIFORM2IPROC glad_glUniform2i = NULL;
PFNGLUNIFORM3IPROC glad_glUniform3i = NULL;
PFNGLUNIFORM4IPROC glad_glUniform4i = NULL;
PFNGLUNIFORM1FVPROC glad_glUniform1fv = NULL;
PFNGLUNIFORM2FVPROC glad_glUniform2fv = NULL;
PFNGLUNIFORM3FVPROC glad_glUniform3fv = NULL;
PFNGLUNIFORM4FVPROC glad_glUniform4fv = NULL;
PFNGLUNIFORMMATRIX2FVPROC glad_glUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC glad_glUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC glad_glUniformMatrix4fv = NULL;
PFNGLGETUNIFORMLOCATIONPROC glad_glGetUniformLocation = NULL;
PFNGLFRAMEBUFFERTEXTUREPROC glad_glFramebufferTexture = NULL;
PFNGLGENFRAMEBUFFERSPROC glad_glGenFramebuffers = NULL;
PFNGLBINDFRAMEBUFFERPROC glad_glBindFramebuffer = NULL;
PFNGLGENRENDERBUFFERSPROC glad_glGenRenderbuffers = NULL;
PFNGLBINDRENDERBUFFERPROC glad_glBindRenderbuffer = NULL;
PFNGLRENDERBUFFERSTORAGEPROC glad_glRenderbufferStorage = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glad_glFramebufferRenderbuffer = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC glad_glFramebufferTexture2D = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glad_glCheckFramebufferStatus = NULL;
PFNGLDELETEFRAMEBUFFERSPROC glad_glDeleteFramebuffers = NULL;
PFNGLDELETERENDERBUFFERSPROC glad_glDeleteRenderbuffers = NULL;
PFNGLDELETEVERTEXARRAYSPROC glad_glDeleteVertexArrays = NULL;
PFNGLDELETEBUFFERSPROC glad_glDeleteBuffers = NULL;
PFNGLBLITFRAMEBUFFERPROC glad_glBlitFramebuffer = NULL; 