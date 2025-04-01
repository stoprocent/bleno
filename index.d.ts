// Type definitions for bleno 0.4
// Project: https://github.com/sandeepmistry/bleno
// Definitions by: Manuel Francisco Naranjo <naranjo.manuel@gmail.com>
// Definitions: https://github.com/DefinitelyTyped/DefinitelyTyped

/// <reference types="node" />

type State = 'poweredOn' | 'poweredOff' | 'unauthorized' | 'unsupported' | 'unknown' | 'resetting';

type Property = 'read' | 'write' | 'indicate' | 'notify' | 'writeWithoutResponse';

type ConnectionHandle = number | string;

// Common callback types
type ReadRequestCallback = (result: number, data?: Buffer) => void;
type UpdateValueCallback = (data?: Buffer) => void;
type WriteRequestCallback = (result: number) => void;

// Common function types
type OnReadRequestFn = (handle: ConnectionHandle, offset: number, callback: ReadRequestCallback) => void;
type OnSubscribeFn = (handle: ConnectionHandle, maxValueSize: number, updateValueCallback: UpdateValueCallback) => void;
type OnUnsubscribeFn = (handle: ConnectionHandle) => void;
type OnWriteRequestFn = (handle: ConnectionHandle, data: Buffer, offset: number, withoutResponse: boolean, callback: WriteRequestCallback) => void;
type OnIndicateFn = (handle: ConnectionHandle) => void;
type OnNotifyFn = (handle: ConnectionHandle) => void;

interface CharacteristicOptions {
    uuid: string;
    properties?: ReadonlyArray<Property> | null;
    secure?: ReadonlyArray<Property> | null;
    value?: Buffer | null;
    descriptors?: ReadonlyArray<Descriptor> | null;
    onIndicate?: OnIndicateFn | null;
    onNotify?: OnNotifyFn | null;
    onReadRequest?: OnReadRequestFn | null;
    onSubscribe?: OnSubscribeFn | null;
    onUnsubscribe?: OnUnsubscribeFn | null;
    onWriteRequest?: OnWriteRequestFn | null;
}

declare class Characteristic {
    uuid: string;
    properties: ReadonlyArray<Property>;
    secure: ReadonlyArray<Property>;
    value: Buffer | null;
    descriptors: ReadonlyArray<Descriptor>;

    constructor(options: CharacteristicOptions);

    onIndicate: OnIndicateFn;
    onNotify: OnNotifyFn;
    onReadRequest: OnReadRequestFn;
    onSubscribe: OnSubscribeFn;
    onUnsubscribe: OnUnsubscribeFn;
    onWriteRequest: OnWriteRequestFn;

    toString(): string;

    readonly RESULT_ATTR_NOT_LONG: number;
    readonly RESULT_INVALID_ATTRIBUTE_LENGTH: number;
    readonly RESULT_INVALID_OFFSET: number;
    readonly RESULT_SUCCESS: number;
    readonly RESULT_UNLIKELY_ERROR: number;

    static readonly RESULT_ATTR_NOT_LONG: number;
    static readonly RESULT_INVALID_ATTRIBUTE_LENGTH: number;
    static readonly RESULT_INVALID_OFFSET: number;
    static readonly RESULT_SUCCESS: number;
    static readonly RESULT_UNLIKELY_ERROR: number;
}

interface DescriptorOptions {
    uuid: string;
    value?: Buffer | string | null;
}

declare class Descriptor {
    uuid: string;
    value: Buffer;

    constructor(options: DescriptorOptions);

    toString(): string;
}

interface PrimaryServiceOptions {
    uuid: string;
    characteristics?: ReadonlyArray<Characteristic> | null;
}

declare class PrimaryService {
    uuid: string;
    characteristics: ReadonlyArray<Characteristic>;

    constructor(options: PrimaryServiceOptions);

    toString(): string;
}

export interface Bleno extends NodeJS.EventEmitter {
    readonly Characteristic: typeof Characteristic;
    readonly Descriptor: typeof Descriptor;
    readonly PrimaryService: typeof PrimaryService;

    readonly address: string;

    readonly mtu: number;

    readonly platform: string;

    readonly rssi: number;

    readonly state: State;

    disconnect(): void;

    stop(): void;

    setAddress(address: string): void;

    setServices(services: ReadonlyArray<PrimaryService>, callback?: (arg: Error | undefined | null) => void): void;

    startAdvertising(name: string, serviceUuids?: ReadonlyArray<string>, callback?: (arg: Error | undefined | null) => void): void;

    startAdvertisingIBeacon(uuid: string, major: number, minor: number, measuredPower: number, callback?: (arg: Error | undefined | null) => void): void;

    startAdvertisingWithEIRData(advertisementData: Buffer, callback?: (arg: Error | undefined | null) => void): void;
    startAdvertisingWithEIRData(advertisementData: Buffer, scanData: Buffer, callback?: (arg: Error | undefined | null) => void): void;

    stopAdvertising(callback?: () => void): void;

    updateRssi(callback?: (err: null, rssi: number) => void): void;

    // Async methods
    waitForPoweredOn(timeout?: number): Promise<void>;

    setAddressAsync(address: string): Promise<void>;
    
    setServicesAsync(services: ReadonlyArray<PrimaryService>): Promise<void>;
    
    startAdvertisingAsync(name: string, serviceUuids?: ReadonlyArray<string>): Promise<void>;
    
    startAdvertisingIBeaconAsync(
        uuid: string,
        major: number,
        minor: number,
        measuredPower: number
    ): Promise<void>;
    
    startAdvertisingWithEIRDataAsync(advertisementData: Buffer): Promise<void>;
    startAdvertisingWithEIRDataAsync(advertisementData: Buffer, scanData: Buffer): Promise<void>;
    
    stopAdvertisingAsync(): Promise<void>;
    
    updateRssiAsync(): Promise<number>;

    on(event: 'stateChange', cb: (state: State) => void): this;
    on(event: 'platform', cb: (platform: NodeJS.Platform) => void): this;
    on(event: 'addressChange', cb: (address: string) => void): this;
    on(event: 'accept', cb: (address: string, handle: ConnectionHandle) => void): this;
    on(event: 'mtuChange', cb: (mtu: number) => void): this;
    on(event: 'disconnect', cb: (address: string, handle: ConnectionHandle) => void): this;
    on(event: 'advertisingStart', cb: (err?: Error | null) => void): this;
    on(event: 'advertisingStartError', cb: (err: Error) => void): this;
    on(event: 'advertisingStop', cb: () => void): this;
    on(event: 'servicesSet', cb: (err?: Error | null) => void): this;
    on(event: 'servicesSetError', cb: (err: Error) => void): this;
    on(event: 'rssiUpdate', cb: (rssi: number) => void): this;
}

type BindingType = 'default' | 'hci' | 'mac';

interface BaseBindingsOptions {
    bindParams?: any;
}

interface HciBindingsOptions extends BaseBindingsOptions {
    bindingType: 'hci';
    hciDriver?: 'usb' | 'uart' | 'native';
}

interface DefaultBindingsOptions extends BaseBindingsOptions {
    bindingType?: 'default' | 'mac';
}

type WithBindingsOptions = HciBindingsOptions | DefaultBindingsOptions;

declare function withBindings(
    bindingType?: BindingType, 
    options?: WithBindingsOptions
): Bleno;

declare const bleno: Bleno;
export = bleno;
export { withBindings };