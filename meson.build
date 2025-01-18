project('my_project', 'c', version: '1.0', meson_version: '>= 0.56.0')

# Dependencies
gtk_dep = dependency('gtk4')
glfw_dep = dependency('glfw3')
glew_dep = dependency('glew')

# Compiler options
cc = meson.get_compiler('c')

# Source files
src_files = ['src/main.c', 'src/gl.c']

# Executable
executable('my_project', src_files,
  dependencies: [glfw_dep, glew_dep, gtk_dep],
  c_args: ['-Wall', '-g', '-O2'],
  link_args: ['-lm'],
  install: true
)

# Build options (optional, like adding a subdir for assets or other)
