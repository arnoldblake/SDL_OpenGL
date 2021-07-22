#include "glew.h"
#include "SDL.h"
#include "SDL_opengl.h"
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_PATH_SIZE 1024

// TODO: Implement Vertex Shader, Fragment Shader, Shader Program into our BObject

/*
    BObject contains a:
        Vertex Array Object
        Vertex Buffer Object
        Element Buffer Object
*/


typedef struct {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
} BObject;

const char* loadShader(const char* filename) {
    char cwd[MAX_PATH_SIZE];
    GetCurrentDirectory(MAX_PATH_SIZE, cwd);
    strcat(cwd, filename);

    FILE *f = fopen(cwd, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* string = malloc(fsize + 1);
    fread(string, 1, fsize, f);
    fclose(f);
    string[fsize] = 0;

    if(f == NULL) {
        printf("Error reading file %s\n", cwd);
    }

    return (const char*) string;
}

void checkForShaderErrors(GLuint* shader) {
    GLint status;
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(*shader, 512, NULL, buffer);
        printf("%s", buffer);
    }
}

void checkForCompileErrors(GLuint* program) {
    GLint status;
    glGetProgramiv(*program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetProgramInfoLog(*program, 512, NULL, buffer);
        printf("%s", buffer);
    }

}

int main (int argc, char *argv[]) {

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    //SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    SDL_Window* window = SDL_CreateWindow("OpenGL", 100, 100, 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);

    glewExperimental = true;
    glewInit();

    const char* vertexSource = loadShader("\\..\\vertexShader.glsl");
    const char* fragmentSource = loadShader("\\..\\fragmentShader.glsl");

    
    // Create and compile the vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    checkForShaderErrors(&vertexShader);

    // Create and compile the fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    checkForShaderErrors(&fragmentShader);

    // Link the vertex and fragment shader into a shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkForCompileErrors(&shaderProgram);

    // Tell GL that we are using our newly compiled shader program
    glUseProgram(shaderProgram);

    BObject bObject;

    GLfloat vertices[] = {
        // Position            // Color             // Texture
        -0.5f,  0.5f, 0.0f,    1.0f, 1.0f, 0.0f,    0.0f, 1.0f, // Top-left
         0.5f,  0.5f, 0.0f,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f, // Top-right
         0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,    1.0f, 0.0f, // Bottom-right
        -0.5f, -0.5f, 0.0f,    0.0f, 0.0f, 1.0f,    0.0f, 0.0f  // Bottom-left
    };
    // Element Buffer Indexes
    GLuint elements[] = {
        0,1,2,
        0,3,2
    };

    // Create Vertex Array Object
    glGenVertexArrays(1, &bObject.vao);
    // Create a Element Buffer Object and copy the vertex data to it
    glGenBuffers(1, &bObject.ebo);
    // Create a Vertex Buffer Object and copy the vertex data to it
    glGenBuffers(1, &bObject.vbo);

    glBindVertexArray(bObject.vao);

    glBindBuffer(GL_ARRAY_BUFFER, bObject.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bObject.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    // Specify the layout of the vertex data for position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Specify the layout of the vertex data for color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Specify the layout of the vertex data for texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Create texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("..\\container.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        printf("Failed to load texture data.\n");
    }
    stbi_image_free(data);

    SDL_Event windowEvent;
    while (true) {
        if (SDL_PollEvent(&windowEvent))
        {
            if (windowEvent.type == SDL_QUIT) break;
        }

        // Clear the screen to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a triangle from the 3 vertices
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}