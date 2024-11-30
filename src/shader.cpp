#include "shader.h"
#include "util/util.h"

const GLchar *vertexSource = "#version 100\n"
                             "uniform mat4 ProjMtx;\n"
                             "attribute vec3 Position;\n"
                             "attribute vec2 UV;\n"
                             "attribute vec4 Color;\n"

                             "varying vec2 Frag_UV;\n"
                             "varying vec4 Frag_Color;\n"
                             "void main()\n"
                             "{\n"
                             "    Frag_UV = UV;\n"
                             "    Frag_Color = Color;\n"
                             "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
                             "}\n";

const GLchar *fragmentSource = "#ifdef GL_ES\n"
                               "    precision mediump float;\n"
                               "#endif\n"
                               "uniform sampler2D Texture;\n"
                               "varying vec2 Frag_UV;\n"
                               "varying vec4 Frag_Color;\n"
                               "void main()\n"
                               "{\n"
                               "    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
                               //        "    gl_FragColor = vec4(0.3,0.5,0.4,0.4);\n"
                               "}\n";

GLuint ShaderProgram() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    GLint status;
    char buffer[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(vertexShader, 512, nullptr, buffer);
    if (status != GL_TRUE) {
        DLOG("Vertex shader:\n%s\n", buffer);
        exit(1);
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    glGetShaderInfoLog(fragmentShader, 512, nullptr, buffer);
    if (status != GL_TRUE) {
        DLOG("Fragment shader:\n%s\n", buffer);
        exit(1);
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, buffer);
        DLOG("Link failed:\n%s\n", buffer);
        exit(1);
    }

    glDetachShader(shaderProgram, fragmentShader);
    glDetachShader(shaderProgram, vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    GLint pos = glGetAttribLocation(shaderProgram, "Position");
    glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(pos);

    GLint uv = glGetAttribLocation(shaderProgram, "UV");
    glVertexAttribPointer(uv, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(uv);

    GLint color = glGetAttribLocation(shaderProgram, "Color");
    glVertexAttribPointer(color, 4, GL_FLOAT, GL_FALSE, 14 * sizeof(GLfloat), (void *)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(color);

    return shaderProgram;
}

void GLAPIENTRY
GlMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam) {
    DLOG(
        "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
        (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
        type,
        severity,
        message
    );
}
