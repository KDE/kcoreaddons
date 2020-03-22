/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KJOBUIDELEGATE_H
#define KJOBUIDELEGATE_H

#include <kcoreaddons_export.h>
#include <QObject>

class KJob;

/**
 * @class KJobUiDelegate kjobuidelegate.h KJobUiDelegate
 *
 * The base class for all KJob UI delegate.
 *
 * A UI delegate is responsible for the events of a
 * job and provides a UI for them (an error message
 * box or warning etc.).
 *
 * @see KJob
 */
class KCOREADDONS_EXPORT KJobUiDelegate : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs a new KJobUiDelegate.
     */
    KJobUiDelegate();

    /**
     * Destroys a KJobUiDelegate.
     */
    ~KJobUiDelegate() override;

protected:
    /**
     * Attach this UI delegate to a job. Once attached it'll track the job events.
     *
     * @return true if the job we're correctly attached to the job, false otherwise.
     */
    virtual bool setJob(KJob *job);

protected:
    /**
     * Retrieves the current job this UI delegate is attached to.
     *
     * @return current job this UI delegate is attached to, or @c nullptr if
     * this UI delegate is not tracking any job
     */
    KJob *job() const;

    friend class KJob;

public:
    /**
     * Display a dialog box to inform the user of the error given by
     * this job.
     * Only call if error is not 0, and only in the slot connected
     * to result.
     */
    virtual void showErrorMessage();

    /**
     * Enable or disable the automatic error handling. When automatic
     * error handling is enabled and an error occurs, then showErrorDialog()
     * is called, right before the emission of the result signal.
     *
     * The default is false.
     *
     * See also isAutoErrorHandlingEnabled , showErrorDialog
     *
     * @param enable enable or disable automatic error handling
     * @see isAutoErrorHandlingEnabled()
     */
    void setAutoErrorHandlingEnabled(bool enable);

    /**
     * Returns whether automatic error handling is enabled or disabled.
     * See also setAutoErrorHandlingEnabled .
     * @return true if automatic error handling is enabled
     * @see setAutoErrorHandlingEnabled()
     */
    bool isAutoErrorHandlingEnabled() const;

    /**
     * Enable or disable the automatic warning handling. When automatic
     * warning handling is enabled and an error occurs, then a message box
     * is displayed with the warning message
     *
     * The default is true.
     *
     * See also isAutoWarningHandlingEnabled , showErrorDialog
     *
     * @param enable enable or disable automatic warning handling
     * @see isAutoWarningHandlingEnabled()
     */
    void setAutoWarningHandlingEnabled(bool enable);

    /**
     * Returns whether automatic warning handling is enabled or disabled.
     * See also setAutoWarningHandlingEnabled .
     * @return true if automatic warning handling is enabled
     * @see setAutoWarningHandlingEnabled()
     */
    bool isAutoWarningHandlingEnabled() const;

protected Q_SLOTS:
    virtual void slotWarning(KJob *job, const QString &plain, const QString &rich);

private:
    void connectJob(KJob *job);

    class Private;
    Private *const d;
};

#endif // KJOBUIDELEGATE_H
