#include <fstream>
#include <istream>
#include <iostream>
#include <string>
#include <csignal>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "fasta_lib/renderer.h"
#include "fasta_lib/renderer_ipr.h"
#include "fasta_lib/shader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/examples/imgui_impl_glfw.h"
#include "imgui/examples/imgui_impl_opengl3.h"

#include "linmath.h/linmath.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include "fasta_utils_lib/logging.h"

namespace logging = boost::log;

// ---------

#include "fsquad.h"

#define win_width 1200
#define win_height 800

using namespace fst;

// some globals
std::unique_ptr<FstRendererIPR> fasta_ipr;
bool show_debug_window = false; // imgui

void print_usage() {
    std::cout << "Usage: fasta [options] [ifd] [outputimage]\n" << std::endl;

    std::cout << "Reads an scene from standard input and renders the image described.\n" << std::endl;

    std::cout << "If the first argument after options ends in .ifd, .ifd.gz or .ifd.sc, xenon" << std::endl;
    std::cout << "will read the scene description from that file.  If the argument does not" << std::endl;
    std::cout << "have an extension it will be used as the output image/device\n" << std::endl;

    std::cout << "    Control Options:" << std::endl;
    std::cout << "        -f file  Read scene file specified instead of reading from stdin" << std::endl;
    std::cout << "        -I  Run interactive window" << std::endl;
    std::cout << "        -V val  Set verbose level (i.e. -V 2)" << std::endl;
    std::cout << "                    0-6  Output varying degrees of rendering statistics" << std::endl;
    std::cout << "        -l logfile  Write log to file" << std::endl;
    std::cout << "        -C      Enable stdin compatibility mode." << std::endl;
}

void signalHandler( int signum ){
    LOG_DBG << "Interrupt signal (" << signum << ") received !";

    // cleanup and close up stuff here
    // terminate program
    exit(signum);
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch(key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_P:
                fasta_ipr->togglePause();
                break;
            case GLFW_KEY_D:
                show_debug_window = !show_debug_window;
                break;
        }
    }
}

int main(int argc, char* argv[]){
    
    // Set up logging level quick 
    logging::core::get()->set_filter( logging::trivial::severity >= logging::trivial::debug );

    int option = 0;
    int verbose_level = 6;
    bool read_stdin = false;
    bool run_interactive = false;

    std::istream *in; // input stream pointer
    std::ifstream ifn; // input file

    signal(SIGTERM, signalHandler);
    signal(SIGABRT, signalHandler);

    /// Specifying the expected rendering options
    while ((option = getopt(argc, argv, "hCIV:L:")) != -1) {
        switch (option) {
            case 'C' : read_stdin = true;
                break;
            case 'V' : verbose_level = atoi(optarg);
                break; 
            case 'I' : run_interactive = true;    
                break; 
            case 'L': 
            case 'h' :
            default: print_usage(); 
                exit(EXIT_FAILURE);
        }
    }

    /// Handle renderer input 
    if(read_stdin){
        LOG_DBG << "Reading commands from stdin:";
        in = &std::cin;
    }else{
        LOG_DBG << "Reading commands from file: " << argv[argc-1];
        ifn.open(argv[argc-1]);
        if(!ifn.good()){
            LOG_DBG << "Unable to open file: " << argv[argc-1];
            exit(EXIT_FAILURE);
        }
        in = &ifn;
    }

    /// Run interactive debug IPR
    if (run_interactive) {
        LOG_DBG << "Fasta debug IPR starting...";

        fasta_ipr = std::make_unique<FstRendererIPR>();

        GLuint vertex_buffer, vertex_shader, fragment_shader, program;
        GLint mvp_location, vpos_location, vcol_location;

        glfwSetErrorCallback(glfw_error_callback);

        if (!glfwInit())
            exit(EXIT_FAILURE);

        const char* glsl_version = "#version 330";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        GLFWwindow* window = glfwCreateWindow(win_width, win_height, "Fasta IPR debug", NULL, NULL);
        if (!window) {
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwSetKeyCallback(window, glfw_key_callback);

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        // Load GLAD
        int gladInitRes = gladLoadGL();
        if (!gladInitRes) {
            fprintf(stderr, "Unable to initialize glad\n");
            glfwDestroyWindow(window);
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // fsquad setup being
        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
     
        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
        glCompileShader(vertex_shader);

        GLint shaderCompiled;
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &shaderCompiled);
        if(shaderCompiled == GL_FALSE)  {
            std::cerr << "vertex shader compile error: " << FstShader::getShaderLog(vertex_shader) << std::endl;
            }
     
        fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &shaderCompiled);
        if(shaderCompiled == GL_FALSE)  {
            std::cerr << "fragment shader compile error: " << FstShader::getShaderLog(fragment_shader) << std::endl;
            }
     
        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
     
        mvp_location = glGetUniformLocation(program, "MVP");
        vpos_location = glGetAttribLocation(program, "vPos");
        vcol_location = glGetAttribLocation(program, "vCol");
     
        glEnableVertexAttribArray(vpos_location);
        glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                              sizeof(vertices[0]), (void*) 0);
        glEnableVertexAttribArray(vcol_location);
        glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                              sizeof(vertices[0]), (void*) (sizeof(float) * 2));
        // fsquad setup end

        int width = win_width, height = win_height;
        float ratio = width / (float) height;

        mat4x4 m, p, mvp;

        // run ipr renderer
        fasta_ipr->run(win_width, win_height, 16);

        while (!glfwWindowShouldClose(window)) {
            glfwGetFramebufferSize(window, &width, &height);
            fasta_ipr->resize(width, height);
            ratio = width / (float) height;

            glViewport(0, 0, width, height);
            glClearColor(0.15f, 0.6f, 0.4f, 1.0f); // nice green
            glClear(GL_COLOR_BUFFER_BIT);

            // fsquad begin
            mat4x4_identity(m);
            mat4x4_rotate_Z(m, m, (float) glfwGetTime());
            mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
            mat4x4_mul(mvp, p, m);
     
            glUseProgram(program);
            //glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // fsquad end

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (show_debug_window) {
                ImGui::Begin("Debug window", &show_debug_window);
                ImGui::Text("This is some useful text.");
                if (ImGui::Button("Close"))
                    show_debug_window = false;
                ImGui::End();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();

        exit(EXIT_SUCCESS);
    }

    // Normal render process
    std::unique_ptr<FstRenderer> renderer = std::make_unique<FstRenderer>();
    renderer->init();

    /*
    FstIfd ifd_reader(in, renderer);
    if(!ifd_reader.process()){
        std::cerr << "Abort. Error processing IFD !!!" << std::endl;
        delete renderer;
        exit(EXIT_FAILURE);
    }
    */

    return 0;
}