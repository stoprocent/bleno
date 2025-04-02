{
  'variables': {
    'openssl_fips' : '' 
  },
  'targets': [
    {
      'target_name': 'binding',
      'sources': [ 
        'src/bleno_winrt.cc',  
        'src/callbacks.cc',
        'src/radio_watcher.cc',
        'src/winrt_cpp.cc',
      ],
      'include_dirs': [
        "<!(node -p \"require('node-addon-api').include_dir\")",
        "<!@(node -p \"require('napi-thread-safe-callback').include\")"
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          'ExceptionHandling': 1,
          'AdditionalOptions': [
            '/await', 
            '/std:c++latest'
          ],
        },
      },
      'msvs_target_platform_version':'10.0.18362.0',
      'msvs_target_platform_minversion':'10.0.18362.0',
      'conditions': [
        ['OS=="win"', { 
          'defines': [ '_HAS_EXCEPTIONS=1', 'NAPI_CPP_EXCEPTIONS' ] 
        }]
      ],
    }
  ]
}