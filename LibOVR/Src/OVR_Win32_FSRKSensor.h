/************************************************************************************

Filename    :   OVR_Win32_FSRKSensor.h
Content     :   Sensor device header interfacing to Freespace FSRK-USB-2 through
                raw system I/O.
Created     :   September 21, 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_Win32_DeviceManager.h"

namespace OVR { namespace Win32 { 

struct FRMessage;

//-------------------------------------------------------------------------------------
// FSRKSensorDeviceFactory enumerates FSRK-USB2 Devices.
class FSRKSensorDeviceFactory : public DeviceFactory
{
public:
    static FSRKSensorDeviceFactory Instance;

    // Enumerates devices, creating and destroying relevant objects in manager.
    virtual void EnumerateDevices(EnumerateVisitor& visitor);

protected:
    DeviceManager* getManager() const { return (DeviceManager*) pManager; }   
};


// Describes a single a FSRK-USB2 Devices and supports creating its instance.
class FSRKSensorDeviceCreateDesc : public DeviceCreateDesc
{
public:
    FSRKSensorDeviceCreateDesc(DeviceFactory* factory, const HIDDeviceDesc& hidDesc)
        : DeviceCreateDesc(factory, Device_Sensor), HIDDesc(hidDesc) { }
    FSRKSensorDeviceCreateDesc(const FSRKSensorDeviceCreateDesc& other)
        : DeviceCreateDesc(other.pFactory, Device_Sensor), HIDDesc(other.HIDDesc) { }

    HIDDeviceDesc HIDDesc;
    
    virtual DeviceCreateDesc* Clone() const
    {
        return new FSRKSensorDeviceCreateDesc(*this);
    }

    virtual DeviceBase* NewDeviceInstance();

    virtual bool MatchDevice(const DeviceCreateDesc& other) const
    {
        if (other.Type != Device_Sensor)
            return false;
        const FSRKSensorDeviceCreateDesc& s2 = (const FSRKSensorDeviceCreateDesc&) other;
        return (HIDDesc.Path == s2.HIDDesc.Path) &&
               (HIDDesc.SerialNumber == s2.HIDDesc.SerialNumber);
    }

    virtual bool        GetDeviceInfo(DeviceInfo* info) const;
};


//-------------------------------------------------------------------------------------

class FSRKSensorDevice : public DeviceImpl<OVR::SensorDevice>,
                         public DeviceManagerThread::Notifier
{
public:
     FSRKSensorDevice(FSRKSensorDeviceCreateDesc* createDesc);
    ~FSRKSensorDevice();

    // DeviceCommaon interface
    virtual bool Initialize(DeviceBase* parent);
    virtual void Shutdown();    

    virtual void SetMessageHandler(MessageHandler* handler);

    // DeviceManager::OverlappedNotify
    virtual void OnOverlappedEvent(HANDLE hevent); 

    // HMD-Mounted sensor has a different coordinate frame.
    virtual void SetCoordinateFrame(CoordinateFrame coordframe)
    { Coordinates = coordframe; }
    virtual CoordinateFrame GetCoordinateFrame() const
    { return Coordinates; }


    virtual bool SetRange(const SensorRange& range, bool waitFlag);
    virtual void GetRange(SensorRange* range) const;

    //virtual UPInt WriteCommand(UByte* data, UPInt size, bool waitFlag);
    virtual bool  SetFeature(UByte* data, UPInt size, bool waitFlag);
    virtual bool  GetFeature(UByte* data, UPInt size);

protected:
    void        initializeRead();
    bool        processReadResult();

    // Called for decoded messages
    void        onFRMessage(FRMessage* message);

    // Helpers to reduce casting.
    FSRKSensorDeviceCreateDesc* getCreateDesc() const
    { return (FSRKSensorDeviceCreateDesc*)pCreateDesc.GetPtr(); }

    HIDDeviceDesc* getHIDDesc() const
    { return &getCreateDesc()->HIDDesc; }
    
    Win32::DeviceManager* getManagerImpl() const
    { return (DeviceManager*)DeviceImpl<OVR::SensorDevice>::GetManager(); }

    enum { ReadBufferSize = 96 };

    // Set if the sensor is located on the HMD.
    CoordinateFrame Coordinates;

    bool          SequenceValid;
    UInt16        LastSequence;

    // Handle to open device, or null.
    HANDLE        hDev;    

    // OVERLAPPED data structure servicing messages to be sent.
    OVERLAPPED    ReadOverlapped;
    bool          ReadRequested;
    UByte         ReadBuffer[ReadBufferSize];
};


}} // namespace OVR::Win32

