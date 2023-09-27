
#######################################################################################
# Package
plasma_install_package(package org.kde.plasma.bellschedulernotifier)


#######################################################################################
# Notes Library
add_definitions(-DTRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.bellschedulernotifier")

set(bellnotifier_SRCS
    plugin/plugin.cpp
    plugin/BellSchedulerIndicator.cpp
    plugin/BellSchedulerIndicatorUtils.cpp
   
)

add_library(bellschedulernotifierplugin SHARED ${bellnotifier_SRCS})

target_link_libraries(bellschedulernotifierplugin
                      Qt5::Quick
                      Qt5::Core
                      Qt5::Widgets
                      KF5::CoreAddons
                      KF5::I18n
                      KF5::Notifications
                      KF5::KIOWidgets
                      Edupals::Base Edupals::N4D

                      )

install(FILES plugin/qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/bellschedulernotifier)
install(TARGETS bellschedulernotifierplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/bellschedulernotifier)

install(FILES "icons/bellschedulernotifier.svg"
        DESTINATION ${DATAROOTDIR}/usr/share/icons/hicolor/scalable/apps/)
install(FILES "knotifications5/bellschedulernotifier.notifyrc"
        DESTINATION ${DATAROOTDIR}/usr/share/knotifications5/)        
#install(FILES "autostart/lliurex-up-indicator"
#        DESTINATION ${DATAROOTDIR}/etc/bin)        
