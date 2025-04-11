import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.bellschedulernotifier 1.0
// Item - the most basic plasmoid component, an empty container.
Item {

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

    Plasmoid.switchWidth: units.gridUnit * 5
    Plasmoid.switchHeight: units.gridUnit * 5

    Plasmoid.icon:bellSchedulerIndicator.iconName
    Plasmoid.toolTipMainText: bellSchedulerIndicator.toolTip
    Plasmoid.toolTipSubText: bellSchedulerIndicator.subToolTip

    Component.onCompleted: {
       plasmoid.removeAction("configure");
       plasmoid.setAction("bellstop", i18n("Stop the bell now"), "media-playback-stop"); 
                  
    }

   
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.fullRepresentation: PlasmaComponents3.Page {
        implicitWidth: PlasmaCore.Units.gridUnit * 12
        implicitHeight: PlasmaCore.Units.gridUnit * 6

        PlasmaExtras.PlaceholderMessage {
            id:phMsg
            anchors.centerIn: parent
            width: parent.width - (PlasmaCore.Units.gridUnit * 4)
            iconName: Plasmoid.icon
            text:bellSchedulerIndicator.placeHolderText
            explanation:bellSchedulerIndicator.placeHolderExplanation
        }
        PlasmaComponents3.Button {
            height:35
            anchors.top:phMsg.bottom
            anchors.horizontalCenter:phMsg.horizontalCenter
            visible:bellSchedulerIndicator.canStopBell?true:false
            display:AbstractButton.TextBesideIcon
            icon.name:"media-playback-stop"
            text:i18n("Stop the bell now")
            onClicked:bellSchedulerIndicator.stopBell()
        } 
        
    }

    function action_bellstop() {
        
        bellSchedulerIndicator.stopBell()

    }

    
 }  
