#
# target: static/shared library
# We build a single static library of all sources, so we can link it into our
# unit tests. Additionally, we build a shared library out of it to install.
#

libcdvar_private = static_library('cdvar-private',
                                  [
                                        'c-dvar.c',
                                        'c-dvar-reader.c',
                                        'c-dvar-type.c',
                                        'c-dvar-writer.c'
                                  ],
                                  c_args: [
                                        '-fvisibility=hidden',
                                        '-fno-common'
                                  ],
                                  pic: true)
libcdvar_dep = declare_dependency(include_directories: include_directories('.'),
                                  link_with: libcdvar_private,
                                  version: meson.project_version())
install_headers('c-dvar.h')
libcdvar_symfile = join_paths(meson.current_source_dir(), 'libcdvar.sym')
libcdvar_shared = shared_library('cdvar',
                                 objects: libcdvar_private.extract_all_objects(),
                                 install: true,
                                 soversion: 0,
                                 link_depends: libcdvar_symfile,
                                 link_args: [
                                        '-Wl,--no-undefined',
                                        '-Wl,--version-script=@0@'.format(libcdvar_symfile)
                                 ])
mod_pkgconfig.generate(libraries: libcdvar_shared,
                       version: meson.project_version(),
                       name: 'libcdvar',
                       filebase: 'libcdvar',
                       description: 'D-Bus Variant Type-System')

#
# target: test-*
# Bunch of unit, coverage, and runtime tests for public symbols and internal
# library behavior.
#

test_api = executable('test-api', ['test-api.c'], link_with: libcdvar_shared)
test('API Symbol Visibility', test_api)

test_basic = executable('test-basic', ['test-basic.c'], dependencies: libcdvar_dep)
test('Basic API Behavior', test_basic)
