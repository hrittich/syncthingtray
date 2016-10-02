#ifndef SYNCTHINGCONNECTION_H
#define SYNCTHINGCONNECTION_H

#include "./global.h"

#include <c++utilities/chrono/datetime.h>

#include <QObject>
#include <QList>
#include <QSslError>
#include <QFileInfo>

#include <functional>
#include <vector>

QT_FORWARD_DECLARE_CLASS(QNetworkAccessManager)
QT_FORWARD_DECLARE_CLASS(QNetworkReply)
QT_FORWARD_DECLARE_CLASS(QNetworkRequest)
QT_FORWARD_DECLARE_CLASS(QUrlQuery)
QT_FORWARD_DECLARE_CLASS(QJsonObject)
QT_FORWARD_DECLARE_CLASS(QJsonArray)

namespace Data {

struct SyncthingConnectionSettings;

QNetworkAccessManager LIB_SYNCTHING_CONNECTOR_EXPORT &networkAccessManager();

enum class SyncthingStatus
{
    Disconnected,
    Reconnecting,
    Idle,
    Scanning,
    NotificationsAvailable,
    Paused,
    Synchronizing
};

enum class DirStatus
{
    Unknown,
    Idle,
    Scanning,
    Synchronizing,
    Paused,
    OutOfSync
};

struct LIB_SYNCTHING_CONNECTOR_EXPORT DirErrors
{
    DirErrors(const QString &message, const QString &path) :
        message(message),
        path(path)
    {}
    QString message;
    QString path;
};

struct LIB_SYNCTHING_CONNECTOR_EXPORT SyncthingItemDownloadProgress
{
    SyncthingItemDownloadProgress(const QString &containingDirPath, const QString &relativeItemPath, const QJsonObject &values);
    QString relativePath;
    QFileInfo fileInfo;
    int blocksCurrentlyDownloading = 0;
    int blocksAlreadyDownloaded = 0;
    int totalNumberOfBlocks = 0;
    unsigned int downloadPercentage = 0;
    int blocksCopiedFromOrigin = 0;
    int blocksCopiedFromElsewhere = 0;
    int blocksReused = 0;
    int bytesAlreadyHandled;
    int totalNumberOfBytes = 0;
    QString label;
    ChronoUtilities::DateTime lastUpdate;
    static constexpr unsigned int syncthingBlockSize = 128 * 1024;
};

struct LIB_SYNCTHING_CONNECTOR_EXPORT SyncthingDir
{
    QString id;
    QString label;
    QString path;
    QStringList devices;
    bool readOnly = false;
    bool ignorePermissions = false;
    bool autoNormalize = false;
    int rescanInterval = 0;
    int minDiskFreePercentage = 0;
    DirStatus status = DirStatus::Idle;
    ChronoUtilities::DateTime lastStatusUpdate;
    int progressPercentage = 0;
    int progressRate = 0;
    std::vector<DirErrors> errors;
    int globalBytes = 0, globalDeleted = 0, globalFiles = 0;
    int localBytes = 0, localDeleted = 0, localFiles = 0;
    int neededByted = 0, neededFiles = 0;
    ChronoUtilities::DateTime lastScanTime;
    ChronoUtilities::DateTime lastFileTime;
    QString lastFileName;
    bool lastFileDeleted = false;
    std::vector<SyncthingItemDownloadProgress> downloadingItems;
    int blocksAlreadyDownloaded = 0;
    int blocksToBeDownloaded = 0;
    unsigned int downloadPercentage = 0;
    QString downloadLabel;

    bool assignStatus(const QString &statusStr, ChronoUtilities::DateTime time);
    bool assignStatus(DirStatus newStatus, ChronoUtilities::DateTime time);
};

enum class DevStatus
{
    Unknown,
    Disconnected,
    OwnDevice,
    Idle,
    Synchronizing,
    OutOfSync,
    Rejected
};

struct LIB_SYNCTHING_CONNECTOR_EXPORT SyncthingDev
{
    QString id;
    QString name;
    QStringList addresses;
    QString compression;
    QString certName;
    DevStatus status;
    int progressPercentage = 0;
    int progressRate = 0;
    bool introducer = false;
    bool paused = false;
    int totalIncomingTraffic = 0;
    int totalOutgoingTraffic = 0;
    QString connectionAddress;
    QString connectionType;
    QString clientVersion;
    ChronoUtilities::DateTime lastSeen;
};

struct LIB_SYNCTHING_CONNECTOR_EXPORT SyncthingLogEntry
{
    SyncthingLogEntry(const QString &when, const QString &message) :
        when(when),
        message(message)
    {}
    QString when;
    QString message;
};

class LIB_SYNCTHING_CONNECTOR_EXPORT SyncthingConnection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString syncthingUrl READ syncthingUrl WRITE setSyncthingUrl)
    Q_PROPERTY(QByteArray apiKey READ apiKey WRITE setApiKey)
    Q_PROPERTY(SyncthingStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(int trafficPollInterval READ trafficPollInterval WRITE setTrafficPollInterval)
    Q_PROPERTY(int devStatsPollInterval READ devStatsPollInterval WRITE setDevStatsPollInterval)
    Q_PROPERTY(QString configDir READ configDir NOTIFY configDirChanged)
    Q_PROPERTY(QString myId READ myId NOTIFY myIdChanged)
    Q_PROPERTY(int totalIncomingTraffic READ totalIncomingTraffic NOTIFY trafficChanged)
    Q_PROPERTY(int totalOutgoingTraffic READ totalOutgoingTraffic NOTIFY trafficChanged)
    Q_PROPERTY(double totalIncomingRate READ totalIncomingRate NOTIFY trafficChanged)
    Q_PROPERTY(double totalOutgoingRate READ totalOutgoingRate NOTIFY trafficChanged)

public:
    explicit SyncthingConnection(const QString &syncthingUrl = QStringLiteral("http://localhost:8080"), const QByteArray &apiKey = QByteArray(), QObject *parent = nullptr);
    ~SyncthingConnection();

    const QString &syncthingUrl() const;
    void setSyncthingUrl(const QString &url);
    const QByteArray &apiKey() const;
    void setApiKey(const QByteArray &apiKey);
    const QString &user() const;
    const QString &password() const;
    void setCredentials(const QString &user, const QString &password);
    SyncthingStatus status() const;
    QString statusText() const;
    bool isConnected() const;
    int trafficPollInterval() const;
    void setTrafficPollInterval(int trafficPollInterval);
    int devStatsPollInterval() const;
    void setDevStatsPollInterval(int devStatsPollInterval);
    const QString &configDir() const;
    const QString &myId() const;
    int totalIncomingTraffic() const;
    int totalOutgoingTraffic() const;
    double totalIncomingRate() const;
    double totalOutgoingRate() const;
    const std::vector<SyncthingDir> &dirInfo() const;
    const std::vector<SyncthingDev> &devInfo() const;
    QMetaObject::Connection requestQrCode(const QString &text, std::function<void (const QByteArray &)> callback);
    QMetaObject::Connection requestLog(std::function<void (const std::vector<SyncthingLogEntry> &)> callback);
    const QList<QSslError> &expectedSslErrors();
    SyncthingDir *findDirInfo(const QString &dirId, int &row);
    SyncthingDev *findDevInfo(const QString &devId, int &row);
    SyncthingDev *findDevInfoByName(const QString &devName, int &row);

public Q_SLOTS:
    void loadSelfSignedCertificate();
    void applySettings(SyncthingConnectionSettings &connectionSettings);
    void connect();
    void disconnect();
    void reconnect();
    void reconnect(SyncthingConnectionSettings &connectionSettings);
    void pause(const QString &devId);
    void pauseAllDevs();
    void resume(const QString &devId);
    void resumeAllDevs();
    void rescan(const QString &dirId);
    void rescanAllDirs();
    void restart();
    void shutdown();
    void considerAllNotificationsRead();

Q_SIGNALS:
    void newConfig(const QJsonObject &config);
    void newDirs(const std::vector<SyncthingDir> &dirs);
    void newDevices(const std::vector<SyncthingDev> &devs);
    void newEvents(const QJsonArray &events);
    void dirStatusChanged(const SyncthingDir &dir, int index);
    void devStatusChanged(const SyncthingDev &dev, int index);
    void downloadProgressChanged();
    void newNotification(ChronoUtilities::DateTime when, const QString &message);
    void error(const QString &errorMessage);
    void statusChanged(SyncthingStatus newStatus);
    void configDirChanged(const QString &newConfigDir);
    void myIdChanged(const QString &myNewId);
    void trafficChanged(int totalIncomingTraffic, int totalOutgoingTraffic);
    void rescanTriggered(const QString &dirId);
    void pauseTriggered(const QString &devId);
    void resumeTriggered(const QString &devId);
    void restartTriggered();
    void shutdownTriggered();

private Q_SLOTS:
    void requestConfig();
    void requestStatus();
    void requestConnections();
    void requestErrors();
    void requestDirStatistics();
    void requestDeviceStatistics();
    void requestEvents();
    void abortAllRequests();

    void readConfig();
    void readDirs(const QJsonArray &dirs);
    void readDevs(const QJsonArray &devs);
    void readStatus();
    void readConnections();
    void readDirStatistics();
    void readDeviceStatistics();
    void readErrors();
    void readEvents();
    void readStartingEvent(const QJsonObject &eventData);
    void readStatusChangedEvent(ChronoUtilities::DateTime eventTime, const QJsonObject &eventData);
    void readDownloadProgressEvent(ChronoUtilities::DateTime eventTime, const QJsonObject &eventData);
    void readDirEvent(ChronoUtilities::DateTime eventTime, const QString &eventType, const QJsonObject &eventData);
    void readDeviceEvent(ChronoUtilities::DateTime eventTime, const QString &eventType, const QJsonObject &eventData);
    void readItemStarted(ChronoUtilities::DateTime eventTime, const QJsonObject &eventData);
    void readItemFinished(ChronoUtilities::DateTime eventTime, const QJsonObject &eventData);
    void readRescan();
    void readPauseResume();
    void readRestart();
    void readShutdown();

    void continueConnecting();
    void continueReconnecting();
    void setStatus(SyncthingStatus status);
    void emitNotification(ChronoUtilities::DateTime when, const QString &message);

private:
    QNetworkRequest prepareRequest(const QString &path, const QUrlQuery &query, bool rest = true);
    QNetworkReply *requestData(const QString &path, const QUrlQuery &query, bool rest = true);
    QNetworkReply *postData(const QString &path, const QUrlQuery &query, const QByteArray &data = QByteArray());

    QString m_syncthingUrl;
    QByteArray m_apiKey;
    QString m_user;
    QString m_password;
    SyncthingStatus m_status;
    bool m_keepPolling;
    bool m_reconnecting;
    int m_lastEventId;
    int m_trafficPollInterval;
    int m_devStatsPollInterval;
    QString m_configDir;
    QString m_myId;
    int m_totalIncomingTraffic;
    int m_totalOutgoingTraffic;
    double m_totalIncomingRate;
    double m_totalOutgoingRate;
    QNetworkReply *m_configReply;
    QNetworkReply *m_statusReply;
    QNetworkReply *m_connectionsReply;
    QNetworkReply *m_errorsReply;
    QNetworkReply *m_eventsReply;
    bool m_unreadNotifications;
    bool m_hasConfig;
    bool m_hasStatus;
    std::vector<SyncthingDir> m_dirs;
    std::vector<SyncthingDev> m_devs;
    ChronoUtilities::DateTime m_lastConnectionsUpdate;
    ChronoUtilities::DateTime m_lastFileTime;
    ChronoUtilities::DateTime m_lastErrorTime;
    QString m_lastFileName;
    bool m_lastFileDeleted;
    QList<QSslError> m_expectedSslErrors;
};

/*!
 * \brief Returns the URL used to connect to Syncthing.
 */
inline const QString &SyncthingConnection::syncthingUrl() const
{
    return m_syncthingUrl;
}

/*!
 * \brief Sets the URL used to connect to Syncthing.
 */
inline void SyncthingConnection::setSyncthingUrl(const QString &url)
{
    m_syncthingUrl = url;
}

/*!
 * \brief Returns the API key used to connect to Syncthing.
 */
inline const QByteArray &SyncthingConnection::apiKey() const
{
    return m_apiKey;
}

/*!
 * \brief Sets the API key used to connect to Syncthing.
 */
inline void SyncthingConnection::setApiKey(const QByteArray &apiKey)
{
    m_apiKey = apiKey;
}

/*!
 * \brief Returns the user name which has been set using setCredentials().
 */
inline const QString &SyncthingConnection::user() const
{
    return m_user;
}

/*!
 * \brief Returns the password which has been set using setCredentials().
 */
inline const QString &SyncthingConnection::password() const
{
    return m_password;
}

/*!
 * \brief Provides credentials used for HTTP authentication.
 */
inline void SyncthingConnection::setCredentials(const QString &user, const QString &password)
{
    m_user = user, m_password = password;
}

/*!
 * \brief Returns the connection status.
 */
inline SyncthingStatus SyncthingConnection::status() const
{
    return m_status;
}

/*!
 * \brief Returns whether the connection has been established.
 */
inline bool SyncthingConnection::isConnected() const
{
    return m_status != SyncthingStatus::Disconnected && m_status != SyncthingStatus::Reconnecting;
}

/*!
 * \brief Returns the interval for polling traffic status (which currently can not be received via event API) in milliseconds.
 * \remarks Default value is 2000 milliseconds.
 */
inline int SyncthingConnection::trafficPollInterval() const
{
    return m_trafficPollInterval;
}

/*!
 * \brief Sets the interval for polling traffic status (which currently can not be received via event API) in milliseconds.
 * \remarks Default value is 2000 milliseconds.
 */
inline void SyncthingConnection::setTrafficPollInterval(int trafficPollInterval)
{
    m_trafficPollInterval = trafficPollInterval;
}

/*!
 * \brief Returns the interval for polling device statistics (which currently can not be received via event API) in milliseconds.
 * \remarks Default value is 60000 milliseconds.
 */
inline int SyncthingConnection::devStatsPollInterval() const
{
    return m_devStatsPollInterval;
}

/*!
 * \brief Sets the interval for polling device statistics (which currently can not be received via event API) in milliseconds.
 * \remarks Default value is 60000 milliseconds.
 */
inline void SyncthingConnection::setDevStatsPollInterval(int devStatsPollInterval)
{
    m_devStatsPollInterval = devStatsPollInterval;
}

/*!
 * \brief Returns the Syncthing home/configuration directory.
 */
inline const QString &SyncthingConnection::configDir() const
{
    return m_configDir;
}

/*!
 * \brief Returns the ID of the own Syncthing device.
 */
inline const QString &SyncthingConnection::myId() const
{
    return m_myId;
}

/*!
 * \brief Returns the total incoming traffic in byte.
 */
inline int SyncthingConnection::totalIncomingTraffic() const
{
    return m_totalIncomingTraffic;
}

/*!
 * \brief Returns the total outgoing traffic in byte.
 */
inline int SyncthingConnection::totalOutgoingTraffic() const
{
    return m_totalOutgoingTraffic;
}

/*!
 * \brief Returns the total incoming transfer rate in kbit/s.
 */
inline double SyncthingConnection::totalIncomingRate() const
{
    return m_totalIncomingRate;
}

/*!
 * \brief Returns the total outgoing transfer rate in kbit/s.
 */
inline double SyncthingConnection::totalOutgoingRate() const
{
    return m_totalOutgoingRate;
}

/*!
 * \brief Returns all available directory information.
 * \remarks The returned object container object is persistent. However, the contained
 *          info objects are invalidated when the newConfig() signal is emitted.
 */
inline const std::vector<SyncthingDir> &SyncthingConnection::dirInfo() const
{
    return m_dirs;
}

/*!
 * \brief Returns all available device information.
 * \remarks The returned object container object is persistent. However, the contained
 *          info objects are invalidated when the newConfig() signal is emitted.
 */
inline const std::vector<SyncthingDev> &SyncthingConnection::devInfo() const
{
    return m_devs;
}

/*!
 * \brief Returns a list of all expected certificate errors. This is meant to allow self-signed certificates.
 * \remarks This list is updated via loadSelfSignedCertificate().
 */
inline const QList<QSslError> &SyncthingConnection::expectedSslErrors()
{
    return m_expectedSslErrors;
}

}

#endif // SYNCTHINGCONNECTION_H
