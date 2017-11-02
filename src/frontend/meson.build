libfrontend_sources = [
    'category-button.c',
    'desktop-button.c',
    'entry-button.c',
    'launcher.c',
    'menu-context.c',
    'menu-grabs.c',
    'menu-keyboard.c',
    'menu-loader.c',
    'menu-search.c',
    'menu-session.c',
    'menu-settings.c',
    'menu-sort.c',
    'menu-window.c',
    'sidebar-scroller.c',
]

libfrontend_dependencies = [
    link_libbackend,
    link_libsession_stub,
    link_libutil,
    dep_gtk3,
]

libfrontend_includes = [
    include_directories('.'),
]

# Contains the main Brisk UI (Windowed)
libfrontend = static_library(
    'brisk-frontend',
    sources: libfrontend_sources,
    dependencies: libfrontend_dependencies,
    include_directories: libfrontend_includes,
    install: false,
)

# Allow linking the frontend
link_libfrontend = declare_dependency(
    link_with: libfrontend,
    dependencies: libfrontend_dependencies,
    include_directories: libfrontend_includes,
)
