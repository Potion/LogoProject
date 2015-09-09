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
    static ParticleSystemRef create();
    ~ParticleSystem();

    void setup();
    void update();
    void updateMouse(ci::ivec2 pos);
    void draw();
    
protected:
    ParticleSystem();
    
private:
    int         mParticleCount;
    GLuint      mVAO;
    GLuint      mPositionBufferA, mPositionBufferB;
    GLuint      mShaderProgram;

    GLint       mPosAttrib;
    GLint       mVelAttrib;
    GLint       mColAttrib;
    GLint       mMousePosUniform;

    const GLchar * loadShaderData(std::string path);
    std::string loadShaderSource(std::string path);
    GLuint createShader(GLenum type, const GLchar* src);
    
    ci::vec2    normalizeMousePos(ci::ivec2 pos);
    ci::ivec2   mLastMousePos;
    
};