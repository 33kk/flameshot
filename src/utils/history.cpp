#include "history.h"
#include "src/utils/confighandler.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QProcessEnvironment>
#include <QStringList>

HistoryItem::HistoryItem() {}
HistoryItem::HistoryItem(QString imageUrl)
{
    this->imageUrl = imageUrl;
}
HistoryItem::HistoryItem(QString imageUrl, QString deleteUrl)
{
    this->imageUrl = imageUrl;
    this->deleteUrl = deleteUrl;
}

History::History()
{
    // Get cache history path
    ConfigHandler config;
#ifdef Q_OS_WIN
    m_historyPath = QDir::homePath() + "/AppData/Roaming/flameshot/history/";
#else
    QString path = QProcessEnvironment::systemEnvironment().value(
      "XDG_CACHE_HOME", QDir::homePath() + "/.cache");
    m_historyPath = path + "/flameshot/history/";
#endif

    // Check if directory for history exists and create if doesn't
    QDir dir = QDir(m_historyPath);
    if (!dir.exists())
        dir.mkpath(".");

    history();
}

const QString& History::path()
{
    return m_historyPath;
}

History* History::m_instance;

History* History::getInstance()
{
    if (m_instance == 0) {
        m_instance = new History();
    }
    return m_instance;
}

void History::saveJson()
{
    QJsonDocument metadataDocument = QJsonDocument();
    QJsonObject metadataObject = QJsonObject();

    foreach (const QString& key, m_history.keys()) {
        HistoryItem historyItem = m_history.value(key);
        QJsonObject obj = QJsonObject();
        obj["imageUrl"] = historyItem.imageUrl;
        if (!historyItem.deleteUrl.isEmpty()) {
            obj["deleteUrl"] = historyItem.deleteUrl;
        }
        metadataObject[key] = obj;
    }
    metadataDocument.setObject(metadataObject);

    QFile metadataFile = QFile(path() + "metadata.json");
    metadataFile.open(QIODevice::WriteOnly);
    metadataFile.write(metadataDocument.toJson());
}

void History::save(const QPixmap& pixmap,
                   const QString& fileName,
                   const QString& imageUrl,
                   const QString& deleteUrl)
{
    history();

    // scale preview only in local disk
    QPixmap pixmapScaled = QPixmap(pixmap);
    if (pixmap.height() / HISTORYPIXMAP_MAX_PREVIEW_HEIGHT >=
        pixmap.width() / HISTORYPIXMAP_MAX_PREVIEW_WIDTH) {
        pixmapScaled = pixmap.scaledToHeight(HISTORYPIXMAP_MAX_PREVIEW_HEIGHT,
                                             Qt::SmoothTransformation);
    } else {
        pixmapScaled = pixmap.scaledToWidth(HISTORYPIXMAP_MAX_PREVIEW_WIDTH,
                                            Qt::SmoothTransformation);
    }

    // save preview
    QFile file(path() + fileName);
    file.open(QIODevice::WriteOnly);
    pixmapScaled.save(&file, "PNG");
    m_history.insert(fileName, HistoryItem(imageUrl, deleteUrl));

    saveJson();
}

void History::remove(const QString& fileName)
{
    history();

    m_history.remove(fileName);
    QFile file(path() + fileName);
    if (file.exists()) {
        file.remove();
    }

    saveJson();
}

const QMap<QString, HistoryItem>& History::history()
{
    m_history = QMap<QString, HistoryItem>();
    QFile file = QFile(path() + "metadata.json");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray jsonBytes = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);
        m_historyMetadata = doc.object();
    } else {
        m_historyMetadata = QJsonObject();
    }

    foreach (const QString& key, m_historyMetadata.keys()) {
        QJsonObject historyItem = m_historyMetadata[key].toObject();
        m_history.insert(key,
                         HistoryItem(historyItem["imageUrl"].toString(),
                                     historyItem["deleteUrl"].toString()));
    }

    return m_history;
}
