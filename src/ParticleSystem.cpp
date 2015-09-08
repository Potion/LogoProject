//
//  ParticleSystem.cpp
//  LogoProject
//
//  Created by Jennifer on 9/8/15.
//
//

#include "ParticleSystem.h"

ParticleSystemRef ParticleSystem::create()
{
    ParticleSystemRef ref(new ParticleSystem());
    ref->setup();
    return ref;
}

ParticleSystem::ParticleSystem()
{}

ParticleSystem::~ParticleSystem()
{
    std::cout << "Calling particle system destructor" << std::endl;
    glDeleteVertexArrays(1, &mVAO);
    //glDeleteBuffers(2, mParticleBufferIDs);
    //glDeleteTransformFeedbacks(1, mTransformFeedbackIDs);
    
}

void ParticleSystem::setup()
{
    //  create and bind the vao
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    
    //  array of values
    mParticleCount = 10000;
    GLfloat positionData[mParticleCount * 7]; // two slots for position, two for velocity, three for color
    
    //  generate random values
    unsigned int time_ui = (unsigned int)( time(NULL) );
    srand(time_ui);
    
    for (int i = 0; i < mParticleCount; i++) {
        
        //  position is completely random
        float randNum = (float)(rand()) / (float)(RAND_MAX);
        randNum *= 2.0;
        randNum -= 1.0;
        float randNum2 = (float)(rand()) / (float)(RAND_MAX);
        randNum2 *= 2.0;
        randNum2 -= 1.0;
        
        //  velocity is normalized vector
        float randNum3 = (float)(rand()) / (float)(RAND_MAX);
        randNum3 *= M_PI * 2.0f;
        
        positionData[(i*7) + 0] = randNum;              // pos.x
        positionData[(i*7) + 1] = randNum2;             // pos.y
        
        positionData[(i*7) + 2] = cos(randNum3) * 0.03; // vel.x
        positionData[(i*7) + 3] = sin(randNum3) * 0.03; // vel.y
        
        positionData[(i*7) + 4] = (float)(rand()) / (float)(RAND_MAX);  // col.r
        positionData[(i*7) + 5] = (float)(rand()) / (float)(RAND_MAX);  // col.g
        positionData[(i*7) + 6] = (float)(rand()) / (float)(RAND_MAX);  // col.b
    }
    
    std::cout << "Sizeof positionData: " << sizeof(positionData) << std::endl;
    
    //  create two buffers to ping-pong back and forth with position data
    glGenBuffers(1, &mPositionBufferA);
    glBindBuffer(GL_ARRAY_BUFFER, mPositionBufferA);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &mPositionBufferB);
    glBindBuffer(GL_ARRAY_BUFFER, mPositionBufferB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), 0, GL_STREAM_DRAW); // don't initialize immediately
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    //  create shader program
//    std::cout << "Creating shaders: ..." << std::endl;
//    const char * vertexShaderSrc = loadShaderData("vertex.glsl");
//    const char * fragShaderSrc = loadShaderData("frag.glsl");

    std::string vertexShaderSrcString = loadShaderSource("vertex.glsl");
    std::string fragmentShaderSrcString = loadShaderSource("frag.glsl");
    
    const char * vertexShaderSrc = vertexShaderSrcString.c_str();
    const char * fragShaderSrc = fragmentShaderSrcString.c_str();
    
    //std::cout << "*********Create the vertex shader" << std::endl;
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSrc);
    //std::cout << "*********Create the fragment shader" << std::endl;
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragShaderSrc);
    mShaderProgram = glCreateProgram();
    
    std::cout << "vertexShader: " << vertexShader << std::endl;
    std::cout << "fragmentShader: " << fragmentShader << std::endl;
    std::cout << "mShaderProgram: " << mShaderProgram << std::endl;
    glAttachShader(mShaderProgram, vertexShader);
    glAttachShader(mShaderProgram, fragmentShader);

    // before linking program, specify which output attributes we want to capture into a buffer
    const GLchar * feedbackVaryings[] = {"outPos", "outVel", "outCol"};
    glTransformFeedbackVaryings(mShaderProgram, 3, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glBindFragDataLocation(mShaderProgram, 0, "outColor");

    // now link and activate the program
    glLinkProgram(mShaderProgram);
    glUseProgram(mShaderProgram);
    
    mPosAttrib = glGetAttribLocation(mShaderProgram, "inPos");
    mVelAttrib = glGetAttribLocation(mShaderProgram, "inVel");
    mColAttrib = glGetAttribLocation(mShaderProgram, "inCol");
    mMousePosUniform = glGetUniformLocation(mShaderProgram, "mousePos");
  
    std::cout << "mPosAttrib: " << mPosAttrib << std::endl;
    std::cout << "mVelAttrib: " << mVelAttrib << std::endl;
    std::cout << "mColAttrib: " << mColAttrib << std::endl;
    std::cout << "mMousePosUniform:" << mMousePosUniform << std::endl;
}

void ParticleSystem::update()
{
    
}

void ParticleSystem::draw()
{
    ci::gl::clear(ci::Color(0, 0, 0));
    glClear(GL_COLOR_BUFFER_BIT);
    
    //  disable the rasterizer
    glEnable(GL_RASTERIZER_DISCARD);
    
    //  specify the source buffer
    glBindBuffer(GL_ARRAY_BUFFER, mPositionBufferA);
    glEnableVertexAttribArray(mPosAttrib);
    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
    
    glEnableVertexAttribArray(mVelAttrib);
    glVertexAttribPointer(mVelAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glEnableVertexAttribArray(mColAttrib);
    glVertexAttribPointer(mColAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    
    
    //  specify target buffer
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mPositionBufferB);
    
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, mParticleCount);
    glEndTransformFeedback();
    glDisableVertexAttribArray(mPosAttrib);
    glDisableVertexAttribArray(mVelAttrib);
    glDisableVertexAttribArray(mColAttrib);
    
    glFlush();
    
    std::swap(mPositionBufferA, mPositionBufferB);
    
    glDisable(GL_RASTERIZER_DISCARD);
    
    //  get the mouse position
//    double mouseX;
//    double mouseY;
//    glfwGetCursorPos(window, &mouseX, &mouseY);
    
    //  normalize the mouse position
//    glm::vec2 normMousePos = normalizeMousePos((float)(mouseX), (float)(mouseY));
//    
//    float mousePosArray[] = {normMousePos.x, normMousePos.y};
    float mousePosArray[] = {0.0f, 0.0f};
    glUniform2fv(mMousePosUniform, 1, mousePosArray);
    
    glBindBuffer(GL_ARRAY_BUFFER, mPositionBufferA);
    glEnableVertexAttribArray(mPosAttrib);
    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
    glEnableVertexAttribArray(mVelAttrib);
    glVertexAttribPointer(mVelAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(mColAttrib);
    glVertexAttribPointer(mColAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    
    
    glDrawArrays(GL_POINTS, 0, mParticleCount);
    glDisableVertexAttribArray(mPosAttrib);
    glDisableVertexAttribArray(mVelAttrib);
    glDisableVertexAttribArray(mColAttrib);
    
}

//  loads data as char * from outside source
//  note this function does not load properly
const GLchar * ParticleSystem::loadShaderData(std::string path)
{
    std::cout << "ParticleSystem::loadShaderData: " << path << std::endl;
    std::string shaderCode;
    std::ifstream shaderStream(ci::app::getAssetPath(path).string(), std::ios::in);
    if (shaderStream.is_open()) {
        std::string Line = "";
        while (getline(shaderStream, Line)) {
            shaderCode += "\n" + Line;
        }
        shaderStream.close();
        const char * code = shaderCode.c_str();
        //std::cout << code << std::endl;
        return code;
    }
    else {
        std::cout << "Error loading shader: " << path << std::endl;
        getchar();
        return 0;
    }
}

std::string ParticleSystem::loadShaderSource(std::string path)
{
    std::ifstream in(ci::app::getAssetPath(path).string());
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    return contents;
}

// create the shader and prints out debugging info
GLuint ParticleSystem::createShader(GLenum type, const GLchar* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    //std::cout << src << std::endl;
    glCompileShader(shader);
    
    // debug shader
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    char buffer[512];
    std::cout << "Shader status: (0 bad, 1 good): " << status << std::endl;
    if (status == 0) {
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        std::cout << "Shader compile log: " << buffer << std::endl;
    }
    
    return shader;
}
