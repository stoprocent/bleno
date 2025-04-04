{
  'variables': {
    'openssl_fips' : '' 
  },
  'targets': [
    {
      'target_name': 'binding',
      'sources': [ 
        "<!@(node -p \"require('fs').readdirSync('src').filter(f=>new RegExp('.*\\\\.(c|cc|cpp|mm)$').test(f)).map(f=>'src/'+f).join(' ')\")",
      ],
      'include_dirs': [
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "../common/include"
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
