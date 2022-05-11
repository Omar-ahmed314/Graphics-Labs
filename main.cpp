#include <iostream>
#include <fstream>
#include <string>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#define width 1900
#define height 1500

GLuint loadShader(const std::string& filePath, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    std::ifstream file(filePath);
    std::string source = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);
    return shader;
}

struct Vertex {
    float x, y, z;
    uint8_t r, g, b, a;
};

struct Color {
    uint8_t r, g, b, a;
};

int main(int, char**) {
    
    if(!glfwInit()){
        std::cerr << "Failed to initialize GLFW" << std::endl;
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(width, height, "Example 1", nullptr, nullptr);
    if(!window){
        std::cerr << "Failed to create window" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    GLuint program = glCreateProgram();
    GLuint vs = loadShader("assets/shaders/sample.vert", GL_VERTEX_SHADER);
    glAttachShader(program, vs);
    glDeleteShader(vs);
    GLuint fs = loadShader("assets/shaders/sample.frag", GL_FRAGMENT_SHADER);
    glAttachShader(program, fs);
    glDeleteShader(fs);
    glLinkProgram(program);
    glUseProgram(program);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    Vertex vertices[] = {
        {-0.5f, -0.5f, 0.0f,   0, 255, 255, 255},
        { 0.5f, -0.5f, 0.0f, 255,   0, 255, 255},
        { 0.5f,  0.5f, 0.0f, 255, 255,   0, 255},
        {-0.5f,  0.5f, 0.0f, 255,   0, 255, 255},
    };

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 4*sizeof(Vertex), vertices, GL_STATIC_DRAW);

    GLint positionLoc = 0; //glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, false, sizeof(Vertex), (void*)0);

    GLint colorLoc = 1; //glGetAttribLocation(program, "color");
    glEnableVertexAttribArray(colorLoc);
    glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, true, sizeof(Vertex), (void*)offsetof(Vertex, r));

    uint16_t elements[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(uint16_t), elements, GL_STATIC_DRAW);

    // stop binding the vertex array as nothing will be recorded
    // glBindVertexArray(0);

    // Get the matrix uniform variable
    GLint matrix_uniform_position = glGetUniformLocation(program, "matrix");


    // 1.0 configure the depth buffer in order to show the objects in order
    //  1.1 enable the depth check
    glEnable(GL_DEPTH_TEST);
    //  1.2 here the function select the least depth as it closer than the far one
    glDepthFunc(GL_LESS);

    // 1.3 depth mask
    glDepthMask(true);
    // 1.4 depth colors mask
    glColorMask(true, true, true, true);

/*
    // enable the back face
    glEnable(GL_CULL_FACE);
    // select the face to be hidden
    glCullFace(GL_BACK);
    // which direction of drawing to consider it a front face
    glFrontFace(GL_CCW);
*/

    /** 
     * view port is where i want to draw the scene
     * this function considers the starting point is at the bottom left of the screen
     * @param x the starting x coordinate
     * @param y the starting y coordinate
     * @param width the width of the screen
     * @param height the height of the screen
    */
    glViewport(width / 2, height / 2, width / 2, height / 2);


    while(!glfwWindowShouldClose(window)){
        
        glClearColor(0.2f, 0.4f, 0.6f, 1.0f);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // generate an angle to rotate with
        float angle = (float)glfwGetTime();
        
        // This matrix is the camera view or where the camera exist
        glm::mat4 view = glm::lookAt(
            glm::vec3(2 * glm::sin(angle), 1, 2 * glm::cos(angle)),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
        );

        // This matrix is the projection view or the angle of the sight
        glm::mat4 projection = glm::perspective(
            glm::pi<float>() / 2,
            width / height * 1.0f,
            0.01f,
            100.0f
        );

        glViewport(width / 2, height / 2, width / 2, height / 2);
        // Try to translate the object into another direction
        for(int z = -1; z <= 1; z++)
        {
            // This model matrix translate the object along with z direction
            glm::mat4 model = glm::translate(
                glm::mat4(1.0f),
                glm::vec3(0, 0, z)
            );

            // A matrix that reflect the whole effect
            glm::mat4 VB = projection * view * model;
            // send the matrix transform to the shaders
            glUniformMatrix4fv(matrix_uniform_position, 1, false, (float*)&VB);
            // glDrawArrays(GL_TRIANGLES, 0, 3);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
        }

        glViewport(0, 0, width / 2, height / 2);
        // Try to translate the object into another direction
        for(int z = -1; z <= 1; z++)
        {
            // This model matrix translate the object along with z direction
            glm::mat4 model = glm::translate(
                glm::mat4(1.0f),
                glm::vec3(0, 0, z)
            );

            // A matrix that reflect the whole effect
            glm::mat4 VB = projection * view * model;
            // send the matrix transform to the shaders
            glUniformMatrix4fv(matrix_uniform_position, 1, false, (float*)&VB);
            // glDrawArrays(GL_TRIANGLES, 0, 3);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0);
        }
        

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
