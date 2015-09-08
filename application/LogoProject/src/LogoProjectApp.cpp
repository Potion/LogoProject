#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "ParticleSystem.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LogoProjectApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    ParticleSystemRef mParticles;
};

void LogoProjectApp::setup()
{
    //  Add correct asset path
    ci::fs::path relativePath = "../../../../../assets";
    ci::fs::path absolutePath = ci::fs::canonical(relativePath);
    absolutePath.make_preferred().native();
    ci::app::addAssetDirectory(relativePath);

    
    //mShaderProg = gl::GlslProg::create( loadAsset( "vertex.glsl" ), loadAsset( "frag.glsl" ) );
    
    //  print out OpenGL version
    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    mParticles = ParticleSystem::create();

}

void LogoProjectApp::mouseDown( MouseEvent event )
{
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
