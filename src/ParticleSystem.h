//
//  ParticleSystem.h
//  LogoProject
//
//  Created by Jennifer on 9/8/15.
//
//

#pragma once
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "Common.h"

class ParticleSystem;
typedef std::shared_ptr<ParticleSystem> ParticleSystemRef;

class ParticleSystem
{
public:
    static ParticleSystemRef create(float &posArray);
    ~ParticleSystem();

    void setup(float &posArray);
    void update();
    void updateMouse(ci::vec2 pos);
    void draw();

protected:
    ParticleSystem();
    
private:
    GLuint              mVAO;
    GLuint              mNewPositions;
    GLuint              mParticleBufferA, mParticleBufferB;
    GLuint              mShaderProgram;

    GLint               mPosAttrib;
    GLint               mVelAttrib;
    GLint               mColAttrib;
    GLint               mMousePosUniform;
    GLint               mNewPosUniform;
    
    float *             mPosArrayPointer;

    std::string         loadShaderSource(std::string path);
    GLuint              createShader(GLenum type, const GLchar* src);
    void                loadTexture();
    ci::gl::TextureRef  mTexture;
    
    ci::vec2            mLastMousePos;
    
    //test functions
    float getRandomFloat(ci::vec2 currentPos);
    float mapFloat(float value, float inputMin, float inputMax, float outputMin, float outputMax);
};