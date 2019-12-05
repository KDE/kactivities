prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: libKActivities
Description: libKActivities is a C++ library for using KDE activities
URL: https://www.kde.org
Requires: Qt5Core
Version: ${KACTIVITIES_VERSION_STRING}
Libs: -L${LIB_INSTALL_DIR} -lKF5Activities
Cflags: -I${INCLUDE_INSTALL_DIR}
