#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

class Buffers {
    unsigned int buffer;
    unsigned int dataSize;
    int type;
    void* data;
public:

    void genBuffers(unsigned int bufferCount) {
        glGenBuffers(bufferCount, &this->buffer);
    }

    void InitializeBuffer(int type, int dataSize, void* data, int usage) {
        this->dataSize = dataSize;
        this->data = data;
        this->type = type;
        glBindBuffer(type, this->buffer);
        glBufferData(type, dataSize, data, usage);
    }

    void RewriteBufferData(void* newData, int* offset) {
        glBufferSubData(type, 0, dataSize, newData);
    }
};

class Player : public Buffers {
public:
    static const int arrayLength = 6;
    float position[arrayLength];
};

Player g_playerBuffer;

struct ShaderProgramSource {
    std::string VertexSource;
    std::string FragmenteSource;
};

float* getNewPosition(float oldPosition[Player::arrayLength], float deltaX, float deltaY) {
    float newPosition[Player::arrayLength];
    for (int i = 0; i < Player::arrayLength; i++) {
        if (i % 2 == 0) {
            newPosition[i] = oldPosition[i] + deltaX;
        }
        else {
            newPosition[i] = oldPosition[i] + deltaY;
        }
    }

    return newPosition;
}

void MovePlayer(float deltaX, float deltaY) {
    float* newPosition = getNewPosition(g_playerBuffer.position, deltaX, deltaY);
    g_playerBuffer.RewriteBufferData(newPosition, 0);

    for (int i = 0; i < Player::arrayLength; i++) {
        g_playerBuffer.position[i] = newPosition[i];
    }
}

void LoadPlayer() {
    float positions[Player::arrayLength] = {
        -0.05f, -0.05f,
         0.0f,   0.05f,
         0.05f, -0.05f
    };

    for (int i = 0; i < Player::arrayLength; i++) {
        g_playerBuffer.position[i] = positions[i];
    }
    g_playerBuffer.genBuffers(1);
    g_playerBuffer.InitializeBuffer(GL_ARRAY_BUFFER, Player::arrayLength * sizeof(float), positions, GL_STATIC_DRAW);

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

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) return;
    std::cout << key << std::endl;

    const float DELTA_MOVE = 0.005f;

    switch (key) {
        case GLFW_KEY_W:
            MovePlayer(0, DELTA_MOVE);
            break;
        case GLFW_KEY_A:
            MovePlayer(-DELTA_MOVE, 0);
            break;
        case GLFW_KEY_S:
            MovePlayer(0, -DELTA_MOVE);
            break;
        case GLFW_KEY_D:
            MovePlayer(DELTA_MOVE, 0);
            break;
    }
}

int main()
{
    /* LIB SETUPS */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(640, 480, "Asteroids", NULL, NULL);
    
    glfwSetKeyCallback(window, KeyCallback);

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