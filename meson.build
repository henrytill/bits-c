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

openssl_dep = dependency('openssl', required: false)
threads_dep = dependency('threads')

python = import('python').find_installation(required: false)
python3_dep = python.dependency(required: false)

inc_dir = include_directories('include')

bits = library(
    'bits',
    sources: [
        'src/libbits/arena.c',
        'src/libbits/fnv.c',
        'src/libbits/hashtable.c',
        'src/libbits/message_queue.c'
    ],
    include_directories: inc_dir,
    dependencies: threads_dep,
    c_args: ['-DNDEBUG'],
)

# Executables
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

message_queue_basic_test = executable(
    'message_queue_basic_test',
    'src/cmd/message_queue_basic.c',
    include_directories: inc_dir,
    link_with: bits,
    dependencies: threads_dep,
)

message_queue_block_test = executable(
    'message_queue_block_test',
    sources: [
        'src/cmd/message_queue_block.c',
        'src/cmd/message_queue_expected.c'
    ],
    include_directories: inc_dir,
    link_with: bits,
    dependencies: threads_dep,
)


# Register tests
test('arena_test', arena_test)
test('fnv_test', fnv_test)
test('hashtable_test', hashtable_test)
test('lambda', lambda)
test('message_queue_basic_test', message_queue_basic_test)
test('message_queue_block_test', message_queue_block_test)

if python.found() and python3_dep.found()
    python.extension_module(
        'hashtable',
        sources:[
            'src/libbits/fnv.c',
            'src/libbits/hashtable.c',
            'src/libbits/hashtable_py.c'
        ],
        include_directories: inc_dir,
        c_args: ['-DNDEBUG'],
        install: true,
    )

    test(
        'hashtable_python_test',
        python,
        args: [files('src/cmd/hashtable_test.py')],
        env: {'PYTHONPATH': meson.current_build_dir()},
    )
endif
