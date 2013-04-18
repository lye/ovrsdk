/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_LatencyTestUtil.h
Content     :   Wraps the lower level LatencyTesterDevice and adds functionality.
Created     :   February 14, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_LatencyTestUtil_h
#define OVR_LatencyTestUtil_h

#include "OVR_Device.h"

namespace OVR {


//-------------------------------------------------------------------------------------
// ***** LatencyTestUtil
//
// LatencyTestUtil wraps the low level LatencyTestDevice and manages the scheduling
// of a latency test. A single test is composed of a series of individual latency measurements
// which are used to derive min, max, and an average latency value. This is necessary since
// individual latency measurements naturally exhibit random variation up to 16mS based on
// how they interact with the display scan.
//
// Developers are required to call the following methods:
//      SetDevice - Sets the LatencyTestDevice to be used for the tests.
//      ProcessInputs - This should be called at the same place in the code where the game engine
//                      reads the headset orientation from LibOVR (typically done by calling
//                      'GetOrientation' on the SensorFusion object). Calling this at the right time
//                      enables us to measure the same latency that occurs for headset orientation
//                      changes.
//      DisplayScreenColor -    The latency tester works by sensing the color of the pixels directly
//                              beneath it. The color of these pixels can be set by drawing a small
//                              quad at the end of the rendering stage. The quad should be small
//                              such that it doesn't significantly impact the rendering of the scene,
//                              but large enough to be 'seen' by the sensor. See the SDK
//                              documentation for more information.

class LatencyTestUtil : public NewOverrideBase
{
public:
    LatencyTestUtil(LatencyTestDevice* device = NULL);
    ~LatencyTestUtil();
    
    // Set the Latency Tester device that we'll use to send commands to and receive
    // notification messages from.
    bool        SetDevice(LatencyTestDevice* device);
    void        SetNumberOfSamples(UInt32 numberOfSamples);

    // Returns true if this LatencyTestUtil has a Latency Tester device.
    bool        HasDevice() const
    { return Handler.IsHandlerInstalled(); }

    void        ProcessInputs();
    bool        DisplayScreenColor(ColorRGB& colorToDisplay);

private:
    LatencyTestUtil* getThis()  { return this; }
    
    void handleMessage(const Message& msg);

    class LatencyTestHandler : public MessageHandler
    {
        LatencyTestUtil*    pLatencyTestUtil;
    public:
        LatencyTestHandler(LatencyTestUtil* latencyTester) : pLatencyTestUtil(latencyTester) { }
        ~LatencyTestHandler();

        virtual void OnMessage(const Message& msg);
    };

    void TransitionToWaitingForStartColorToSettle();
    void TransitionToWaitingForChangeColorSignal();
    void ProcessResults();
    void UpdateForTimeouts();

    Ptr<LatencyTestDevice>    Device;
    UInt32                      NumberOfSamples;

    LatencyTestHandler        Handler;

    enum TesterState
    {
        State_Default,
        State_WaitingForStartColorToSettle,
        Scene_WaitingForChangeColorSignal,
        Scene_WaitingForColorDetectedSignal
    };
    TesterState             State;

    UInt32                  ActiveTimerMillis;
    ColorRGB                RenderColorSignalled;
    ColorRGB                RenderColor;

    Array<int>              Results;
};

} // namespace OVR

#endif // OVR_LatencyTestUtil_h
