//
//  objc_cpp.mm
//  bleno-mac-native
//
//  Created by Georg Vienna on 30.08.18.
//
#include "objc_cpp.h"

#if defined(MAC_OS_X_VERSION_10_13)
// In the 10.13 SDK, CBPeripheral became a subclass of CBPeer, which defines
// -[CBPeer identifier] as partially available. Pretend it still exists on
// CBPeripheral. At runtime the implementation on CBPeer will be invoked.
@interface CBPeripheral (HighSierraSDK)
@property(readonly, nonatomic) NSUUID* identifier;
@end
#else
std::string stateToString(CBCentralManagerState state)
{
    switch(state) {
        case CBCentralManagerStatePoweredOff:
            return "poweredOff";
        case CBCentralManagerStatePoweredOn:
            return "poweredOn";
        case CBCentralManagerStateResetting:
            return "resetting";
        case CBCentralManagerStateUnauthorized:
            return "unauthorized";
        case CBCentralManagerStateUnknown:
            return "unknown";
        case CBCentralManagerStateUnsupported:
            return "unsupported";
    }
    return "unknown";
}
#endif

std::string StringFromCBPeripheralState(CBManagerState state) {
    switch(state) {
        case CBManagerStatePoweredOff:
            return "poweredOff";
        case CBManagerStatePoweredOn:
            return "poweredOn";
        case CBManagerStateUnauthorized:
            return "unauthorized";
        case CBManagerStateUnsupported:
            return "unsupported";
        case CBManagerStateResetting:
            return "resetting";
        case CBManagerStateUnknown:
        default:
            return "unknown";
    }
}

NSString* getNSUuid(CBPeripheral* peripheral) {
    return peripheral.identifier.UUIDString;
}

std::string getUuid(CBPeripheral* peripheral) {
    return std::string([peripheral.identifier.UUIDString UTF8String]);
}

std::string convertToBlenoAddress(NSUUID* uuid) {
    NSString* addressString = [[uuid.UUIDString stringByReplacingOccurrencesOfString:@"-" withString:@""] lowercaseString];
    return std::string([addressString UTF8String]);
}

std::vector<std::string> getServices(NSArray<CBService*>* services) {
    std::vector<std::string> result;
    if(services) {
        for (CBService* service in services) {
            result.push_back([[service.UUID UUIDString] UTF8String]);
        }
    }
    return result;
}

#define TEST_PROP(type, str) if((characteristic.properties & type) == type) { properties.push_back(str); }

std::vector<std::pair<std::string, std::vector<std::string>>> getCharacteristics(NSArray<CBCharacteristic*>* characteristics) {
    std::vector<std::pair<std::string, std::vector<std::string>>> result;
    if(characteristics) {
        for (CBCharacteristic* characteristic in characteristics) {
            auto uuid = [[characteristic.UUID UUIDString] UTF8String];
            auto properties = std::vector<std::string>();
            TEST_PROP(CBCharacteristicPropertyBroadcast, "broadcast");
            TEST_PROP(CBCharacteristicPropertyRead, "read");
            TEST_PROP(CBCharacteristicPropertyWriteWithoutResponse, "writeWithoutResponse");
            TEST_PROP(CBCharacteristicPropertyWrite, "write");
            TEST_PROP(CBCharacteristicPropertyNotify, "notify");
            TEST_PROP(CBCharacteristicPropertyIndicate, "indicate");
            TEST_PROP(CBCharacteristicPropertyAuthenticatedSignedWrites, "authenticatedSignedWrites");
            TEST_PROP(CBCharacteristicPropertyExtendedProperties, "extendedProperties");
            TEST_PROP(CBCharacteristicPropertyNotifyEncryptionRequired, "notifyEncryptionRequired");
            TEST_PROP(CBCharacteristicPropertyIndicateEncryptionRequired, "indicateEncryptionRequired");
            result.push_back(std::make_pair(uuid, properties));
        }
    }
    return result;
}

std::vector<std::string> getDescriptors(NSArray<CBDescriptor*>* descriptors) {
    std::vector<std::string> result;
    if(descriptors) {
        for (CBDescriptor* descriptor in descriptors) {
            result.push_back([[descriptor.UUID UUIDString] UTF8String]);
        }
    }
    return result;
}
