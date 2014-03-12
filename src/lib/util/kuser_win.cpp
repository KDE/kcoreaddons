/*
 *  KUser - represent a user/account (Windows)
 *  Copyright (C) 2007 Bernhard Loos <nhuh.put@web.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include "kuser.h"

#include <QtCore/QMutableStringListIterator>
#include <QtCore/QDebug>
#include <QtCore/QDir>

#include <memory> // unique_ptr
#include <windows.h>
#include <lm.h>
#include <sddl.h>

class KUser::Private : public QSharedData
{
public:
    PUSER_INFO_11 userInfo;
    PSID sid;

    Private() : userInfo(0), sid(0) {}

    Private(PUSER_INFO_11 userInfo_, PSID sid_ = 0) : userInfo(userInfo_) {}

    Private(const QString &name, PSID sid_ = 0) : userInfo(0), sid(NULL)
    {
        LPBYTE servername;
        NET_API_STATUS status = NetGetAnyDCName(0, 0, &servername);
        if (status != NERR_Success) {
            servername = NULL;
        }

        if (NetUserGetInfo((LPCWSTR) servername, (LPCWSTR) name.utf16(), 11, (LPBYTE *) &userInfo) != NERR_Success) {
            goto error;
        }
        if (servername) {
            NetApiBufferFree(servername);
            servername = 0;
        }

        if (!sid_) {
            DWORD size = 0;
            SID_NAME_USE nameuse;
            DWORD cchReferencedDomainName = 0;
            WCHAR *referencedDomainName = NULL;

            // the following line definitely fails:
            // both the sizes for sid and for referencedDomainName are Null
            // the needed sizes are set in size and cchReferencedDomainName
            LookupAccountNameW(NULL, (LPCWSTR) name.utf16(), sid, &size, referencedDomainName, &cchReferencedDomainName, &nameuse);
            sid = (PSID) new SID[size + 1];
            referencedDomainName = new WCHAR[cchReferencedDomainName + 1];
            if (!LookupAccountNameW(NULL, (LPCWSTR) name.utf16(), sid, &size, referencedDomainName, &cchReferencedDomainName, &nameuse)) {
                delete[] referencedDomainName;
                goto error;
            }

            // if you want to see both the DomainName and the sid of the user
            // uncomment the following lines
            /*                LPWSTR sidstring;
                            ConvertSidToStringSidW(sid, &sidstring);
                            qDebug() << QString("\\\\") + QString::fromUtf16(reinterpret_cast<ushort*>(referencedDomainName)) + \
                                        "\\" + name + "(" + QString::fromUtf16(reinterpret_cast<ushort*>(sidstring)) + ")";

                            LocalFree(sidstring);*/
            delete[] referencedDomainName;
        } else {
            if (!IsValidSid(sid_)) {
                goto error;
            }

            DWORD sidlength = GetLengthSid(sid_);
            sid = (PSID) new BYTE[sidlength];
            if (!CopySid(sidlength, sid, sid_)) {
                goto error;
            }
        }

        return;

    error:
        delete[] sid;
        sid = 0;
        if (userInfo) {
            NetApiBufferFree(userInfo);
            userInfo = 0;
        }
        if (servername) {
            NetApiBufferFree(servername);
            servername = 0;
        }
    }

    ~Private()
    {
        if (userInfo) {
            NetApiBufferFree(userInfo);
        }

        delete[] sid;
    }
};

KUser::KUser(UIDMode mode)
    : d(0)
{
    Q_UNUSED(mode)

    DWORD bufferLen = UNLEN + 1;
    ushort buffer[UNLEN + 1];

    if (GetUserNameW((LPWSTR) buffer, &bufferLen)) {
        d = new Private(QString::fromUtf16(buffer));
    }
}

KUser::KUser(K_UID uid)
    : d(0)
{
    DWORD bufferLen = UNLEN + 1;
    ushort buffer[UNLEN + 1];
    DWORD domainBufferLen = UNLEN + 1;
    WCHAR domainBuffer[UNLEN + 1];
    SID_NAME_USE eUse;

    if (uid && LookupAccountSidW(NULL, uid, (LPWSTR)buffer, &bufferLen, domainBuffer, &domainBufferLen, &eUse)) {
        d = new Private(QString::fromUtf16(buffer), uid);
    }
}

KUser::KUser(KUserId uid)
: d(0)
{
    DWORD bufferLen = UNLEN + 1;
    ushort buffer[UNLEN + 1];
    DWORD domainBufferLen = UNLEN + 1;
    WCHAR domainBuffer[UNLEN + 1];
    SID_NAME_USE eUse;

    if (uid.isValid() && LookupAccountSidW(NULL, uid.nativeId(), (LPWSTR)buffer, &bufferLen, domainBuffer, &domainBufferLen, &eUse)) {
        d = new Private(QString::fromUtf16(buffer), uid.nativeId());
    }
}

KUser::KUser(const QString &name)
    : d(new Private(name))
{
}

KUser::KUser(const char *name)
    : d(new Private(QString::fromLocal8Bit(name)))
{
}

KUser::KUser(const KUser &user)
    : d(user.d)
{
}

KUser &KUser::operator=(const KUser &user)
{
    d = user.d;
    return *this;
}

bool KUser::operator==(const KUser &user) const
{
    if (!isValid() || !user.isValid()) {
        return false;
    }
    return EqualSid(d->sid, user.d->sid);
}

bool KUser::operator !=(const KUser &user) const
{
    return !operator==(user);
}

bool KUser::isValid() const
{
    return d->userInfo != 0 && d->sid != 0;
}

bool KUser::isSuperUser() const
{
    return d->userInfo && d->userInfo->usri11_priv == USER_PRIV_ADMIN;
}

QString KUser::loginName() const
{
    return (d->userInfo ? QString::fromUtf16((ushort *) d->userInfo->usri11_name) : QString());
}

QString KUser::homeDir() const
{
    return QDir::fromNativeSeparators(QString::fromLocal8Bit(qgetenv("USERPROFILE")));
}

QString KUser::faceIconPath() const
{
    // FIXME: this needs to be adapted to windows systems (BC changes)
    return QString();
}

QString KUser::shell() const
{
    return QString::fromLatin1("cmd.exe");
}

QList<KUserGroup> KUser::groups() const
{
    QList<KUserGroup> result;

    Q_FOREACH (const QString &name, groupNames()) {
        result.append(KUserGroup(name));
    }

    return result;
}

QStringList KUser::groupNames() const
{
    QStringList result;

    if (!d->userInfo) {
        return result;
    }

    PGROUP_USERS_INFO_0 pGroups = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    NET_API_STATUS nStatus;

    nStatus = NetUserGetGroups(NULL, d->userInfo->usri11_name, 0, (LPBYTE *) &pGroups, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries);

    if (nStatus == NERR_Success) {
        for (DWORD i = 0; i < dwEntriesRead; ++i) {
            result.append(QString::fromUtf16((ushort *) pGroups[i].grui0_name));
        }
    }

    if (pGroups) {
        NetApiBufferFree(pGroups);
    }

    return result;
}

K_UID KUser::uid() const
{
    return d->sid;
}

QVariant KUser::property(UserProperty which) const
{
    if (which == FullName) {
        return QVariant(d->userInfo ? QString::fromUtf16((ushort *) d->userInfo->usri11_full_name) : QString());
    }

    return QVariant();
}

QList<KUser> KUser::allUsers()
{
    QList<KUser> result;

    NET_API_STATUS nStatus;
    PUSER_INFO_11 pUser = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;

    KUser tmp;

    do {
        nStatus = NetUserEnum(NULL, 11, 0, (LPBYTE *) &pUser, 1, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);

        if ((nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) && dwEntriesRead > 0) {
            tmp.d = new Private(pUser);
            result.append(tmp);
        }
    } while (nStatus == ERROR_MORE_DATA);

    return result;
}

QStringList KUser::allUserNames()
{
    QStringList result;

    NET_API_STATUS nStatus;
    PUSER_INFO_0 pUsers = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;

    nStatus = NetUserEnum(NULL, 0, 0, (LPBYTE *) &pUsers, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, NULL);

    if (nStatus == NERR_Success) {
        for (DWORD i = 0; i < dwEntriesRead; ++i) {
            result.append(QString::fromUtf16((ushort *) pUsers[i].usri0_name));
        }
    }

    if (pUsers) {
        NetApiBufferFree(pUsers);
    }

    return result;
}

KUser::~KUser()
{
}

class KUserGroup::Private : public QSharedData
{
public:
    PGROUP_INFO_0 groupInfo;

    Private() : groupInfo(NULL) {}
    Private(PGROUP_INFO_0 groupInfo_) : groupInfo(groupInfo_) {}
    Private(const QString &Name) : groupInfo(NULL)
    {
        NetGroupGetInfo(NULL, (PCWSTR) Name.utf16(), 0, (PBYTE *) &groupInfo);
    }

    ~Private()
    {
        if (groupInfo) {
            NetApiBufferFree(groupInfo);
        }
    }
};

KUserGroup::KUserGroup(const QString &_name)
    : d(new Private(_name))
{
}

KUserGroup::KUserGroup(const char *_name)
    : d(new Private(QLatin1String(_name)))
{
}

KUserGroup::KUserGroup(KGroupId gid)
: d(0)
{
    DWORD bufferLen = UNLEN + 1;
    ushort buffer[UNLEN + 1];
    DWORD domainBufferLen = UNLEN + 1;
    WCHAR domainBuffer[UNLEN + 1];
    SID_NAME_USE eUse;

    if (gid.isValid() && LookupAccountSidW(NULL, gid.nativeId(), (LPWSTR)buffer, &bufferLen, domainBuffer, &domainBufferLen, &eUse)) {
        d = new Private(QString::fromUtf16(buffer));
    }
}

KUserGroup::KUserGroup(const KUserGroup &group)
    : d(group.d)
{
}

KUserGroup &KUserGroup::operator =(const KUserGroup &group)
{
    d = group.d;
    return *this;
}

bool KUserGroup::operator==(const KUserGroup &group) const
{
    if (d->groupInfo == NULL || group.d->groupInfo == NULL) {
        return false;
    }
    return wcscmp(d->groupInfo->grpi0_name, group.d->groupInfo->grpi0_name) == 0;
}

bool KUserGroup::operator!=(const KUserGroup &group) const
{
    return !operator==(group);
}

bool KUserGroup::isValid() const
{
    return d->groupInfo != NULL;
}

QString KUserGroup::name() const
{
    if (d && d->groupInfo) {
        return QString::fromUtf16((ushort *) d->groupInfo->grpi0_name);
    }
    return QString();
}

QList<KUser> KUserGroup::users() const
{
    QList<KUser> Result;

    Q_FOREACH (const QString &user, userNames()) {
        Result.append(KUser(user));
    }

    return Result;
}

QStringList KUserGroup::userNames() const
{
    QStringList result;

    if (!d->groupInfo) {
        return result;
    }

    PGROUP_USERS_INFO_0 pUsers = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    NET_API_STATUS nStatus;

    nStatus = NetGroupGetUsers(NULL, d->groupInfo->grpi0_name, 0, (LPBYTE *) &pUsers, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, NULL);

    if (nStatus == NERR_Success) {
        for (DWORD i = 0; i < dwEntriesRead; ++i) {
            result.append(QString::fromUtf16((ushort *) pUsers[i].grui0_name));
        }
    }

    if (pUsers) {
        NetApiBufferFree(pUsers);
    }

    return result;
}

QList<KUserGroup> KUserGroup::allGroups()
{
    QList<KUserGroup> result;

    NET_API_STATUS nStatus;
    PGROUP_INFO_0 pGroup = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;

    KUserGroup tmp("");

    do {
        nStatus = NetGroupEnum(NULL, 0, (LPBYTE *) &pGroup, 1, &dwEntriesRead, &dwTotalEntries, (PDWORD_PTR)&dwResumeHandle);

        if ((nStatus == NERR_Success || nStatus == ERROR_MORE_DATA) && dwEntriesRead > 0) {
            tmp.d = new Private(pGroup);
            result.append(tmp);
        }
    } while (nStatus == ERROR_MORE_DATA);

    return result;
}

QStringList KUserGroup::allGroupNames()
{
    QStringList result;

    NET_API_STATUS nStatus;
    PGROUP_INFO_0 pGroups = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;

    nStatus = NetGroupEnum(NULL, 0, (LPBYTE *) &pGroups, MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, NULL);

    if (nStatus == NERR_Success) {
        for (DWORD i = 0; i < dwEntriesRead; ++i) {
            result.append(QString::fromUtf16((ushort *) pGroups[i].grpi0_name));
        }
    }

    if (pGroups) {
        NetApiBufferFree(pGroups);
    }

    return result;
}

KUserGroup::~KUserGroup()
{
}

struct WindowsSIDWrapper : public QSharedData {
    char sidBuffer[SECURITY_MAX_SID_SIZE];
    /** @return a copy of @p sid or null if sid is not valid or an error occurs */
    static WindowsSIDWrapper* copySid(PSID sid)
    {
        if (!sid || !IsValidSid(sid)) {
            return nullptr;
        }
        //create a copy of sid
        WindowsSIDWrapper* copy = new WindowsSIDWrapper();
        bool success = CopySid(SECURITY_MAX_SID_SIZE, copy->sidBuffer, sid);
        if (!success) {
            char* sidString = nullptr;
            ConvertSidToStringSidA(sid, &sidString);
            qWarning("Failed to copy SID %s, error = %d", sidString, GetLastError());
            LocalFree(sidString);
            delete copy;
            return nullptr;
        }
        return copy;
    }
};

template<>
KUserOrGroupId<void*>::KUserOrGroupId()
{
}

template<>
KUserOrGroupId<void*>::~KUserOrGroupId()
{
}

template<>
KUserOrGroupId<void*>::KUserOrGroupId(const KUserOrGroupId<void*> &other)
    : data(other.data)
{
}

template<>
inline KUserOrGroupId<void*>& KUserOrGroupId<void*>::operator=(const KUserOrGroupId<void*>& other)
{
    data = other.data;
    return *this;
}

template<>
KUserOrGroupId<void*>::KUserOrGroupId(void* nativeId)
    : data(WindowsSIDWrapper::copySid(nativeId))
{
}

template<>
bool KUserOrGroupId<void*>::isValid() const
{
    return data;
}

template<>
void* KUserOrGroupId<void*>::nativeId() const
{
    if (!data) {
        return nullptr;
    }
    return data->sidBuffer;
}

template<>
bool KUserOrGroupId<void*>::operator==(const KUserOrGroupId<void*>& other) const
{
    if (data) {
        if (!other.data) {
            return false;
        }
        return EqualSid(data->sidBuffer, other.data->sidBuffer);
    }
    return !other.data; //only equal if other data is also invalid
}

template<>
bool KUserOrGroupId<void*>::operator!=(const KUserOrGroupId<void*> &other) const
{
    return !(*this == other);
}

static const auto invalidSidString = QStringLiteral("<invalid SID>");

static QString sidToString(void* sid) {
    if (!sid || !IsValidSid(sid)) {
        return invalidSidString;
    }
    WCHAR* sidStr; // allocated by ConvertStringSidToSidW, must be freed using LocalFree()
    if (!ConvertSidToStringSidW(sid, &sidStr)) {
        return invalidSidString;
    }
    QString ret = QString::fromWCharArray(sidStr);
    LocalFree(sidStr);
    return ret;
}

template<>
QString KUserOrGroupId<void*>::toString() const
{
    return sidToString(data ? data->sidBuffer : nullptr);
}

static bool sidFromName(const QString& name, char(*buffer)[SECURITY_MAX_SID_SIZE], PSID_NAME_USE sidType)
{
    DWORD sidLength = SECURITY_MAX_SID_SIZE;
    // ReferencedDomainName must be passed or LookupAccountNameW fails
    // Documentation says it is optional, however if not passed the function fails and returns the required size
    // we only want the SID, so pass a buffer dummy buffer of sufficient size
    WCHAR dummyBuffer[1024];
    DWORD dummyBufferSize = 1024;
    bool ok = LookupAccountNameW(nullptr, (LPCWSTR)name.utf16(), *buffer, &sidLength, dummyBuffer, &dummyBufferSize, sidType);
    if (!ok) {
        //TODO: error string
        qWarning() << "Failed to lookup account" << name << "error code =" << GetLastError();
        return false;
    }
    return true;
}

KUserId KUserId::fromName(const QString& name) {
    char sidBuffer[SECURITY_MAX_SID_SIZE];
    SID_NAME_USE sidType;
    if (!sidFromName(name, &sidBuffer, &sidType)) {
        return KUserId();
    }
    if (sidType != SidTypeUser && sidType != SidTypeDeletedAccount) {
        qWarning().nospace() << "Failed to lookup user name " << name
            << ": resulting SID " << sidToString(sidBuffer) << " is not a user."
            " Got SID type " << sidType << " instead.";
        return KUserId();
    }
    return KUserId(sidBuffer);
}

KGroupId KGroupId::fromName(const QString& name) {
    char sidBuffer[SECURITY_MAX_SID_SIZE];
    SID_NAME_USE sidType;
    if (!sidFromName(name, &sidBuffer, &sidType)) {
        return KGroupId();
    }
    if (sidType != SidTypeGroup && sidType != SidTypeWellKnownGroup) {
        qWarning().nospace() << "Failed to lookup user name " << name
            << ": resulting SID " << sidToString(sidBuffer) << " is not a group."
            " Got SID type " << sidType << " instead.";
        return KGroupId();
    }
    return KGroupId(sidBuffer);
}

struct HANDLEDeleter {
    static inline void cleanup(HANDLE h) { CloseHandle(h); }
};
typedef QScopedPointer<HANDLE, HANDLEDeleter> ScopedHANDLE;

static std::unique_ptr<char[]> queryProcessInformation(TOKEN_INFORMATION_CLASS type)
{
    HANDLE _token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &_token)) {
        qWarning("Failed to get the token for the current process: %d", GetLastError());
        return false;
    }
    // automatically free the resources on exit
    const auto handleCloser = [](HANDLE h) { CloseHandle(h); };
    std::unique_ptr<void, decltype(handleCloser)> token(_token, handleCloser);
    std::unique_ptr<char[]> buffer;
    // query required size
    DWORD requiredSize;
    if (!GetTokenInformation(token.get(), type, nullptr, 0, &requiredSize)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
            qWarning("Failed to get the required size for the token information %d: %d",
                type, GetLastError());
            return nullptr;
        }
    }
    buffer.reset(new char[requiredSize]);
    if (!GetTokenInformation(token.get(), type, buffer.get(), requiredSize, &requiredSize)) {
        qWarning("Failed to get token information %d from current process: %d",
            type, GetLastError());
        return nullptr;
    }
    return buffer;
}

inline KUserId KUserId::currentUserId()
{
    std::unique_ptr<char[]> userTokenBuffer = queryProcessInformation(TokenUser);
    TOKEN_USER* userToken = (TOKEN_USER*)userTokenBuffer.get();
    return KUserId(userToken->User.Sid);
}
inline KGroupId KGroupId::currentGroupId()
{
    std::unique_ptr<char[]> primaryGroupBuffer = queryProcessInformation(TokenPrimaryGroup);
    TOKEN_PRIMARY_GROUP* primaryGroup = (TOKEN_PRIMARY_GROUP*)primaryGroupBuffer.get();
    return KGroupId(primaryGroup->PrimaryGroup);
}

KUserId KUserId::currentEffectiveUserId() { return currentUserId(); }

KGroupId KGroupId::currentEffectiveGroupId() { return currentGroupId(); }
