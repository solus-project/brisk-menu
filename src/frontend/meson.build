libfrontend_sources = [
    'entry-button.c',
    'launcher.c',
    'menu-context.c',
    'menu-grabs.c',
    'menu-keyboard.c',
    'menu-loader.c',
    'menu-loader.c',
    'menu-search.c',
    'menu-session.c',
    'menu-settings.c',
    'menu-sort.c',
    'menu-window.c',
    'classic/category-button.c',
    'classic/classic-entry-button.c',
    'classic/classic-window.c',
    'classic/desktop-button.c',
    'classic/sidebar-scroller.c',
    'dash/category-button.c',
    'dash/dash-entry-button.c',
    'dash/dash-window.c',
]

libfrontend_dependencies = [
    link_libbackend,
    link_libsession_stub,
    link_libutil,
    dep_gtk3,
]

libfrontend_includes = [
    include_directories('.'),
    include_directories('../../data'),
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
