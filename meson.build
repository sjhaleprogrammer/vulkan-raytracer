project('Vulkan Render', 'c', version: '1.0', meson_version: '>= 0.56.0')

# Dependencies
gtk_dep = dependency('gtk4')
glfw_dep = dependency('glfw3')
glew_dep = dependency('glew')

# Compiler options
cc = meson.get_compiler('c')

# Source files
src_files = ['src/main.c','src/render.c','src/window.c','lib/glad-vulkan1.4/src/vulkan.c']

# Executable
executable('vkrender', src_files,
  dependencies: [glfw_dep, glew_dep, gtk_dep],
  c_args: ['-Wall', '-g', '-O2'],
  link_args: ['-lm'],
  install: true
)

# Build options (optional, like adding a subdir for assets or other)
build_options = {'buildtype': 'debug', 'optimization': 'g'}