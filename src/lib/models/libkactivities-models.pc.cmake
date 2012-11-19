prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${BIN_INSTALL_DIR}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

Name: libkkactivities-models
Description: libkkactivities is a C++ library that provides KDE activity-related item models
URL: http://www.kde.org
Requires:
Version: ${KACTIVITIES_MODELS_LIB_VERSION_STRING}
Libs: -L${LIB_INSTALL_DIR} -lkactivities-models
Cflags: -I${INCLUDE_INSTALL_DIR}
