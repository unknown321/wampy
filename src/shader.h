#ifndef WAMPY_SHADER_H
#define WAMPY_SHADER_H

#include <glad/glad.h>

GLuint ShaderProgram();

void GLAPIENTRY GlMessageCallback(GLenum source,
                                  GLenum type,
                                  GLuint id,
                                  GLenum severity,
                                  GLsizei length,
                                  const GLchar *message,
                                  const void *userParam);


#endif //WAMPY_SHADER_H
