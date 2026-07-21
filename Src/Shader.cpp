#include "Shader.h"

#include <glew.h>        
#include <gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

Shader::Shader(
    const char* vertexPath,
    const char* fragmentPath)
{
    //--------------------------------------------------
    // Open Shader Files
    //--------------------------------------------------

    std::ifstream vertexFile(
        vertexPath);

    std::ifstream fragmentFile(
        fragmentPath);

    if (!vertexFile.is_open())
    {
        std::cout
            << "ERROR OPENING "
            << vertexPath
            << std::endl;
    }

    if (!fragmentFile.is_open())
    {
        std::cout
            << "ERROR OPENING "
            << fragmentPath
            << std::endl;
    }

    //--------------------------------------------------
    // Read Shader Source
    //--------------------------------------------------

    std::stringstream vertexStream;
    std::stringstream fragmentStream;

    vertexStream
        << vertexFile.rdbuf();

    fragmentStream
        << fragmentFile.rdbuf();

    std::string vertexCode =
        vertexStream.str();

    std::string fragmentCode =
        fragmentStream.str();

    const char* vertexSource =
        vertexCode.c_str();

    const char* fragmentSource =
        fragmentCode.c_str();

    //--------------------------------------------------
    // Compile Status
    //--------------------------------------------------

    GLint success;

    GLchar infoLog[1024];

    //--------------------------------------------------
    // Vertex Shader
    //--------------------------------------------------

    GLuint vertexShader =
        glCreateShader(
            GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexSource,
        nullptr);

    glCompileShader(
        vertexShader);

    glGetShaderiv(
        vertexShader,
        GL_COMPILE_STATUS,
        &success);

    if (!success)
    {
        glGetShaderInfoLog(
            vertexShader,
            sizeof(infoLog),
            nullptr,
            infoLog);

        std::cout
            << "VERTEX SHADER ERROR:"
            << std::endl
            << infoLog
            << std::endl;
    }

    //--------------------------------------------------
    // Fragment Shader
    //--------------------------------------------------

    GLuint fragmentShader =
        glCreateShader(
            GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentSource,
        nullptr);

    glCompileShader(
        fragmentShader);

    glGetShaderiv(
        fragmentShader,
        GL_COMPILE_STATUS,
        &success);

    if (!success)
    {
        glGetShaderInfoLog(
            fragmentShader,
            sizeof(infoLog),
            nullptr,
            infoLog);

        std::cout
            << "FRAGMENT SHADER ERROR:"
            << std::endl
            << infoLog
            << std::endl;
    }

    //--------------------------------------------------
    // Shader Program
    //--------------------------------------------------

    programID =
        glCreateProgram();

    glAttachShader(
        programID,
        vertexShader);

    glAttachShader(
        programID,
        fragmentShader);

    glLinkProgram(
        programID);

    glGetProgramiv(
        programID,
        GL_LINK_STATUS,
        &success);

    if (!success)
    {
        glGetProgramInfoLog(
            programID,
            sizeof(infoLog),
            nullptr,
            infoLog);

        std::cout
            << "PROGRAM LINK ERROR:"
            << std::endl
            << infoLog
            << std::endl;
    }

    //--------------------------------------------------
    // Cleanup
    //--------------------------------------------------

    glDeleteShader(
        vertexShader);

    glDeleteShader(
        fragmentShader);
}

void Shader::SetMat4(const std::string& name,
    const glm::mat4& matrix)
{
    glUniformMatrix4fv(
        glGetUniformLocation(programID, name.c_str()),
        1,
        GL_FALSE,
        glm::value_ptr(matrix));
}

//--------------------------------------------------
// Use Program
//--------------------------------------------------

void Shader::Use()
{
    glUseProgram(
        programID);
}

//--------------------------------------------------
// Accessor
//--------------------------------------------------

unsigned int Shader::GetID() const
{
    return programID;
}

//--------------------------------------------------
// Destructor
//--------------------------------------------------

Shader::~Shader()
{
    glDeleteProgram(
        programID);
}