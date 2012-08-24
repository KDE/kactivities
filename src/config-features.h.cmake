#ifndef CONFIG_FEATURES_H_
#define CONFIG_FEATURES_H_

#cmakedefine HAVE_NEPOMUK
#cmakedefine HAVE_QZEITGEIST

#cmakedefine KAMD_DATA_DIR "@KAMD_DATA_DIR@"

#cmakedefine FUSERMOUNT_PATH "@FUSERMOUNT_PATH@"

#ifndef FUSERMOUNT_PATH
    #define FUSERMOUNT_PATH "/bin/fusermount"
#endif

#cmakedefine ENCFS_PATH "@ENCFS_PATH@"

#ifndef ENCFS_PATH
    #define ENCFS_PATH "/usr/bin/encfs"
#endif

// #cmakedefine UI_HANDLER "@UI_HANDLER"

#define KDIALOG_UI_HANDLER "activitymanager_uihandler_kdialog"
#define DECLARATIVE_UI_HANDLER "activitymanager_uihandler_declarative"

// #ifndef UI_HANDLER
//     #define UI_HANDLER "activitymanager_uihandler_kdialog"
//     #define UI_HANDLER "activitymanager_uihandler_declarative"
// #endif

#cmakedefine HAVE_CXX11_AUTO
#cmakedefine HAVE_CXX11_NULLPTR
#cmakedefine HAVE_CXX11_LAMBDA
#cmakedefine HAVE_CXX11_OVERRIDE
#cmakedefine HAVE_CXX_OVERRIDE_ATTR

#endif
