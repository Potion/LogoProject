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
    glDeleteBuffers(1, &mPositionBufferA);
    glDeleteBuffers(1, &mPositionBufferB);
    //glDeleteBuffers(2, mParticleBufferIDs);
    //glDeleteTransformFeedbacks(1, mTransformFeedbackIDs);
    
}

void ParticleSystem::setup()
{
    //  create and bind the vao
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    
    mLastMousePos = ci::vec2(0.0f, 0.0f);
    
    //  array of values
    mParticleCount = 100000;
    GLfloat positionData[mParticleCount * 7]; // two slots for position, two for velocity, three for color
    
    for (int i = 0; i < mParticleCount; i++) {
        
        //  position is completely random
        float randNum = ci::Rand::randFloat(-1.0f, 1.0f);
        float randNum2 = ci::Rand::randFloat(-1.0f, 1.0f);
        //  velocity is normalized vector
        float randNum3 = ci::Rand::randFloat(0.0f, M_PI * 2.0f);
        
        positionData[(i*7) + 0] = randNum;              // pos.x
        positionData[(i*7) + 1] = randNum2;             // pos.y
        
        positionData[(i*7) + 2] = cos(randNum3) * 0.03; // vel.x
        positionData[(i*7) + 3] = sin(randNum3) * 0.03; // vel.y
        
        positionData[(i*7) + 4] = ci::Rand::randFloat();  // col.r
        positionData[(i*7) + 5] = ci::Rand::randFloat();  // col.g
        positionData[(i*7) + 6] = ci::Rand::randFloat();  // col.b
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


    //  load the external files
    std::string vertexShaderSrcString = loadShaderSource("vertex.glsl");
    std::string fragmentShaderSrcString = loadShaderSource("frag.glsl");
    
    //  convert to const char *
    const char * vertexShaderSrc = vertexShaderSrcString.c_str();
    const char * fragShaderSrc = fragmentShaderSrcString.c_str();
    
    //  create the shader program
    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSrc);
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

void ParticleSystem::updateMouse(ci::ivec2 pos)
{
    glLinkProgram(mShaderProgram);
    glUseProgram(mShaderProgram);
    mMousePosUniform = glGetUniformLocation(mShaderProgram, "mousePos");
//    std::cout << "ParticleSystem::updateMouse: mMousePositionUniform: " << mMousePosUniform << std::endl;
    mLastMousePos = pos;

//    ci::vec2 normMousePos = normalizeMousePos(pos);
//    float mousePosArray[] = {normMousePos.x, normMousePos.y};
//    glUniform2fv(mMousePosUniform, 1, mousePosArray);
}

void ParticleSystem::draw()
{
    glLinkProgram(mShaderProgram);
    glUseProgram(mShaderProgram);
    ci::vec2 normMousePos = normalizeMousePos(mLastMousePos);
    float mousePosArray[] = {normMousePos.x, normMousePos.y};
    glUniform2fv(mMousePosUniform, 1, mousePosArray);

    //ci::gl::clear(ci::Color(0, 0, 0));
    //glClear(GL_COLOR_BUFFER_BIT);
    
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

ci::vec2 ParticleSystem::normalizeMousePos(ci::ivec2 pos)
{
    ci::vec2    normPos;
    float normX, normY;
    normX = (float)pos.x / (float)ci::app::getWindowWidth();
    normX *= 2.0f;
    normX -= 1.0f;

    normY = (float)pos.y / (float)ci::app::getWindowHeight();
    normY *= 2.0f;
    normY -= 1.0f;
    normY *= -1.0f;

    normX = glm::clamp(normX, -1.0f, 1.0f);
    normY = glm::clamp(normY, -1.0f, 1.0f);
    
    normPos.x = normX;
    normPos.y = normY;
    return normPos;
}
