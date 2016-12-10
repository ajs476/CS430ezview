#include <OpenGL/gl.h>
#include <GLFW/glfw3.h>

//#include "glfw3.h"
#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>



//intialize variables
FILE *fp;
FILE *fpOut;
int tempChar;
int i;
int imageSize;
int imageDimensionX;
int imageDimensionY;
int imageDimensionColor;
char* magicNumber;


float rotation = 0;
float shearX = 0;
float shearY = 0;
float panX = 0;
float panY = 0;
float scale = 1;


typedef struct {
    float Position[2];
    float TexCoord[2];
} Vertex;

// (-1, 1)  (1, 1)
// (-1, -1) (1, -1)

Vertex vertexes[] = {
    {{1, -1}, {0.99999, 0.99999}},
    {{1, 1},  {0.99999, 0}},
    {{-1, 1}, {0, 0}},
    {{-1, 1}, {0, 0}},
    {{-1, -1},  {0, 0.99999}},
    {{1, -1}, {0.99999, 0.99999}}
};

static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec2 TexCoordIn;\n"
"attribute vec2 vPos;\n"
"varying vec2 TexCoordOut;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    TexCoordOut = TexCoordIn;\n"
"}\n";

static const char* fragment_shader_text =
"varying vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        scale += 0.3;
    
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        rotation += 90*3.14/180;
    
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        scale -= 0.3;
    
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        rotation -= 90*3.14/180;
    
    if (key == GLFW_KEY_S && action == GLFW_PRESS)
        shearX += 0.3;
    
    if (key == GLFW_KEY_A && action == GLFW_PRESS)
        shearX -= 0.3;
    
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
        shearY += 0.3;
    
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        shearY -= 0.3;
    
    if (key == GLFW_KEY_L && action == GLFW_PRESS)
        panX += 0.3;
    
    if (key == GLFW_KEY_K && action == GLFW_PRESS)
        panX -= 0.3;
    
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        panY += 0.3;
    
    if (key == GLFW_KEY_I && action == GLFW_PRESS)
        panY -= 0.3;
    
    
    
}

void glCompileShaderOrDie(GLuint shader) {
    GLint compiled;
    glCompileShader(shader);
    glGetShaderiv(shader,
                  GL_COMPILE_STATUS,
                  &compiled);
    if (!compiled) {
        GLint infoLen = 0;
        glGetShaderiv(shader,
                      GL_INFO_LOG_LENGTH,
                      &infoLen);
        char* info = malloc(infoLen+1);
        GLint done;
        glGetShaderInfoLog(shader, infoLen, &done, info);
        printf("Unable to compile shader: %s\n", info);
        exit(1);
    }
}



// IMAGE READER STUFF ////////////////////////

// create pixel struct
typedef struct Pixel{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} Pixel;

// function skips through comments in ppm files
void skipComments(){
    if(tempChar == '#'){
        // comment exists, lets get to the end of it
        //printf("Comment found.. \n");
        while(tempChar != '\n'){
            tempChar = fgetc(fp);
        }
        // is the next line another comment?
        tempChar = fgetc(fp);
        skipComments();
    }
    else{
        // get image dimensions
        //printf("End of comments.. \n");
        // reset our file pointer to the correct position
        fseek(fp, -2, SEEK_CUR);
    }
    
}

// function gets dimensions of image given
void getDimensions(){
    // get image width
    fscanf(fp, "%d",  &imageDimensionX);
    //printf("Image Dimensions X: %d\n", imageDimensionX);
    
    // get image height
    fscanf(fp, "%d\n",  &imageDimensionY);
    //printf("Image Dimensions Y: %d\n", imageDimensionY);
}

// function gets color range of image given
void getColorRange(){
    // get image color range
    fscanf(fp, "%d\n",  &imageDimensionColor);
    //printf("Image Color Range: %d\n", imageDimensionColor);
    if(imageDimensionColor > 255){
        fprintf(stderr, "Error: Image color not 8 bit per channel");
    }
}

// function checks magic number of image given
void getFileType(){
    magicNumber = malloc(sizeof(char)*2);
    char* tempBuffer = malloc(sizeof(char)*64);
    fscanf(fp, "%s\n",  tempBuffer);
    //printf("Hello");
    strcpy(magicNumber, tempBuffer);
    //printf("Magic Number: %s\n", magicNumber);
    tempChar = fgetc(fp);
}

// function calls helper functions that gather all relevant header info from image given
void getHeaderInfo(){
    
    getFileType();
    skipComments();
    getDimensions();
    getColorRange();	
}

/////////////////////////////////////////

int main(int argc, char **argv)
{
    
    //printf("Hello");
    
    // open up our image fo reading
    fp = fopen(argv[1], "rb");
    
    // make sure file opened exists
    if(fp == NULL ){
        fprintf(stderr, "Error reading file... \n");
        return 1;
    }
    
    
    
    //get size of image
    fseek(fp, 0L, SEEK_END);
    imageSize = ftell(fp);
    rewind(fp);

        // read header info from file
    getHeaderInfo();
    

    // create char array to store file contents into
//    char *imageBuffer = malloc(imageSize + 1);
//    int i = 0;
//    while(!feof(fp)){
//        imageBuffer[i] = fgetc(fp);
//        i++;
//    }
//    imageBuffer[i] = 0;
    
    int imageDataSize = sizeof(Pixel)*imageDimensionX*imageDimensionY;
    int count = 0;
    //fpOut = fopen(argv[3], "wb+");
    Pixel myPixel;
    Pixel *image = malloc(imageDataSize);
    
    
    // check magic number of file to determine conversion method
    if(magicNumber[0] == 80 && magicNumber[1] == 51){
            while(!feof(fp)){
                fscanf(fp, "%hhu ", &myPixel.red);
                fscanf(fp, "%hhu ", &myPixel.green);
                fscanf(fp, "%hhu ", &myPixel.blue);
                image[count] = myPixel;
                count++;
            }
        }
    
    else if(magicNumber[0] == 80 && magicNumber[1] == 54){
            while(!feof(fp)){
                fread(&myPixel.red, 1, 1, fp);
                fread(&myPixel.green, 1, 1, fp);
                fread(&myPixel.blue, 1, 1, fp);
                image[count] = myPixel;
                count++;
            }
        }
    else{
        printf("ERROR");
        return -1;
    }

    
    
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;
    
    glfwSetErrorCallback(error_callback);
    
    if (!glfwInit())
        exit(EXIT_FAILURE);
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    
    glfwSetKeyCallback(window, key_callback);
    
    glfwMakeContextCurrent(window);
    // gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);
    
    // NOTE: OpenGL error checks have been omitted for brevity
    
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
    
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShaderOrDie(vertex_shader);
    
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShaderOrDie(fragment_shader);
    
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    // more error checking! glLinkProgramOrDie!
    
    mvp_location = glGetUniformLocation(program, "MVP");
    assert(mvp_location != -1);
    
    vpos_location = glGetAttribLocation(program, "vPos");
    assert(vpos_location != -1);
    
    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);
    
    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);
    
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          (void*) 0);
    
    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location,
                          2,
                          GL_FLOAT,
                          GL_FALSE,
                          sizeof(Vertex),
                          (void*) (sizeof(float) * 2));
    
    int image_width = imageDimensionX;
    int image_height = imageDimensionY;
    
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, image);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);
    
    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;
        
        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
        
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        mat4x4 s = {
            scale, 0.0f, 0.0f, 0.0f,
            0.0f, scale, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        
        mat4x4 shear = {
            1.0f, shearY, 0.0f, 0.0f,
            shearX, 1.0, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        
        // rotation
        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, rotation);
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        
        // translate
        mat4x4_translate_in_place(m, panX, panY, 1.0);
        
        // scale
        mat4x4_mul(m, s, m);
        
        // shear
        mat4x4_mul(m, shear, m);
        
        // do stuff
        mat4x4_mul(mvp, p, m);
        
        
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
