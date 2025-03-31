//
//  ble_peripheral_manager.mm
//  bleno-mac-native
//
//  Created by Georg Vienna on 28.08.18.
//
#include "ble_peripheral_manager.h"

#include <dispatch/dispatch.h>

#include "objc_cpp.h"

@interface BLEPeripheralManager () <CBPeripheralManagerDelegate>

@property (nonatomic, strong) dispatch_queue_t queue;
@property (nonatomic, strong) NSMutableArray<NSDictionary *> *pendingNotifications;
@property (nonatomic, strong) CBPeripheralManager *peripheralManager;

@end

@implementation BLEPeripheralManager

- (instancetype)init {
    if (self = [super init]) {
        self.queue = dispatch_queue_create("CBqueue", DISPATCH_QUEUE_SERIAL);
        self.pendingNotifications = [NSMutableArray array];
    }
    return self;
}

- (void)sendNotifications {
    while (self.pendingNotifications.count > 0) {
        NSDictionary *notification = self.pendingNotifications.firstObject;
        NSData *data = notification[@"data"];
        CBMutableCharacteristic *characteristic = notification[@"characteristic"];
        CBCentral *central = notification[@"central"];

        BOOL success = [self.peripheralManager updateValue:data
                                         forCharacteristic:characteristic
                                      onSubscribedCentrals:@[central]];

        if (!success) {
            break;  // Wait until next ready callback
        }

        // Remove the successfully sent notification
        [self.pendingNotifications removeObjectAtIndex:0];
    }
}

#pragma mark - API

- (void)start {
    self.peripheralManager = [[CBPeripheralManager alloc] initWithDelegate:self
                                                                    queue:self.queue];
}

- (void)startAdvertising:(nonnull NSString *)name serviceUUIDs:(nonnull NSArray<CBUUID *> *)serviceUUIDs {
    if (self.peripheralManager.isAdvertising) {
        return;
    }

    [self.peripheralManager startAdvertising: @{
        CBAdvertisementDataLocalNameKey: name,
        CBAdvertisementDataServiceUUIDsKey: serviceUUIDs,
    }];
}

- (void)stopAdvertising {
    [self.peripheralManager stopAdvertising];
}

- (void)setServices:(NSArray<CBMutableService *> *)services {
    for (CBMutableService *service in services) {
        [self.peripheralManager addService:service];
    }
}

- (void)disconnect {

}

- (void)updateRssi {

}

#pragma mark - CBPeripheralManagerDelegate

- (void)peripheralManagerDidUpdateState:(CBPeripheralManager *)peripheral {
    auto state = StringFromCBPeripheralState(peripheral.state);
    emit.StateChange(state);
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral willRestoreState:(NSDictionary<NSString *, id> *)dict {

}

- (void)peripheralManagerDidStartAdvertising:(CBPeripheralManager *)peripheral error:(nullable NSError *)error {
    emit.AdvertisingStart(error);
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral didAddService:(CBService *)service error:(nullable NSError *)error {
    emit.ServicesSet(error);
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral central:(CBCentral *)central didSubscribeToCharacteristic:(CBMutableCharacteristic *)characteristic {
    CBUUID *uuid = characteristic.UUID;

    for (auto it = emitters.begin(); it != emitters.end(); ++it) {
        if ([it->first isEqual:uuid]) {
            auto cb = [=](NSData *data) {
                // Dispatch since cb is called from node
                dispatch_async(self.queue, ^{
                    NSDictionary *notification = @{
                        @"data": data,
                        @"characteristic": characteristic,
                        @"central": central
                    };
                    [self.pendingNotifications addObject:notification];
                    [self sendNotifications];
                });
            };

            it->second.Subscribe((uint16_t)central.maximumUpdateValueLength, cb);
        }
    }
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral central:(CBCentral *)central didUnsubscribeFromCharacteristic:(CBCharacteristic *)characteristic {
    CBUUID *uuid = characteristic.UUID;

    for (auto it = emitters.begin(); it != emitters.end(); ++it) {
        if ([it->first isEqual:uuid]) {
            it->second.Unsubscribe();
        }
    }
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral didReceiveReadRequest:(CBATTRequest *)request {
    CBCharacteristic *characteristic = request.characteristic;
    CBUUID *uuid = characteristic.UUID;

    for (auto it = emitters.begin(); it != emitters.end(); ++it) {
        if ([it->first isEqual:uuid]) {
            auto cb = [peripheral, request](int result, NSData *data) {
                request.value = data;

                [peripheral respondToRequest:request
                                  withResult:(CBATTError)result];
            };

            it->second.ReadRequest((uint16_t)request.offset, cb);
        }
    }
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral didReceiveWriteRequests:(NSArray<CBATTRequest *> *)requests {
    for (CBATTRequest *request in requests) {
        CBCharacteristic *characteristic = request.characteristic;
        CBUUID *uuid = characteristic.UUID;

        for (auto it = emitters.begin(); it != emitters.end(); ++it) {
            if ([it->first isEqual:uuid]) {
                bool sendResponse = (request.characteristic.properties & CBCharacteristicPropertyWrite) == CBCharacteristicPropertyWrite;

                auto cb = [peripheral, request, sendResponse](int result) {
                    if (sendResponse) {
                        [peripheral respondToRequest:request
                                          withResult:(CBATTError)result];
                    }
                };

                it->second.WriteRequest(request.value,
                                        (uint16_t)request.offset,
                                        !sendResponse,
                                        cb);
            }
        }
    }
}

- (void)peripheralManagerIsReadyToUpdateSubscribers:(CBPeripheralManager *)peripheral {
    [self sendNotifications];
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral didPublishL2CAPChannel:(CBL2CAPPSM)PSM error:(nullable NSError *)error {

}

- (void)peripheralManager:(CBPeripheralManager *)peripheral didUnpublishL2CAPChannel:(CBL2CAPPSM)PSM error:(nullable NSError *)error {

}

- (void)peripheralManager:(CBPeripheralManager *)peripheral didOpenL2CAPChannel:(nullable CBL2CAPChannel *)channel error:(nullable NSError *)error {

}

@end
