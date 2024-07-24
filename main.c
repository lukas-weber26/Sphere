#include <assert.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define ball_radious 0.04

const char * vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    //"uniform vec2 transform;\n" 
    "void main()\n"
    "{\n"
    "gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    //"gl_Position = -vec4(transform, 0.0, 0.0) + vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char * fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "FragColor = vec4(1.0,0.5,0.2,1.0);\n"
    "}\0";

void checkLink(unsigned int shaderProgram);
void checkSuccess(unsigned int vertexShader);
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void processInput(GLFWwindow * window);

float random_float () {
    return (float) rand() / (float)(RAND_MAX/0.5);
}

int main()
{

    //these seem like pretty normal c library things! 
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //fortunately glew does not require too much crap

    GLFWwindow * window = glfwCreateWindow(800, 800, "First Window", NULL, NULL);
    if (window == NULL) {
	printf("GLFW Window fail.");
	glfwTerminate();
	exit(1);
    }

    //binds gl functions to the current window
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwPollEvents(); //this seems to be an issue with wayland 
    
    glViewport(0, 0, 800, 800);
    
    //YEAH TURNS OUT THE POSITION OF THIS MATTERS A LOT. HAS TO BE AFTER THE WINDOW HAS BEEN CREATED!
    glewExperimental = GL_TRUE; 
    glewInit();
    
    int verts_per_level = 102;
    int attribs_per_vert = 3;
    int number_of_levels = 100;
    float delta_h = (float)2.0/number_of_levels;
    float vertices[verts_per_level*attribs_per_vert*number_of_levels];
    float max_radious = 1;

    for (int j = 0; j < number_of_levels; j++) {
	//note, first and last must repeat
	float height1 = 1-j*delta_h;
	float height2 = height1-delta_h;
	
	printf("P: %f\n", height2);

	float r1 = sqrt(1-height1*height1);
	float r2;

	if (height2 < -0.99) {
	    r2 = 0;
	} else {
	    r2 = sqrt(1-height2*height2);
	}

	for (int i = 0; i <= verts_per_level; i+= 2){		
	    float angle = 2*M_PI*((float)(i/100.0));

	    //upper
	    int offset = j*verts_per_level*attribs_per_vert;
	    vertices[offset + 3*i + 0] = r1*cos(angle);
	    vertices[offset +3*i + 1] = height1;
	    vertices[offset +3*i + 2] = r1*sin(angle);

	    //lower
	    vertices[offset +3*i + 3] = r2*cos(angle);
	    vertices[offset +3*i + 4] = height2;
	    vertices[offset +3*i + 5] = r2*sin(angle);
	}
    }

    //tell gl to generate vertex arrays and bind them
    unsigned int VAO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);
     
    unsigned int VBO;
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO); //you have to bind buffers to make the following gl operations affect said buffer
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices), vertices, GL_STATIC_DRAW); //note that GL_STATIC_DRAW means that this is not expected to change much
   
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER); 
    glShaderSource(vertexShader,1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkSuccess(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkSuccess(fragmentShader);

    unsigned int shaderProgram; 
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkLink(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    //final step here: Make the gpu interpret the shader program in a meaningful way!
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glEnable(GL_MULTISAMPLE);
    
    //int transform_loc = glGetUniformLocation(shaderProgram,"transform");
    
    while(!glfwWindowShouldClose(window)) {
	processInput(window);
	
	glClearColor(0.14f, 0.14f, 0.14f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	
	//glUniform2f(transform_loc,balls[i].x, balls[i].y);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, verts_per_level*number_of_levels);
	
	glfwSwapBuffers(window);
	glfwPollEvents(); //this seems to be an issue with wayland 

    }
    
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow * window, int width, int height) { 
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow * window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);	
    }
}

void checkSuccess(unsigned int vertexShader) {
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success) {
	char infoLog[512];
	glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
	printf("%s\n",infoLog);
	exit(0);
    }
}

void checkLink(unsigned int shaderProgram) {
    int success;
    glGetProgramiv(shaderProgram,GL_LINK_STATUS, &success);
    if (!success) {
	char out[512];
	glGetProgramInfoLog(shaderProgram,512,NULL,out);
	printf("%s",out);
	exit(0);
    }
}
