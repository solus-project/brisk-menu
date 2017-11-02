brisk-menu
==========

[![Coverity Scan](https://img.shields.io/coverity/scan/11139.svg)](https://scan.coverity.com/projects/solus-project-brisk-menu) [![License](https://img.shields.io/badge/License-GPL%202.0-blue.svg)](https://opensource.org/licenses/GPL-2.0)

Modern, efficient menu for the MATE Desktop Environment.

![screenshot](https://raw.githubusercontent.com/solus-project/brisk-menu/master/.github/Brisk_Menu_0.4.0.png)

This project is currently in very early stages, and as such is under very heavy development. The primary motivation is to provide a very usable menu, as seen in other desktops, but without the bloat and/or performance problems that plague current offerings.

This project is being spearheaded by Solus, as a *collaborative* effort between the various MATE communities (chiefly, Ubuntu MATE) to reinvigorate the MATE Desktop with modern, first-class options. As such, Brisk Menu is a **distro-agnostic** desktop. Please ensure to report any issues of non portability!

brisk-menu is a [Solus project](https://solus-project.com/).

![logo](https://build.solus-project.com/logo.png)

Building Brisk Menu
-------------------

 * Please ensure you have GTK 3.18 *minimum*
 * Please ensure you are using a GTK 3.18 build of MATE 1.16 or newer
 * Ensure you have a **modern** `meson` (`0.40.x`+) and `ninja` (Ubuntu users, use xenial-backports!)

```bash
    meson --buildtype plain build --prefix=/usr
    ninja -C build -j$(($(getconf _NPROCESSORS_ONLN)+1))
    sudo ninja -C build install
````

**Development on Solus:**

```bash
    meson build --buildtype debugoptimized --prefix=/usr --sysconfdir=/etc --libdir=/usr/lib64 --libexecdir=/usr/lib64/brisk-menu
    ninja -C build -j$(($(getconf _NPROCESSORS_ONLN)+1))
    sudo ninja -C build install
```

Features
--------

 - Keyboard centric (mice welcome too, of course)
 - Hotkey support (defaults to <kbd>Super</kbd>, configurable in gsettings)
 - Stupid-fast
 - Efficient, useful searching with prioritised listings
 - Modular backend design split from the frontend, allowing new backends in future
   (hint: the frontend is not tied to `.desktop` files)
 - Configurable label (hide/text)
 - Automatically adapt to vertical panels
 - Automatically reload
 - Filter via categories
 - Session/screensaver controls
 - Drag & drop support for launchers
 - Sidebar launcher support
 - GTK3 + CSS styling options
 - Fully correct `X11` WM integration (grab policy and window types)
 - Not Python.

**Planned**

Some items that are currently known to be definite options in the roadmap:

 - Settings UI to control further visual aspects (labels/icons/options)
 - "Pin to" support (Favourites, desktop, docks..)
 - Enhanced vertical panel support (i.e. dash-style menu mode)
 - Improved styling for the window edge + search entry.

More will be added in time as we go along.

License
--------

Copyright © 2016-2017 Brisk Menu Developers

`brisk-menu` is available under the terms of the `GPL-2.0` license.

The `brisk_system-log-out-symbolic.svg` icon is a copy of `application-exit-symbolic.svg`
to use within Brisk.

This icon is copyright © Sam Hewitt, from the [Paper Icons](https://github.com/snwh/paper-icon-theme) theme, available
under the terms of the [CC-BY-SA-4.0 license](https://creativecommons.org/licenses/by-sa/4.0/).
