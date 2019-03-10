#include <iostream>
#include <cstring>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tinyrenderer/geometry.h>
#include "model.h"

bool animate = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    std::cerr << "key_callback(" << window << ", " << key << ", " << scancode << ", " << action << ", " << mode << ");" << std::endl;
    if (GLFW_RELEASE == action) {
        return;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        animate = !animate;
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

GLuint set_shaders(const std::string vsfile, const std::string fsfile) {
    GLuint vert_hdlr, frag_hdlr;
    read_n_compile_shader(vsfile, vert_hdlr, GL_VERTEX_SHADER);
    read_n_compile_shader(fsfile, frag_hdlr, GL_FRAGMENT_SHADER);

    std::cerr << "Linking shaders... ";
    GLuint prog_hdlr = glCreateProgram();
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
    return prog_hdlr;
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

Matrix rot_z(const float angle) {
    Matrix R = Matrix::identity();
    R[0][0] = R[2][2] = cos(angle);
    R[2][0] = sin(angle);
    R[0][2] = -R[2][0];
    return R;
}

template <size_t DimRows,size_t DimCols,class T> void export_row_major(mat<DimRows,DimCols,T> m, float *arr) {
    for (size_t i=DimRows; i--; )
        for (size_t j=DimCols; j--; arr[i+j*DimCols]=m[i][j]);
}


int main(int argc, char** argv) {
    std::cout << "Usage: " << argv[0] << " model.obj" << std::endl;
    std::string file_obj ("../lib/tinyrenderer/obj/diablo3_pose/diablo3_pose.obj");
    if (2==argc) {
        file_obj  = std::string(argv[1]);
    }
    Model model(file_obj.c_str());

    const GLuint width = 800, height = 800;
    GLFWwindow* window;
    if (setup_window(window, width, height)) {
        glfwTerminate();
        return -1;
    }

    GLuint prog_hdlr = set_shaders("../src/vertex.glsl", "../src/fragment.glsl");

    Matrix M = Matrix::identity();
    Matrix V = Matrix::identity();
    Matrix P = Matrix::identity();

    // Get handles to our uniforms
    GLuint LightID = glGetUniformLocation(prog_hdlr, "LightPosition_worldspace");
    GLuint MatrixID = glGetUniformLocation(prog_hdlr, "MVP");
    GLuint ViewMatrixID  = glGetUniformLocation(prog_hdlr, "V");
    GLuint ModelMatrixID = glGetUniformLocation(prog_hdlr, "M");

    std::vector<GLfloat>    vertices(3*3*model.nfaces(), 0);
    std::vector<GLfloat>     normals(3*3*model.nfaces(), 0);

    for (int i=0; i<model.nfaces(); i++) {
        for (int j=0; j<3; j++) {
            for (int k=0; k<3; k++) vertices[(i*3+j)*3 + k] = model.point(model.vert(i, j))[k];
            for (int k=0; k<3; k++)  normals[(i*3+j)*3 + k] = model.normal(i, j)[k];
        }
    }

    // create the VAO that we use when drawing
    GLuint vao = 0;
    glGenVertexArrays(1, &vao); // allocate and assign a Vertex Array Object to our handle
    glBindVertexArray(vao);     // bind our Vertex Array Object as the current used object

    glEnableVertexAttribArray(0);
    GLuint vertexbuffer = 0;
    glGenBuffers(1, &vertexbuffer);              // allocate and assign one Vertex Buffer Object to our handle
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); // bind our VBO as being the active buffer and storing vertex attributes (coordinates)
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*vertices.size(), vertices.data(), GL_STATIC_DRAW); // copy the vertex data to our buffer. The buffer contains sizeof(GLfloat) * 3 * nverts bytes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glEnableVertexAttribArray(1);
    GLuint normalbuffer = 0;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

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

        { // Send our transformation to the currently bound shader
            { // rotate the model around the z axis with each frame
                Matrix R = rot_z(0.01);
                if (animate) M = R*M;
            }

            float tmp[16] = {0};
            export_row_major(M, tmp);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, tmp);
            export_row_major(V, tmp);
            glUniformMatrix4fv(ViewMatrixID,  1, GL_FALSE, tmp);
            export_row_major(P*V*M, tmp);
            glUniformMatrix4fv(MatrixID,      1, GL_FALSE, tmp);
        }
        const float lightpos[3] = {40, 40, 40}; // the light position sent to the vertex shader
        glUniform3fv(LightID, 1, lightpos);

        // draw the triangles!
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate all the resources once they have outlived their purpose
    glUseProgram(0);
    glDeleteProgram(prog_hdlr); // note that the shader objects are automatically detached and deleted, since they were flagged for deletion by a previous call to glDeleteShader
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteVertexArrays(1, &vao);

    glfwTerminate();
    return 0;
}

