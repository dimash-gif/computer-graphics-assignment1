#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <iostream>

const unsigned int SCR_WIDTH = 600;
const unsigned int SCR_HEIGHT = 600;

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

// -------- Shader Utils ----------
unsigned int createShaderProgram(const char* vSource, const char* fSource) {
    unsigned int vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vSource, NULL);
    glCompileShader(vShader);
    int success; char infoLog[512];
    glGetShaderiv(vShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader error:\n" << infoLog << std::endl;
    }

    unsigned int fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fSource, NULL);
    glCompileShader(fShader);
    glGetShaderiv(fShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader error:\n" << infoLog << std::endl;
    }

    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vShader);
    glAttachShader(prog, fShader);
    glBindAttribLocation(prog, 0, "vPos");
    glBindAttribLocation(prog, 1, "vColor");
    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(prog, 512, NULL, infoLog);
        std::cerr << "Shader link error:\n" << infoLog << std::endl;
    }
    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return prog;
}

// -------- Shape Builders ----------
std::vector<float> buildRectangle(float w, float h, float r, float g, float b) {
    return {
        -w, -h, r, g, b,
         w, -h, r, g, b,
         w,  h, r, g, b,

        -w, -h, r, g, b,
         w,  h, r, g, b,
        -w,  h, r, g, b
    };
}

std::vector<float> buildTriangle(float size, float r, float g, float b) {
    return {
         0.0f,  size, r, g, b,
        -size, -size, r, g, b,
         size, -size, r, g, b
    };
}

std::vector<float> buildCircle(int segments, float radius, float r, float g, float b) {
    std::vector<float> verts;
    verts.push_back(0.0f); verts.push_back(0.0f);
    verts.push_back(r); verts.push_back(g); verts.push_back(b);
    for (int i = 0; i <= segments; i++) {
        float theta = 2.0f * M_PI * i / segments;
        float x = radius * cos(theta);
        float y = radius * sin(theta);
        verts.push_back(x); verts.push_back(y);
        verts.push_back(r); verts.push_back(g); verts.push_back(b);
    }
    return verts;
}

// -------- VBO helper ----------
unsigned int createVBO(const std::vector<float>& vertices) {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    return VBO;
}

void drawVBO(unsigned int VBO, int count, int mode) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glDrawArrays(mode, 0, count);
}

// -------- Main ----------
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Table with Blocks", NULL, NULL);
    if (!window) { std::cerr << "Failed to create GLFW window\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { std::cerr << "Failed to init GLAD\n"; return -1; }

    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    // Table top
    std::vector<float> table = buildRectangle(0.9f, 0.1f, 0.6f, 0.3f, 0.0f);
    unsigned int tableVBO = createVBO(table);

    // Table leg (reuse same rectangle for both)
    std::vector<float> leg = buildRectangle(0.05f, 0.3f, 0.4f, 0.2f, 0.0f);
    unsigned int legVBO = createVBO(leg);

    // Blocks
    unsigned int redVBO   = createVBO(buildRectangle(0.15f, 0.15f, 1.0f, 0.0f, 0.0f));
    unsigned int greenVBO = createVBO(buildRectangle(0.1f, 0.2f, 0.0f, 1.0f, 0.0f));
    unsigned int blueVBO  = createVBO(buildRectangle(0.2f, 0.1f, 0.0f, 0.0f, 1.0f));

    // Extra shapes
    unsigned int circleVBO   = createVBO(buildCircle(40, 0.12f, 1.0f, 1.0f, 0.0f)); // yellow
    unsigned int triangleVBO = createVBO(buildTriangle(0.15f, 1.0f, 0.0f, 1.0f));   // magenta

    int offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    int scaleLoc  = glGetUniformLocation(shaderProgram, "scale");

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.8f, 0.9f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Table top
        glUniform2f(offsetLoc, 0.0f, -0.6f);
        glUniform1f(scaleLoc, 1.0f);
        drawVBO(tableVBO, table.size()/5, GL_TRIANGLES);

        // Table legs (2 only: left + right)
        glUniform2f(offsetLoc, -0.8f, -0.9f);
        drawVBO(legVBO, leg.size()/5, GL_TRIANGLES);
        glUniform2f(offsetLoc, 0.8f, -0.9f);
        drawVBO(legVBO, leg.size()/5, GL_TRIANGLES);

        // Blocks on table
        glUniform2f(offsetLoc, -0.4f, -0.4f);
        drawVBO(redVBO, 6, GL_TRIANGLES);
        glUniform2f(offsetLoc, 0.0f, -0.4f);
        drawVBO(greenVBO, 6, GL_TRIANGLES);
        glUniform2f(offsetLoc, 0.4f, -0.4f);
        drawVBO(blueVBO, 6, GL_TRIANGLES);

        // Circle on table
        glUniform2f(offsetLoc, -0.6f, -0.4f);
        drawVBO(circleVBO, 42, GL_TRIANGLE_FAN);

        // Triangle on table
        glUniform2f(offsetLoc, 0.6f, -0.4f);
        drawVBO(triangleVBO, 3, GL_TRIANGLES);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

