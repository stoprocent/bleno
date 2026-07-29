#pragma once

#include <napi.h>
#include <vector>
#include <functional>

class ThreadSafeCallback {
public:
    // More descriptive type aliases
    using ArgumentVector = std::vector<napi_value>;
    using ArgumentFunction = std::function<void(napi_env, ArgumentVector&)>;

    // Constructor with validation
    ThreadSafeCallback(const Napi::Value& receiver, const Napi::Function& jsCallback);
    
    // Destructor
    ~ThreadSafeCallback();

    // Delete copy and move operations explicitly
    ThreadSafeCallback(const ThreadSafeCallback&) = delete;
    ThreadSafeCallback& operator=(const ThreadSafeCallback&) = delete;
    ThreadSafeCallback& operator=(ThreadSafeCallback&&) = delete;

    // Public interface
    void call(ArgumentFunction argFunction);

private:
    struct CallbackContext {
        Napi::Reference<Napi::Value> receiver;
    };

    // Static callback handler
    static void callJsCallback(Napi::Env env,
                             Napi::Function jsCallback,
                             CallbackContext* context,
                             ArgumentFunction* argFn);

    // Type alias for the thread-safe function
    using ThreadSafeFunc = Napi::TypedThreadSafeFunction<
        CallbackContext,
        ArgumentFunction,
        callJsCallback>;

    // Member variables
    ThreadSafeFunc threadSafeFunction_;
};

// Implementation

inline ThreadSafeCallback::ThreadSafeCallback(
    const Napi::Value& receiver,
    const Napi::Function& jsCallback) {
    
    if (!(receiver.IsObject() || receiver.IsFunction())) {
        throw Napi::Error::New(jsCallback.Env(),
            "Callback receiver must be an object or function");
    }
    if (!jsCallback.IsFunction()) {
        throw Napi::Error::New(jsCallback.Env(),
            "Callback must be a function");
    }

    auto* context = new CallbackContext();
    context->receiver = Napi::Persistent(receiver);
    try {
        threadSafeFunction_ = ThreadSafeFunc::New(
            jsCallback.Env(),
            jsCallback,
            "ThreadSafeCallback callback",
            0, 1,
            context,
            [](Napi::Env, void*, CallbackContext* callbackContext) {
                delete callbackContext;
            });
    } catch (...) {
        delete context;
        throw;
    }
}

inline ThreadSafeCallback::~ThreadSafeCallback() {
    threadSafeFunction_.Release();
}

inline void ThreadSafeCallback::call(ArgumentFunction argFunction) {
    auto argFn = new ArgumentFunction(argFunction);
    if (threadSafeFunction_.NonBlockingCall(argFn) != napi_ok) {
        delete argFn;
    }
}

inline void ThreadSafeCallback::callJsCallback(
    Napi::Env env,
    Napi::Function jsCallback,
    CallbackContext* context,
    ArgumentFunction* argFn) {

    std::unique_ptr<ArgumentFunction> argumentFunction(argFn);
    if (argumentFunction == nullptr ||
        env == nullptr ||
        jsCallback == nullptr ||
        context == nullptr) {
        return;
    }

    ArgumentVector args;
    (*argumentFunction)(env, args);
    jsCallback.Call(context->receiver.Value(), args);
}
