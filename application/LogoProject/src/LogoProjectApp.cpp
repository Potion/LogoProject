#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"
#include "cinder/Utilities.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/videoio/videoio.hpp"

#include "CinderOpenCV.h"

#include "ParticleSystem.h"

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
    //cv::VideoCapture      mOpenCVCam;
    
    //  openCV
    cv::Mat             convertToOCVMat(ci::Surface &surface);
    cv::Mat             mPrevMat;
    
    //  particles
    ParticleSystemRef   mParticles;

    //  motion detection
    int                 mNumFramesForRestart;
    int                 mLastGoodFrame;
    int                 mThreshold;
    
    std::vector<ci::vec2>   getWhitePixels(cv::Mat &mat);
    ci::vec2            chooseRandomWhiteSpot(std::vector<ci::vec2> &vector);
    void                sendRandomPoint(ci::vec2 point);
    void                update250RandPoints(std::vector<ci::vec2> &vector);
    
    float               mNewPositions[500];
    
    ci::vec2            normalizePosition(ci::vec2 &pos, int width, int height);
    
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

    mParticles = ParticleSystem::create(*mNewPositions);
    
    //  camera
    if (!findDepthSense()) {
        quit();
    }
    try {
        mCapture = Capture::create( 640, 480, findDepthSense() );
        mCapture->start();
    }
    catch( ci::Exception &exc ) {
        CI_LOG_EXCEPTION( "Failed to init capture ", exc );
    }
    
    //mOpenCVCam = cv::VideoCapture(0);
    //if (!mOpenCVCam.isOpened()) {
    //    std::cout << "Problems!!" << std::endl;
    //}
    
    mPrevMat = cv::Mat(480, 640, CV_8U);
    mLastGoodFrame = 1;
    mNumFramesForRestart = 60;
    mThreshold = 70;
}

void LogoProjectApp::mouseDown( MouseEvent event )
{
}

void LogoProjectApp::mouseMove( MouseEvent event )
{
    mParticles->updateMouse(event.getPos());
}

void LogoProjectApp::keyDown(cinder::app::KeyEvent event)
{
    std::cout << "LogoProjectApp::keyDown:" << std::endl;
}

void LogoProjectApp::update()
{
    
    //  wait until we have both images before we start comparison
    if (!mCapture || !mCapture->checkNewFrame()) {
        //  if we're not getting new frames at all, stop and restart capture
        if (ci::app::getElapsedFrames() - mLastGoodFrame > mNumFramesForRestart) {
            restartCamera();
        }
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
    
    //std::cout << "\n\nNew Frame***********************\n" << std::endl;
    //std::cout << "mCurrentMat->size-> " << mCurrentMat.size() <<" " << mCurrentMat.type() << std::endl;
    //std::cout << "mPrevMat->size-> " << mPrevMat.size() << " " << mPrevMat.type() << std::endl;
    //std::cout << "mDiffMat->size-> " << mDiffMat.size() << " " << mDiffMat.type() << std::endl;
    //std::cout << "tempDiff->size-> " << tempDiff.size() << " " << tempDiff.type() << std::endl;
    //std::cout << "blackAndWhite->size-> " << blackAndWhite.size() << " " << blackAndWhite.type() << std::endl;
    
//    if ( blackAndWhite.type() == mPrevMat.type() )
//    {
        mPrevMat.copyTo( tempCopyPrev );
        cv::absdiff(blackAndWhite, tempCopyPrev, tempDiff );
//    }
//    else {
//        std::cout << "The types do not match" << std::endl;
//    }

    //  save the B&W version as the previous image
    blackAndWhite.copyTo(mPrevMat);
    
    //std::cout << "\n\nPostCopy***********************\n" << std::endl;
    //std::cout << "mCurrentMat->size-> " << mCurrentMat.size() <<" " << mCurrentMat.type() << std::endl;
    //std::cout << "mPrevMat->size-> " << mPrevMat.size() << " " << mPrevMat.type() << std::endl;
    //std::cout << "mDiffMat->size-> " << mDiffMat.size() << " " << mDiffMat.type() << std::endl;
    //std::cout << "tempDiff->size-> " << tempDiff.size() << " " << tempDiff.type() << std::endl;
    //std::cout << "blackAndWhite->size-> " << blackAndWhite.size() << " " << blackAndWhite.type() << std::endl;
    

    //  threshold the changes
    cv::threshold(tempDiff, tempDiff, mThreshold, 255, CV_THRESH_BINARY);
    
    //  send one random movement location to particle system
    std::vector<ci::vec2> whitePixels = getWhitePixels(tempDiff);
    if (whitePixels.size() > 0) {
//        sendRandomPoint(chooseRandomWhiteSpot(whitePixels));
        update250RandPoints(whitePixels);
    }

    //  convert OpenCV mats to Cinder-usable images
    ImageSourceRef imageRef = fromOcv(tempDiff);
    Surface8u tempSurface = Surface8u(imageRef);
    
    if (!mTexture) {
        std::cout << "update::Creating new texture" << std::endl;
        mTexture = ci::gl::Texture::create(imageRef);
    } else {
        mTexture->update(tempSurface);
    }
}

void LogoProjectApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

    if (mTexture) {
        gl::draw(mTexture);
    }

    //  Draw the FPS
    ci::gl::drawString( "Framerate: " + ci::toString(ci::app::App::get()->getAverageFps()), ci::vec2( 10.0f, 10.0f ), ci::Color(1,0,0) );
    
    mParticles->draw();
}

//
//  print out names of cameras
//
void LogoProjectApp::printDevices()
{
    std::cout << "Getting devices, number attached: " <<  Capture::getDevices().size() << std::endl;
    for( const auto &device : Capture::getDevices() ) {
        console() << "Device: " << device->getName() << ", Id# " << device->getUniqueId() << endl;
    }
}

//
//  return the DepthSense camera
//
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

//
//  restart the camera if it hasn't given a new frame in 30 frames
//
void LogoProjectApp::restartCamera()
{
    std::cout << "Frame # " << ci::app::getElapsedFrames() << ": LogoProjectApp::restartCamera" << std::endl;
    mCapture->stop();
    mCapture->start();
    mLastGoodFrame = ci::app::getElapsedFrames();
}

//
//  convert Cinder surface to OpenCV mat
//
cv::Mat LogoProjectApp::convertToOCVMat(ci::Surface &surface)
{
    return cv::Mat( surface.getHeight(), surface.getWidth(), CV_MAKETYPE( CV_8U, surface.hasAlpha()?4:3), surface.getData(), surface.getRowBytes() );
}

std::vector<ci::vec2> LogoProjectApp::getWhitePixels(cv::Mat &mat)
{
    std::vector<ci::vec2> whitePixels;
    //  check for white pixels
    for ( int y = 0; y < mat.rows; y++) {
        for ( int x = 0; x < mat.cols; x++) {
            if (mat.at<unsigned char>(y, x) == 255) {
                whitePixels.push_back(ci::vec2(x, y));
            }
        }
    }
    return whitePixels;
}

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

void LogoProjectApp::sendRandomPoint(ci::vec2 point)
{
    mParticles->updateMouse(point);
}

void LogoProjectApp::update250RandPoints(std::vector<ci::vec2> &vector)
{
//    std::cout << "New Frame! ******************************" << std::endl;
    
//    std::cout << "******" << vector.size() << " particles!" << std::endl;
    //  if we have 250 or less, send all of them to the particle system
    if (vector.size() < 251) {
        //std::cout << "250 or Fewer!!!!" << std::endl;
        int index = 0;
        
        //  grab all positions
        for (ci::vec2 eachVector : vector) {
            eachVector = normalizePosition(eachVector, 640, 480);
            mNewPositions[index] = eachVector.x;
            mNewPositions[index + 1] = eachVector.y;
            index += 2;
        }
        
        //  fill the rest with 0.0, 0.0
        for (int i = index; i < 500; i++) {
            mNewPositions[index] = 0.0f;
        }
    }
    
    //  otherwise, pick a random sampling of 250
    else {
        //std::cout << "251 or more!!!" << std::endl;
        //  make a list from 1 to how many white pixels and shuffle it
        std::vector<unsigned int> indices(vector.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::random_shuffle(indices.begin(), indices.end());
        
//        std::cout << "    The new order: " << std::endl;
//        for (int i = 0; i < indices.size(); i++) {
//            std::cout << "        index: " << indices[i] << std::endl;
//        }
        
        //  put the first 250 in the array
        for (int i = 0; i < 250; i+=2) {
            vector[indices[i]] = normalizePosition(vector[indices[i]], 640, 480);
            mNewPositions[i] = vector[indices[i]].x;
            mNewPositions[i+1] = vector[indices[i]].y;
        }
    }
    
    //ci::app::console() << "LogoProjectApp::send250...: Address of mNewPositions: " << mNewPositions << std::endl;
    
//    std::cout << "LogoProjectApp::update250...: first ten values of array: " << std::endl;
//    for (int i = 0; i < 10; i++) {
//        std::cout << "    " << mNewPositions[i] << std::endl;
//    }

//    for (int i = 0; i < 250; i++) {
//        std::cout << "new position: x: " << mNewPositions[i] << ", y: " << mNewPositions[i+1] << std::endl;
//    }
}

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

CINDER_APP( LogoProjectApp, RendererGl )
