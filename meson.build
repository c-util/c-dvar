project(
        'c-dvar',
        'c',
        default_options: [
                'c_std=c11',
        ],
        license: 'Apache',
        meson_version: '>=0.60.0',
        version: '1.0.0',
)
major = meson.project_version().split('.')[0]
project_description = 'D-Bus Variant Type-System'

mod_pkgconfig = import('pkgconfig')

dep_cstdaux = dependency('libcstdaux-1')
dep_cutf8 = dependency('libcutf8-1')
dep_typenum = dependency('libdbus-typenum', version: '>=1', required: false)
add_project_arguments(dep_cstdaux.get_variable('cflags').split(' '), language: 'c')

subdir('src')

meson.override_dependency('libcdvar-'+major, libcdvar_dep, static: true)
