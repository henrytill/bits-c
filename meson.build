project(
    'bits-c',
    ['c', 'cpp'],
    version: '0.1.0',
    default_options: [
        'warning_level=3',
        'c_std=c89',
        'cpp_std=c++17',
    ],
)

add_project_arguments('-D_DEFAULT_SOURCE', language: 'c')

openssl_dep = dependency('openssl', required: false)
threads_dep = dependency('threads')

inc_dir = include_directories('include')

bits = library(
    'bits',
    sources: [
        'src/libbits/arena.c',
        'src/libbits/fnv.c',
        'src/libbits/hashtable.c',
        'src/libbits/channel.c',
        'src/libbits/printf.c',
    ],
    include_directories: inc_dir,
    dependencies: threads_dep,
    c_args: ['-DNDEBUG'],
)

arena_test = executable(
    'arena_test',
    'src/cmd/arena_test.c',
    include_directories: inc_dir,
    link_with: bits,
)

executable(
    'base64',
    'src/cmd/base64.c',
    include_directories: inc_dir,
    dependencies: openssl_dep,
    link_with: bits,
    install: true,
    install_dir: 'bin',
)

executable(
    'demo_oop',
    'src/cmd/demo_oop.c',
    include_directories: inc_dir,
    install: true,
    install_dir: 'bin',
)

fnv_test = executable(
    'fnv_test',
    'src/cmd/fnv_test.c',
    include_directories: inc_dir,
    link_with: bits,
)

hashtable_test = executable(
    'hashtable_test',
    'src/cmd/hashtable_test.c',
    include_directories: inc_dir,
    link_with: bits,
)

executable(
    'hello',
    'src/cmd/hello.cpp',
    include_directories: inc_dir,
    install: true,
    install_dir: 'bin',
)

lambda = executable(
    'lambda',
    'src/cmd/lambda.cpp',
    include_directories: inc_dir,
)

channel_basic_test = executable(
    'channel_basic_test',
    'src/cmd/channel_basic.c',
    include_directories: inc_dir,
    link_with: bits,
    dependencies: threads_dep,
)

channel_block_test = executable(
    'channel_block_test',
    sources: ['src/cmd/channel_block.c', 'src/cmd/channel_expected.c'],
    include_directories: inc_dir,
    link_with: bits,
    dependencies: threads_dep,
)

test('arena_test', arena_test)
test('fnv_test', fnv_test)
test('hashtable_test', hashtable_test)
test('lambda', lambda)
test('channel_basic_test', channel_basic_test)
test('channel_block_test', channel_block_test)
