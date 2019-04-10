#
# target: libcdvar.so
#

libcdvar_symfile = join_paths(meson.current_source_dir(), 'libcdvar.sym')

libcdvar_deps = [
        dep_cstdaux,
        dep_cutf8,
]

libcdvar_private = static_library(
        'cdvar-private',
        [
                'c-dvar.c',
                'c-dvar-common.c',
                'c-dvar-reader.c',
                'c-dvar-type.c',
                'c-dvar-writer.c',
        ],
        c_args: [
                '-fvisibility=hidden',
                '-fno-common',
        ],
        dependencies: libcdvar_deps,
        pic: true,
)

libcdvar_shared = shared_library(
        'cdvar',
        objects: libcdvar_private.extract_all_objects(),
        dependencies: libcdvar_deps,
        install: not meson.is_subproject(),
        soversion: 0,
        link_depends: libcdvar_symfile,
        link_args: [
                '-Wl,--no-undefined',
                '-Wl,--version-script=@0@'.format(libcdvar_symfile),
        ],
)

libcdvar_dep = declare_dependency(
        include_directories: include_directories('.'),
        link_with: libcdvar_private,
        dependencies: libcdvar_deps,
        version: meson.project_version(),
)

if not meson.is_subproject()
        install_headers(
                'c-dvar.h',
                'c-dvar-type.h',
        )

        mod_pkgconfig.generate(
                libraries: libcdvar_shared,
                version: meson.project_version(),
                name: 'libcdvar',
                filebase: 'libcdvar',
                description: project_description,
        )
endif

#
# target: test-*
#

test_api = executable('test-api', ['test-api.c'], link_with: libcdvar_shared)
test('API Symbol Visibility', test_api)

test_basic = executable('test-basic', ['test-basic.c'], dependencies: libcdvar_dep)
test('Basic API Behavior', test_basic)

if dep_typenum.found()
        test_enumerated = executable('test-enumerated', ['test-enumerated.c'], dependencies: [ libcdvar_dep, dep_typenum ])
        test('Type and Data Verification with Enumerated Types', test_enumerated)
endif

test_string = executable('test-string', ['test-string.c'], dependencies: libcdvar_dep)
test('D-Bus String Restrictions', test_string)

test_type = executable('test-type', ['test-type.c'], dependencies: libcdvar_dep)
test('Type and Signature Parser', test_type)
