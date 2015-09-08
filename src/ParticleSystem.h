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

class ParticleSystem;
typedef std::shared_ptr<ParticleSystem> ParticleSystemRef;

class ParticleSystem
{
public:
    static ParticleSystemRef create();
    ~ParticleSystem();

    void setup();
    void update();
    void draw();
    
protected:
    ParticleSystem();
    
private:
    int     mParticleCount;
    GLuint  mVAO;
    GLuint  mPositionBufferA, mPositionBufferB;
    GLuint  mShaderProgram;

    GLint   mPosAttrib;
    GLint   mVelAttrib;
    GLint   mColAttrib;
    GLint   mMousePosUniform;

    const GLchar * loadShaderData(std::string path);
    GLuint createShader(GLenum type, const GLchar* src);
    
    std::string loadShaderSource(std::string path);
    
};