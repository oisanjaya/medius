project('medius', 'cpp', 'c',
  version: '0.1.0',
  default_options: [
    'cpp_std=c++20',
    'c_std=c11'
  ]
)

gnome = import('gnome')
cmake = import('cmake')

inc_dirs = include_directories(
  'include',
  'include/Lyra/include'
)

ckdl_proj = cmake.subproject('ckdl')
ckdl_dep = ckdl_proj.dependency('kdlpp')

gtk_dep = dependency('gtkmm-4.0')
spdlog_dep = dependency('spdlog')
gtk4_layer_shell_dep = dependency('gtk4-layer-shell-0')
librsvg_deps = dependency('librsvg-2.0')

subdir('data')
subdir('src')
