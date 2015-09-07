#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class LogoProjectApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
    GLuint mVAO;
    GLuint mPositionBufferA, mPositionBufferB;
    
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

    //  create and bind the vao
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);

    //  array of values
    const int particleCount = 10000;
    GLfloat positionData[particleCount * 7]; // two slots for position, two for velocity, three for color
    
    for (int i = 0; i < particleCount; i++) {
        
        //  position is completely random
        float randNum = (float)(rand()) / (float)(RAND_MAX);
        randNum *= 2.0;
        randNum -= 1.0;
        float randNum2 = (float)(rand()) / (float)(RAND_MAX);
        randNum2 *= 2.0;
        randNum2 -= 1.0;
        
        //  velocity is normalized vector
        float randNum3 = (float)(rand()) / (float)(RAND_MAX);
        randNum3 *= M_PI * 2.0f;
        
        positionData[(i*7) + 0] = randNum;              // pos.x
        positionData[(i*7) + 1] = randNum2;             // pos.y
        
        positionData[(i*7) + 2] = cos(randNum3) * 0.03; // vel.x
        positionData[(i*7) + 3] = sin(randNum3) * 0.03; // vel.y
        
        positionData[(i*7) + 4] = (float)(rand()) / (float)(RAND_MAX);  // col.r
        positionData[(i*7) + 5] = (float)(rand()) / (float)(RAND_MAX);  // col.g
        positionData[(i*7) + 6] = (float)(rand()) / (float)(RAND_MAX);  // col.b
    }

    std::cout << "Sizeof positionData: " << sizeof(positionData) << std::endl;
    
    //  create two buffers to ping-pong back and forth with position data
    glGenBuffers(1, &mPositionBufferA);
    glBindBuffer(GL_ARRAY_BUFFER, mPositionBufferA);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), positionData, GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &mPositionBufferB);
    glBindBuffer(GL_ARRAY_BUFFER, mPositionBufferB);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positionData), 0, GL_STREAM_DRAW); // don't initialize immediately
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // before linking program, specify which output attributes we want to capture into a buffer
    const GLchar * feedbackVaryings[] = {"outPos", "outVel", "outCol"};
    glTransformFeedbackVaryings(mShaderProg, 3, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glBindFragDataLocation(mShaderProg, 0, "outColor");
    // now link and activate the program
    glLinkProgram(mShaderProg);
    glUseProgram(mShaderProg);

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
}

CINDER_APP( LogoProjectApp, RendererGl )
