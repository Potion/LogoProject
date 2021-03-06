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
    void draw(int amountOfMotion);
    void changeBackground();
    
    void setMotionlessness(int truefalse) { mIsMotionless = truefalse; }
    
    
    //  parameters to control during run
    void setColorCycleSpeed(float speed) { mColorCycleSpeed = speed; }
    void setGravity( float gravity ) { mGravity = gravity; }
    void setParticleOpacity (float opacity) { mParticleOpacity = opacity; }
    void setParticleLifespan( float lifespan ) { mParticleLifespan = lifespan; }
    void setSlipperiness( float slippy ) { mSlipperiness = slippy; }
    
    float getColorCycleSpeed() { return mColorCycleSpeed; }
    float getGravity() { return mGravity; }
    float getParticleOpacity() { return mParticleOpacity; }
    float getParticleLifespan() { return mParticleLifespan; }
    float getSlipperiness() { return mSlipperiness; }
    
    void toggleShrinkMode();
    void toggleMotionBasedHue();

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
    GLint               mCurrentHueAttrib;
    GLint               mBornTimeAttrib;
    
    GLint               mMousePosUniform;
    GLint               mNewPosUniform;
    GLint               mDeltaTimeUniform;
    GLint               mTimeUniform;
    GLint               mHueUniform;
    GLint               mGravityUniform;
    GLint               mShrinkUniform;
    GLint               mParticleOpacityUniform;
    GLint               mParticleLifeUniform;
    GLint               mSlipperinessUniform;
    GLint               mMotionlessUniform;

    GLint               mParticleTexUniform;
    GLint               mBackgroundTexUniform;
    
    float *             mPosArrayPointer;

    std::string         loadShaderSource(std::string path);
    GLuint              createShader(GLenum type, const GLchar* src);
    
    void                loadTextures();
    ci::gl::TextureRef  mParticleTex, mPotionTex;
    ci::gl::TextureRef  mBackgroundTex;
    
    ci::vec2            mLastMousePos;
    double              mLastFrameTime;
    float               mDeltaTime;
    
    //  adjustable parameters
    float               mColorCycleSpeed;
    float               mGravity;
    int                 mPixelsDoShrink;
    float               mParticleOpacity;
    float               mParticleLifespan;
    bool                mMotionBasedHue;
    float               mSlipperiness;
    int                 mIsMotionless;
    
    
    //test functions
    float getRandomFloat(ci::vec2 currentPos);
    float mapFloat(float value, float inputMin, float inputMax, float outputMin, float outputMax);
    float lerp(float start, float stop, float amt);
};