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
    void changeBackground();

protected:
    ParticleSystem();
    
private:
    GLuint              mVAO;
    GLuint              mParticleBufferA, mParticleBufferB;
    GLuint              mShaderProgram;

    GLint               mPosAttrib;
    GLint               mVelAttrib;
    GLint               mColAttrib;
    GLint               mSizeAttrib;
    GLint               mBornTimeAttrib;
    
    GLint               mMousePosUniform;
    GLint               mNewPosUniform;
    GLint               mDeltaTimeUniform;
    GLint               mTimeUniform;
    GLint               mHueUniform;

    GLint               mParticleTexUniform;
    GLint               mBackgroundTexUniform;
    
    float *             mPosArrayPointer;

    std::string         loadShaderSource(std::string path);
    GLuint              createShader(GLenum type, const GLchar* src);
    
    void                loadTextures();
    ci::gl::TextureRef  mParticleTex, mPotionTex, mPhillipTex;
    ci::gl::TextureRef  mBackgroundTex;
    
    ci::vec2            mLastMousePos;
    double              mLastFrameTime;
    float               mDeltaTime;
    
    //test functions
    float getRandomFloat(ci::vec2 currentPos);
    float mapFloat(float value, float inputMin, float inputMax, float outputMin, float outputMax);
    float lerp(float start, float stop, float amt);
};