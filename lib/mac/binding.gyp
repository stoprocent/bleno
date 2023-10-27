{
  'variables': {
    'openssl_fips' : '' 
  },
  'targets': [
    {
      'target_name': 'binding',
      'sources': [ 
        'src/bleno_mac.mm', 
        'src/napi_objc.mm', 
        'src/ble_peripheral_manager.mm', 
        'src/objc_cpp.mm',
        'src/callbacks.mm' 
      ],
      'include_dirs': [
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "<!@(node -p \"require('napi-thread-safe-callback').include\")"
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      "defines": ["NAPI_CPP_EXCEPTIONS"],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'MACOSX_DEPLOYMENT_TARGET': '10.13',
        'CLANG_CXX_LIBRARY': 'libc++',
        'OTHER_CFLAGS': [
          '-fobjc-arc',
          '-arch x86_64',
          '-arch arm64'
        ],
        'OTHER_LDFLAGS': [
          '-framework CoreBluetooth',
          '-arch x86_64',
          '-arch arm64'
        ]
      }
    }
  ]
}
