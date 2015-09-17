#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/videoio/videoio.hpp"

#include "CinderOpenCV.h"

#include "ParticleSystem.h"
#include "Common.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class LogoProjectApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
    
    //  camera
    void                    printDevices();
    ci::Capture::DeviceRef  findDepthSense();
    void                    restartCamera();
    CaptureRef              mCapture;
    gl::TextureRef          mTexture;
    gl::TextureRef          mBlackBox;
    
    //  openCV
    cv::Mat             convertToOCVMat(ci::Surface &surface);
    cv::Mat             mPrevMat;
    
    //  particles
    ParticleSystemRef   mParticles;

    //  motion detection
    int                 mNumFramesForRestart;
    int                 mLastGoodFrame;
    int                 mThreshold;
    float               mAmountOfTimeForMotionlessness;
    float               mLastTimeWithMotion;
    bool                mIsMotionlessNow;
    
    std::vector<cv::Point2i>   getWhitePixels(cv::Mat &mat);
    ci::vec2            chooseRandomWhiteSpot(std::vector<ci::vec2> &vector);
    void                sendRandomPoint(ci::vec2 point);
    void                updateNewPositions(std::vector<cv::Point2i> &vector);
    void                resetPosArray();
    
    float               mNewPositions[logo::NUM_NEW_POSITIONS * 2];
    
    ci::vec2            normalizePosition(ci::vec2 &pos, int width, int height);
    cv::Point2f         normalizePositionCV(cv::Point2i &pos, int width, int height);
    
    //  GUI information
    params::InterfaceGlRef  mParams;
    bool                mIsDrawingParams;
    bool                mIsDrawingFramerate;
    
    void                setUpParams();
    void                toggleMotionBasedColor();
    float               mBGOpacity;
    float               mBaseParticleOpacity;
    bool                mMotionBasedHue;
    int                 mAmountOfMotion;
};

void LogoProjectApp::setup()
{
    //  Seed Random
    time_t t = time(0);
    struct tm *now = localtime(&t);
    ci::randSeed(now->tm_sec);
    
    //  Add correct asset path
    ci::fs::path relativePath = "../../../../../assets";
    ci::fs::path absolutePath = ci::fs::canonical(relativePath);
    absolutePath.make_preferred().native();
    ci::app::addAssetDirectory(relativePath);

    //  print out OpenGL version
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    
    ci::gl::enableAlphaBlending();
    
    mParticles = ParticleSystem::create(*mNewPositions);
    
    mBlackBox = ci::gl::Texture::create(ci::loadImage(ci::app::loadAsset("blackBox.png")));
    
    //  camera
    if (!findDepthSense()) {
        quit();
    }
    try {
        mCapture = Capture::create( logo::CAM_RES_WIDTH, logo::CAM_RES_HEIGHT, findDepthSense() );
        mCapture->start();
    }
    catch( ci::Exception &exc ) {
        CI_LOG_EXCEPTION( "Failed to init capture ", exc );
    }
    
    //  initialize array
    for (int i = 0; i < logo::NUM_NEW_POSITIONS * 2; i++) {
        mNewPositions[i] = ci::randFloat(-1.0f, 1.0f);
    }
    
    mPrevMat = cv::Mat(logo::CAM_RES_HEIGHT, logo::CAM_RES_WIDTH, CV_8U);

    //  set defaults and set up parameter GUI
    mBGOpacity = 0.81;
    mThreshold = 70;
    setUpParams();
    mBaseParticleOpacity = 1.0;
    mMotionBasedHue = false;
    mAmountOfMotion = 0;
    mIsDrawingParams = false;
    mIsDrawingFramerate = false;

    mLastGoodFrame = 1;
    mNumFramesForRestart = 60;
    mAmountOfTimeForMotionlessness = 7.0f;
    mLastTimeWithMotion = 1;
    mIsMotionlessNow = false;
    
    ci::app::App::get()->setWindowSize(1024, 768);
    ci::app::App::get()->setFullScreen(true);
}

//******************************************
//  input
//******************************************
void LogoProjectApp::mouseDown( MouseEvent event )
{
}

void LogoProjectApp::mouseMove( MouseEvent event )
{
    ci::vec2 normPos(float(event.getPos().x), float(event.getPos().y));
    normPos = normalizePosition(normPos, logo::CAM_RES_WIDTH, logo::CAM_RES_HEIGHT);
    //std::cout << "Moved mouse: " << normPos << std::endl;
    mParticles->updateMouse(normPos);
}

void LogoProjectApp::keyDown(cinder::app::KeyEvent event)
{
    std::cout << "LogoProjectApp::keyDown: " << event.getChar() << std::endl;
    if (event.getChar() == ' ') {
        std::cout << "    [Spacebar]" << std::endl;
        //mParticles->changeBackground();
    }
    
    if (event.getChar() == 'f') {
        ci::app::App::get()->setFullScreen(!ci::app::App::get()->isFullScreen());
    }
    
    if (event.getChar() == 'p') {
        mIsDrawingParams = !mIsDrawingParams;
    }
    
    if (event.getChar() == 'r') {
        mIsDrawingFramerate = !mIsDrawingFramerate;
    }
    
}

//******************************************
//  update
//******************************************
void LogoProjectApp::update()
{
    mParticles->update();
    
    //  if we don't have a new frame, skip the rest
    if (!mCapture || !mCapture->checkNewFrame()) {
        //  if we're not getting new frames at all, stop and restart capture
        if (ci::app::getElapsedFrames() - mLastGoodFrame > mNumFramesForRestart) {
            restartCamera();
        }
        //resetPosArray();
        return;
    }

    //  update the frames
    mLastGoodFrame = ci::app::getElapsedFrames();
    
    //  colorInput is feed from camera
    cv::Mat colorInput = convertToOCVMat(*mCapture->getSurface());
    
    //  create OpenCV mats to save and manipulate our camera image
    cv::Mat matPreFlip;
    cv::Mat blackAndWhite;
    cv::Mat tempCopyPrev;
    cv::Mat tempDiff;
    
    //  convert color input to blackandwhite
    cv::cvtColor(colorInput, matPreFlip, CV_RGB2GRAY);
    
    //  flip it
    cv::flip(matPreFlip, blackAndWhite, 1);
    
    mPrevMat.copyTo( tempCopyPrev );
    cv::absdiff(blackAndWhite, tempCopyPrev, tempDiff );

    //  save the B&W, flipped version as the previous image
    blackAndWhite.copyTo(mPrevMat);
    
    //  threshold the changes
    cv::threshold(tempDiff, tempDiff, mThreshold, 255, CV_THRESH_BINARY);
    
    //  send one random movement location to particle system
    std::vector<cv::Point2i> whitePixels = getWhitePixels(tempDiff);
    mAmountOfMotion = whitePixels.size();
    if (mAmountOfMotion > 0) {
        //sendRandomPoint(chooseRandomWhiteSpot(whitePixels));
        updateNewPositions(whitePixels);
        mLastTimeWithMotion = ci::app::getElapsedSeconds();
        if (mIsMotionlessNow) {
            mParticles->setMotionlessness(0);
            mIsMotionlessNow = false;
        }
    } else {
        //std::cout << "LogoProjectApp::update: no movement pixels" << std::endl;
        resetPosArray();
    }
    
    //  if it's been still long enough, alert particle system no motion
    if (!mIsMotionlessNow && ci::app::getElapsedSeconds() - mLastTimeWithMotion > mAmountOfTimeForMotionlessness) {
        mParticles->setMotionlessness(1);
        mIsMotionlessNow = true;
    }

    //  for debugging, convert OpenCV mats to Cinder-usable images
    ImageSourceRef imageRef = fromOcv(tempDiff);
    Surface8u tempSurface = Surface8u(imageRef);
    
    if (!mTexture) {
        mTexture = ci::gl::Texture::create(imageRef);
    } else {
        mTexture->update(tempSurface);
    }
}

//******************************************
//  draw
//******************************************
void LogoProjectApp::draw()
{
    //gl::clear( Color( 0, 0, 0 ) );

    //  debugging only
    ci::gl::color(ColorA(0.0, 0.0, 0.0, mBGOpacity));
    //if (mTexture) {
    //    //gl::draw(mTexture);
    //}
    if (mBlackBox) {
        gl::draw(mBlackBox, getWindowBounds());
    }
    
    //**************Lots of stuff that didn't work*****************
    //ci::Rectf rect(0.0, 0.0, ci::app::getWindowWidth(), ci::app::getWindowHeight());
    //ci::gl::drawSolidRect(rect);
    //
    //ci::gl::clear(ci::ColorA(0.0, 0.0, 0.0, 0.001));
    //ci::gl::color(0.0, 0.0, 0.0, 0.0005);
    //
    //ci::gl::enableAlphaBlending();
    //ci::gl::clear(ci::ColorA(0.0, 0.0, 0.0, 0.001));
    //gl::drawSolidRect(ci::Rectf(0, 1, 1, 0), vec2(0, 1), vec2(1, 0));
    //
    //auto ctx = ci::gl::context();
    //ctx->getDefaultVao()->bind();
    //ctx->setDefaultShaderVars();
    //**************************************************************
    
    //  Draw the FPS
    if (mIsDrawingFramerate) {
        ci::gl::drawString( "Framerate: " + ci::toString(ci::app::App::get()->getAverageFps()), ci::vec2( 10.0f, 10.0f ), ci::Color(1,0,0) );
    }
    
    mParticles->draw(mAmountOfMotion);
    if (mIsDrawingParams) {
        mParams->draw();
    }
}

//******************************************
//  print out names of cameras
//******************************************
void LogoProjectApp::printDevices()
{
    std::cout << "Getting devices, number attached: " <<  Capture::getDevices().size() << std::endl;
    for( const auto &device : Capture::getDevices() ) {
        console() << "Device: " << device->getName() << ", Id# " << device->getUniqueId() << endl;
    }
}

//******************************************
//  return the DepthSense camera
//******************************************
ci::Capture::DeviceRef LogoProjectApp::findDepthSense()
{
    for( const auto &device : Capture::getDevices() ) {
        if (device->getName().find("DepthSense") != std::string::npos) {
            return device;
        }
    }
    std::cout << "No DepthSense Camera attached" << std::endl;
    return 0;
}

//******************************************
//  restart the camera (called if no new frame within certain number of frames)
//******************************************
void LogoProjectApp::restartCamera()
{
    std::cout << "LogoProjectApp::restartCamera: Frame # " << ci::app::getElapsedFrames() << std::endl;
    mCapture->stop();
    mCapture->start();
    mLastGoodFrame = ci::app::getElapsedFrames();
}

//******************************************
//  convert Cinder surface to OpenCV mat
//******************************************
cv::Mat LogoProjectApp::convertToOCVMat(ci::Surface &surface)
{
    return cv::Mat( surface.getHeight(), surface.getWidth(), CV_MAKETYPE( CV_8U, surface.hasAlpha()?4:3), surface.getData(), surface.getRowBytes() );
}

//******************************************
//  get a vector of all white pixels from motion detection
//******************************************
std::vector<cv::Point2i> LogoProjectApp::getWhitePixels(cv::Mat &mat)
{
    std::vector<cv::Point2i> whitePixelsCV;
    cv::findNonZero(mat, whitePixelsCV);

    return whitePixelsCV;
}

//******************************************
//  choose one random white pixel
//******************************************
ci::vec2 LogoProjectApp::chooseRandomWhiteSpot(std::vector<ci::vec2> &vector)
{
    if (vector.size() > 0) {
        int p = ci::Rand::randInt(0, vector.size());
        return vector[p];
    }
    else {
        return ci::vec2(0.0f, 0.0f);
    }
}

//******************************************
//  send one pixel as though it's mouse (debug only)
//******************************************
void LogoProjectApp::sendRandomPoint(ci::vec2 point)
{
    mParticles->updateMouse(point);
}

//******************************************
//  choose random xxx-point sampling of white pixels
//******************************************
void LogoProjectApp::updateNewPositions(std::vector<cv::Point2i> &vector)
{
    //  if we have max position number or less, send all of them to the particle system
    if (vector.size() < logo::NUM_NEW_POSITIONS) {
        //std::cout << "Less than" << logo::NUM_NEW_POSITIONS << "!!!!" << std::endl;
        int index = 0;
        
        //  grab all positions
        for (cv::Point2i eachVector : vector) {
            cv::Point2f newPoint2f = normalizePositionCV(eachVector, logo::CAM_RES_WIDTH, logo::CAM_RES_HEIGHT);
            mNewPositions[index] = newPoint2f.x;
            mNewPositions[index + 1] = newPoint2f.y;
            index += 2;
        }
        
        //  fill the rest with points offscreen
        for (int i = index; i < logo::NUM_NEW_POSITIONS * 2; i+=2) {
            mNewPositions[i] = ci::Rand::randFloat(-1.0, 1.0);
            mNewPositions[i+1] = ci::Rand::randFloat(-2.0, -1.5);
        }
    }
    
    //  otherwise, pick a random sampling
    else {
        //std::cout << logo::NUM_NEW_POSITIONS << " or more!!!!" << std::endl;
        //  make a list from 1 to how many white pixels and shuffle it
        std::vector<unsigned int> indices(vector.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::random_shuffle(indices.begin(), indices.end());
        
        //  put the first ones in the array
        for (int i = 0; i < logo::NUM_NEW_POSITIONS; i+=2) {
            cv::Point2f newPoint2f = normalizePositionCV(vector[indices[i]], logo::CAM_RES_WIDTH, logo::CAM_RES_HEIGHT);
            mNewPositions[i] = newPoint2f.x;
            mNewPositions[i+1] = newPoint2f.y;
        }
    }
}

//******************************************
//  set up GUI parameters
//******************************************
void LogoProjectApp::setUpParams()
{
    mParams = params::InterfaceGl::create(getWindow(), "Parameters", toPixels(ci::ivec2(300, 200)));
    mParams->addParam("Motion threshold", &mThreshold).min(0).max(100).step(5);
    
    mParams->addParam("BG Opacity", &mBGOpacity).min(0.01).max(1.0).step(0.01);

    function<void(float)> setter = bind(&ParticleSystem::setColorCycleSpeed, mParticles, std::placeholders::_1);
    function<float()> getter = bind(&ParticleSystem::getColorCycleSpeed, mParticles);
    mParams->addParam("Color Cycle Speed", setter, getter).step(.01);

    function<void(float)> setter2 = bind(&ParticleSystem::setGravity, mParticles, std::placeholders::_1);
    function<float()> getter2 = bind(&ParticleSystem::getGravity, mParticles);
    mParams->addParam("Gravity", setter2, getter2).step(.0005);
    
    mParams->addButton("Change Shrink", bind(&ParticleSystem::toggleShrinkMode, mParticles));
    mParams->addButton("Change MotionBasedHue", bind(&ParticleSystem::toggleMotionBasedHue, mParticles));
    
    function<void(float)> setter3 = bind(&ParticleSystem::setParticleLifespan, mParticles, std::placeholders::_1);
    function<float()> getter3 = bind(&ParticleSystem::getParticleLifespan, mParticles);
    mParams->addParam("ParticleLifespan", setter3, getter3).step(.01);

    
    function<void(float)> setter4 = bind(&ParticleSystem::setParticleOpacity, mParticles, std::placeholders::_1);
    function<float()> getter4 = bind(&ParticleSystem::getParticleOpacity, mParticles);
    mParams->addParam("Particle Opacity", setter4, getter4).step(.01);
    
    function<void(float)> setter5 = bind(&ParticleSystem::setSlipperiness, mParticles, std::placeholders::_1);
    function<float()> getter5 = bind(&ParticleSystem::getSlipperiness, mParticles);
    mParams->addParam("Slipperiness", setter5, getter5).step(.01);
    
}

//******************************************
//  toggle whether color determined based on motion
//******************************************
void LogoProjectApp::toggleMotionBasedColor()
{
    mMotionBasedHue = !mMotionBasedHue;
}

//******************************************
//  fill position array with all points offscreen
//******************************************
void LogoProjectApp::resetPosArray()
{
    //std::cout << "LogoProjectApp::resetPosArray: " << std::endl;
    for (int i = 0; i < logo::NUM_NEW_POSITIONS; i++) {
        mNewPositions[i*2] = ci::randFloat(-1.0, 1.0);
        mNewPositions[i*2 + 1] = ci::randFloat(-2.0, -1.5);
    }
}

//******************************************
//  change position to use -1.0 to 1.0 convention for shaders
//******************************************
ci::vec2 LogoProjectApp::normalizePosition(ci::vec2 &pos, int width, int height)
{
    ci::vec2    normPos;
    float normX, normY;
    normX = (float)pos.x / (float)width;
    normX *= 2.0f;
    normX -= 1.0f;
    
    normY = (float)pos.y / (float)height;
    normY *= 2.0f;
    normY -= 1.0f;
    normY *= -1.0f;
    
    normX = glm::clamp(normX, -1.0f, 1.0f);
    normY = glm::clamp(normY, -1.0f, 1.0f);
    
    normPos.x = normX;
    normPos.y = normY;
    return normPos;
}

cv::Point2f LogoProjectApp::normalizePositionCV(cv::Point2i &pos, int width, int height)
{
    cv::Point2f    normPos;
    float normX, normY;
    normX = (float)pos.x / (float)width;
    normX *= 2.0f;
    normX -= 1.0f;
    
    normY = (float)pos.y / (float)height;
    normY *= 2.0f;
    normY -= 1.0f;
    normY *= -1.0f;
    
    normX = glm::clamp(normX, -1.0f, 1.0f);
    normY = glm::clamp(normY, -1.0f, 1.0f);
    
    normPos.x = normX;
    normPos.y = normY;
    return normPos;
}


CINDER_APP( LogoProjectApp, RendererGl )
