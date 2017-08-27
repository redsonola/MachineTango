#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MagneticGardelApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void MagneticGardelApp::setup()
{
}

void MagneticGardelApp::mouseDown( MouseEvent event )
{
}

void MagneticGardelApp::update()
{
}

void MagneticGardelApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( MagneticGardelApp, RendererGl )
