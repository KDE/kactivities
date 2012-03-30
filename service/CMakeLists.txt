project(ActivityManager)

define_property(GLOBAL PROPERTY KACTIVITIES_BUILD_NEPOMUK_PLUGIN
    BRIEF_DOCS "Should the nepomuk plugin be built?"
    FULL_DOCS "The Nepomuk plugin is deprecated and replaced by sqlite
    plugin which uses sqlite for statistics processing. If Nepomuk is enabled,
    the sqlite plugin will export important data to it.")

define_property(GLOBAL PROPERTY KACTIVITIES_BUILD_DUMMY_PLUGIN
    BRIEF_DOCS "Should the dummy plugin be built?"
    FULL_DOCS "The dummy plugin does nothing. Enable only if you want to test
    the plugin overriding mechanisms.")


set(ADDITIONAL_LINK_LIBS)

# Checking for Nepomuk
macro_optional_find_package(Nepomuk)
macro_log_feature(
        Nepomuk_FOUND
        "Nepomuk" "Nepomuk" "http://www.kde.org" FALSE ""
        "STRONGLY_RECOMMENDED: Nepomuk is needed for some activity-related info")

set(sdo_SRCS "")

if(Nepomuk_FOUND)
    set(HAVE_NEPOMUK 1)
    include_directories(${SOPRANO_INCLUDE_DIR})
    set(ADDITIONAL_LINK_LIBS
        ${ADDITIONAL_LINK_LIBS}
        ${NEPOMUK_LIBRARIES}
        ${NEPOMUK_QUERY_LIBRARIES}
        ${SOPRANO_LIBRARIES}
    )

    soprano_add_ontology(
        sdo_SRCS
        ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig
        "NIE" "Nepomuk::Vocabulary" "trig"
    )

    soprano_add_ontology(
        sdo_SRCS
        ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig
        "NFO" "Nepomuk::Vocabulary" "trig"
    )

    soprano_add_ontology(
        sdo_SRCS
        ${CMAKE_SOURCE_DIR}/ontologies/kao.trig
        "KAO" "Nepomuk::Vocabulary" "trig"
    )

endif(Nepomuk_FOUND)

# config file
configure_file(config-features.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-features.h )

# Standard stuff
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

add_subdirectory(plugins)
add_subdirectory(ui/plugins)

set(activity_manager_SRCS
    ActivityManager.cpp
    ActivityManager_Resources.cpp
    ActivityManager_StartStop.cpp
    ActivityManager_Basics.cpp
    ActivityManager_Windowing.cpp

    NepomukActivityManager.cpp
    EventProcessor.cpp
    Plugin.cpp
    Event.cpp
    SharedInfo.cpp
    main.cpp

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

    jobs/nepomuk/Move.cpp

    ui/UiHandler.cpp
    ui/Ui.cpp
    ${sdo_SRCS}
    )

qt4_add_dbus_adaptor(
    activity_manager_SRCS org.kde.ActivityManager.xml
    ActivityManager.h ActivityManager
    )

kde4_add_executable(activity-manager ${activity_manager_SRCS})

target_link_libraries(
    activity-manager
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS} # TODO: Remove kdeui
    ${KDE4_KIO_LIBS}   # TODO: Remove kio
    ${ADDITIONAL_LINK_LIBS}
    )

set_target_properties(activity-manager PROPERTIES OUTPUT_NAME kactivitymanagerd)

########### install application ###############

install(
    FILES kactivitymanagerd.desktop DESTINATION ${SERVICES_INSTALL_DIR}
    )

install(
    TARGETS activity-manager ${INSTALL_TARGETS_DEFAULT_ARGS}
    )

install(FILES
    activitymanager-plugin.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR}
    )


