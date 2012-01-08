project(ActivityManager)

set(ADDITIONAL_LINK_LIBS)

add_subdirectory(kded)
# add_subdirectory(ontologies)

add_subdirectory(plugins)

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
        ${CMAKE_SOURCE_DIR}/ontologies/kext.trig
        "KEXT" "Nepomuk::Vocabulary" "trig"
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

set(activity_manager_SRCS
    ActivityManager.cpp
    NepomukActivityManager.cpp
    EventProcessor.cpp
    Plugin.cpp
    Event.cpp
    SharedInfo.cpp
    encryption/EncryptionManager.cpp
    encryption/EncfsInterface.cpp
    main.cpp
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
    ${KDE4_KDEUI_LIBS}
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


