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
@property (nonatomic, strong) dispatch_queue_t processingQueue;
@property (nonatomic, strong) NSMutableArray<NSDictionary *> *pendingNotifications;
@property (nonatomic, strong) CBPeripheralManager *peripheralManager;
@property (nonatomic, strong, readwrite) NSMutableSet<NSUUID *> *connectedCentrals;
@property (nonatomic, strong, readwrite) NSMutableArray<CBMutableService *> *currentServices;
@property (nonatomic, assign) BOOL triedWithRestoration;
@property (nonatomic, assign) BOOL retriedWithoutRestoration;
@end

@implementation BLEPeripheralManager

- (instancetype)init 
{
    if (self = [super init]) {
        self.processingQueue = dispatch_queue_create("com.bleno.processing.queue", DISPATCH_QUEUE_SERIAL);
        self.pendingNotifications = [NSMutableArray array];
        self.connectedCentrals = [NSMutableSet set];
        self.currentServices = [NSMutableArray array];
    }
    return self;
}

- (void)dealloc 
{
    [self removeAllServices];
    self.peripheralManager.delegate = nil;
    [self.pendingNotifications removeAllObjects];
    [self.connectedCentrals removeAllObjects];
    [self.currentServices removeAllObjects];
}

#pragma mark - Notification Management

- (void)addNotification:(NSData *)data
         characteristic:(CBMutableCharacteristic *)characteristic
                 central:(CBCentral *)central 
{
    // All operations are now done on the same serial queue.
    dispatch_async(self.processingQueue, ^{
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
    // Since this runs on the serial queue, no additional synchronization is needed.
    while (self.pendingNotifications.count > 0) {
        NSDictionary *notification = self.pendingNotifications.firstObject;
        NSData *data = notification[@"data"];
        CBMutableCharacteristic *characteristic = notification[@"characteristic"];
        CBCentral *central = notification[@"central"];
        
        BOOL success = [self.peripheralManager updateValue:data
                                         forCharacteristic:characteristic
                                      onSubscribedCentrals:@[central]];
        if (success) {
            [self.pendingNotifications removeObjectAtIndex:0];
        } else {
            // Stop processing if updateValue fails; will retry on next callback.
            break;
        }
    }
}

#pragma mark - API

- (void)start 
{
    [self startWithRestoration:YES];
}

- (void)startWithRestoration:(BOOL)useRestoration 
{
    // Clean up any existing peripheral manager
    if (self.peripheralManager) {
        self.peripheralManager.delegate = nil;
        self.peripheralManager = nil;
    }
    
    NSDictionary *options;
    if (useRestoration) {
        // Initialize with state restoration to recover from previous state
        options = @{
            CBPeripheralManagerOptionRestoreIdentifierKey: kBlenoRestorationIdentifier,
            CBPeripheralManagerOptionShowPowerAlertKey: @YES
        };
        self.triedWithRestoration = YES;
    } else {
        // Initialize without state restoration
        options = @{
            CBPeripheralManagerOptionShowPowerAlertKey: @YES
        };
        self.triedWithRestoration = NO;
    }
    
    self.peripheralManager = [[CBPeripheralManager alloc] initWithDelegate:self
                                                                     queue:self.processingQueue
                                                                   options:options];
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
    // Store services for cleanup later
    [self.currentServices addObjectsFromArray:services];
    
    for (CBMutableService *service in services) {
        [self.peripheralManager addService:service];
    }
}

- (void)removeAllServices 
{
    if (self.peripheralManager) {
        [self.peripheralManager removeAllServices];
    }
    [self.currentServices removeAllObjects];
    emitters.clear();
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
    // Check if state is unsupported and we tried with restoration - retry without it
    if (peripheral.state == CBManagerStateUnsupported && 
        self.triedWithRestoration && 
        !self.retriedWithoutRestoration) {
        self.retriedWithoutRestoration = YES;
        
        // Dispatch async to avoid modifying peripheral manager during delegate callback
        dispatch_async(self.processingQueue, ^{
            [self startWithRestoration:NO];
        });
        return;
    }
    
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
    // Track connected centrals and emit accept event for new connections
    BOOL isNewConnection = ![self.connectedCentrals containsObject:central.identifier];
    if (isNewConnection) {
        [self.connectedCentrals addObject:central.identifier];
        emit.Accept(central.identifier);
    }
    
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
    // Track connected centrals on read requests too
    BOOL isNewConnection = ![self.connectedCentrals containsObject:request.central.identifier];
    if (isNewConnection) {
        [self.connectedCentrals addObject:request.central.identifier];
        emit.Accept(request.central.identifier);
    }
    
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
        // Track connected centrals on write requests too
        BOOL isNewConnection = ![self.connectedCentrals containsObject:request.central.identifier];
        if (isNewConnection) {
            [self.connectedCentrals addObject:request.central.identifier];
            emit.Accept(request.central.identifier);
        }
        
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

#pragma mark - State Restoration

- (void)peripheralManager:(CBPeripheralManager *)peripheral willRestoreState:(NSDictionary<NSString *, id> *)dict 
{
    // Restore services that were previously registered
    NSArray<CBMutableService *> *restoredServices = dict[CBPeripheralManagerRestoredStateServicesKey];
    if (restoredServices) {
        [self.currentServices addObjectsFromArray:restoredServices];
        
        // Iterate through restored services to find subscribed centrals
        for (CBMutableService *service in restoredServices) {
            if (service.characteristics) {
                for (CBMutableCharacteristic *characteristic in service.characteristics) {
                    // Check for subscribed centrals on this characteristic
                    if (characteristic.subscribedCentrals && characteristic.subscribedCentrals.count > 0) {
                        for (CBCentral *central in characteristic.subscribedCentrals) {
                            if (![self.connectedCentrals containsObject:central.identifier]) {
                                [self.connectedCentrals addObject:central.identifier];
                                // Emit accept event for the restored connection
                                emit.Accept(central.identifier);
                            }
                        }
                    }
                }
            }
        }
    }
}

@end
