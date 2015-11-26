#
# Copyright (c) 2015 ASPECTRON Inc.
# All Rights Reserved.
#
# This file is part of IrisCrypt (https://github.com/aspectron/iris-crypt) project.
#
# Distributed under the MIT software license, see the accompanying
# file LICENSE
#
{
    'variables': {
        'modver_file':     'src/modver.js',
        'target_platform': '<!(node -e "console.log(process.platform)")',
        'target_modules':  '<!(node <(modver_file) <(nodedir))',
        'root_dir':        '<!(node -e "console.log(process.cwd())")',
        'target_dir':      '<(root_dir)/../iris-crypt-bin',
        'addon_name':      'iris-crypt_<(target_platform)_<(target_arch)_m<(target_modules)',
    },
    'targets': [
        {
            'target_name': 'iris-crypt',
            'product_name': '<(addon_name)',
            'dependencies': ['extern/extern.gyp:*'],
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
            'xcode_settings': {
                'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
                'GCC_ENABLE_CPP_RTTI': 'YES',
                'MACOSX_DEPLOYMENT_TARGET': '10.7',
                'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11', '-stdlib=libc++'],
                'OTHER_LDFLAGS': ['-stdlib=libc++'],
            },
            'configurations': {
                'Release': { 'msvs_settings': { 'VCCLCompilerTool': {
                    'ExceptionHandling': 1,
                    'RuntimeTypeInfo': 'true',
                }}},
                'Debug': { 'msvs_settings': { 'VCCLCompilerTool': {
                    'ExceptionHandling': 1,
                    'RuntimeTypeInfo': 'true',
                }}},
            },
        },
        {
            'target_name': 'iris-crypt-dist',
            'type': 'none',
            'dependencies': ['iris-crypt'],
            'copies': [
                {
                    'destination': '<(root_dir)/bin',
                    'files': ['<(PRODUCT_DIR)/<(addon_name).node'],
                },
                {
                    'destination': '<(target_dir)/bin',
                    'files': ['<(PRODUCT_DIR)/<(addon_name).node'],
                },
                {
                    'destination': '<(target_dir)',
                    'files': [
                        '<(root_dir)/tests/',
                        '<(root_dir)/index.js',
                        '<(root_dir)/package.json',
                        '<(root_dir)/LICENSE',
                        '<(root_dir)/README.md',
                    ],
                },
            ],
        },
    ]
}
