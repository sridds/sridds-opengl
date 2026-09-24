#include <stdio.h>
#include <math.h>
#include <../core/mathf.h>

#include <external/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

using namespace std;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// the process of transforming 3D coordinates to 2D pixels is managed by the graphics pipeline
// The graphics pipeline can be divided into two large parts:
//      - the first transforms 3D coordinates into 2D coordinates
//      - the second part transforms 2D coordinates into colored pixels
// The pipeline takes a set of 3D coordinates

// The graphics pipeline can be divided into several, highly specialized steps that can easily be executed in parallel
// Because of how parrallelized the pipeline is, todays graphics cards have thousands of small processing cores to quickly process data
// The processing cores run small programs on the GPU for each step of the pipeline, called shaders

// as input to the graphics pipeline, we pass in a list of 3D coordinates that should form a triangle in an array called vertex data.
// vertex data is a collection of vertices, a vertex is a collection of data per 3D coordinate.
// vertex data is represented using vertex attributes that can contain any data we like (for simplicity's sake let's assume each vertex consists of a 3D pos and some color value)

// in order rfor OpenGL to know what to make of the collection of coordinates and color values,
// open gl requires you to hind what kind of render types you want to form with the data
// do we want the data rendered as a collection of points? triangles? or just one long line?

// the first part of the pipeline is the vertex shader, which takes a single vertex as input
//      - transforms 3D coordinates into different 3D coordinates
//      - the vertex shader lets us do basic processing on the attributes


// the primitive assembly stage takes all the vertices as input. assembles all points into the primitive shape given
// The output is passed to the rasterization stage, where it maps the resulting primitives to corresponding pixels on the final screen
// This results in fragments for the fragment shader to use
// before the fragment shaders run, clipping is performed to discard fragments outside of view

// In OpenGL, a fragment is all the data required for OpenGL to render a single pixel
// The main purpose of a fragment shader is to calculate the final color of a pixel
// Usually, the fragment shader contains data about the 3D scene that it can use to calculate the final pixel color (lights, shadows, etc)

// After all corresponding color values have been determined, the final object passes through an alpha test, and the blending stage
// This stage checks the corresponding depth (and stencil) value of the fragment and uses those to check if the resulting fragment is in front or behind other objects and should get discarded
// This stage also checks for alpha values and blends the objects accordingly

// * even if a pixel output color is calculated in the fragment shader, the final pixel color could be somethign entirely different when rendering multiple triangles

// defined in normalized device coordinates
// after being processed by the vertex shader, vertex coordinates become normalized device coordinates
// these alues vary on x, y, and z from -1 to 1
// NDC coordinates then get transformed to screen-space coordinates via the viewport transform
float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f
};

const char *vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"uniform float time;\n"
"void main()\n"
"{\n"
"   float sinY = sin(time + aPos.x) * 0.5f;\n"
"   gl_Position = vec4(aPos.x, sinY + aPos.y, aPos.z, 1.0);\n"
"}\0";

const char *fragmentShaderSource =
"#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\0";

int main() {
    printf("Initializing...");
    
    if (!glfwInit())
    {
        printf("GLFW failed to initialize!");
        return 1;
    }
    
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL Learning", NULL, NULL);
    if (window == NULL)
    {
        printf("GLFW failed to create window!");
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD!\n");
        return 1;
    }
    
    // CREATE VERTEX SHADER
    unsigned int vertexShader; // we create a shader object in C using an unsigned int for our ID
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // CREATE FRAGMENT SHADER
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }
    
    // CREATE SHADER PROGRAM (LINKER)
    // a shader program object is the final linked version of multiple shadersd combined
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }
    
    glUseProgram(shaderProgram);
    // we can delete the shaders because they've already been linked into the shader program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);  
    
    // Create vertex buffer object
    // We need to create memory on the GPU as a location to store vertex data and configure how OpenGL should interpret the memory and how it should be sent to the graphics card
    // VBOs store a large umber of vertices in the GPU's memory. The advantage is that we can send large batches of data all at once to the graphics card without having to send one vertex at a time
    // Sending data to the graphics card from the GPU is slow, so wherever we can we try to send as much data possible at once
    // Once this data is in the graphics card's memory, the vertex shader has instant accest to these vertices and is extremely fast
    
    // Additionally, this is the first occurance of an object in open GL
    // Because OpenGL is written in C, it cannot support objects by default, so it was written with abstractions in mind.
    // An object in OpenGL is a collection of options that represents a subset of OpenGL's state
    // This is why you'll see a recurring pattern of:
    //      - Create and store a reference to an object as an ID
    //      - Bind the object (using its ID) to the target location of the context
    //      - Set options
    //      - Unbind the object
    //     * The options we set are stored in the object referenced by objectID and restored as soon as we bind the object back
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);  
    
    // GL_STATIC_DRAW: the data is set only once and used many times
    // since our triangle does not change positions, and is used a lot, its best to use this option.
    // a buffer with data that is likely to change frequently is better suited for GL_DYNAMIC_DRAW
    
    // The vertexshader allows us to specify any input we want in the form of vertex attributes
    // tell OpenGL how it should interpret vertex data (per vertex attribute)
    // vertexAttribPointer breakdown
    //  parameter 1 - specifies which vertex attribute we want to configure. recall that we set the location of the position vertex attributee to be layout (location = 0)
    //  parameter 2 - specifies the size of the vertex attribute
    //  parameter 3 - the type of data
    //  parameter 4 - do we want the data to be normalized?
    //  parameter 5 - the stride (space between consecutive vertex attributes)
    //  parameter 6 - the offset where the position data begins in the buffer
    
    // each vertex attribute takes its data from memory managed by a VBO and which VBO it takes data from (you can have multiple VBOs) is determined by the VBO currently bound to GL_ARRAY_BUFFER
    
    // The VAO remembers how OpenGL should interpret the vertex data when you draw it
    // It does not store the vertex data
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindVertexArray(0);
    
    // the vertex array obeject is bound like a VBO. This has the advantage that when configuring vertex attribute pointers, you only have to make those calls once
    // whenever we want to draw the object, we can just bind the corresponding VAO. This makes switching between different vertex data and attribute configurations as easy as binding a different VAO
    
    int timeLoc = glGetUniformLocation(shaderProgram, "time");
    
    while (!glfwWindowShouldClose(window))
    {
        float time = glfwGetTime();
        
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniform1f(timeLoc, time);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    return 0;
}
