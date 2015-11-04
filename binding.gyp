{
    'targets': [
        {
            'target_name': 'iris-crypt',
            'dependencies': [
                'extern/extern.gyp:yas',
                'extern/extern.gyp:v8pp',
            ],
            'sources': [
                'src/binding.cpp',
                'src/base32.hpp',
                'src/package.hpp',
                'src/package.cpp',
                'src/path.hpp',
                'src/path.cpp',
            ],
            'cflags_cc': ['-std=c++11'],
            'cflags_cc!': ['-fno-rtti', '-fno-exceptions'],
            'configurations': { 'Release': { 'msvs_settings': { 'VCCLCompilerTool': {
                'ExceptionHandling': 1,
                'RuntimeTypeInfo': 'true',
            }}}},
            'xcode_settings': {
                'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                'GCC_ENABLE_CPP_RTTI': 'YES',
                'MACOSX_DEPLOYMENT_TARGET': '10.7',
                'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11', '-stdlib=libc++'],
                'OTHER_LDFLAGS': ['-stdlib=libc++'],
            },
        },
        {
            'target_name': 'iris-crypt-dist',
            'type': 'none',
            'dependencies': ['iris-crypt'],
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
                    'files': ['<(PRODUCT_DIR)/iris-crypt.node'],
                },
                {
                    'destination': '<(target_dir)/bin/<(node_platform)/<(node_arch)',
                    'files': ['<(PRODUCT_DIR)/iris-crypt.node'],
                },
                {
                    'destination': '<(target_dir)',
                    'files': [
                        '<(root_dir)/tests/',
                        '<(root_dir)/main.js',
                        '<(root_dir)/package.json',
                        '<(root_dir)/README.md',
                    ],
                },
            ],
        },
    ]
}