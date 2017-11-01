mate_applet_sources = [
    'applet.c',
    'applet-position.c',
    'main.c',
]

mate_applet_dependencies = [
    link_libbackend,
    link_libfrontend,
    link_libresources,
    link_libutil,
    dep_mate_applet,
    dep_notify,
]

mate_applet = executable(
    'brisk-menu',
    sources: mate_applet_sources,
    dependencies: mate_applet_dependencies,
    include_directories: [
        config_h_dir,
    ],
    install_dir: path_libexecdir,
    install: true,
)
