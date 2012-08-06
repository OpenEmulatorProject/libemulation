
/**
 * libemulation
 * Apple III System Control
 * (C) 2012 by Marc S. Ressl (mressl@umich.edu)
 * Released under the GPL
 *
 * Implements Apple III system control
 */

#include "AppleIIISystemControl.h"

#include "MOS6522.h"
#include "MemoryInterface.h"
#include "AppleIIIInterface.h"

AppleIIISystemControl::AppleIIISystemControl()
{
    cpu = NULL;
    memoryBus = NULL;
    extendedMemoryBus = NULL;
    memoryAddressDecoder = NULL;
    memory = NULL;
    dac = NULL;
    
    zeroPage = 0;
}

bool AppleIIISystemControl::setRef(string name, OEComponent *ref)
{
    if (name == "cpu")
        cpu = ref;
    if (name == "memoryBus")
        memoryBus = ref;
    else if (name == "extendedMemoryBus")
        extendedMemoryBus = ref;
    else if (name == "memoryAddressDecoder")
        memoryAddressDecoder = ref;
    else if (name == "memory")
        memory = ref;
    else if (name == "dac")
        dac = ref;
    else
        return false;
    
    return true;
}

bool AppleIIISystemControl::init()
{
    if (!cpu)
    {
        logMessage("cpu not connected");
        
        return false;
    }
    
    if (!memoryBus)
    {
        logMessage("memoryBus not connected");
        
        return false;
    }
    
    if (!extendedMemoryBus)
    {
        logMessage("extendedMemoryBus not connected");
        
        return false;
    }
    
    if (!memoryAddressDecoder)
    {
        logMessage("memoryAddressDecoder not connected");
        
        return false;
    }
    
    if (!memory)
    {
        logMessage("memory not connected");
        
        return false;
    }
    
    if (!dac)
    {
        logMessage("dac not connected");
        
        return false;
    }
    
    return true;
}

OEChar AppleIIISystemControl::read(OEAddress address)
{
    logMessage("R " + getString(address));
    
    switch (address)
    {
        case 0x0:
            return 0;
            
        case 0x1:
            return 0;
            
        case 0x2:
            return 0xb0;    // For now, return IRQ's clear
            
        case 0x3:
            return 0;
    }
    
    return 0;
}

void AppleIIISystemControl::write(OEAddress address, OEChar value)
{
    switch (address)
    {
        case 0x0:
        {
            environment = value;
            
            bool romSel1 = OEGetBit(environment, (1 << 0));
            bool ramWP = OEGetBit(environment, (1 << 3));
            bool resetEnabled = OEGetBit(environment, (1 << 4));
            bool videoEnabled = OEGetBit(environment, (1 << 5));
            bool ioEnabled = OEGetBit(environment, (1 << 6));
            bool slowSpeed = OEGetBit(environment, (1 << 7));
            
            OEInt memoryControl = 0;
            
            OESetBit(memoryControl, APPLEIII_RAMWP, ramWP);
            OESetBit(memoryControl, APPLEIII_IO, ioEnabled);
            OESetBit(memoryControl, APPLEIII_ROM, romSel1);
            
            memoryAddressDecoder->postMessage(this, APPLEIII_SET_MEMORYCONTROL, &memoryControl);
            
            updateNormalStack();
            
            break;
        }
        case 0x1:
        {
            zeroPage = value;
            
            AddressOffsetMap offsetMap;
            
            offsetMap.startAddress = 0x0000;
            offsetMap.endAddress = 0x00ff;
            offsetMap.offset = 0x100 * zeroPage;
            
            memoryBus->postMessage(this, ADDRESSOFFSET_MAP, &offsetMap);
            extendedMemoryBus->postMessage(this, ADDRESSOFFSET_MAP, &offsetMap);
            
            updateNormalStack();
            
            bool extendedMemoryEnabled = ((zeroPage & 0xf8) == 0x18);
            
            cpu->postMessage(this, APPLEIII_SET_EXTENDEDMEMORYENABLE, &extendedMemoryEnabled);
            
            break;
        }
        case 0x2:
        {
            OEChar bank = value & 0xf;
            
            AddressOffsetMap offsetMap;
            
            offsetMap.startAddress = 0x2000;
            offsetMap.endAddress = 0x9fff;
            offsetMap.offset = 0x8000 * bank + 0x6000;
            
            memory->postMessage(this, ADDRESSOFFSET_MAP, &offsetMap);
            
            bool appleIIMode = OEGetBit(value, (1 << 6));
            
            memoryAddressDecoder->postMessage(this, APPLEIII_SET_APPLEIIMODE, &appleIIMode);
            
            break;
        }
        case 0x3:
        {
            OEChar audioSample = value << 2;
            // bool BL = OEGetBit(value, (1 << 6));
            // bool ioNoNMI = OEGetBit(value, (1 << 7));
            
            dac->write(0, audioSample);
            dac->write(1, audioSample);
        }
    }
    
    logMessage("W " + getString(address) + ":" + getHexString(value));
}

void AppleIIISystemControl::updateNormalStack()
{
    bool normalStack = OEGetBit(environment, (1 << 2));
    
    AddressOffsetMap offsetMap;
    
    offsetMap.startAddress = 0x0100;
    offsetMap.endAddress = 0x01ff;
    offsetMap.offset = normalStack ? 0 : 0x100 * (zeroPage ^ 0x01);
    
    memoryBus->postMessage(this, ADDRESSOFFSET_MAP, &offsetMap);
    extendedMemoryBus->postMessage(this, ADDRESSOFFSET_MAP, &offsetMap);
}
