#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"

#include "ParticleSystem.h"
#include "opencv2/core/core.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/videoio/videoio.hpp"
//#include "opencv2/highgui/highgui.hpp"

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
    
    void                printDevices();
    void                restartCamera();
    CaptureRef          mCapture;
    gl::TextureRef      mTexture;
    gl::TextureRef      mBWTexture;
    cv::VideoCapture    mOpenCVCam;
    
    cv::Mat             convertToOCVMat(ci::Surface &surface);
    ci::Surface8uRef    convertToSurface(const cv::Mat &matrix);
    void                compareSurfacesOpenCV(cv::Mat &current, cv::Mat &prev);
    void                compareSurfacesCinderOnly(ci::Surface current, ci::Surface *prev);
    
    void                compareChannels(const ci::Channel &current, ci::Channel *prev);
    
    ci::Surface         mCurrentSurface;
    ci::Surface         mPrevSurface;
    ci::Channel         mCurrentChannel;
    ci::Channel         mPrevChannel;
    ci::Channel         mDiffChannel;
    
    cv::Mat             mCurrentMat;
    cv::Mat             mPrevMat;
    
    ParticleSystemRef   mParticles;
    
    int                 mLastGoodFrame;
    bool                mComparisonHasStarted;
    int                 mThreshold;
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

    mParticles = ParticleSystem::create();
    
    //  camera
    printDevices();
    
    try {
        mCapture = Capture::create( 640, 480 );
        mCapture->start();
    }
    catch( ci::Exception &exc ) {
        CI_LOG_EXCEPTION( "Failed to init capture ", exc );
    }

    mLastGoodFrame = 1;
    mComparisonHasStarted = false;
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
    std::cout << "LogoProjectApp::keyDown: Saving image" << std::endl;
    if (mCurrentSurface.getData()) {
    }
}

void LogoProjectApp::update()
{
    //  wait until we have both images before we start comparison
    if (!mComparisonHasStarted) {
        if (mPrevSurface.getData() && mCurrentSurface.getData()) {
            mComparisonHasStarted = true;
        }
    }
    
    //  see if we have a new frame
    if (!mCapture || !mCapture->checkNewFrame()) {
        //  if we're not getting new frames at all, stop and restart capture
        if (ci::app::getElapsedFrames() - mLastGoodFrame > 30) {
            restartCamera();
        }
        return;
    }

    //  update the frames
    mLastGoodFrame = ci::app::getElapsedFrames();
    
    if (!mComparisonHasStarted) {
        mPrevSurface = mCurrentSurface;
        mCurrentSurface = *mCapture->getSurface();
    } else {
        mPrevChannel = mCurrentChannel;
    }

    mCurrentChannel = ci::Channel(*mCapture->getSurface());
    
    //  use color until we have our information
    if (!mComparisonHasStarted) {
        if( ! mTexture ) {
            // Capture images come back as top-down, and it's more efficient to keep them that way
            mTexture = gl::Texture::create( mCurrentSurface, gl::Texture::Format().loadTopDown() );
        } else {
            //            if (!mComparisonHasStarted) {
            mTexture->update( *mCapture->getSurface() );
            //            } else {
            //                mTexture->update(mPrevChannel);
            //            }
        }
    }

    //  use B&W images once we start comparing
    else {
        if (! mBWTexture) {
            mBWTexture = gl::Texture::create(mCurrentChannel, gl::Texture::Format().loadTopDown());
        }
    }
    
    
    //  do the channel comparison
    if (mComparisonHasStarted) {
        compareChannels(mCurrentChannel, &mPrevChannel);
        mBWTexture->update(mPrevChannel);
    }
}

void LogoProjectApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    if (!mComparisonHasStarted && mTexture) {
        //gl::ScopedModelMatrix modelScope; // this was from cinder video capture sample
        gl::draw(mTexture);
    }
    else if( mComparisonHasStarted && mBWTexture ) {
        gl::draw(mBWTexture);
    }
    mParticles->draw();
}

void LogoProjectApp::printDevices()
{
    std::cout << "Getting devices, number attached: " <<  Capture::getDevices().size() << std::endl;
    for( const auto &device : Capture::getDevices() ) {
        console() << "Device: " << device->getName() << endl;
    }
}

void LogoProjectApp::restartCamera()
{
    std::cout << "Frame # " << ci::app::getElapsedFrames() << ": LogoProjectApp::restartCamera" << std::endl;
    mCapture->stop();
    mCapture->start();
    mLastGoodFrame = ci::app::getElapsedFrames();
}

cv::Mat LogoProjectApp::convertToOCVMat(ci::Surface &surface)
{
    return cv::Mat( surface.getHeight(), surface.getWidth(), CV_MAKETYPE( CV_8U, surface.hasAlpha()?4:3), surface.getData(), surface.getRowBytes() );
}

ci::Surface8uRef LogoProjectApp::convertToSurface( const cv::Mat & matrix )
{
    cv::Mat output;
    if( matrix.type() != CV_8UC3 )
    matrix.convertTo( output,CV_8UC3 );
    else
    matrix.copyTo( output );
    
    ci::Surface8uRef outRef = ci::Surface8u::create( ci::Surface8u( (uint8_t*)output.data, output.cols, output.rows, output.step, ci::SurfaceChannelOrder::RGB ) );
    return outRef;
}

void LogoProjectApp::compareChannels(const ci::Channel &current, ci::Channel *prev)
{
    Channel::ConstIter currentIter = current.getIter();
    Channel::Iter prevIter = prev->getIter();
    
    while (currentIter.line() && prevIter.line()) {
        while (currentIter.pixel() && prevIter.pixel()) {
            uint8_t pixelDiff = abs(currentIter.v() - prevIter.v());
            if (pixelDiff > mThreshold) {
                prevIter.v() = 255;
            } else {
                prevIter.v() = 0;
            }
        }
    }
    
//    std::cout << "got through the comparison" << std::endl;
}

void LogoProjectApp::compareSurfacesCinderOnly(ci::Surface current, ci::Surface *prev)
{
    ci::Surface::ConstIter prevIter(prev->getIter());
    ci::Surface::ConstIter currentIter(current.getIter());
    
    while (prevIter.line()) {
        currentIter.line();
        while(prevIter.pixel()) {
            currentIter.pixel();
            uint8_t red = abs(prevIter.r() - currentIter.r());
            uint8_t green = abs(prevIter.g() - currentIter.g());
            uint8_t blue = abs(prevIter.b() - currentIter.b());
            
            uint8_t diff = red + green + blue;
            
            if (diff > mThreshold) {
//                prevIter.r() = 255;
            }
        }
    }
    
}

void LogoProjectApp::compareSurfacesOpenCV(cv::Mat &current, cv::Mat &prev)
{
    
}



CINDER_APP( LogoProjectApp, RendererGl )
