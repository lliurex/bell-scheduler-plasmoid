#include "plugin.h"
#include "BellSchedulerIndicator.h"
#include "BellSchedulerIndicatorUtils.h"

#include <QtQml>

void BellSchedulerIndicatorPlugin::registerTypes (const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.private.bellschedulernotifier"));
    qmlRegisterType<BellSchedulerIndicator>(uri, 1, 0, "BellSchedulerIndicator");
    qmlRegisterType<BellSchedulerIndicatorUtils>(uri, 1, 0, "BellSchedulerIndicatorUtils");
}
