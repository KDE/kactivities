project (ActivityManager)

# C++11

string (COMPARE EQUAL "${CMAKE_CXX_COMPILER_ID}" "Clang" CMAKE_COMPILER_IS_CLANG)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG)
    message (STATUS "We have GNU or Clang, adding -std=c++0x flag")
    add_definitions ("-std=c++0x")
    set (ADDITIONAL_DEFINITIONS "-std=c++0x")
endif ()

# General

find_package(KDeclarative)

set (ADDITIONAL_LINK_LIBS)

set (sdo_SRCS)

if (NepomukCore_FOUND AND Soprano_FOUND)
    include_directories (${NEPOMUK_CORE_INCLUDE_DIR} ${SOPRANO_INCLUDE_DIR})

    set (
        ADDITIONAL_LINK_LIBS
        ${ADDITIONAL_LINK_LIBS}
        ${NEPOMUK_CORE_LIBRARY}
        ${SOPRANO_LIBRARIES}
        )

    soprano_add_ontology (
        sdo_SRCS
        ${CMAKE_SOURCE_DIR}/src/ontologies/kao.trig
        "KAO" "KDE::Vocabulary" "trig"
        )

endif ()

# Standard stuff

include_directories (
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

set (plugin_implementation_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/Plugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Module.cpp
    )

add_subdirectory (plugins)
add_subdirectory (ui/plugins)

set (activity_manager_SRCS
    Application.cpp
    ${CMAKE_SOURCE_DIR}/src/common/dbus/org.kde.ActivityManager.Activities.cpp

    Activities.cpp
    Resources.cpp
    Features.cpp

    ${plugin_implementation_SRCS}

    NepomukActivityManager.cpp

    Event.cpp

    jobs/Job.cpp
    jobs/JobFactory.cpp

    jobs/schedulers/Abstract.cpp
    jobs/schedulers/Ordered.cpp
    jobs/schedulers/Fallible.cpp
    jobs/schedulers/Given.cpp
    jobs/schedulers/Retry.cpp
    jobs/schedulers/Switch.cpp
    jobs/schedulers/Test.cpp

    jobs/general/Call.cpp

    jobs/ui/Message.cpp
    jobs/ui/AskPassword.cpp
    jobs/ui/Ask.cpp
    jobs/ui/SetBusy.cpp

    jobs/encryption/private/Encfs.cpp
    jobs/encryption/Common.cpp
    jobs/encryption/Mount.cpp
    jobs/encryption/InitializeStructure.cpp

    jobs/ksmserver/KSMServer.cpp

    ui/UiHandler.cpp
    ui/Ui.cpp

    ${sdo_SRCS}
    )

if (NepomukCore_FOUND)
    set (
        activity_manager_SRCS
        ${activity_manager_SRCS}
        jobs/nepomuk/Move.cpp
        )
endif ()

qt4_add_dbus_adaptor (
    activity_manager_SRCS
    ../common/dbus/org.kde.ActivityManager.Activities.xml
    Activities.h Activities
    )

qt4_add_dbus_adaptor (
    activity_manager_SRCS
    ../common/dbus/org.kde.ActivityManager.Resources.xml
    Resources.h Resources
    )

qt4_add_dbus_adaptor (
    activity_manager_SRCS
    ../common/dbus/org.kde.ActivityManager.Features.xml
    Features.h Features
    )

kde4_add_executable (activity-manager ${activity_manager_SRCS})

target_link_libraries (activity-manager
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS} # TODO: Remove kdeui
    ${KDE4_KIO_LIBS}   # TODO: Remove kio
    ${KDECLARATIVE_LIBRARIES} # TODO: Can this be avoided?
    ${ADDITIONAL_LINK_LIBS}
    )

set_target_properties (activity-manager
    PROPERTIES OUTPUT_NAME kactivitymanagerd
    )

########### install application ###############

install (FILES
    files/kactivitymanagerd.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
    )

install (TARGETS
    activity-manager ${INSTALL_TARGETS_DEFAULT_ARGS}
    )

install (FILES
    files/activitymanager-plugin.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
    )

