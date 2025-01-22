// SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
// SPDX-FileCopyrightText: 2025 Harald Sitter <sitter@kde.org>

#pragma once

#include <QObject>
#include <QQmlEngine>

#include "kosrelease.h"

// TODO KF7: remove final on KOSRelease class declaration so we can more easily extend it without having to proxy all functions

class KOSReleaseProxy : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(KOSRelease)
    QML_SINGLETON

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QStringList idLike READ idLike CONSTANT)
    Q_PROPERTY(QString versionCodename READ versionCodename CONSTANT)
    Q_PROPERTY(QString versionId READ versionId CONSTANT)
    Q_PROPERTY(QString prettyName READ prettyName CONSTANT)
    Q_PROPERTY(QString ansiColor READ ansiColor CONSTANT)
    Q_PROPERTY(QString cpeName READ cpeName CONSTANT)
    Q_PROPERTY(QString homeUrl READ homeUrl CONSTANT)
    Q_PROPERTY(QString documentationUrl READ documentationUrl CONSTANT)
    Q_PROPERTY(QString supportUrl READ supportUrl CONSTANT)
    Q_PROPERTY(QString bugReportUrl READ bugReportUrl CONSTANT)
    Q_PROPERTY(QString privacyPolicyUrl READ privacyPolicyUrl CONSTANT)
    Q_PROPERTY(QString buildId READ buildId CONSTANT)
    Q_PROPERTY(QString variant READ variant CONSTANT)
    Q_PROPERTY(QString variantId READ variantId CONSTANT)
    Q_PROPERTY(QString logo READ logo CONSTANT)

public:
    using QObject::QObject;

    [[nodiscard]] QString name() const;
    [[nodiscard]] QString version() const;
    [[nodiscard]] QString id() const;
    [[nodiscard]] QStringList idLike() const;
    [[nodiscard]] QString versionCodename() const;
    [[nodiscard]] QString versionId() const;
    [[nodiscard]] QString prettyName() const;
    [[nodiscard]] QString ansiColor() const;
    [[nodiscard]] QString cpeName() const;
    [[nodiscard]] QString homeUrl() const;
    [[nodiscard]] QString documentationUrl() const;
    [[nodiscard]] QString supportUrl() const;
    [[nodiscard]] QString bugReportUrl() const;
    [[nodiscard]] QString privacyPolicyUrl() const;
    [[nodiscard]] QString buildId() const;
    [[nodiscard]] QString variant() const;
    [[nodiscard]] QString variantId() const;
    [[nodiscard]] QString logo() const;

    Q_INVOKABLE [[nodiscard]] QStringList extraKeys() const;
    Q_INVOKABLE [[nodiscard]] QString extraValue(const QString &key) const;

private:
    KOSRelease os;
};
