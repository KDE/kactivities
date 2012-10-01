prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: libkkactivities
Description: libkkactivities is a C++ library for using KDE activities
URL: http://www.kde.org
Requires:
Version: ${KACTIVITIES_LIB_VERSION_STRING}
Libs: -L${LIB_INSTALL_DIR} -lkactivities
Cflags: -I${INCLUDE_INSTALL_DIR}
