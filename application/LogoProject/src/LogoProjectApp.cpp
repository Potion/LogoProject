#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"
#include "cinder/Capture.h"
#include "cinder/Log.h"

#include "ParticleSystem.h"
#include "opencv2/core/core.hpp"
//#include "opencv2/video/video.hpp"
#include "opencv2/videoio/videoio.hpp"
//#include "opencv2/opencv.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class LogoProjectApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    void printDevices();
    CaptureRef          mCapture;
    gl::TextureRef      mTexture;
    
    ParticleSystemRef mParticles;
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
    
    std::cout << "Got this far" << std::endl;
}

void LogoProjectApp::mouseDown( MouseEvent event )
{
}

void LogoProjectApp::mouseMove( MouseEvent event )
{
    mParticles->updateMouse(event.getPos());
}

void LogoProjectApp::update()
{
    if( mCapture && mCapture->checkNewFrame() ) {
        std::cout << "Frame # " << ci::app::getElapsedFrames() << " got new frame" << std::endl;
        if( ! mTexture ) {
            // Capture images come back as top-down, and it's more efficient to keep them that way
            mTexture = gl::Texture::create( *mCapture->getSurface(), gl::Texture::Format().loadTopDown() );
        }
        else {
            mTexture->update( *mCapture->getSurface() );
        }
    } else {
        if (!mCapture) {
            std::cout << "Frame # " << ci::app::getElapsedFrames() << "no mCapture" << std::endl;
        }
        
        if (!mCapture->checkNewFrame()) {
            std::cout << "Frame # " << ci::app::getElapsedFrames() << " mCapture exists, but no new frame" << std::endl;
        }
    }
}

void LogoProjectApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    if( mTexture ) {
        gl::ScopedModelMatrix modelScope;
        gl::draw(mTexture);
    }
//    mParticles->draw();
}

void LogoProjectApp::printDevices()
{
    std::cout << "Getting devices, number attached: " <<  Capture::getDevices().size() << std::endl;
    for( const auto &device : Capture::getDevices() ) {
        console() << "Device: " << device->getName() << endl;
    }
}

CINDER_APP( LogoProjectApp, RendererGl )
