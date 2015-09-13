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

class ParticleSystem;
typedef std::shared_ptr<ParticleSystem> ParticleSystemRef;

class ParticleSystem
{
public:
    static ParticleSystemRef create(float &posArray);
    ~ParticleSystem();

    void setup(float &posArray);
    void update();
    void updateMouse(ci::ivec2 pos);
    void draw();

protected:
    ParticleSystem();
    
private:
    int         mParticleCount;
    int         mMaxNewPositions;
    GLuint      mVAO;
    GLuint      mNewPositions;
    GLuint      mParticleBufferA, mParticleBufferB;
    GLuint      mTFBufferA, mTFBufferB;
    //GLuint      mTransformFeedbacks[2];
    GLuint      mShaderProgram;

    GLint       mPosAttrib;
    GLint       mVelAttrib;
    GLint       mColAttrib;
    //GLint       mNewPosAttrib;
    GLint       mMousePosUniform;
    GLint       mNumNewPosUniform;
    GLint       mNewPosUniform;
    
    float *     mPosArrayPointer;

    //const GLchar *  loadShaderData(std::string path);
    std::string     loadShaderSource(std::string path);
    GLuint          createShader(GLenum type, const GLchar* src);
    
    ci::ivec2   mLastMousePos;
    
    //bool        mIsFirst;
};