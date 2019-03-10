#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    std::cerr << "key_callback(" << window << ", " << key << ", " << scancode << ", " << action << ", " << mode << ");" << std::endl;
    if (GLFW_RELEASE == action) {
        return;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void read_n_compile_shader(const std::string filename, GLuint &hdlr, GLenum shaderType) {
    std::cerr << "Loading shader file " << filename << "... ";
    std::ifstream is(filename, std::ios::in|std::ios::binary|std::ios::ate);
    if (!is.is_open()) {
        std::cerr << "failed" << std::endl;
        return;
    }
    std::cerr << "ok" << std::endl;

    long size = is.tellg();
    char *buffer = new char[size+1];
    is.seekg(0, std::ios::beg);
    is.read(buffer, size);
    is.close();
    buffer[size] = 0;

    std::cerr << "Compiling the shader " << filename << "... ";
    hdlr = glCreateShader(shaderType);
    glShaderSource(hdlr, 1, (const GLchar**)&buffer, NULL);
    glCompileShader(hdlr);
    GLint success;
    glGetShaderiv(hdlr, GL_COMPILE_STATUS, &success);
    std::cerr << (success ? "ok" : "failed") << std::endl;

    GLint log_length;
    glGetShaderiv(hdlr, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length>0) {
        std::vector<char> v(log_length, 0);
        glGetShaderInfoLog(hdlr, log_length, NULL, v.data());
        if (strlen(v.data())>0) {
            std::cerr << v.data() << std::endl;
        }
    }

    delete [] buffer;
}

void set_shaders(GLuint &prog_hdlr, const std::string vsfile, const std::string fsfile) {
    GLuint vert_hdlr, frag_hdlr;
    read_n_compile_shader(vsfile, vert_hdlr, GL_VERTEX_SHADER);
    read_n_compile_shader(fsfile, frag_hdlr, GL_FRAGMENT_SHADER);

    std::cerr << "Linking shaders... ";
    prog_hdlr = glCreateProgram();
    glAttachShader(prog_hdlr, vert_hdlr);
    glAttachShader(prog_hdlr, frag_hdlr);
    glDeleteShader(vert_hdlr);
    glDeleteShader(frag_hdlr);
    glLinkProgram(prog_hdlr);

    GLint success;
    glGetProgramiv(prog_hdlr, GL_LINK_STATUS, &success);
    std::cerr << (success ? "ok" : "failed") << std::endl;

    GLint log_length;
    glGetProgramiv(prog_hdlr, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length>0) {
        std::vector<char> v(log_length);
        glGetProgramInfoLog(prog_hdlr, log_length, NULL, v.data());
        if (strlen(v.data())>0) {
            std::cerr << v.data() << std::endl;
        }
    }
}

int setup_window(GLFWwindow* &window, const GLuint width, const GLuint height) {
    std::cerr << "Starting GLFW context, OpenGL 3.3" << std::endl;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(width, height, "OpenGL starter pack", NULL, NULL);
    glfwMakeContextCurrent(window);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        return -1;
    }

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize OpenGL context" << std::endl;
        return -1;
    }

    return 0;
}

int main(int argc, char** argv) {
    const GLuint width = 800, height = 800;
    GLFWwindow* window;
    if (setup_window(window, width, height)) {
        glfwTerminate();
        return -1;
    }

    GLuint prog_hdlr;
    set_shaders(prog_hdlr, "../src/vertex.glsl", "../src/fragment.glsl");

    glViewport(0, 0, width, height);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(0);
    glDepthFunc(GL_GREATER); // accept fragment if it is closer to the camera than the former one
    glUseProgram(prog_hdlr); // specify the shaders to use

    auto start = std::chrono::steady_clock::now();
    while (!glfwWindowShouldClose(window)) {
        { // 20ms sleep to reach 50 fps, do something useful instead of sleeping
            auto end = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() < 20) { 
                std::this_thread::sleep_for(std::chrono::milliseconds(3));
                continue;
            }
            start = end;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // wipe the screen buffers

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate all the resources once they have outlived their purpose
    glUseProgram(0);
    glDeleteProgram(prog_hdlr); // note that the shader objects are automatically detached and deleted, since they were flagged for deletion by a previous call to glDeleteShader

    glfwTerminate();
    return 0;
}

