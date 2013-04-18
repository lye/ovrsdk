/************************************************************************************

Filename    :   OVR_Win32_HMDDevice.h
Content     :   Win32 HMDDevice implementation
Created     :   September 21, 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#ifndef OVR_Win32_HMDDevice_h
#define OVR_Win32_HMDDevice_h

#include "OVR_Win32_DeviceManager.h"

namespace OVR { namespace Win32 {

class HMDDevice;


//-------------------------------------------------------------------------------------

// HMDDeviceFactory enumerates attached Oculus HMD devices.
//
// This is currently done by matching montior device strings.

class HMDDeviceFactory : public DeviceFactory
{
public:
    static HMDDeviceFactory Instance;

    // Enumerates devices, creating and destroying relevant objects in manager.
    virtual void EnumerateDevices(EnumerateVisitor& visitor);

protected:
    DeviceManager* getManager() const { return (DeviceManager*) pManager; }
};


class HMDDeviceCreateDesc : public DeviceCreateDesc
{
    friend class HMDDevice;
public:
    HMDDeviceCreateDesc(DeviceFactory* factory, 
                        const String& deviceId, const String& displayDeviceName)
        : DeviceCreateDesc(factory, Device_HMD),
          DeviceId(deviceId), DisplayDeviceName(displayDeviceName),
          HResolution(0), VResolution(0), HScreenSize(0), VScreenSize(0) { }
    HMDDeviceCreateDesc(const HMDDeviceCreateDesc& other)
        : DeviceCreateDesc(other.pFactory, Device_HMD),
          DeviceId(other.DeviceId), DisplayDeviceName(other.DisplayDeviceName), 
          HResolution(other.HResolution), VResolution(other.VResolution),
          HScreenSize(other.HScreenSize), VScreenSize(other.VScreenSize) { }

    virtual DeviceCreateDesc* Clone() const
    {
        return new HMDDeviceCreateDesc(*this);
    }

    virtual DeviceBase* NewDeviceInstance();

    virtual bool MatchDevice(const DeviceCreateDesc& other) const
    {
        if (other.Type != Device_HMD)
            return false;
        const HMDDeviceCreateDesc& s2 = (const HMDDeviceCreateDesc&) other;
        return (DeviceId == s2.DeviceId) &&
               (DisplayDeviceName == s2.DisplayDeviceName);
    }

    virtual bool GetDeviceInfo(DeviceInfo* info) const;

    void  SetScreenParameters(unsigned hres, unsigned vres, float hsize, float vsize)
    {
        HResolution = hres;
        VResolution = vres;
        HScreenSize = hsize;
        VScreenSize = vsize;
    }

    bool IsSLA1() const;

protected:
    String      DeviceId;
    String      DisplayDeviceName;
    unsigned    HResolution, VResolution;
    float       HScreenSize, VScreenSize;
};


//-------------------------------------------------------------------------------------

// HMDDevice represents an Oculus HMD device unit. An instance of this class
// is typically created from the DeviceManager.
//  After HMD device is created, we its sensor data can be obtained by 
//  first creating a Sensor object and then wrappig it in SensorFusion.

class HMDDevice : public DeviceImpl<OVR::HMDDevice>
{
public:
    HMDDevice(HMDDeviceCreateDesc* createDesc);
    ~HMDDevice();

    virtual bool Initialize(DeviceBase* parent);
    virtual void Shutdown();

    // Query associated sensor.
    virtual OVR::SensorDevice* GetSensor();  
};


}} // namespace OVR::Win32

#endif // OVR_Win32_HMDDevice_h

