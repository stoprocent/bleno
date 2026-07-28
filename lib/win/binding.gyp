{
  'variables': {
    'openssl_fips': ''
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
        '../common/include',
      ],
      'defines': [
        'NAPI_CPP_EXCEPTIONS',
        'NOMINMAX',
        'WIN32_LEAN_AND_MEAN',
      ],
      'libraries': [
        'windowsapp.lib',
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          'ExceptionHandling': 1,
          'AdditionalOptions': [
            '/await',
            '/permissive-',
            '/std:c++17',
          ],
        },
      },
      'msvs_target_platform_version': '10.0',
      'msvs_target_platform_minversion': '10.0.17763.0',
    },
  ],
}
