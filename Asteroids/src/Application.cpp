#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

/*
    TODOS:
        -Add diagonal movement
        -Add asteroids
        -Add collision system
        -Add shooting
*/

class Buffers {
    unsigned int buffer;
    unsigned int dataSize;
    int type;
    void* data;
public:
    static float deltaMovement;

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
        glBindBuffer(type, this->buffer);
        glBufferSubData(type, 0, dataSize, newData);
    }

    void attribPointer(unsigned int index, int vecSize, unsigned int GLDataType, int typeSize, void* offset) {
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(index, vecSize, GLDataType, GL_FALSE, typeSize * vecSize, offset);
    }
};

class Player : public Buffers {
public:
    static const int arrayLength = 6;
    float position[arrayLength];
};

/*class Asteroids : public Buffers {
public:

};*/

struct ShaderProgram {
    static std::string VertexSource;
    static std::string FragmentSource;

    static void ParseShader(const std::string& filepath) {
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

        ShaderProgram::VertexSource = ss[0].str();
        ShaderProgram::FragmentSource = ss[1].str();
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

    static int CreateShaders(const std::string& filepath) {
        unsigned int program = glCreateProgram();
        ParseShader(filepath);
        unsigned int vs = CompileShader(GL_VERTEX_SHADER, VertexSource);
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, FragmentSource);

        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glLinkProgram(program);
        glValidateProgram(program);

        glDetachShader(program, vs);
        glDetachShader(program, fs);

        glDeleteShader(vs);

        return program;
    }
};

Player g_playerBuffer;
float Buffers::deltaMovement;
std::string ShaderProgram::VertexSource;
std::string ShaderProgram::FragmentSource;

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

    g_playerBuffer.attribPointer(0, 2, GL_FLOAT, sizeof(float), 0);
}







void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_RELEASE) return;
    std::cout << key << std::endl;

    const int MOVEMENT_RATIO = 12;
    float distanceToMove = MOVEMENT_RATIO / Buffers::deltaMovement;

    switch (key) {
        case GLFW_KEY_W:
            MovePlayer(0, distanceToMove);
            break;
        case GLFW_KEY_A:
            MovePlayer(-distanceToMove, 0);
            break;
        case GLFW_KEY_S:
            MovePlayer(0, -distanceToMove);
            break;
        case GLFW_KEY_D:
            MovePlayer(distanceToMove, 0);
            break;
    }
}

void GLAPIENTRY MessageCallback
(
    unsigned int source,
    unsigned int type,
    unsigned int id,
    unsigned int severity,
    int length,
    const char* message,
    const void* userParam
) {
    std::cout <<
        "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") << std::endl <<
        "type: " << type << std::endl <<
        "severity: " << severity << std::endl <<
        "message" << message << std::endl;
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
    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW INIT ERROR" << std::endl;
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    LoadPlayer();

    unsigned int shader = ShaderProgram::CreateShaders("src/shaders/Basic.shader");
    glUseProgram(shader);

    double previousTime = glfwGetTime();
    int frameCount = 0;

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        frameCount++;

        Buffers::deltaMovement = (frameCount / (currentTime - previousTime));
        if (currentTime - previousTime >= 1.0) {
            std::cout << Buffers::deltaMovement << std::endl;
            frameCount = 0;
            previousTime = currentTime;
        }
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