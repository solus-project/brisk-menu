libutil_sources = [
    'key-binder.c',
]

libutil_dependencies = [
    dep_x11,
    dep_gdkx11,
    dep_gtk3,
]

# Util library handles keybinder
libutil = static_library(
    'brisk-util',
    sources: libutil_sources,
    dependencies: libutil_dependencies,
)

link_libutil = declare_dependency(
    link_with: libutil,
    include_directories: [
        include_directories('.'),
    ],
    dependencies: libutil_dependencies,
)

# Ensure others can access this directory without having to link
lib_h_dir = include_directories('.')
