#pragma once

#include <QObject>

class EXPORT_MACRO MyPlugin : public QObject
{
    Q_OBJECT
public:
    MyPlugin(QObject *parent);
    ~MyPlugin() override;
};

class EXPORT_MACRO MyPlugin2 : public QObject
{
    Q_OBJECT
public:
    MyPlugin2(QObject *parent);
    ~MyPlugin2() override;
};
