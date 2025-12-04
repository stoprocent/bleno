//
//  ble_peripheral_manager.h
//  bleno-mac-native
//
//  Created by Georg Vienna on 28.08.18.
//

#pragma once

#include <map>
#include <set>
#include "callbacks.h"

#import <CoreBluetooth/CoreBluetooth.h>

// Restoration identifier for state restoration across restarts
static NSString * _Nonnull const kBlenoRestorationIdentifier = @"com.bleno.peripheral.manager";

@interface BLEPeripheralManager : NSObject {
    @public Emit emit;
    @public std::map<CBUUID *, EmitCharacteristic> emitters;
}

// Track connected centrals by their identifier
@property (nonatomic, strong, readonly) NSMutableSet<NSUUID *> * _Nullable connectedCentrals;
// Track current services for proper cleanup
@property (nonatomic, strong, readonly) NSMutableArray<CBMutableService *> * _Nullable currentServices;

- (nonnull instancetype)init NS_DESIGNATED_INITIALIZER;
- (void)start;
- (void)startAdvertising:(NSString * _Nonnull)name serviceUUIDs:(NSArray<CBUUID *> * _Nonnull)serviceUUIDs;
- (void)stopAdvertising;
- (void)setServices:(NSArray<CBMutableService *> * _Nonnull)services;
- (void)removeAllServices;
- (void)disconnect;
- (void)updateRssi;

@end
