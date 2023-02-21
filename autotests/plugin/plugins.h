#pragma once

#include <QObject>

class MyPlugin : public QObject
{
    Q_OBJECT
public:
    MyPlugin(QObject *parent);
    ~MyPlugin() override;
};

class MyPlugin2 : public QObject
{
    Q_OBJECT
public:
    MyPlugin2(QObject *parent);
    ~MyPlugin2() override;
};
