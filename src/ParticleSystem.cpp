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
    
    loadTextures();
    
    //  array of values
    GLfloat particleData[logo::NUM_PARTICLES * 8]; // two slots for position, two for velocity, three for color, one for size
    
    for (int i = 0; i < logo::NUM_PARTICLES; i++) {
        //  position is completely random
        float randNum = ci::Rand::randFloat(-1.0f, 1.0f);
        float randNum2 = ci::Rand::randFloat(-1.0f, 1.0f);
        //  velocity is normalized vector
        float randNum3 = ci::Rand::randFloat(0.0f, M_PI * 2.0f);
        
        particleData[(i*8) + 0] = randNum;                          // pos.x
        particleData[(i*8) + 1] = randNum2;                         // pos.y
        
        particleData[(i*8) + 2] = cos(randNum3) * 0.03;             // vel.x
        particleData[(i*8) + 3] = sin(randNum3) * 0.03;             // vel.y
        
        particleData[(i*8) + 4] = ci::Rand::randFloat(-0.1, 0.1);            // col.r
        if (particleData[(i*8) + 4] < 0.0) {
            particleData[(i*8) + 4] += 1.0;
        }
        particleData[(i*8) + 5] = ci::Rand::randFloat();            // col.g
        particleData[(i*8) + 6] = ci::Rand::randFloat();            // col.b
        
        particleData[(i*8) + 7] = ci::Rand::randFloat(3.0f, 9.0f); // baseSize
    }
    
    std::cout << "ParticleSystem::setup" << std::endl;
    std::cout << "    Sizeof particleData: " << sizeof(particleData) << std::endl;
    
    //  create two buffers to ping-pong back and forth with position data
    glGenBuffers(1, &mParticleBufferA);
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferA);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleData), particleData, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &mParticleBufferB);
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particleData), particleData, GL_STREAM_DRAW); // don't initialize immediately
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
    
    std::cout << "ParticleSystem::setup" << std::endl;
    std::cout << "    vertexShader: " << vertexShader << std::endl;
    std::cout << "    fragmentShader: " << fragmentShader << std::endl;
    std::cout << "    mShaderProgram: " << mShaderProgram << std::endl;
    glAttachShader(mShaderProgram, vertexShader);
    glAttachShader(mShaderProgram, fragmentShader);

    // before linking program, specify which output attributes we want to capture into a buffer
    const GLchar * feedbackVaryings[4] = {"outPos", "outVel", "outCol", "outSize"};
    glTransformFeedbackVaryings(mShaderProgram, 4, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glBindFragDataLocation(mShaderProgram, 0, "outColor");

    // link and activate the program
    glLinkProgram(mShaderProgram);
    glUseProgram(mShaderProgram);
    
    mPosAttrib = glGetAttribLocation(mShaderProgram, "inPos");
    mVelAttrib = glGetAttribLocation(mShaderProgram, "inVel");
    mColAttrib = glGetAttribLocation(mShaderProgram, "inCol");
    mSizeAttrib = glGetAttribLocation(mShaderProgram, "inSize");

    mMousePosUniform = glGetUniformLocation(mShaderProgram, "mousePos");
    mNewPosUniform = glGetUniformLocation(mShaderProgram, "newPositions");
    
    mParticleTexUniform = glGetUniformLocation(mShaderProgram, "ParticleTex");
    mBackgroundTexUniform = glGetUniformLocation(mShaderProgram, "BackgroundTex");
    
    std::cout << "    Max uniform locations: " << GL_MAX_UNIFORM_LOCATIONS << std::endl;
  
    std::cout << "    mPosAttrib: " << mPosAttrib << std::endl;
    std::cout << "    mVelAttrib: " << mVelAttrib << std::endl;
    std::cout << "    mColAttrib: " << mColAttrib << std::endl;
    std::cout << "    mSizeAttrib: " << mSizeAttrib << std::endl;

    std::cout << "    mMousePosUniform:" << mMousePosUniform << std::endl;
    std::cout << "    mNewPosUniform: " << mNewPosUniform << std::endl;
    std::cout << "    mParticleTexUniform: " << mParticleTexUniform << std::endl;
    std::cout << "    mBackgroundTexUniform: " << mBackgroundTexUniform << std::endl;
    
    
    mPosArrayPointer = &posArray;
}

//******************************************
//  nothing happens in update
//******************************************
void ParticleSystem::update()
{
    
}

//******************************************
//  temp method for debugging
//******************************************
void ParticleSystem::updateMouse(ci::vec2 pos)
{
    mLastMousePos = pos;
}

//******************************************
//  draw function; main updates happen here
//******************************************
void ParticleSystem::draw()
{
    glUseProgram(mShaderProgram);
    
    //  make a copy of the array from the main app
    float testArray[logo::NUM_NEW_POSITIONS * 2];
    for (int i = 0; i < logo::NUM_NEW_POSITIONS * 2; i++) {
        testArray[i] = mPosArrayPointer[i];
    }

    //  pass mouse position
    //float mousePos[2] = {float(mLastMousePos.x), float(mLastMousePos.y)};
    //glUniform2fv(mMousePosUniform, 1, mousePos);
    
    //  pass in the array of new positions
    glUniform2fv(mNewPosUniform, logo::NUM_NEW_POSITIONS, testArray);
    
    
    //  disable the rasterizer
    glEnable(GL_RASTERIZER_DISCARD);
    
    //  specify the source buffer
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferA);
    glEnableVertexAttribArray(mPosAttrib);
    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    
    glEnableVertexAttribArray(mVelAttrib);
    glVertexAttribPointer(mVelAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glEnableVertexAttribArray(mColAttrib);
    glVertexAttribPointer(mColAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
    
    glEnableVertexAttribArray(mSizeAttrib);
    glVertexAttribPointer(mSizeAttrib, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
    
    
    //  specify target buffer
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mParticleBufferB);
    
    //  do transform feedback
    glBeginTransformFeedback(GL_POINTS);
    
    glDrawArrays(GL_POINTS, 0, logo::NUM_PARTICLES);
    glEndTransformFeedback();
    
    glDisableVertexAttribArray(mPosAttrib);
    glDisableVertexAttribArray(mVelAttrib);
    glDisableVertexAttribArray(mColAttrib);
    glDisableVertexAttribArray(mSizeAttrib);
    
    glFlush();

    std::swap(mParticleBufferA, mParticleBufferB);
    
    //  draw the particles
    glDisable(GL_RASTERIZER_DISCARD);
    
    //  Cinder openGL calls to render textures as points
    // glUniform1i(mParticleTexUniform, 0);
    ci::gl::ScopedTextureBind texScope( mParticleTex , 0 );
    ci::gl::ScopedTextureBind texScope2( mBackgroundTex, 1 );
    glUniform1i(mBackgroundTexUniform, 1);
    
    ci::gl::ScopedState	stateScope( GL_PROGRAM_POINT_SIZE, true );
    ci::gl::ScopedBlend blendScope( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//    ci::gl::ScopedBlendAdditive additive;

    
    glBindBuffer(GL_ARRAY_BUFFER, mParticleBufferA);
    glEnableVertexAttribArray(mPosAttrib);
    glVertexAttribPointer(mPosAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glEnableVertexAttribArray(mVelAttrib);
    glVertexAttribPointer(mVelAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(mColAttrib);
    glVertexAttribPointer(mColAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(mSizeAttrib);
    glVertexAttribPointer(mSizeAttrib, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
    
    glDrawArrays(GL_POINTS, 0, logo::NUM_PARTICLES);
    glDisableVertexAttribArray(mPosAttrib);
    glDisableVertexAttribArray(mVelAttrib);
    glDisableVertexAttribArray(mColAttrib);
    glDisableVertexAttribArray(mSizeAttrib);
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
//  load textures
//******************************************
void ParticleSystem::loadTextures()
{
    ci::gl::Texture::Format textureFormat;
    textureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR ).mipmap().internalFormat( GL_RGBA );
    mParticleTex = ci::gl::Texture::create( ci::loadImage( ci::app::loadAsset( "smoke_blur.png" ) ), textureFormat );
    
//    ci::gl::Texture::Format textureFormat2;
//    textureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR ).mipmap().internalFormat( GL_RGBA );
    mPotionTex = ci::gl::Texture::create(ci::loadImage(ci::app::loadAsset("potionBubbles.png")), textureFormat);
    
//    ci::gl::Texture::Format textureFormat3;
//    textureFormat.magFilter( GL_LINEAR ).minFilter( GL_LINEAR ).mipmap().internalFormat( GL_RGBA );
    mPhillipTex = ci::gl::Texture::create(ci::loadImage(ci::app::loadAsset("phillipHeadThreshold.png")), textureFormat);
    
    mBackgroundTex = mPotionTex;
    
    
    std::cout << "ParticleSystem::loadTextures" << std::endl;
    std::cout << "    mParticleTex id: " << mParticleTex->getId() << std::endl;
    std::cout << "    mPotionTex id: " << mPotionTex->getId() << std::endl;
    
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
    std::cout << "ParticleSystem::createShader: " << std::endl;
    std::cout << "    Shader status: (0 bad, 1 good): " << status << std::endl;
    if (status == 0) {
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        std::cout << "Shader compile log: " << buffer << std::endl;
    }
    
    return shader;
}

void ParticleSystem::changeBackground()
{
    mBackgroundTex = mPhillipTex;
    
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

float ParticleSystem::lerp(float start, float stop, float amt)
{
    return start + (fabs(stop-start) * amt);
}
