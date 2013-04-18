/************************************************************************************

Filename    :   OVR_Win32_FSRKSensor.cpp
Content     :   Sensor device implementation that talks to Freespace FSRK-USB-2                
                module through direct system I/O.
Created     :   October 23, 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_Win32_FSRKSensor.h"

#include "Kernel/OVR_Timer.h"


namespace OVR { namespace Win32 {


//-------------------------------------------------------------------------------------
// ***** FSRKUSB2-specific packet data structures

enum {
    FSRK_VendorId  = 0x1D5A,
    FSRK_ProductId = 0xC080    
};

enum FRPacketSelect
{
    FRPacket_None               = 0,
    FRPacket_Mouse              = 1,
    FRPacket_BodyFrame          = 2,
    FRPacket_UserFrame          = 3,
    FRPacket_BodyUserFrame      = 4,
    FRPacket_DCEOut             = 5, // Doesn't work.
    FRPacket_MotionEngineOutput = 8
};


// Need minimal communication data structures:
//  1) DATAMODECONTROLV2REQUEST to enable BodyFrame
//  2) MessageBodyFrame

struct FRDMControlV2Request
{    
    enum {
        PacketSize = 9,
        BufferSize = 12
    };
    UByte Buffer[BufferSize];
    
    FRDMControlV2Request(FRPacketSelect packetSelect, UByte modeAndStatus, UByte format = 0)
    {
        memset(Buffer, 0, BufferSize);
        Buffer[0] = 7;
        Buffer[1] = PacketSize;
        Buffer[2] = 4;          // Destination 'reserved address' value of 4.
        Buffer[3] = 0;          // Source for 'system host'.
        
        Buffer[4] = 20;         // DATAMODECONTROLV2REQUEST
        Buffer[5] = modeAndStatus;
        Buffer[6] = (UByte)packetSelect;
        Buffer[7] = format;
        Buffer[8] = 0; // ff0... ff7 bits
    }
};


// Reported data is little-endian now
UInt16 DecodeUInt16(const UByte* buffer)
{
    return (UInt16(buffer[1]) << 8) | UInt16(buffer[0]);
}
SInt16 DecodeSInt16(const UByte* buffer)
{
    return (SInt16(buffer[1]) << 8) | SInt16(buffer[0]);
}


// Messages we care for
enum FRMessageType
{
    FRMessage_None                = 0,
    FRMessage_BodyFrame           = 1,
    FRMessage_DMControlV2Response = 2,
    FRMessage_Unknown             = 0x100,
    FRMessage_SizeError           = 0x101,
};

struct FRBodyFrame
{
    SByte   DeltaX, DeltaY;
    SByte   DeltaWheel;

    UInt16  SequenceNumber;
    SInt16  LinearAccelX, LinearAccelY, LinearAccelZ;
    SInt16  AngularVelX,  AngularVelY,  AngularVelZ;


    FRMessageType Decode(const UByte* buffer, int size)
    {
        if (size < 22)
            return FRMessage_SizeError;

        DeltaX     = (SByte)buffer[5];
        DeltaY     = (SByte)buffer[6];
        DeltaWheel = (SByte)buffer[7];

        SequenceNumber= DecodeUInt16(buffer + 8);
        LinearAccelX  = DecodeSInt16(buffer + 10);
        LinearAccelY  = DecodeSInt16(buffer + 12);
        LinearAccelZ  = DecodeSInt16(buffer + 14);
        AngularVelX   = DecodeSInt16(buffer + 16);
        AngularVelY   = DecodeSInt16(buffer + 18);
        AngularVelZ   = DecodeSInt16(buffer + 20);

        return FRMessage_BodyFrame;
    }
};

struct FRDMControlV2Responce
{
    UByte          ModeAndStatus;
    FRPacketSelect PacketSelect;

    FRMessageType Decode(const UByte* buffer, int size)
    {
        if (size < 9)
            return FRMessage_SizeError;
        ModeAndStatus = buffer[5];
        PacketSelect  = (FRPacketSelect)buffer[6];

        return FRMessage_DMControlV2Response;
    }
};

struct FRMessage
{
    FRMessageType Type;
    union {
        FRBodyFrame             BodyFrame;
        FRDMControlV2Responce   DMControlV2Responce;
    };
};

bool DecodeFRMessage(FRMessage* message, UByte* buffer, int size)
{
    memset(message, 0, sizeof(FRMessage));

    if (size < 4)
    {
        message->Type = FRMessage_SizeError;
        return false;
    }

    if (buffer[0] == 32)
    {
        message->Type = message->BodyFrame.Decode(buffer, size);
    }
    else if ((buffer[0] == 5) && (buffer[4] == 20))
    {
        message->Type = message->DMControlV2Responce.Decode(buffer, size);
    }

    return (message->Type < FRMessage_Unknown) && (message->Type != FRMessage_None);
}


//-------------------------------------------------------------------------------------
// ***** FSRKSensorDeviceFactory

FSRKSensorDeviceFactory FSRKSensorDeviceFactory::Instance;


void FSRKSensorDeviceFactory::EnumerateDevices(EnumerateVisitor& visitor)
{

    class FSKSensorEnumerator : public HIDEnumerateVisitor
    {
        // Assign not supported; suppress MSVC warning.
        void operator = (const FSKSensorEnumerator&) { }

        DeviceFactory*     pFactory;
        EnumerateVisitor&  ExternalVisitor;   
    public:
        FSKSensorEnumerator(DeviceFactory* factory, EnumerateVisitor& externalVisitor)
            : pFactory(factory), ExternalVisitor(externalVisitor) { }

        virtual bool MatchVendorProduct(UInt16 vendorId, UInt16 productId)
        {
            return (vendorId == FSRK_VendorId) && (productId == FSRK_ProductId);
        }

        virtual void Visit(const HIDDeviceDesc& desc)
        {
            FSRKSensorDeviceCreateDesc createDesc(pFactory, desc);          
            ExternalVisitor.Visit(createDesc);
            
            //  LogText("FSK Device found. Path=\"%s\"\n", path.ToCStr());
        }
    };

    //double start = Timer::GetProfileSeconds();

    FSKSensorEnumerator sensorEnumerator(this, visitor);
    getManager()->HIDInterface.Enumerate(&sensorEnumerator);

    //double totalSeconds = Timer::GetProfileSeconds() - start; 
}


//-------------------------------------------------------------------------------------
// ***** FSRKSensorDeviceCreateDesc

DeviceBase* FSRKSensorDeviceCreateDesc::NewDeviceInstance()
{
    return new FSRKSensorDevice(this);
}

bool FSRKSensorDeviceCreateDesc::GetDeviceInfo(DeviceInfo* info) const
{
    if ((info->InfoClassType != Device_Sensor) &&
        (info->InfoClassType != Device_None))
        return false;

    OVR_strcpy(info->ProductName, DeviceInfo::MaxNameLength, HIDDesc.Product.ToCStr());
    OVR_strcpy(info->Manufacturer,DeviceInfo::MaxNameLength, HIDDesc.Manufacturer.ToCStr());
    info->Type    = Device_Sensor;
    info->Version = 0;

    if (info->InfoClassType == Device_Sensor)
    {
        SensorInfo* sinfo = (SensorInfo*)info;
        sinfo->VendorId  = HIDDesc.VendorId;
        sinfo->ProductId = HIDDesc.ProductId;
        sinfo->MaxRanges = SensorRange(4.0f * 9.81f, DegreeToRad(2000.0f));
        OVR_strcpy(sinfo->SerialNumber, sizeof(sinfo->SerialNumber),HIDDesc.SerialNumber.ToCStr());
    }
    return true;
}


//-------------------------------------------------------------------------------------
// ***** Freespace::SensorDevice

FSRKSensorDevice::FSRKSensorDevice(FSRKSensorDeviceCreateDesc* createDesc)
    : OVR::DeviceImpl<OVR::SensorDevice>(createDesc, 0),
      Coordinates(SensorDevice::Coord_Sensor),
      hDev(NULL), ReadRequested(false)
{
    SequenceValid = false;
    LastSequence  = 0;
    
    memset(&ReadOverlapped, 0, sizeof(OVERLAPPED));
}

FSRKSensorDevice::~FSRKSensorDevice()
{
    // Check that Shutdown() was called.
    OVR_ASSERT(!pCreateDesc->pDevice);    
}

// Internal creation APIs.
bool FSRKSensorDevice::Initialize(DeviceBase* parent)
{
    HIDDeviceDesc& hidDesc = *getHIDDesc();

    if (ReadBufferSize < hidDesc.InputReportByteLength)
    {
        OVR_ASSERT(false);
        return false;
    }

    DeviceManager*     manager = getManagerImpl();
    Win32HIDInterface& hid     = manager->HIDInterface;

    hDev = hid.CreateHIDFile(hidDesc.Path.ToCStr());
    if (hDev == INVALID_HANDLE_VALUE)
    {
        LogText("OVR::FSRKSensorDevice - Failed to open '%s'\n", hidDesc.Path.ToCStr());
        return false;
    }

    if (!hid.HidD_SetNumInputBuffers(hDev, 128))
    {
        ::CloseHandle(hDev);
        hDev = 0;
        return false;
    }


    // Create a manual-reset non-signaled event
    memset(&ReadOverlapped, 0, sizeof(OVERLAPPED)); 
    ReadOverlapped.hEvent = ::CreateEvent(0, TRUE, FALSE, 0);

    if (!ReadOverlapped.hEvent)
    {
        ::CloseHandle(hDev);
        hDev = 0;
        return false;
    }


    LogText("OVR::FSRKSensorDevice - Opened '%s'\n"
            "                    Manufacturer:'%s'  Product:'%s'  Serial#:'%s'\n",
            hidDesc.Path.ToCStr(),
            hidDesc.Manufacturer.ToCStr(), hidDesc.Product.ToCStr(),
            hidDesc.SerialNumber.ToCStr());


    // TBD: This seems to be required for write
    OVR_ASSERT(FRDMControlV2Request::BufferSize == hidDesc.OutputReportByteLength);

    // Disabling mouse motion.
    FRDMControlV2Request disableMouseRequest(FRPacket_None, 0);
    UPInt written1 = hid.Write(hDev, disableMouseRequest.Buffer, FRDMControlV2Request::BufferSize);
    if (written1 < FRDMControlV2Request::BufferSize)
    {
        LogText("OVR::FSRKSensorDevice - Write failure - DataModeControlV2Request wrote %d bytes\n",
                written1);
    }

    // Request body frame messages.
    FRDMControlV2Request bodyFrameRequest(FRPacket_BodyFrame, (0) | (4 << 1));
    UPInt written2 = hid.Write(hDev, bodyFrameRequest.Buffer, FRDMControlV2Request::BufferSize);
    if (written2 < FRDMControlV2Request::BufferSize)
    {
        LogText("OVR::FSRKSensorDevice - Write failure - DataModeControlV2Request wrote %d bytes\n",
                written2);
    }

    initializeRead();

    // AddRef() to parent, forcing chain to stay alive.
    pParent = parent;
    return true;
}


void FSRKSensorDevice::Shutdown()
{   
    // Remove the handler, if any.
    HandlerRef.SetHandler(0);

    if (ReadRequested)
    {
        DeviceManager* manager = getManagerImpl();
        manager->pThread->RemoveOverlappedEvent(this, ReadOverlapped.hEvent);
        ReadRequested = false;

        // Must call this to avoid Win32 assertion; CloseHandle is not enough.
        ::CancelIo(hDev);
    }

    ::CloseHandle(ReadOverlapped.hEvent);
     memset(&ReadOverlapped, 0, sizeof(OVERLAPPED));

    ::CloseHandle(hDev);
    hDev = 0;
    LogText("OVR::FSRKSensorDevice - Closed '%s'\n", getHIDDesc()->Path.ToCStr());

    pParent.Clear();
}



void FSRKSensorDevice::initializeRead()
{
    DeviceManager* manager = getManagerImpl();

    if (!ReadRequested)
    {        
        manager->pThread->AddOverlappedEvent(this, ReadOverlapped.hEvent);
        ReadRequested = true;
    }
   
    // Read resets the event...
    while(::ReadFile(hDev, ReadBuffer, getHIDDesc()->InputReportByteLength, 0, &ReadOverlapped))
    {
        // Read sets event on completion, so we can proceed without redundant SetEvent.
        //OVR_ASSERT(::WaitForSingleObject(ReadOverlapped.hEvent, 0) == WAIT_OBJECT_0);
        processReadResult();
    }
    
    if (GetLastError() != ERROR_IO_PENDING)
    {
        // Some other error (such as unplugged).
        manager->pThread->RemoveOverlappedEvent(this, ReadOverlapped.hEvent);
        ReadRequested = false;
    }
}


bool FSRKSensorDevice::processReadResult()
{
    OVR_ASSERT(ReadRequested);

    DWORD bytesRead = 0;

    if (GetOverlappedResult(hDev, &ReadOverlapped, &bytesRead, FALSE))
    {
        // We got data.
        FRMessage message;
        if (DecodeFRMessage(&message, ReadBuffer, bytesRead))     
            onFRMessage(&message);

        // TBD: Not needed?
        // Event should be reset by Read call...
        ReadOverlapped.Pointer = 0;
        ReadOverlapped.Internal = 0;
        ReadOverlapped.InternalHigh = 0;
        return true;
    }
    else
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            // Some other error
            DeviceManager* manager = getManagerImpl();
            manager->pThread->RemoveOverlappedEvent(this, ReadOverlapped.hEvent);
            ReadRequested = false;
            return false;
        }
    }

    return false;
}


void FSRKSensorDevice::OnOverlappedEvent(HANDLE hevent)
{
    OVR_UNUSED(hevent); // ASSERT only usage.
    OVR_ASSERT(hevent == ReadOverlapped.hEvent);

    if (processReadResult())
    {
        // Proceed to read further.
        initializeRead();
    }
}


bool FSRKSensorDevice::SetRange(const SensorRange& range, bool waitFlag)
{
    OVR_UNUSED2(range, waitFlag);
    OVR_DEBUG_LOG(("SensorDevice::SetRange not supported for FSRK device"));
    return false;
}

void FSRKSensorDevice::GetRange(SensorRange* range) const
{
    *range = SensorRange(4.0f * 9.81f, DegreeToRad(2000.0f));
}


bool FSRKSensorDevice::SetFeature(UByte* data, UPInt size, bool waitFlag)
{
    OVR_UNUSED3(data, size, waitFlag);
    // For now, don't support writing to FSRK since we don't want
    // writable API ambiguity...
    return false;
}
bool FSRKSensorDevice::GetFeature(UByte* data, UPInt size)
{
    OVR_UNUSED2(data, size);
    return false;
}


void FSRKSensorDevice::SetMessageHandler(MessageHandler* handler)
{
    if (handler)
    {
        SequenceValid = false;
        DeviceBase::SetMessageHandler(handler);       
    }
    else
    {       
        DeviceBase::SetMessageHandler(handler);
    }    
}


// FSRK Sensor reports data in the following coordinate system:
// Accelerometer: 10^-3 m/s^2; X forward, Y right, Z Down.
// Gyro:          10^-3 rad/s; X positive roll right, Y positive pitch up; Z positive yaw right.


// We need to convert it to the following RHS coordinate system:
// X right, Y Up, Z Back (out of screen)
//
Vector3f AccelFromBodyFrameUpdate(const FRBodyFrame& update, bool hmd)
{
    if (hmd)
    {
        return Vector3f( (float)update.LinearAccelX,
                         (float)update.LinearAccelZ,
                        -(float)update.LinearAccelY) * 0.001f;
    }

    return Vector3f( (float)update.LinearAccelY,
                    -(float)update.LinearAccelZ,
                    -(float)update.LinearAccelX) * 0.001f;
}


Vector3f EulerFromBodyFrameUpdate(const FRBodyFrame& update, bool hmd)
{
    if (hmd)
    {
        return Vector3f( (float)update.AngularVelX,
                         (float)update.AngularVelZ,
                        -(float)update.AngularVelY) * 0.001f;

    }

    return Vector3f( (float)update.AngularVelY,
                    -(float)update.AngularVelZ,
                    -(float)update.AngularVelX) * 0.001f;
}


void FSRKSensorDevice::onFRMessage(FRMessage* message)
{
    if (message->Type != FRMessage_BodyFrame)
        return;

    float dt = 0;
    FRBodyFrame& bf = message->BodyFrame;

    if (SequenceValid)
    {
        if (bf.SequenceNumber < LastSequence)
            dt = (float)((((int)bf.SequenceNumber) + 0x10000) - (int)LastSequence);
        else
            dt = (float)(bf.SequenceNumber - LastSequence);
    }
    else
    {
        dt = 1.0f;
        SequenceValid = true;
    }

    LastSequence = bf.SequenceNumber;

    // Call OnMessage() within a lock to avoid conflicts with handlers.
    Lock::Locker scopeLock(HandlerRef.GetLock());

    if (HandlerRef.GetHandler())
    {
        dt *= 1.0f / 250.0f; // Scale to frequency

        MessageBodyFrame mbf(this);
        mbf.TimeDelta    = dt;
        mbf.Acceleration = AccelFromBodyFrameUpdate(bf, (Coordinates == Coord_HMD));
        mbf.RotationRate = EulerFromBodyFrameUpdate(bf, (Coordinates == Coord_HMD));
        mbf.MagneticField= Vector3f(0);        

        HandlerRef.GetHandler()->OnMessage(mbf);
    }
}


}} // namespace OVR::Win32


