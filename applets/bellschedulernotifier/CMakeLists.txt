
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.bellschedulernotifier)


#######################################################################################
# Notes Library
set(PLUGIN_TARGET "bellschedulernotifierplugin")
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.bellschedulernotifier")
ecm_add_qml_module(${PLUGIN_TARGET} URI org.kde.plasma.private.bellschedulernotifier VERSION 2.1)

target_sources(${PLUGIN_TARGET} PRIVATE
    plugin/plugin.cpp
    plugin/BellSchedulerIndicator.cpp
    plugin/BellSchedulerIndicatorUtils.cpp
    plugin/plugin.h
    plugin/BellSchedulerIndicator.h
    plugin/BellSchedulerIndicatorUtils.h
   
)

target_link_libraries(${PLUGIN_TARGET} PRIVATE
                      Qt6::Quick
                      Qt6::Core
                      Qt6::Widgets
                      Qt6::Concurrent
                      KF6::CoreAddons
                      KF6::I18n
                      KF6::Notifications
                      KF6::KIOWidgets
                      Edupals::Base Edupals::N4D

                      )

target_include_directories(${PLUGIN_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/plugin)

ecm_finalize_qml_module(${PLUGIN_TARGET})

install(TARGETS ${PLUGIN_TARGET} DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/plasma/private/bellschedulernotifier)
install(FILES "icons/bellschedulernotifier-error.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "icons/bellschedulernotifier.svg"
        DESTINATION ${KDE_INSTALL_ICONDIR}/hicolor/scalable/apps/)
install(FILES "knotifications6/bellschedulernotifier.notifyrc"
        DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR})

