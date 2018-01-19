# Allow inclusion of "backend/item.h" etc.
tree_include = include_directories('.')

# Build the resources separately so we can control our compiler warnings to
# remove a whole bunch of noise.
libresources = static_library(
    'brisk-resources',
    sources: brisk_resources,
    dependencies: [
        dep_gio_unix,
    ],
)

link_libresources = declare_dependency(
    link_with: libresources,
    include_directories: [
        data_include,
    ],
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

# Build common library (currently just keybinder really.)
subdir('lib')

# Build the backend component
subdir('backend')

# Now build our main UI
subdir('frontend')

# Finally, we can build the MATE Applet itself
subdir('mate-applet')
