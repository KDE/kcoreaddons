#include <kpluginfactory.h>

class PluginWithoutMetaData : public QObject
{
    Q_OBJECT
public:
    PluginWithoutMetaData(const QObject *, const QVariantList &)
        : QObject(){

        };
};

K_PLUGIN_CLASS(PluginWithoutMetaData)

#include "pluginwithoutmetadata.moc"
