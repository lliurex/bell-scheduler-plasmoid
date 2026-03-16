#ifndef BELL_SCHEDULER_NOTIFIER_PLUGIN_H
#define BELL_SCHEDULER_NOTIFIER_PLUGIN_H

#include <QQmlEngineExtensionPlugin>

class BellSchedulerIndicatorPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid)

public:
    using QQmlEngineExtensionPlugin::QQmlEngineExtensionPlugin;
};

#endif //BELL_SCHEDULER_NOTIFIER_PLUGIN
