#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rand.h"

#include "ParticleSystem.h"
#include "opencv2/core/core.hpp"

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

    cv::Mat depthMatrix;
    
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
}

void LogoProjectApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    mParticles->draw();
}

CINDER_APP( LogoProjectApp, RendererGl )
