/************************************************************************************

Filename    :   OVR_Win32_HMDDevice.cpp
Content     :   Win32 Interface to HMD - detects HMD display
Created     :   September 21, 2012
Authors     :   Michael Antonov

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

*************************************************************************************/

#include "OVR_Win32_HMDDevice.h"

// Needed to tag sensor as having HMD coordinates.
#include "OVR_Win32_FSRKSensor.h"
#include "OVR_Win32_Sensor.h"

namespace OVR { namespace Win32 {



const wchar_t* FormatDisplayStateFlags(wchar_t* buff, int length, DWORD flags)
{
    buff[0] = 0;
    if (flags & DISPLAY_DEVICE_ACTIVE)
        wcscat_s(buff, length, L"Active ");
    if (flags & DISPLAY_DEVICE_MIRRORING_DRIVER)
        wcscat_s(buff, length, L"Mirroring_Driver ");
    if (flags & DISPLAY_DEVICE_MODESPRUNED)
        wcscat_s(buff, length, L"ModesPruned ");
    if (flags & DISPLAY_DEVICE_PRIMARY_DEVICE)
        wcscat_s(buff, length, L"Primary ");
    if (flags & DISPLAY_DEVICE_REMOVABLE)
        wcscat_s(buff, length, L"Removable ");
    if (flags & DISPLAY_DEVICE_VGA_COMPATIBLE)
        wcscat_s(buff, length, L"VGA_Compatible ");
    return buff;
}



//-------------------------------------------------------------------------------------
// ***** HMDDeviceFactory

HMDDeviceFactory HMDDeviceFactory::Instance;

void HMDDeviceFactory::EnumerateDevices(EnumerateVisitor& visitor)
{
   // DeviceManager* manager = getManager();
    DISPLAY_DEVICE dd, ddm;
    UINT           i, j;    

    for (i = 0; 
        (ZeroMemory(&dd, sizeof(dd)), dd.cb = sizeof(dd),
        EnumDisplayDevices(0, i, &dd, 0)) != 0;  i++)
    {
        
        /*
        wchar_t buff[500], flagsBuff[200];
        
        swprintf_s(buff, 500, L"\nDEV: \"%s\" \"%s\" 0x%08x=%s\n     \"%s\" \"%s\"\n",
            dd.DeviceName, dd.DeviceString,
            dd.StateFlags, FormatDisplayStateFlags(flagsBuff, 200, dd.StateFlags),
            dd.DeviceID, dd.DeviceKey);
        ::OutputDebugString(buff);
        */

        for (j = 0; 
            (ZeroMemory(&ddm, sizeof(ddm)), ddm.cb = sizeof(ddm),
            EnumDisplayDevices(dd.DeviceName, j, &ddm, 0)) != 0;  j++)
        {
            // Our monitor hardware has string "RTD2205" in it
            // Nate's device "CVT0003"
            if (wcsstr(ddm.DeviceID, L"RTD2205") || 
                wcsstr(ddm.DeviceID, L"CVT0003") || 
                wcsstr(ddm.DeviceID, L"MST0030") ||
                wcsstr(ddm.DeviceID, L"OVR0001") ) // SLA 1.
            {
                String deviceId(ddm.DeviceID);
                String displayDeviceName(ddm.DeviceName);

                HMDDeviceCreateDesc hmdCreateDesc(this, deviceId, displayDeviceName);

                if (hmdCreateDesc.IsSLA1())
                {
                    // Physical dimension of SLA screen.
                    hmdCreateDesc.SetScreenParameters(1280, 800, 0.14976f, 0.0936f);
                }
                else
                {
                    hmdCreateDesc.SetScreenParameters(1280, 800, 0.12096f, 0.0756f);
                }

                OVR_DEBUG_LOG_TEXT(("DeviceManager - HMD Found %s - %s\n",
                                    deviceId.ToCStr(), displayDeviceName.ToCStr()));

                // Notify caller about detected device. This will call EnumerateAddDevice
                // if the this is the first time device was detected.
                visitor.Visit(hmdCreateDesc);
                break;
            }

          /*  
            wchar_t mbuff[500];
            swprintf_s(mbuff, 500, L"MON: \"%s\" \"%s\" 0x%08x=%s\n     \"%s\" \"%s\"\n",
                ddm.DeviceName, ddm.DeviceString,
                ddm.StateFlags, FormatDisplayStateFlags(flagsBuff, 200, ddm.StateFlags),
                ddm.DeviceID, ddm.DeviceKey);
            ::OutputDebugString(mbuff);
            */
            
        }
    }

}

DeviceBase* HMDDeviceCreateDesc::NewDeviceInstance()
{
    return new HMDDevice(this);
}

bool HMDDeviceCreateDesc::IsSLA1() const
{
    return strstr(DeviceId.ToCStr(), "OVR0001") != 0;
}

bool HMDDeviceCreateDesc::GetDeviceInfo(DeviceInfo* info) const
{
    if ((info->InfoClassType != Device_HMD) &&
        (info->InfoClassType != Device_None))
        return false;

    bool isSLA = IsSLA1();

    OVR_strcpy(info->ProductName,  DeviceInfo::MaxNameLength,
               isSLA ? "Oculus Rift DK1-SLA1" : "Oculus Rift DK1-Prototype");
    OVR_strcpy(info->Manufacturer, DeviceInfo::MaxNameLength, "Oculus VR");
    info->Type    = Device_HMD;
    info->Version = 0;

    // Display detection.
    if (info->InfoClassType == Device_HMD)
    {
        HMDInfo* hmdInfo = static_cast<HMDInfo*>(info);

        hmdInfo->HResolution            = HResolution;
        hmdInfo->VResolution            = VResolution;
        hmdInfo->HScreenSize            = HScreenSize;
        hmdInfo->VScreenSize            = VScreenSize;
        hmdInfo->VScreenCenter          = VScreenSize * 0.5f;
        hmdInfo->InterpupillaryDistance = 0.064f;  // Default IPD; should be configurable.
        hmdInfo->LensSeparationDistance = 0.064f;
                
        if (IsSLA1())
        {
            // 7" screen.
            hmdInfo->DistortionK0        = 1.0f;
            hmdInfo->DistortionK1        = 0.22f;
            hmdInfo->DistortionK2        = 0.24f;
            hmdInfo->EyeToScreenDistance = 0.041f;
        }
        else
        {
            hmdInfo->DistortionK0        = 1.0f;
            hmdInfo->DistortionK1        = 0.18f;
            hmdInfo->DistortionK2        = 0.115f;
            hmdInfo->EyeToScreenDistance = 0.0387f;
        }

        OVR_strcpy(hmdInfo->DisplayDeviceName, sizeof(hmdInfo->DisplayDeviceName),
                   DisplayDeviceName.ToCStr());
    }

    return true;
}

//-------------------------------------------------------------------------------------
// ***** HMDDevice

HMDDevice::HMDDevice(HMDDeviceCreateDesc* createDesc)
    : OVR::DeviceImpl<OVR::HMDDevice>(createDesc, 0)
{
}
HMDDevice::~HMDDevice()
{
}

bool HMDDevice::Initialize(DeviceBase* parent)
{
    pParent = parent;
    return true;
}
void HMDDevice::Shutdown()
{
    pParent.Clear();
}

OVR::SensorDevice* HMDDevice::GetSensor()
{
    // Just return first sensor found since we have no way to match it yet.
    OVR::SensorDevice* sensor = GetManager()->EnumerateDevices<SensorDevice>().CreateDevice();
    if (sensor)
        sensor->SetCoordinateFrame(SensorDevice::Coord_HMD);
    return sensor;
}



}} // namespace OVR::Win32


