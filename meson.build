project(
    'bits-c',
    ['c', 'cpp'],
    version: '0.1.0',
    default_options: [
        'warning_level=2',
        'c_std=gnu11',
        'cpp_std=c++17',
    ],
)

# Dependencies
libcurl_dep = dependency('libcurl', required: false)
openssl_dep = dependency('openssl', required: false)
gl_dep = dependency('gl', required: false)
glu_dep = dependency('glu', required: false)
x11_dep = dependency('x11', required: false)
glew_dep = dependency('glew', required: false)
openmp_dep = dependency('openmp', required: false)
threads_dep = dependency('threads')

blocks_dep = []
cc = meson.get_compiler('c')
if cc.get_id() == 'clang'
    blocks_dep = dependency('blocks', required: false)
endif

# Python dependencies for hashtable extension
python = import('python').find_installation(required: false)
python3_dep = python.dependency(required: false)

# Include directories
inc_dir = include_directories('include')

# Core library objects
fnv_lib = static_library(
    'fnv',
    'lib/fnv.c',
    include_directories: inc_dir,
    c_args: ['-DNDEBUG'],
)

hashtable_lib = static_library(
    'hashtable',
    'lib/hashtable.c',
    include_directories: inc_dir,
    c_args: ['-DNDEBUG'],
)

arena_lib = static_library(
    'arena',
    'lib/arena.c',
    include_directories: inc_dir,
)

coroutine_lib = static_library(
    'coroutine',
    'lib/coroutine.c',
    include_directories: inc_dir,
)

expect_lib = static_library(
    'expect',
    'lib/expect.c',
    include_directories: inc_dir,
)

message_queue_lib = static_library(
    'message_queue',
    'lib/message_queue.c',
    include_directories: inc_dir,
)

# Executables
executable(
    'base64',
    'bin/base64.c',
    include_directories: inc_dir,
    dependencies: openssl_dep,
    install: true,
    install_dir: 'bin',
)

if libcurl_dep.found()
    executable(
        'curling',
        'bin/curling.c',
        include_directories: inc_dir,
        dependencies: libcurl_dep,
        install: true,
        install_dir: 'bin',
    )
endif

executable(
    'demo_oop',
    'bin/demo_oop.c',
    include_directories: inc_dir,
    install: true,
    install_dir: 'bin',
)

executable(
    'hello',
    'bin/hello.cpp',
    include_directories: inc_dir,
    install: true,
    install_dir: 'bin',
)


if openmp_dep.found()
    executable(
        'hello_omp',
        'bin/hello_omp.c',
        include_directories: inc_dir,
        dependencies: openmp_dep,
        install: true,
        install_dir: 'bin',
    )
endif

executable(
    'notramp',
    'bin/notramp.c',
    include_directories: inc_dir,
    install: true,
    install_dir: 'bin',
)

executable(
    'overflow',
    'bin/overflow.c',
    include_directories: inc_dir,
    c_args: ['-ftrapv'],
    install: true,
    install_dir: 'bin',
)

executable(
    'poll',
    'bin/poll.c',
    include_directories: inc_dir,
    install: true,
    install_dir: 'bin',
)

executable(
    'sum_tree',
    'bin/sum_tree.c',
    include_directories: inc_dir,
    dependencies: blocks_dep,
    link_with: arena_lib,
    install: true,
    install_dir: 'bin',
)

executable(
    'threadtest',
    'bin/threadtest.c',
    include_directories: inc_dir,
    dependencies: threads_dep,
    c_args: ['-D_POSIX_C_SOURCE=200809L'],
    install: true,
    install_dir: 'bin',
)

if gl_dep.found() and glu_dep.found() and x11_dep.found() and glew_dep.found()
    executable(
        'window',
        'bin/window.c',
        include_directories: inc_dir,
        dependencies: [x11_dep, gl_dep, glu_dep, glew_dep],
        install: true,
        install_dir: 'bin',
    )
endif

# Tests
arena_test = executable(
    'arena_test',
    'test/arena_test.c',
    include_directories: inc_dir,
    link_with: arena_lib,
)

coroutine_test = executable(
    'coroutine_test',
    'test/coroutine_test.c',
    include_directories: inc_dir,
    link_with: coroutine_lib,
)

expect_test = executable(
    'expect_test',
    'test/expect_test.c',
    include_directories: inc_dir,
    link_with: expect_lib,
)

fnv_test = executable(
    'fnv_test',
    'test/fnv_test.c',
    include_directories: inc_dir,
    link_with: fnv_lib,
)

hashtable_test = executable(
    'hashtable_test',
    'test/hashtable_test.c',
    include_directories: inc_dir,
    link_with: [hashtable_lib, fnv_lib],
)

message_queue_basic_test = executable(
    'message_queue_basic_test',
    'test/message_queue_basic.c',
    include_directories: inc_dir,
    link_with: message_queue_lib,
    dependencies: threads_dep,
)

message_queue_expected_lib = static_library(
    'message_queue_expected',
    'test/message_queue_expected.c',
    include_directories: inc_dir,
)

message_queue_block_test = executable(
    'message_queue_block_test',
    'test/message_queue_block.c',
    include_directories: inc_dir,
    link_with: [message_queue_lib, message_queue_expected_lib],
    dependencies: threads_dep,
)

lambda_test = executable(
    'lambda_test',
    'test/lambda_test.cpp',
    include_directories: inc_dir,
)

# Register tests
test('arena_test', arena_test)
test('coroutine_test', coroutine_test)
test('expect_test', expect_test)
test('fnv_test', fnv_test)
test('hashtable_test', hashtable_test)
test('lambda_test', lambda_test)
test('message_queue_basic_test', message_queue_basic_test)
test('message_queue_block_test', message_queue_block_test)

if python.found() and python3_dep.found()
    python.extension_module(
        'hashtable',
        'lib/hashtable_py.c',
        'lib/hashtable.c',
        'lib/fnv.c',
        include_directories: inc_dir,
        c_args: ['-DNDEBUG'],
        install: true,
    )

    test(
        'hashtable_python_test',
        python,
        args: [files('test/hashtable_test.py')],
        env: {'PYTHONPATH': meson.current_build_dir()},
    )
endif
