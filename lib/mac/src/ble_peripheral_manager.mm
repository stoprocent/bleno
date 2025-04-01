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

- (void)addNotification:(NSData *)data 
         characteristic:(CBMutableCharacteristic *)characteristic 
                 central:(CBCentral *)central;
- (void)processNotificationQueue;
- (BOOL)sendNextNotification;

@end

@implementation BLEPeripheralManager

- (instancetype)init 
{
    if (self = [super init]) {
        self.queue = dispatch_queue_create("com.bleno.notificationQueue", DISPATCH_QUEUE_SERIAL);
        self.pendingNotifications = [NSMutableArray array];
    }
    return self;
}

- (void)dealloc 
{
    self.peripheralManager.delegate = nil;
    [self.pendingNotifications removeAllObjects];
}

#pragma mark - Notification Management

- (void)addNotification:(NSData *)data 
         characteristic:(CBMutableCharacteristic *)characteristic 
                 central:(CBCentral *)central 
{
    dispatch_async(self.queue, ^{
        NSDictionary *notification = @{
            @"data": data,
            @"characteristic": characteristic,
            @"central": central
        };
        [self.pendingNotifications addObject:notification];
        [self processNotificationQueue];
    });
}

- (void)processNotificationQueue 
{
    dispatch_async(self.queue, ^{
        while (self.pendingNotifications.count > 0) {
            if (![self sendNextNotification]) {
                break; // Wait for next ready callback
            }
        }
    });
}

- (BOOL)sendNextNotification {
    NSDictionary *notification = self.pendingNotifications.firstObject;
    if (!notification) return NO;

    NSData *data = notification[@"data"];
    CBMutableCharacteristic *characteristic = notification[@"characteristic"];
    CBCentral *central = notification[@"central"];

    BOOL success = [self.peripheralManager updateValue:data
                                     forCharacteristic:characteristic
                                  onSubscribedCentrals:@[central]];

    if (success) {
        [self.pendingNotifications removeObjectAtIndex:0];
        return YES;
    }

    return NO;
}

#pragma mark - API

- (void)start 
{
    self.peripheralManager = [[CBPeripheralManager alloc] initWithDelegate:self queue:self.queue];
}

- (void)startAdvertising:(nonnull NSString *)name serviceUUIDs:(nonnull NSArray<CBUUID *> *)serviceUUIDs 
{
    if (self.peripheralManager.isAdvertising) {
        return;
    }

    [self.peripheralManager startAdvertising: @{
        CBAdvertisementDataLocalNameKey: name,
        CBAdvertisementDataServiceUUIDsKey: serviceUUIDs,
    }];
}

- (void)stopAdvertising 
{
    [self.peripheralManager stopAdvertising];
}

- (void)setServices:(NSArray<CBMutableService *> *)services 
{
    for (CBMutableService *service in services) {
        [self.peripheralManager addService:service];
    }
}

- (void)disconnect 
{

}

- (void)updateRssi 
{

}

#pragma mark - CBPeripheralManagerDelegate

- (void)peripheralManagerDidUpdateState:(CBPeripheralManager *)peripheral 
{
    auto state = StringFromCBPeripheralState(peripheral.state);
    emit.StateChange(state);
}

- (void)peripheralManagerDidStartAdvertising:(CBPeripheralManager *)peripheral 
                                       error:(nullable NSError *)error 
{
    emit.AdvertisingStart(error);
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral 
            didAddService:(CBService *)service 
                    error:(nullable NSError *)error 
{
    emit.ServicesSet(error);
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral central:(CBCentral *)central didSubscribeToCharacteristic:(CBMutableCharacteristic *)characteristic 
{
    for (auto it = emitters.begin(); it != emitters.end(); ++it) {
        if ([it->first isEqual:characteristic.UUID] == NO) { continue; }
        auto cb = [weakSelf = self, characteristic, central](NSData *data) {
            [weakSelf addNotification:data characteristic:characteristic central:central];
        };
        it->second.Subscribe(central.identifier, (uint16_t)central.maximumUpdateValueLength, cb);
    }
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral central:(CBCentral *)central didUnsubscribeFromCharacteristic:(CBCharacteristic *)characteristic 
{
    for (auto it = emitters.begin(); it != emitters.end(); ++it) {
        if ([it->first isEqual:characteristic.UUID] == NO) { continue; }
        it->second.Unsubscribe(central.identifier);
    }
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral didReceiveReadRequest:(CBATTRequest *)request 
{
    for (auto it = emitters.begin(); it != emitters.end(); ++it) {
        if ([it->first isEqual:request.characteristic.UUID] == NO) { continue; }
        auto cb = [peripheral, request](int result, NSData *data) {
            request.value = data;
            [peripheral respondToRequest:request withResult:(CBATTError)result];
        };
        it->second.ReadRequest(request.central.identifier, (uint16_t)request.offset, cb);
    }
}

- (void)peripheralManager:(CBPeripheralManager *)peripheral didReceiveWriteRequests:(NSArray<CBATTRequest *> *)requests 
{
    for (CBATTRequest *request in requests) {
        CBCharacteristic *characteristic = request.characteristic;
        for (auto it = emitters.begin(); it != emitters.end(); ++it) {
            if ([it->first isEqual:characteristic.UUID] == NO) { continue; }
            bool sendResponse = (request.characteristic.properties & CBCharacteristicPropertyWrite) == CBCharacteristicPropertyWrite;

            auto cb = [peripheral, request, sendResponse](int result) {
                if (!sendResponse) { return; }
                [peripheral respondToRequest:request withResult:(CBATTError)result];
            };

            it->second.WriteRequest(request.central.identifier,
                                    request.value,
                                    (uint16_t)request.offset,
                                    !sendResponse,
                                    cb);
        }
    }
}

- (void)peripheralManagerIsReadyToUpdateSubscribers:(CBPeripheralManager *)peripheral 
{
    [self processNotificationQueue];
}

@end
