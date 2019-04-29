project('memory-pool', 'c',
        version : '0.0.1',
        default_options : [
            'c_std=gnu11',
            'warning_level=3',
            'werror=true',
            'buildtype=release',
            'strip=true',
        ],
        license : 'MIT',
)

cc = meson.get_compiler('c')

configuration_inc = include_directories('src')

flags = ['-Wshadow', '-Wstrict-prototypes', '-Wmissing-prototypes',
         '-Wno-padded',
         '-DCONFIG_LOG2_CPU_CACHELINE_SIZE=6',
         '-DCONFIG_LOG2_CPU_PAGE_SIZE=12',
]
foreach f : flags
    if cc.has_argument(f)
        add_project_arguments(f, language : 'c')
    endif
endforeach

if get_option('error')
    add_project_arguments('-Werror', language : 'c')
endif # error

if get_option('memcheck')
    add_project_arguments('-DMEMCHECK', language : 'c')
endif # memcheck

headers = files('src/mpool.h')
sources = files(
        'src/common.h',
        'src/mpool.c',
        'src/mpool.h',
        'src/mpool_memcheck.h',
)

all_tests_sources = files(
    'test/test_mpool.c',
    'test/test_mthread_mpool.c',
    'test/test_mpool_overload.c',
)

install_headers('src/mpool.h')

libthread = dependency('threads')
mpool = shared_library('mpool',
        sources,
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
            'test/test_mpool.c',
            include_directories : include_directories('src', 'test'),
            link_with : mpool)

    test('simple mpool test', simple)

    mthread = executable('test_mthread_mpool',
            'test/test_mthread_mpool.c',
            link_with : mpool,
            include_directories : include_directories('src', 'test'),
            dependencies : libthread
    )
    test('simple multithreaded mpool test', mthread)

    libdl = cc.find_library('dl', required : true)
    mpool_overload = shared_library('test_mpool_overload',
            'test/test_mpool_overload.c',
            link_with : mpool,
            include_directories : include_directories('src', 'test'),
            dependencies : libdl
    )
endif # tests
