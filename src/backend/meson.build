libbackend_sources = [
    'backend.c',
    'item.c',
    'section.c',
    'all-items/all-backend.c',
    'all-items/all-section.c',
    'apps/apps-backend.c',
    'apps/apps-item.c',
    'apps/apps-section.c',
    'favourites/favourites-backend.c',
    'favourites/favourites-desktop.c',
    'favourites/favourites-section.c',
]

libbackend_dependencies = [
    dep_mate_menu,
    dep_gio_unix,
]

libbackend_includes = [
    lib_h_dir,
    include_directories('.'),
    tree_include,
]

# Contains the bulk of the internal Brisk Menu code
libbackend = static_library(
    'brisk-backend',
    sources: libbackend_sources,
    dependencies: libbackend_dependencies,
    include_directories: libbackend_includes,
    install: false,
)

link_libbackend = declare_dependency(
    link_with: libbackend,
    dependencies: libbackend_dependencies,
    include_directories: libbackend_includes,
)
