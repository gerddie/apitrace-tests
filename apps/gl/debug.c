/**************************************************************************
 *
 * Copyright 2013 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"


static int win;


enum DebugExtension {
    KHR_DEBUG,
    ARB_DEBUG_OUTPUT,
    AMD_DEBUG_OUTPUT,
};

static enum DebugExtension debugExtension = ARB_DEBUG_OUTPUT;


static const char *debugExtensionString = "GL_ARB_debug_output";


static void
parseArgs(int argc, char** argv)
{
   int i;

   for (i = 1; i < argc; ++i) {
      const char *arg = argv[i];
      if (strcmp(arg, "GL_KHR_debug") == 0) {
         debugExtension = KHR_DEBUG;
      } else if (strcmp(arg, "GL_ARB_debug_output") == 0) {
         debugExtension = ARB_DEBUG_OUTPUT;
      } else if (strcmp(arg, "GL_AMD_debug_output") == 0) {
         debugExtension = AMD_DEBUG_OUTPUT;
      } else {
         fprintf(stderr, "error: unknown extension %s\n", arg);
         exit(1);
      }
      debugExtensionString = arg;
   }
}


typedef void (GLAPIENTRY * PFNDEBUGMESSAGEINSERT)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);

static void GLAPIENTRY
dummyDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)
{
    /* No-op */
}


static PFNDEBUGMESSAGEINSERT debugMessageInsert = dummyDebugMessageInsert;

void
debugMessage(GLsizei length, const GLchar *buf)
{
   debugMessageInsert(GL_DEBUG_SOURCE_APPLICATION_ARB, GL_DEBUG_TYPE_OTHER_ARB, 0, GL_DEBUG_SEVERITY_MEDIUM_ARB, length, buf);
}


static void
Init(void)
{
   const GLubyte *extensions;
   GLboolean hasDebugExtension;
      
   extensions = glGetString(GL_EXTENSIONS);
   checkGlError("glGetString(GL_EXTENSIONS)");
   hasDebugExtension = checkExtension(debugExtensionString, extensions);

   if (GLEW_VERSION_3_0) {
      GLboolean hasDebugExtension3 = GL_FALSE;
      GLint i;

      GLint num_extensions = 0;
      glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
      checkGlError("glGetIntegerv(GL_NUM_EXTENSIONS)");

      for (i = 0; i < num_extensions; ++i) {
         const char *extension;

         extension = (const char *)glGetStringi(GL_EXTENSIONS, i);
         checkGlError("glGetStringi(GL_EXTENSIONS, i)");

         if (strlen(extension) == 0) {
            fprintf(stderr, "error: glGetStringi returned empty string\n");
            exit(1);
         }

         if (strcmp(extension, debugExtensionString) == 0) {
            hasDebugExtension3 = GL_TRUE;
         }
      }

      if (hasDebugExtension != hasDebugExtension3) {
         fprintf(stderr, "error: %s not consistently supported by GL3\n", debugExtensionString);
         exit(1);
      }
   }

   glewInit();

   if (hasDebugExtension != glewIsSupported(debugExtensionString)) {
      fprintf(stderr, "error: %s not consistently supported by GLEW\n", debugExtensionString);
      exit(1);
   }

   if (hasDebugExtension) {
      switch (debugExtension) {
      case KHR_DEBUG:
#ifdef GL_KHR_debug
          debugMessageInsert = glDebugMessageInsert;
#else
          fprintf(stderr, "error: GL_KHR_debug not supported by this version of GLEW\n");
          exit(1);
#endif
          break;
      case ARB_DEBUG_OUTPUT:
          debugMessageInsert = glDebugMessageInsertARB;
          break;
      case AMD_DEBUG_OUTPUT:
          debugMessageInsert = glDebugMessageInsertAMD;
          break;
      }
   } else {
      fprintf(stderr, "error: %s not supported\n", debugExtensionString);
      exit(1);
   }

   debugMessage(strlen("Init"), "Init - this should not be included");

   glClearColor(0.3, 0.1, 0.3, 1.0);
}

static void Reshape(int width, int height)
{
   debugMessage(-1, __FUNCTION__);

   glViewport(0, 0, (GLint)width, (GLint)height);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
   glMatrixMode(GL_MODELVIEW);
}

static void Draw(void)
{
   debugMessage(-1, __FUNCTION__);

   glClear(GL_COLOR_BUFFER_BIT); 

   glBegin(GL_TRIANGLES);
   glColor3f(.8,0,0); 
   glVertex3f(-0.9, -0.9, -30.0);
   glColor3f(0,.9,0); 
   glVertex3f( 0.9, -0.9, -30.0);
   glColor3f(0,0,.7); 
   glVertex3f( 0.0,  0.9, -30.0);
   glEnd();

   glFlush();

   glutDestroyWindow(win);
   
   exit(0);
}


int
main(int argc, char **argv)
{
   parseArgs(argc, argv);

   glutInit(&argc, argv);

   glutInitWindowSize(250, 250);

   glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_SINGLE);

   win = glutCreateWindow(*argv);
   if (!win) {
      exit(1);
   }

   Init();

   glutReshapeFunc(Reshape);
   glutDisplayFunc(Draw);
   glutMainLoop();
   return 0;
}