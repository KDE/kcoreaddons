#pragma once

#include <QObject>

class Q_DECL_EXPORT MyPlugin : public QObject
{
    Q_OBJECT
public:
    MyPlugin(QObject *parent);
    ~MyPlugin() override;
};

class Q_DECL_EXPORT MyPlugin2 : public QObject
{
    Q_OBJECT
public:
    MyPlugin2(QObject *parent);
    ~MyPlugin2() override;
};
