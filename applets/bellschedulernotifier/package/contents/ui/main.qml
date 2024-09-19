import QtQuick
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami

import org.kde.plasma.private.bellschedulernotifier 1.0
// Item - the most basic plasmoid component, an empty container.

PlasmoidItem {

    id:bellschedulerApplet
    
    BellSchedulerIndicator{
        id:bellSchedulerIndicator

    }


    Plasmoid.status: {
        /* Warn! Enum types are accesed through ClassName not ObjectName */
        switch (bellSchedulerIndicator.status){
            case BellSchedulerIndicator.ActiveStatus:
                return PlasmaCore.Types.ActiveStatus
            case BellSchedulerIndicator.PassiveStatus:
                return PlasmaCore.Types.PassiveStatus
           
        }
        return  PlasmaCore.Types.ActiveStatus
        
    }

    switchWidth: Kirigami.Units.gridUnit * 5
    switchHeight: Kirigami.Units.gridUnit * 5

    Plasmoid.icon:bellSchedulerIndicator.iconName
    toolTipMainText: bellSchedulerIndicator.toolTip
    toolTipSubText: bellSchedulerIndicator.subToolTip

    Component.onCompleted: {
       Plasmoid.setInternalAction("configure", configureAction)

    }

    fullRepresentation: PlasmaComponents3.Page {
        implicitWidth: Kirigami.Units.gridUnit * 12
        implicitHeight: Kirigami.Units.gridUnit * 6

        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.gridUnit * 4)
            iconName: Plasmoid.icon
            text:bellSchedulerIndicator.placeHolderText
            explanation:bellSchedulerIndicator.placeHolderExplanation
          
        }
    }

    function action_bellstop() {
        
        bellSchedulerIndicator.stopBell()

    }

    PlasmaCore.Action {
        id: configureAction
        text: i18n("Stop the bell now")
        icon.name:"media-playback-stop.svg"
        onTriggered:action_bellstop()
    }
    
 }  
