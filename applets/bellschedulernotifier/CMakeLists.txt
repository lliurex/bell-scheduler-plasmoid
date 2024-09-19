
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.bellschedulernotifier)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.bellschedulernotifier")
ecm_add_qml_module(bellschedulernotifierplugin URI org.kde.plasma.private.bellschedulernotifier)

target_sources(bellschedulernotifierplugin PRIVATE
    plugin/plugin.cpp
    plugin/BellSchedulerIndicator.cpp
    plugin/BellSchedulerIndicatorUtils.cpp
   
)

target_link_libraries(bellschedulernotifierplugin PRIVATE
                      Qt6::Quick
                      Qt6::Core
                      Qt6::Widgets
                      KF6::CoreAddons
                      KF6::I18n
                      KF6::Notifications
                      KF6::KIOWidgets
                      Edupals::Base Edupals::N4D

                      )

ecm_finalize_qml_module(bellschedulernotifierplugin)

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/bellschedulernotifier)
install(TARGETS bellschedulernotifierplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/bellschedulernotifier)
install(FILES "icons/bellschedulernotifier-error.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "icons/bellschedulernotifier.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "knotifications6/bellschedulernotifier.notifyrc"
        DESTINATION ${DATAROOTDIR}/usr/share/knotifications6/)

