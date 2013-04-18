/************************************************************************************

Filename    :   OVR_LatencyTestUtil.cpp
Content     :   Wraps the lower level LatencyTester interface and adds functionality.
Created     :   February 14, 2013
Authors     :   Lee Cooper

Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_LatencyTestUtil.h"

#include "Kernel/OVR_Log.h"
#include "Kernel/OVR_Timer.h"

namespace OVR {

static const UInt32     DEFAULT_NUMBER_OF_SAMPLES = 10;
static const UInt32     TIME_TO_WAIT_FOR_FIRST_COLOR_TO_SETTLE_MILLIS = 100;
static const UInt32     TIMEOUT_WAITING_FOR_START_SIGNAL_MILLIS = 100;
static const ColorRGB   START_COLOR(0, 0, 0);
static const ColorRGB   END_COLOR(255, 255, 255);
static const ColorRGB   SENSOR_DETECT_THRESHOLD(128, 255, 255);

//-------------------------------------------------------------------------------------
// ***** LatencyTestUtil

LatencyTestUtil::LatencyTestUtil(LatencyTestDevice* device)
 :  Handler(getThis()), State(State_Default), NumberOfSamples(DEFAULT_NUMBER_OF_SAMPLES)
{
    if (device != NULL)
    {
        SetDevice(device);
    }
}

LatencyTestUtil::~LatencyTestUtil()
{
}

bool LatencyTestUtil::SetDevice(LatencyTestDevice* device)
{

    if (device != Device)
    {
        if (device != NULL)
        {
            if (device->GetMessageHandler() != NULL)
            {
                OVR_DEBUG_LOG(
                    ("LatencyTestUtil::AttachToDevice failed - device %p already has handler", device));
                return false;
            }
        }

        if (Device != NULL)
        {
            Device->SetMessageHandler(0);
        }
        Device = device;

        if (Device != NULL)
        {
            Device->SetMessageHandler(&Handler);

            // Set trigger threshold.
            LatencyTestConfiguration configuration(SENSOR_DETECT_THRESHOLD, false);     // No samples streaming.
            Device->SetConfiguration(configuration, true);
        }
    }

    return true;
}

void LatencyTestUtil::SetNumberOfSamples(UInt32 numberOfSamples)
{
    NumberOfSamples = numberOfSamples;
}

void LatencyTestUtil::handleMessage(const Message& msg)
{

    // For debugging.
/*  if (msg.Type == Message_LatencyTestSamples)
    {
        MessageLatencyTestSamples* pSamples = (MessageLatencyTestSamples*) &msg;

        if (pSamples->Samples.GetSize() > 0)
        {
            // Just show the first one for now.
            ColorRGB c = pSamples->Samples[0];
            OVR_DEBUG_LOG(("%d %d %d", c.R, c.G, c.B));
        }
        return;
    }
*/


    if (msg.Type == Message_DeviceRemoved)
    {
        // Reset.
        State = State_Default;
    }
    else if (State == State_Default)
    {
        if (msg.Type == Message_LatencyTestButton)
        {
            TransitionToWaitingForStartColorToSettle();
            OVR_DEBUG_LOG(("** 1 - Initiated."));
        }
    }
    else if (State == Scene_WaitingForChangeColorSignal)
    {        
        if (msg.Type == Message_LatencyTestChangeColor)
        {
            // Set screen to the color specified in the message.
            MessageLatencyTestChangeColor* pChange = (MessageLatencyTestChangeColor*) &msg;
            RenderColorSignalled = pChange->TargetValue;

            State = Scene_WaitingForColorDetectedSignal;

            OVR_DEBUG_LOG(("** 3 - Received 'change color' signal."));
        }
    }
    else if (State == Scene_WaitingForColorDetectedSignal)
    {
        if (msg.Type == Message_LatencyTestColorDetected)
        {
            MessageLatencyTestColorDetected* pDetected = (MessageLatencyTestColorDetected*) &msg;
            UInt16 elapsedTime = pDetected->Elapsed;
            OVR_DEBUG_LOG(("** 4 - Received 'color detected'. Result = %d", elapsedTime));
            Results.PushBack(elapsedTime);

            if (Results.GetSize() < NumberOfSamples)
            {
                // Take another measurement.
                TransitionToWaitingForStartColorToSettle();
            }
            else
            {
                // We're done.
                ProcessResults();

                Results.Clear();
                State = State_Default;
            }
        }
    }
}

LatencyTestUtil::LatencyTestHandler::~LatencyTestHandler()
{
    RemoveHandlerFromDevices();
}

void LatencyTestUtil::LatencyTestHandler::OnMessage(const Message& msg)
{
    pLatencyTestUtil->handleMessage(msg);
}

void LatencyTestUtil::ProcessInputs()
{
    UpdateForTimeouts();

    RenderColor = RenderColorSignalled;
}

bool LatencyTestUtil::DisplayScreenColor(ColorRGB& colorToDisplay)
{
    UpdateForTimeouts();

    if (State == State_Default)
    {
        return false;
    }

    colorToDisplay = RenderColor;
    return true;
}

void LatencyTestUtil::ProcessResults()
{
    int minTime = INT_MAX;
    int maxTime = INT_MIN;

    float averageTime = 0.0f;

    for (UInt32 i=0; i<Results.GetSize(); i++)
    {
        int res = Results[i];

        minTime = min(res, minTime);
        maxTime = max(res, maxTime);

        averageTime += (float) res;
    }

    averageTime /= (float) Results.GetSize();

    LogText("LATENCY TESTER - min:%d max:%d average:%.2f [", minTime, maxTime, averageTime);    
    for (UInt32 i=0; i<Results.GetSize(); i++)
    {
        LogText("%d", Results[i]);
        if (i != Results.GetSize()-1)
        {
            LogText(",");
        }
    }
    LogText("]\n");
}

void LatencyTestUtil::UpdateForTimeouts()
{
    UInt32 timeMillis = Timer::GetTicksMs();

    if (State == State_WaitingForStartColorToSettle)
    {
        if (timeMillis > ActiveTimerMillis)
        {
            TransitionToWaitingForChangeColorSignal();
            OVR_DEBUG_LOG(("** 2 - Send 'start test' signal."));
        }
    }
    else if (State == Scene_WaitingForChangeColorSignal)
    {
        if (timeMillis > ActiveTimerMillis)
        {
            OVR_DEBUG_LOG(("** ! - Timed out waiting for 'change color' signal. Resend 'start test'."));
            TransitionToWaitingForChangeColorSignal();
        }
    }
}

void LatencyTestUtil::TransitionToWaitingForStartColorToSettle()
{
    // Set screen to black and wait a while for it to settle.
    ActiveTimerMillis = Timer::GetTicksMs() + TIME_TO_WAIT_FOR_FIRST_COLOR_TO_SETTLE_MILLIS;

    RenderColorSignalled = START_COLOR;
    RenderColor = START_COLOR;
    State = State_WaitingForStartColorToSettle;
}

void LatencyTestUtil::TransitionToWaitingForChangeColorSignal()
{
    LatencyTestStartTest start(END_COLOR);
    Device->SetStartTest(start, true);
    ActiveTimerMillis = Timer::GetTicksMs() + TIMEOUT_WAITING_FOR_START_SIGNAL_MILLIS;
    State = Scene_WaitingForChangeColorSignal;
}

} // namespace OVR

