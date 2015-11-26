#
# Copyright (c) 2015 ASPECTRON Inc.
# All Rights Reserved.
#
# This file is part of IrisCrypt (https://github.com/aspectron/iris-crypt) project.
#
# Distributed under the MIT software license, see the accompanying
# file LICENSE
#
# External libraries built with non-GYP scripts
{
    'targets': [
        {
            'target_name': 'v8pp',
            'type': 'none',
            'direct_dependent_settings': {
                'include_dirs': ['.'],
            },
        },
        {
            'target_name': 'yas',
            'type': 'none',
            'direct_dependent_settings': {
                'include_dirs': ['yas/include'],
            },
        },
    ],
}
