#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct Buffers {
    unsigned int buffer;

    Buffers(unsigned int bufferCount) {
        glGenBuffers(bufferCount, &this->buffer);
    }

    void initializeBuffer(int type, int dataSize, void* data, int usage) {
        glBindBuffer(type, this->buffer);
        glBufferData(type, dataSize, data, usage);
    }

};

struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmenteSource;
};

void LoadPlayer() {
    const int POSITION_LENGTH = 6;
    float positions[POSITION_LENGTH] = {
        -0.05f, -0.05f,
         0.0f,  0.05f,
         0.05f, -0.05f
    };

    Buffers buffers(1);
    buffers.initializeBuffer(GL_ARRAY_BUFFER, POSITION_LENGTH * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);
}

static ShaderProgramSource ParseShader(const std::string& filepath) {
    std::ifstream stream(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            ss[(int)type] << line << "\n";
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
     unsigned int id = glCreateShader(type);
     const char* src = source.c_str();
     glShaderSource(id, 1, &src, nullptr);
     glCompileShader(id);

     int result;
     glGetShaderiv(id, GL_COMPILE_STATUS, &result);
     if (result == GL_FALSE) {
         int length;
         glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
         char* message = (char*)alloca(length * sizeof(char));
         glGetShaderInfoLog(id, length, &length, message);

         std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!"
             << std::endl;
         std::cout << message << std::endl;
         glDeleteShader(id);
         return 0;
     }

     return id;
}

static int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    //glDetachShader(program, vs);
    //glDetachShader(program, fs);

    glDeleteShader(vs);

    return program;
}


int GLFWSetup(GLFWwindow* window) {
    /* Make the window's context current */
    
    
    return 0;
}

int main()
{
    /* LIB SETUPS */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(640, 480, "Asteroids", NULL, NULL);;
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW INIT ERROR" << std::endl;
        return -1;
    }

    LoadPlayer();

    ShaderProgramSource source = ParseShader("src/shaders/Basic.shader");
    
    unsigned int shader = CreateShader(source.VertexSource, source.FragmenteSource);
    glUseProgram(shader);
    
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}