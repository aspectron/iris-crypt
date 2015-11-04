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
