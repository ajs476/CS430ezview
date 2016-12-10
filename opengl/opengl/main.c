//
//  main.c
//  opengl
//
//  Created by Alexander Sears on 12/9/16.
//  Copyright © 2016 Alexander Sears. All rights reserved.
//

#include <GL/glew.h>
#include <GLFW/glfw3.h>


int main(int argc, const char * argv[]) {
    
    GLFWwindow *Window;
    
    if (!glfwInit()){
        return -1;
    }
    
    Window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    
    if(!Window){
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(Window);
    
    while (!glfwWindowShouldClose(Window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers(Window);
        
        glfwPollEvents();
        
    }
    
    glfwTerminate();
    return 0;
}
