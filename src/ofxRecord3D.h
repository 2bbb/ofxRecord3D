#pragma once

#ifndef OFX_RECORD_3D_H
#define OFX_RECORD_3D_H
#endif

#include "ofLog.h"
#include "ofImage.h"
#include "ofThreadChannel.h"

#include <record3d/Record3DStream.h>

struct ofxRecord3D {
    struct Frame {
        Record3D::IntrinsicMatrixCoeffs intrinsic;
        ofImage color;
        ofFloatImage depth;
    };
    
    Record3D::DeviceInfo currentDevice;
    std::vector<Record3D::DeviceInfo> getConnectedDevices() const
    { return Record3D::Record3DStream::GetConnectedDevices(); };
    
    bool setup(std::size_t deviceIndex) {
        const auto &devices = Record3D::Record3DStream::GetConnectedDevices();
        if(devices.empty()) {
            ofLogError("ofxRecord3D") <<
                     "No iOS devices found. Ensure you have connected your iDevice via USB to this computer.";
            return false;
        } else if(devices.size() <= deviceIndex){
            ofLogError("ofxRecord3D") <<
                     "deviceIndex is out of bounds: found " << devices.size() << " devices";
            return false;
        }
        return setup(devices[deviceIndex]);
    }
    
    bool setup(Record3D::DeviceInfo selectedDevice) {
        stream.onStreamStopped = [=] {
            onStreamStopped();
        };
        stream.onNewFrame = [=](const Record3D::BufferRGB &rgbFrame,
                                const Record3D::BufferDepth &depthFrame,
                                uint32_t rgbWidth,
                                uint32_t rgbHeight,
                                uint32_t depthWidth,
                                uint32_t depthHeight,
                                Record3D::DeviceType deviceType,
                                Record3D::IntrinsicMatrixCoeffs K)
        {
            onNewFrame(rgbFrame,
                       depthFrame,
                       rgbWidth,
                       rgbHeight,
                       depthWidth,
                       depthHeight,
                       deviceType,
                       K);
        };

        ofLogNotice("ofxRecord3D") << "Trying to connect to device with ID: " <<  selectedDevice.productId;
        
        bool isConnected = stream.ConnectToDevice(selectedDevice);
        if(!isConnected) {
            ofLogWarning("ofxRecord3D") << "Could not connect to iDevice. Make sure you have opened the Record3D iOS app (https://record3d.app/).";
        } else {
            currentDevice = selectedDevice;
        }
        return isConnected;
    }
    
    void disconnect() {
        stream.Disconnect();
        currentDevice = {};
    }
    
    bool isConnected() const
    { return currentDevice.udid != ""; };
    
    bool isLidar() const
    { return currentDeviceType == Record3D::R3D_DEVICE_TYPE__LIDAR; };
    
    bool getFrame(Frame &frame) {
        if(frameQueue.empty()) return false;
        while(!frameQueue.empty()) frameQueue.receive(frame);
        frame.color.setUseTexture(true);
        frame.color.update();
        frame.depth.setUseTexture(true);
        frame.depth.update();
        return true;
    }
    
protected:
    Record3D::Record3DStream stream;
    Record3D::DeviceType currentDeviceType;
    ofThreadChannel<Frame> frameQueue;
    
    void onStreamStopped() {
        ofLogError("ofxRecord3D") << "Stream stopped!";
    }
    
    void onNewFrame(const Record3D::BufferRGB &rgbFrame,
                    const Record3D::BufferDepth &depthFrame,
                    uint32_t rgbWidth,
                    uint32_t rgbHeight,
                    uint32_t depthWidth,
                    uint32_t depthHeight,
                    Record3D::DeviceType deviceType,
                    Record3D::IntrinsicMatrixCoeffs K)
    {
        currentDeviceType = deviceType;
        Frame frame;
        frame.intrinsic = K;
        
        frame.color.setUseTexture(false);
        frame.color.setFromPixels(rgbFrame.data(),
                                  rgbWidth,
                                  rgbHeight,
                                  OF_IMAGE_COLOR);
        
        frame.depth.setUseTexture(false);
        frame.depth.setFromPixels((float *)depthFrame.data(),
                                  depthWidth,
                                  depthHeight,
                                  OF_IMAGE_GRAYSCALE);
        
        frameQueue.send(std::move(frame));
    }

};
