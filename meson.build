project('fgsim_control', 'cpp',
  version : '0.1.0',
  default_options : [
    'cpp_std=c++23',
    'warning_level=3',
    'buildtype=debug'
  ]
)

# Add compiler flags
cpp_compiler = meson.get_compiler('cpp')
if cpp_compiler.get_id() == 'gcc' or cpp_compiler.get_id() == 'clang'
  add_project_arguments('-Wall', '-Wextra', '-Wpedantic', language: 'cpp')
endif

# Find Boost dependency
boost_dep = dependency('boost', modules : ['system', 'thread'], version : '>=1.74.0')

# Process subdirectories
subdir('libs')
subdir('apps')
