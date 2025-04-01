//
//  ble_peripheral_manager.h
//  bleno-mac-native
//
//  Created by Georg Vienna on 28.08.18.
//

#pragma once

#include <map>
#include "callbacks.h"

#import <CoreBluetooth/CoreBluetooth.h>

@interface BLEPeripheralManager : NSObject {
    @public Emit emit;
    @public std::map<CBUUID *, EmitCharacteristic> emitters;
}

- (nonnull instancetype)init NS_DESIGNATED_INITIALIZER;
- (void)start;
- (void)startAdvertising:(NSString * _Nonnull)name serviceUUIDs:(NSArray<CBUUID *> * _Nonnull)serviceUUIDs;
- (void)stopAdvertising;
- (void)setServices:(NSArray<CBMutableService *> * _Nonnull)services;
- (void)disconnect;
- (void)updateRssi;

@end
