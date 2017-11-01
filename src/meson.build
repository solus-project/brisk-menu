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

# Build common library (currently just keybinder really.)
subdir('lib')

# Build the backend component
subdir('backend')

# Build session glue to help the frontend out
subdir('session')

# Now build our main UI
subdir('frontend')

# Finally, we can build the MATE Applet itself
subdir('mate-applet')
