project(
        'c-dvar',
        'c',
        version: '1',
        license: 'Apache',
        default_options: [
                'c_std=c11',
        ],
)
project_description = 'D-Bus Variant Type-System'

add_project_arguments('-D_GNU_SOURCE', language: 'c')
mod_pkgconfig = import('pkgconfig')

sub_cstdaux = subproject('c-stdaux')
sub_cutf8 = subproject('c-utf8')

dep_cstdaux = sub_cstdaux.get_variable('libcstdaux_dep')
dep_cutf8 = sub_cutf8.get_variable('libcutf8_dep')
dep_typenum = dependency('libdbus-typenum', version: '>=1', required: false)

subdir('src')
