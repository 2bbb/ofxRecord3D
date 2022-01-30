#include "ofMain.h"
#include "ofxRecord3D.h"

class ofApp : public ofBaseApp {
    ofxRecord3D record3d;
    ofxRecord3D::Frame frame;
public:
    void setup() {
        const auto &devices = record3d.getConnectedDevices();
        if(devices.size()) {
            const auto &device = devices.front();
            bool res = record3d.setup(device);
            if(!res) {
                ofLogError("can't connect to device") << device.udid;
                ofExit();
            }
        } else {
            ofLogError("no connected devices");
            ofExit();
        }
    }
    void update() {
        if(record3d.getFrame(frame)) {
            ofLogNotice() << "received frame";
        }
    }
    void draw() {
        if(frame.color.isAllocated()) {
            frame.color.draw(0.0f, 0.0f);
            frame.depth.draw(0.0f, 0.0f);
        }
    }
    void exit() {
        
    }
    
    void keyPressed(int key) {}
    void keyReleased(int key) {}
    void mouseMoved(int x, int y) {}
    void mouseDragged(int x, int y, int button) {}
    void mousePressed(int x, int y, int button) {}
    void mouseReleased(int x, int y, int button) {}
    void mouseEntered(int x, int y) {}
    void mouseExited(int x, int y) {}
    void windowResized(int w, int h) {}
    void dragEvent(ofDragInfo dragInfo) {}
    void gotMessage(ofMessage msg) {}
};

int main() {
    ofSetupOpenGL(1024, 768, OF_WINDOW);
    ofRunApp(new ofApp());
}
