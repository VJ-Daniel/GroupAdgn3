#ifndef TEXTURE_H
#define TEXTURE_H

#include <glew.h>

class Texture
{
private:
    GLuint textureID;

    int width;
    int height;
    int channels;

public:
    Texture();
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    bool LoadFromFile(
        const char* filePath,
        bool useNearestFiltering = true
    );

    void Bind(
        unsigned int textureUnit = 0
    ) const;

    GLuint GetID() const;

    int GetWidth() const;
    int GetHeight() const;

    bool IsLoaded() const;

    ~Texture();
};

#endif