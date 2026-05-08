/// <reference types="node" />
declare module '@stoprocent/bluetooth-hci-socket' {
    type DriverType = any; // Fallback type
    interface BindParams {
        [key: string]: any;
    }
}

declare module '@stoprocent/bleno' {
    import { EventEmitter } from 'events';

    export type State = 'poweredOn' | 'poweredOff' | 'unauthorized' | 'unsupported' | 'unknown' | 'resetting';

    export type Property = 'read' | 'write' | 'indicate' | 'notify' | 'writeWithoutResponse';

    export type ConnectionHandle = number | string;

    // Common callback types
    export type ReadRequestCallback = (result: number, data?: Buffer) => void;
    export type UpdateValueCallback = (data?: Buffer) => void;
    export type WriteRequestCallback = (result: number) => void;

    // Common function types
    export type OnReadRequestFn = (handle: ConnectionHandle, offset: number, callback: ReadRequestCallback) => void;
    export type OnSubscribeFn = (handle: ConnectionHandle, maxValueSize: number, updateValueCallback: UpdateValueCallback) => void;
    export type OnUnsubscribeFn = (handle: ConnectionHandle) => void;
    export type OnWriteRequestFn = (handle: ConnectionHandle, data: Buffer, offset: number, withoutResponse: boolean, callback: WriteRequestCallback) => void;
    export type OnIndicateFn = (handle: ConnectionHandle) => void;
    export type OnNotifyFn = (handle: ConnectionHandle) => void;

    export interface CharacteristicOptions {
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

    export class Characteristic extends EventEmitter {
        uuid: string;
        properties: ReadonlyArray<Property>;
        secure: ReadonlyArray<Property>;
        value: Buffer | null;
        descriptors: ReadonlyArray<Descriptor>;

        constructor(options: CharacteristicOptions);

        onIndicate(handle: ConnectionHandle): void;
        onNotify(handle: ConnectionHandle): void;
        onReadRequest(handle: ConnectionHandle, offset: number, callback: ReadRequestCallback): void;
        onSubscribe(handle: ConnectionHandle, maxValueSize: number, updateValueCallback: UpdateValueCallback): void;
        onUnsubscribe(handle: ConnectionHandle): void;
        onWriteRequest(handle: ConnectionHandle, data: Buffer, offset: number, withoutResponse: boolean, callback: WriteRequestCallback): void;

        toString(): string;

        on(event: 'readRequest', listener: (handle: ConnectionHandle, offset: number, callback: ReadRequestCallback) => void): this;
        on(event: 'writeRequest', listener: (handle: ConnectionHandle, data: Buffer, offset: number, withoutResponse: boolean, callback: WriteRequestCallback) => void): this;
        on(event: 'subscribe', listener: (handle: ConnectionHandle, maxValueSize: number, updateValueCallback: UpdateValueCallback) => void): this;
        on(event: 'unsubscribe', listener: (handle: ConnectionHandle) => void): this;
        on(event: 'notify', listener: (handle: ConnectionHandle) => void): this;
        on(event: 'indicate', listener: (handle: ConnectionHandle) => void): this;
        on(event: string | symbol, listener: (...args: any[]) => void): this;

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

    export interface DescriptorOptions {
        uuid: string;
        value?: Buffer | string | null;
    }

    export class Descriptor {
        uuid: string;
        value: Buffer;

        constructor(options: DescriptorOptions);

        toString(): string;
    }

    export interface PrimaryServiceOptions {
        uuid: string;
        characteristics?: ReadonlyArray<Characteristic> | null;
    }

    export class PrimaryService extends EventEmitter {
        uuid: string;
        characteristics: ReadonlyArray<Characteristic>;

        constructor(options: PrimaryServiceOptions);

        toString(): string;
    }

    export class Bleno extends EventEmitter {
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
        waitForPoweredOnAsync(timeout?: number): Promise<void>;

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

    export type BindingType = 'default' | 'hci' | 'mac';

    export interface BaseBindingsOptions {

    }

    export interface HciBindingsOptions extends BaseBindingsOptions {
        /** Driver Type ('default' | 'uart' | 'usb' | 'native') */
        hciDriver?: import('@stoprocent/bluetooth-hci-socket').DriverType;
        /** Bind Params (for USB and UART Hci Drivers only) */
        bindParams?: import('@stoprocent/bluetooth-hci-socket').BindParams;
        /** HCI Device ID (Linux Only), Default is 0 */
        deviceId?: number;
        /** Uses User channel instead of Raw HCI Channel */
        userChannel?: boolean;
    }

    export interface MacBindingsOptions extends BaseBindingsOptions {

    }

    export type WithBindingsOptions = HciBindingsOptions | MacBindingsOptions;

    export function withBindings(
        bindingType?: BindingType, 
        options?: WithBindingsOptions
    ): Bleno;

    // Define a default export
    const BlenoDefault: Bleno;
    export default BlenoDefault;
}