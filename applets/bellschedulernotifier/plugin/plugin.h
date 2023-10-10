#ifndef BELL_SCHEDULER_NOTIFIER_WATCH_PLUGIN_H
#define BELL_SCHEDULER_NOTIFIER_WATCH_PLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class BellSchedulerIndicatorPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
/*    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")*/
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri) Q_DECL_OVERRIDE;
};

#endif // LLIUREX_QUOTA_WATCH_PLUGIN_H
