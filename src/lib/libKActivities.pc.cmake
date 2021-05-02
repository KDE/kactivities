prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${KDE_INSTALL_BINDIR}
libdir=${KDE_INSTALL_LIBDIR}
includedir=${KDE_INSTALL_INCLUDEDIR}

Name: libKActivities
Description: libKActivities is a C++ library for using KDE activities
URL: https://www.kde.org
Requires: Qt5Core
Version: ${KACTIVITIES_VERSION}
Libs: -L${KDE_INSTALL_LIBDIR} -lKF5Activities
Cflags: -I${KDE_INSTALL_INCLUDEDIR}
