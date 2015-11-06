{
    'variables' : {
        'source_files': [
            'src/binding.cpp',
            'src/base32.hpp',
            'src/package.hpp',
            'src/package.cpp',
            'src/path.hpp',
            'src/path.cpp',
         ],
    },
    'targets': [
        {
            'target_name': 'iris-encrypt',
            'dependencies': ['extern/extern.gyp:*'],
            'sources': ['<@(source_files)'],
            'defines': ['IRIS_ENCRYPT'],
            'cflags_cc': ['-std=c++11'],
            'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
            'xcode_settings': {
                'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                'GCC_ENABLE_CPP_RTTI': 'YES',
                'MACOSX_DEPLOYMENT_TARGET': '10.7',
                'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11', '-stdlib=libc++'],
                'OTHER_LDFLAGS': ['-stdlib=libc++'],
            },
            'configurations': { 'Release': { 'msvs_settings': { 'VCCLCompilerTool': {
                'ExceptionHandling': 1,
                'RuntimeTypeInfo': 'true',
            }}}},
        },
        {
            'target_name': 'iris-decrypt',
            'dependencies': ['extern/extern.gyp:*'],
            'sources': ['<@(source_files)'],
            'defines': ['IRIS_DECRYPT'],
            'cflags_cc': ['-std=c++11'],
            'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
            'xcode_settings': {
                'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                'GCC_ENABLE_CPP_RTTI': 'YES',
                'MACOSX_DEPLOYMENT_TARGET': '10.7',
                'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11', '-stdlib=libc++'],
                'OTHER_LDFLAGS': ['-stdlib=libc++'],
            },
            'configurations': { 'Release': { 'msvs_settings': { 'VCCLCompilerTool': {
                'ExceptionHandling': 1,
                'RuntimeTypeInfo': 'true',
            }}}},
        },
        {
            'target_name': 'iris-crypt-dist',
            'type': 'none',
            'dependencies': ['iris-encrypt', 'iris-decrypt'],
            'variables': {
                'variables': {
                    'node_platform': '<!(node -e "console.log(process.platform)")',
                    'node_arch':     '<!(node -e "console.log(process.arch)")',
                    'root_dir':      '<!(node -e "console.log(process.cwd())")',
                },

                'node_platform%': '<(node_platform)',
                'node_arch%':     '<(node_arch)',
                'root_dir%':      '<(root_dir)',
                'target_dir':     '<(root_dir)/../iris-crypt-bin',
                },
            'copies': [
                {
                    'destination': '<(root_dir)/bin/<(node_platform)/<(node_arch)',
                    'files': [
                        '<(PRODUCT_DIR)/iris-encrypt.node',
                        '<(PRODUCT_DIR)/iris-decrypt.node',
                    ],
                },
                {
                    'destination': '<(target_dir)/bin/<(node_platform)/<(node_arch)',
                    'files': [
                        '<(PRODUCT_DIR)/iris-encrypt.node',
                        '<(PRODUCT_DIR)/iris-decrypt.node',
                    ],
                },
                {
                    'destination': '<(target_dir)',
                    'files': [
                        '<(root_dir)/tests/',
                        '<(root_dir)/index.js',
                        '<(root_dir)/package.json',
                        '<(root_dir)/README.md',
                    ],
                },
            ],
        },
    ]
}
