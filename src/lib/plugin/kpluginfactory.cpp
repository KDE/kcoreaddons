/*
    This file is part of the KDE project

    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2007 Bernhard Loos <nhuh.put@web.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kpluginfactory.h"
#include "kpluginfactory_p.h"

#include <QObjectCleanupHandler>
#include "kcoreaddons_debug.h"

Q_GLOBAL_STATIC(QObjectCleanupHandler, factorycleanup)

extern int kLibraryDebugArea();

KPluginFactory::KPluginFactory()
    : d_ptr(new KPluginFactoryPrivate)
{
    Q_D(KPluginFactory);
    d->q_ptr = this;

    factorycleanup()->add(this);
}

KPluginFactory::KPluginFactory(KPluginFactoryPrivate &d)
    : d_ptr(&d)
{
    factorycleanup()->add(this);
}

KPluginFactory::~KPluginFactory()
{
    delete d_ptr;
}

void KPluginFactory::registerPlugin(const QString &keyword, const QMetaObject *metaObject, CreateInstanceFunction instanceFunction)
{
    Q_D(KPluginFactory);

    Q_ASSERT(metaObject);

    // we allow different interfaces to be registered without keyword
    if (!keyword.isEmpty()) {
        if (d->createInstanceHash.contains(keyword)) {
            qCWarning(KCOREADDONS_DEBUG) << "A plugin with the keyword" << keyword << "was already registered. A keyword must be unique!";
        }
        d->createInstanceHash.insert(keyword, KPluginFactoryPrivate::Plugin(metaObject, instanceFunction));
    } else {
        const QList<KPluginFactoryPrivate::Plugin> clashes(d->createInstanceHash.values(keyword));
        const QMetaObject *superClass = metaObject->superClass();
        if (superClass) {
            for (const KPluginFactoryPrivate::Plugin &plugin : clashes) {
                for (const QMetaObject *otherSuper = plugin.first->superClass(); otherSuper;
                        otherSuper = otherSuper->superClass()) {
                    if (superClass == otherSuper) {
                        qCWarning(KCOREADDONS_DEBUG) << "Two plugins with the same interface(" << superClass->className() << ") were registered. Use keywords to identify the plugins.";
                    }
                }
            }
        }
        for (const KPluginFactoryPrivate::Plugin &plugin : clashes) {
            superClass = plugin.first->superClass();
            if (superClass) {
                for (const QMetaObject *otherSuper = metaObject->superClass(); otherSuper;
                        otherSuper = otherSuper->superClass()) {
                    if (superClass == otherSuper) {
                        qCWarning(KCOREADDONS_DEBUG) << "Two plugins with the same interface(" << superClass->className() << ") were registered. Use keywords to identify the plugins.";
                    }
                }
            }
        }
        d->createInstanceHash.insert(keyword, KPluginFactoryPrivate::Plugin(metaObject, instanceFunction));
    }
}

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(4, 0)
QObject *KPluginFactory::createObject(QObject *parent, const char *className, const QStringList &args)
{
    Q_UNUSED(parent);
    Q_UNUSED(className);
    Q_UNUSED(args);
    return nullptr;
}
#endif

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(4, 0)
KParts::Part *KPluginFactory::createPartObject(QWidget *parentWidget, QObject *parent, const char *classname, const QStringList &args)
{
    Q_UNUSED(parent);
    Q_UNUSED(parentWidget);
    Q_UNUSED(classname);
    Q_UNUSED(args);
    return nullptr;
}
#endif

QObject *KPluginFactory::create(const char *iface, QWidget *parentWidget, QObject *parent, const QVariantList &args, const QString &keyword)
{
    Q_D(KPluginFactory);

    QObject *obj = nullptr;

#if KCOREADDONS_BUILD_DEPRECATED_SINCE(4, 0)
    if (keyword.isEmpty()) {

        const QStringList argsStringList = variantListToStringList(args);

        if ((obj = reinterpret_cast<QObject *>(createPartObject(parentWidget, parent, iface, argsStringList)))) {
            Q_EMIT objectCreated(obj);
            return obj;
        }

        if ((obj = createObject(parent, iface, argsStringList))) {
            Q_EMIT objectCreated(obj);
            return obj;
        }
    }
#endif

    const QList<KPluginFactoryPrivate::Plugin> candidates(d->createInstanceHash.values(keyword));
    // for !keyword.isEmpty() candidates.count() is 0 or 1

    for (const KPluginFactoryPrivate::Plugin &plugin : candidates) {
        for (const QMetaObject *current = plugin.first; current; current = current->superClass()) {
            if (0 == qstrcmp(iface, current->className())) {
                if (obj) {
                    qCWarning(KCOREADDONS_DEBUG) << "ambiguous interface requested from a DSO containing more than one plugin";
                }
                obj = plugin.second(parentWidget, parent, args);
                break;
            }
        }
    }

    if (obj) {
        emit objectCreated(obj);
    }
    return obj;
}

QStringList KPluginFactory::variantListToStringList(const QVariantList &list)
{
    QStringList stringlist;
    for (const QVariant &var : list) {
        stringlist << var.toString();
    }
    return stringlist;
}

QVariantList KPluginFactory::stringListToVariantList(const QStringList &list)
{
    QVariantList variantlist;
    for (const QString &str : list) {
        variantlist << QVariant(str);
    }
    return variantlist;
}

