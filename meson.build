project('memory-pool', 'c',
        default_options : [
            'warning_level=3',
            'werror=true',
            'buildtype=release',
            'strip=true',
        ],
        license : 'MIT',
        meson_version: '>= 0.47',
)

# follow semantic versioning (https://semver.org)
major = '0' # incompatible API changes
minor = '1' # add backwards-compatible functionality
patch = '0' # backwards-compatible bug fixes
version = major + '.' + minor + '.' + patch

cc = meson.get_compiler('c')
configuration_inc = include_directories('src')

flags = ['-Wshadow', '-Wstrict-prototypes', '-Wmissing-prototypes',
         '-Wno-padded',
         '-DCONFIG_LOG2_CPU_CACHELINE_SIZE=6',
         '-DCONFIG_LOG2_CPU_PAGE_SIZE=12',
]
add_project_arguments(cc.get_supported_arguments(flags), language : 'c')

if get_option('memcheck')
    add_project_arguments('-DMEMCHECK', language : 'c')
endif # memcheck

sources = files(
        'src/common.h',
        'src/mpool.c',
        'src/mpool.h',
        'src/mpool_memcheck.h',
)
public_headers = files('src/mpool.h')
install_headers(public_headers)


all_tests_sources = files(
    'test/test_mpool.c',
    'test/test_mthread_mpool.c',
    'test/test_mpool_overload.c',
    'test/xmalloc-test.c',
)

libthread = dependency('threads')
mpool = shared_library('mpool',
        sources,
        version : version,
        install : true,
        include_directories : configuration_inc,
        dependencies : [libthread],
)


#
# DEVTOOLS
#

uncrustify = find_program('uncrustify', required : false)
if uncrustify.found()
    run_target('checkstyle',
        command : [
            uncrustify,
            '-l', 'c',
            '-c', join_paths(meson.source_root(), 'devtools', 'uncrustify.cfg'),
            '--check',
            sources,
            all_tests_sources,
        ],
    )
    run_target('fixstyle',
        command : [
            uncrustify,
            '-l', 'c',
            '-c', join_paths(meson.source_root(), 'devtools', 'uncrustify.cfg'),
            '--replace',
            sources,
            all_tests_sources,
        ],
    )
endif # uncrustify

codespell = find_program('codespell', required : false)
if codespell.found()
    run_target('spelling',
        command : [
            codespell,
            sources,
            all_tests_sources,
        ]
    )
endif # codespell


#
# TESTS
#

if get_option('tests')
    simple = executable('test_pool',
            files('test/test_mpool.c'),
            include_directories : include_directories('src', 'test'),
            link_with : mpool)

    test('simple mpool test', simple)

    mthread = executable('test_mthread_mpool',
            files('test/test_mthread_mpool.c'),
            link_with : mpool,
            include_directories : include_directories('src', 'test'),
            dependencies : libthread
    )
    test('simple multithreaded mpool test', mthread)

    libdl = cc.find_library('dl', required : true)
    mpool_overload = shared_library('test_mpool_overload',
            files('test/test_mpool_overload.c'),
            link_with : mpool,
            include_directories : include_directories('src', 'test'),
            dependencies : libdl
    )

    system_alloc = executable('system-alloc',
            files('test/test_system_allocs.c'),
            include_directories : include_directories('src', 'test'),
    )

    xmalloc_test = executable('xmalloc-test',
            files('test/xmalloc-test.c'),
            include_directories : include_directories('src', 'test'),
            link_with : mpool,
            dependencies : libthread
    )
endif # tests
