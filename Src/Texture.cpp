#include "Headers/Texture.h"

#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "Dependencies/stb_image.h"

Texture::Texture()
    : textureID(0),
    width(0),
    height(0),
    channels(0)
{}

bool Texture::LoadFromFile(
    const char* filePath,
    bool useNearestFiltering)
{
    if (filePath == nullptr)
    {
        std::cerr << "Texture path was null.\n";
        return false;
    }

    if (textureID != 0)
    {
        glDeleteTextures(
            1,
            &textureID
        );

        textureID = 0;
    }

    unsigned char* imageData = stbi_load(
        filePath,
        &width,
        &height,
        &channels,
        0
    );

    if (imageData == nullptr)
    {
        std::cerr
            << "Failed to load texture: "
            << filePath
            << "\nReason: "
            << stbi_failure_reason()
            << '\n';

        return false;
    }

    GLenum imageFormat;

    if (channels == 4)
    {
        imageFormat = GL_RGBA;
    }
    else if (channels == 3)
    {
        imageFormat = GL_RGB;
    }
    else if (channels == 1)
    {
        imageFormat = GL_RED;
    }
    else
    {
        std::cerr
            << "Unsupported number of texture channels: "
            << channels
            << '\n';

        stbi_image_free(imageData);
        return false;
    }

    glGenTextures(
        1,
        &textureID
    );

    glBindTexture(
        GL_TEXTURE_2D,
        textureID
    );

    glPixelStorei(
        GL_UNPACK_ALIGNMENT,
        1
    );

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        imageFormat,
        width,
        height,
        0,
        imageFormat,
        GL_UNSIGNED_BYTE,
        imageData
    );

    glGenerateMipmap(
        GL_TEXTURE_2D
    );

    GLint filter;

    if (useNearestFiltering)
    {
        filter = GL_NEAREST;
    }
    else
    {
        filter = GL_LINEAR;
    }

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        filter
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        filter
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );

    glBindTexture(
        GL_TEXTURE_2D,
        0
    );

    stbi_image_free(
        imageData
    );

    std::cout
        << "Loaded texture: "
        << filePath
        << " ("
        << width
        << " x "
        << height
        << ")\n";

    return true;
}

void Texture::Bind(
    unsigned int textureUnit) const
{
    if (textureID == 0)
    {
        return;
    }

    glActiveTexture(
        GL_TEXTURE0 + textureUnit
    );

    glBindTexture(
        GL_TEXTURE_2D,
        textureID
    );
}

GLuint Texture::GetID() const
{
    return textureID;
}

int Texture::GetWidth() const
{
    return width;
}

int Texture::GetHeight() const
{
    return height;
}

bool Texture::IsLoaded() const
{
    return textureID != 0;
}

Texture::~Texture()
{
    if (textureID != 0)
    {
        glDeleteTextures(
            1,
            &textureID
        );

        textureID = 0;
    }
}