#include "Texture.h"

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"

GLuint texture_load(const char* path) {
  GLuint id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_2D, id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, channels;
  stbi_set_flip_vertically_on_load(1);
  unsigned char *data = stbi_load(path, &width, &height, &channels, 0);

  if (!data) {
    printf("Failed to load texture: %s\n", path);
    glDeleteTextures(1, &id);
    return 0;
  }

  GLenum format = channels == 4 ? GL_RGBA : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);
  return id;
}

void texture_free(GLuint texture_id) {
  glDeleteTextures(1, &texture_id);
}

