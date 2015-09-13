//
//  ParticleSystem.cpp
//  LogoProject
//
//  Created by Jennifer on 9/8/15.
//
//

#include "ParticleSystem.h"

ParticleSystemRef ParticleSystem::create(float &posArray)
{
    ParticleSystemRef ref(new ParticleSystem());
    ref->setup(posArray);
    return ref;
}

ParticleSystem::ParticleSystem()
{}

ParticleSystem::~ParticleSystem()
{
    std::cout << "Calling particle system destructor" << std::endl;
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mParticleBufferA);
    glDeleteBuffers(1, &mParticleBufferB);
}

void ParticleSystem::setup(float &posArray)
{
    //  create and bind the vao
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    
    mLastMousePos = ci::vec2(0.0f, 0.0f);
    
    //  array of values
    GLfloat positionData[logo::NUM_PARTICLES * 7]; // two slots for position, two for velocity, three for color
    //GLfloat newPositionData[mMaxNewPositions * 2];
    
    for (int i = 0; i < logo::NUM_PARTICLES; i++) {
        
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
    glGenBuffers(1, &mParticleBufferA);
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferA);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &mParticleBufferB);
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData, GL_STREAM_DRAW); // don't initialize immediately
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
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
    const GLchar * feedbackVaryings[3] = {"outPos", "outVel", "outCol"};
    glTransformFeedbackVaryings(mShaderProgram, 3, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glBindFragDataLocation(mShaderProgram, 0, "outColor");

    // link and activate the program
    glLinkProgram(mShaderProgram);
    glUseProgram(mShaderProgram);
    
    mPosAttrib = glGetAttribLocation(mShaderProgram, "inPos");
    mVelAttrib = glGetAttribLocation(mShaderProgram, "inVel");
    mColAttrib = glGetAttribLocation(mShaderProgram, "inCol");

    mMousePosUniform = glGetUniformLocation(mShaderProgram, "mousePos");
    //mNumNewPosUniform = glGetUniformLocation(mShaderProgram, "numNewPositions");
    mNewPosUniform = glGetUniformLocation(mShaderProgram, "newPositions");
    
    std::cout << "Max uniform locations: " << GL_MAX_UNIFORM_LOCATIONS << std::endl;
  
    std::cout << "mPosAttrib: " << mPosAttrib << std::endl;
    std::cout << "mVelAttrib: " << mVelAttrib << std::endl;
    std::cout << "mColAttrib: " << mColAttrib << std::endl;

    std::cout << "mMousePosUniform:" << mMousePosUniform << std::endl;
    //std::cout << "mNumNewPosUniform: " << mNumNewPosUniform << std::endl;
    std::cout << "mNewPosUniform: " << mNewPosUniform << std::endl;
    
    mPosArrayPointer = &posArray;
    
}

//
//
//
void ParticleSystem::update()
{
    
}

//******************************************
//  temp method for debugging
//******************************************
void ParticleSystem::updateMouse(ci::vec2 pos)
{
    //glLinkProgram(mShaderProgram);
    //glUseProgram(mShaderProgram);
    //mMousePosUniform = glGetUniformLocation(mShaderProgram, "mousePos");
    mLastMousePos = pos;

//    ci::vec2 normMousePos = normalizeMousePos(pos);
//    float mousePosArray[] = {normMousePos.x, normMousePos.y};
//    glUniform2fv(mMousePosUniform, 1, mousePosArray);
}

//******************************************
//  draw function; main updates happen here
//******************************************
void ParticleSystem::draw()
{
    //glLinkProgram(mShaderProgram);
    glUseProgram(mShaderProgram);
    
    //  make a copy of the array from the main app
    float testArray[logo::NUM_NEW_POSITIONS * 2];
    for (int i = 0; i < logo::NUM_NEW_POSITIONS * 2; i++) {
        testArray[i] = mPosArrayPointer[i];
    }
    
    //std::cout << "ParticleSystem: draw: print testArray, frame# " << ci::app::getElapsedFrames() << std::endl;
    //for (int i = 0; i < logo::NUM_NEW_POSITIONS; i++) {
    //    std::cout << "    " << i << ": (" << testArray[i*2] << ", " << testArray[i*2 + 1] << std::endl;
    //}
    
    //  pass mouse position
    //float mousePos[2] = {float(mLastMousePos.x), float(mLastMousePos.y)};
    //glUniform2fv(mMousePosUniform, 1, mousePos);
    
    //  pass in the array of new positions
    glUniform2fv(mNewPosUniform, logo::NUM_NEW_POSITIONS, testArray);
    
//    std::cout << "ParticleSystem::draw: first ten values of array: " << std::endl;
//    for (int i = 0; i < 20; i+=2) {
//        std::cout << "    x: " << mPosArrayPointer[i] << ", y: " << mPosArrayPointer[i+1] << std::endl;
//        float randomFloat = getRandomFloat(ci::vec2(mPosArrayPointer[i], mPosArrayPointer[i+1]));
//        std::cout << "    Random number generated from this: " << randomFloat << std::endl;
//        std::cout << "    And mapped: " << mapFloat(randomFloat, 0.0, 1.0, 0.0, 250.0) << std::endl;
//    }
    
    //  disable the rasterizer
    glEnable(GL_RASTERIZER_DISCARD);
    
    //  specify the source buffer
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferA);
    glEnableVertexAttribArray(mPosAttrib);
    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
    
    glEnableVertexAttribArray(mVelAttrib);
    glVertexAttribPointer(mVelAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glEnableVertexAttribArray(mColAttrib);
    glVertexAttribPointer(mColAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    
    
    //  specify target buffer
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBufferB);
    
    //  query; hope to debug
    //GLuint query;
    //glGenQueries(1, &query);
    //glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
    
    //  do transform feedback
    glBeginTransformFeedback(GL_POINTS);
    
    glDrawArrays(GL_POINTS, 0, logo::NUM_PARTICLES);
    glEndTransformFeedback();
    
    ////  query
    //glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
    //GLuint primitives;
    //glGetQueryObjectuiv(query, GL_QUERY_RESULT, &primitives);
    
    //std::cout << "ParticleSystem::draw: Number of primitives: Frame # " << ci::app::getElapsedFrames() << ", Primitives: " << primitives << std::endl;
    
    
    glDisableVertexAttribArray(mPosAttrib);
    glDisableVertexAttribArray(mVelAttrib);
    glDisableVertexAttribArray(mColAttrib);
    
    glFlush();
    
    //GLfloat feedback[logo::NUM_PARTICLES * 2];
    //glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, sizeof(feedback), feedback);
    //std::cout << "ParticleSystem::draw: Reading transform feedback info" << std::endl;
    //for (int i = 0; i < logo::NUM_PARTICLES; i++) {
    //    std::cout << "    " << i << ": (" << testArray[i*2] << ", " << testArray[i*2 + 1] << ")" << std::endl;
    //}
    
    //std::cout << "ParticleSystem::draw: Reading transform feedback info: WHOLE THING" << std::endl;
    //for (int i = 0; i < logo::NUM_PARTICLES; i++) {
    //    std::cout << "    " << i << ": (" << testArray[i*7] << ", " << testArray[i*7 + 1] << ", " << testArray[i*7 + 2] <<", " << testArray[i*7 + 3] <<", " << testArray[i*7 + 4] <<", " << testArray[i*7 + 5] <<", " << testArray[i*7 + 6] <<")" << std::endl;
    //}
    
    std::swap(mParticleBufferA, mParticleBufferB);
    
    //  draw the particles
    glDisable(GL_RASTERIZER_DISCARD);
    
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferA);
    glEnableVertexAttribArray(mPosAttrib);
    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
    glEnableVertexAttribArray(mVelAttrib);
    glVertexAttribPointer(mVelAttrib, 2, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(mColAttrib);
    glVertexAttribPointer(mColAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    
    glDrawArrays(GL_POINTS, 0, logo::NUM_PARTICLES);
    glDisableVertexAttribArray(mPosAttrib);
    glDisableVertexAttribArray(mVelAttrib);
    glDisableVertexAttribArray(mColAttrib);
}

//******************************************
//  load shader from plain text
//******************************************
std::string ParticleSystem::loadShaderSource(std::string path)
{
    std::ifstream in(ci::app::getAssetPath(path).string());
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    return contents;
}

//******************************************
// create shader and print out debugging info
//******************************************
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

//******************************************
// functions from shaders for testing
//******************************************

float ParticleSystem::getRandomFloat(ci::vec2 currentPos) {
    //return fract(sin(dot(currentPos.xy, ci::vec2(12.9898, 78.233))) * 43758.5453);
    float a = 12.9898;
    float b = 78.233;
    float c = 43758.5453;
    float dt = dot(ci::vec2(currentPos.x, currentPos.y), ci::vec2(a, b));
    //float sn = mod(dt, 3.14);
    float sn = dt - 3.14 * floor(dt/3.14);
    return (sin(sn) * c) - floor(sin(sn) * c);
}

float ParticleSystem::mapFloat(float value, float inputMin, float inputMax, float outputMin, float outputMax)
{
    const float Epsilon = 0.0000001;
    if (fabs(inputMin - inputMax) < Epsilon) {
        return outputMin;
    }
    
    float outVal = ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
    // clamp result
    if (outVal > outputMax) {
        outVal = outputMax;
    }
    if (outVal < outputMin) {
        outVal = outputMin;
    }
    return outVal;
}

//ci::vec2 ParticleSystem::normalizeMousePos(ci::ivec2 pos)
//{
//    ci::vec2    normPos;
//    float normX, normY;
//    normX = (float)pos.x / (float)ci::app::getWindowWidth();
//    normX *= 2.0f;
//    normX -= 1.0f;
//
//    normY = (float)pos.y / (float)ci::app::getWindowHeight();
//    normY *= 2.0f;
//    normY -= 1.0f;
//    normY *= -1.0f;
//
//    normX = glm::clamp(normX, -1.0f, 1.0f);
//    normY = glm::clamp(normY, -1.0f, 1.0f);
//    
//    normPos.x = normX;
//    normPos.y = normY;
//    return normPos;
//}
