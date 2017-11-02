project(
    'brisk-menu',
    ['c'],
    version: '0.5.0',
    license: [
        'GPL-2.0',
        'CC-BY-SA-4.0',
    ],
    default_options: [
        'c_std=c11',
        'prefix=/usr',
        'sysconfdir=/etc',
    ],
)

am_cflags = [
    '-fstack-protector',
    '-Wall',
    '-pedantic',
    '-Wstrict-prototypes',
    '-Wundef',
    '-fno-common',
    '-Werror-implicit-function-declaration',
    '-Wformat',
    '-Wformat-security',
    '-Werror=format-security',
    '-Wconversion',
    '-Wunused-variable',
    '-Wunreachable-code',
    '-W',
    '-D_FORTIFY_SOURCE=2',
]

gtk_version_flags = [
    '-DGDK_VERSION_MAX_ALLOWED=GDK_VERSION_3_18',
    '-DGDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_18',
    '-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_2_44',
    '-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_44',
]

# Add our main flags
add_global_arguments(am_cflags, language: 'c')

# Handle glib schemas & icons
meson.add_install_script('meson_post_install.sh')

# Ensure we limit the GTK version used!
add_global_arguments(gtk_version_flags, language: 'c')

# Need GNOME module throughout Brisk build
gnome = import('gnome')

# Required minimum versions
gtk_min_version = '>= 3.18.0'
mate_min_version = '>= 1.16.0'
glib_min_version = '>= 2.44.0'

# GTK/UI deps
dep_gtk3 = dependency('gtk+-3.0', version: gtk_min_version)
dep_gio_unix = dependency('gio-unix-2.0', version: glib_min_version)
dep_gdkx11 = dependency('gdk-x11-3.0', version: gtk_min_version)
dep_x11 = dependency('x11')

# MATE dependencies
dep_mate_applet = dependency('libmatepanelapplet-4.0', version: mate_min_version)
dep_mate_menu = dependency('libmate-menu', version: mate_min_version)

# Misc
dep_notify = dependency('libnotify', version: '>= 0.7.0')

# Get configuration bits together
path_prefix = get_option('prefix')
path_sysconfdir = join_paths(path_prefix, get_option('sysconfdir'))
path_datadir = join_paths(path_prefix, get_option('datadir'))
path_libexecdir = join_paths(path_prefix, get_option('libexecdir'))
path_bindir = join_paths(path_prefix, get_option('bindir'))
path_localedir = join_paths(path_prefix, get_option('localedir'))

# Sort out config.h now
cdata = configuration_data()

# General options..
cdata.set_quoted('PACKAGE_VERSION', meson.project_version())
cdata.set_quoted('PACKAGE_URL', 'https://solus-project.com')

# Ensure locales will work
cdata.set_quoted('GETTEXT_PACKAGE', meson.project_name())
cdata.set_quoted('MATELOCALEDIR', path_localedir)
cdata.set('ENABLE_NLS', '1')

# Write config.h now
config_h = configure_file(
     configuration: cdata,
     output: 'config.h',
)
config_h_dir = include_directories('.')

# We're going to allow controlling this as either building a .so variant of
# Brisk Menu, or as a standalone binary
#
# For now just limit to stand-alone..
with_applet_type = 'stand-alone'

# Handle data before anything else
subdir('data')

# Now go build the source
subdir('src')

# And now translations
subdir('po')

report = [
    '    Build configuration:',
    '    ====================',
    '',
    '    prefix:                                 @0@'.format(path_prefix),
    '    datadir:                                @0@'.format(path_datadir),
    '    sysconfdir:                             @0@'.format(path_sysconfdir),
    '    bindir:                                 @0@'.format(path_bindir),
    '    libexecdir:                             @0@'.format(path_libexecdir),
    '',
    '    MATE Applet:',
    '    ============',
    '',
    '    applet type:                            @0@'.format(with_applet_type),
]

# Output some stuff to validate the build config
message('\n\n\n' + '\n'.join(report) + '\n\n')
