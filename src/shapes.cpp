#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>

const unsigned int SCR_WIDTH = 500;
const unsigned int SCR_HEIGHT = 500;

// ---------- Shaders ----------
const char* vertexShaderSource = R"(
#version 130
attribute vec2 vPos;
attribute vec3 vColor;
varying vec3 ourColor;

uniform vec2 offset;
uniform float scale;

void main() {
    gl_Position = vec4((vPos * scale) + offset, 0.0, 1.0);
    ourColor = vColor;
}
)";

const char* fragmentShaderSource = R"(
#version 130
varying vec3 ourColor;
void main() {
    gl_FragColor = vec4(ourColor, 1.0);
}
)";

// ---------- Utils ----------
unsigned int createShaderProgram(const char* vSource, const char* fSource) {
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compile error:\n" << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compile error:\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    // manual binding since GLSL 130 has no layout qualifiers
    glBindAttribLocation(shaderProgram, 0, "vPos");
    glBindAttribLocation(shaderProgram, 1, "vColor");

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader link error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// ---------- Shape Builders ----------
void buildCircle(std::vector<float>& vertices, int segments, float r, bool gradient) {
    vertices.clear();
    vertices.push_back(0.0f); vertices.push_back(0.0f); 
    vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);

    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = r * cos(theta);
        float y = r * sin(theta);
        vertices.push_back(x);
        vertices.push_back(y);

        if (gradient) {
            float t = (sin(theta) + 1.0f) / 2.0f;
            vertices.push_back(1.0f - t); vertices.push_back(0.0f); vertices.push_back(0.0f);
        } else {
            vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
        }
    }
}

void buildEllipse(std::vector<float>& vertices, int segments, float rx, float ry) {
    vertices.clear();
    vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);

    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * float(i) / float(segments);
        float x = rx * cos(theta);
        float y = ry * sin(theta);
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(1.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
    }
}

void buildTriangle(std::vector<float>& vertices) {
    vertices = {
        0.0f,  0.2f,  1.0f, 0.0f, 0.0f,
       -0.2f, -0.2f,  0.0f, 1.0f, 0.0f,
        0.2f, -0.2f,  0.0f, 0.0f, 1.0f
    };
}

void buildZebraSquare(std::vector<float>& vertices, int layers) {
    vertices.clear();
    float step = 0.9f / layers;

    for (int i = 0; i < layers; i++) {
        float size = 0.9f - i * step;
        float color = (i % 2 == 0) ? 1.0f : 0.0f;

        float sq[30] = {
            -size, -size, color, color, color,
             size, -size, color, color, color,
             size,  size, color, color, color,

            -size, -size, color, color, color,
             size,  size, color, color, color,
            -size,  size, color, color, color
        };

        vertices.insert(vertices.end(), sq, sq + 30);
    }
}

// ---------- VBO helper ----------
unsigned int createVBO(const std::vector<float>& vertices) {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    return VBO;
}

// ---------- Main ----------
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Figures (GLSL 130)", NULL, NULL);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr << "Failed to init GLAD\n"; return -1; }

    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    std::vector<float> ellipseVerts, triangleVerts, circleVerts, squareVerts;
    buildEllipse(ellipseVerts, 64, 0.25f, 0.15f);
    buildTriangle(triangleVerts);
    buildCircle(circleVerts, 64, 0.2f, true);
    buildZebraSquare(squareVerts, 8);

    unsigned int ellipseVBO = createVBO(ellipseVerts);
    unsigned int triangleVBO = createVBO(triangleVerts);
    unsigned int circleVBO   = createVBO(circleVerts);
    unsigned int squareVBO   = createVBO(squareVerts);

    int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    int scaleLoc = glGetUniformLocation(shaderProgram, "scale");

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Ellipse
        glUniform2f(offsetLoc, -0.7f, 0.8f);
        glUniform1f(scaleLoc, 1.0f);
        glBindBuffer(GL_ARRAY_BUFFER, ellipseVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, ellipseVerts.size() / 5);

        // Triangle
        glUniform2f(offsetLoc, 0.0f, 0.8f);
        glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLES, 0, triangleVerts.size() / 5);

        // Circle
        glUniform2f(offsetLoc, 0.7f, 0.8f);
        glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLE_FAN, 0, circleVerts.size() / 5);

        // Zebra Square
        glUniform2f(offsetLoc, 0.0f, -0.2f);
        glUniform1f(scaleLoc, 0.8f);
        glBindBuffer(GL_ARRAY_BUFFER, squareVBO);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glDrawArrays(GL_TRIANGLES, 0, squareVerts.size() / 5);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

