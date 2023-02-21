#include "plugins.h"

MyPlugin::MyPlugin(QObject *parent)
    : QObject(parent)
{
}
MyPlugin::~MyPlugin() = default;
MyPlugin2::~MyPlugin2() = default;
MyPlugin2::MyPlugin2(QObject *parent)
    : QObject(parent)
{
}

#include "moc_plugins.cpp"
