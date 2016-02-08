prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: libKActivities
Description: libKActivities is a C++ library for using KDE activities
URL: http://www.kde.org
Requires:
Version: ${KACTIVITIES_LIB_VERSION_STRING}
Libs: -L${LIB_INSTALL_DIR} -lKF5Activities
Cflags: -I${INCLUDE_INSTALL_DIR}
