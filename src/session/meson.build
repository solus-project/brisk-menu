# libsaver_glue provides dbus code for the MATE screensaver
libsaver_glue = gnome.gdbus_codegen(
    'libsaver-glue',
    'org.mate.ScreenSaver.xml',
    interface_prefix : 'org.mate.',
    namespace : 'Mate',
)

# libsession_glue provides dbus code for the MATE/GNOME session
libsession_glue = gnome.gdbus_codegen(
    'libsession-glue',
    'org.gnome.SessionManager.xml',
    interface_prefix : 'org.gnome.',
    namespace : 'Gnome',
)

# Build the Cs into a simple static library and control compiler noise
libsession_stub = static_library(
    'brisk-glue',
    sources: [
        libsaver_glue,
        libsession_glue,
    ],
    c_args: [
        '-Wno-unused-parameter',
    ],
    dependencies: dep_gio_unix,
    install: false,
)

# Allow linking to the stub
link_libsession_stub = declare_dependency(
    link_with: libsession_stub,
    dependencies: [
        dep_gio_unix,
    ],
    include_directories: [
        include_directories('.'),
    ],
)
