import QtQuick 2.6
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.12 as Kirigami


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

   
    //Plasmoid.onActivated: action_bellstop()

    Plasmoid.fullRepresentation:Item{

        implicitWidth: Kirigami.Units.gridUnit*12
        implicitHeight: Kirigami.Units.gridUnit*12

        Kirigami.PlaceholderMessage{
            icon.name:bellSchedulerIndicator.iconName
            anchors.centerIn:parent
            width:parent.width

            Row{
                width:450
    
                PC3.Label{
                    text:bellSchedulerIndicator.subToolTip
                    font.pointSize:13
                    font.bold:true
                    width:450
                    color:"#6a6d6f"
                    wrapMode:Text.WordWrap
                }
            }
        }
    }

    function action_bellstop() {
        
        bellSchedulerIndicator.stopBell()

    }

    
 }  
