#include "history.h"
#include "src/utils/confighandler.h"
#include <QDir>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QProcessEnvironment>
#include <QStringList>

HistoryItem::HistoryItem() {}
HistoryItem::HistoryItem(QString description, QString fileName, QString imageUrl, QString deleteUrl)
{
    this->description = description;
    this->fileName = fileName;
    this->imageUrl = imageUrl;
    this->deleteUrl = deleteUrl;
}

History::History()
{
    // Get cache history path
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
    QJsonDocument historyDocument = QJsonDocument();
    QJsonArray history = QJsonArray();

    int toDelete = m_history.count() - ConfigHandler().uploadHistoryMaxSizeValue();

    foreach (const long long timeStamp, m_history.keys()) {
        HistoryItem historyItem = m_history.value(timeStamp);
        if (--toDelete >= 0) {
            QFile file = QFile(path() + historyItem.fileName);
            if (file.exists()) {
                file.remove();
            }
            m_history.remove(timeStamp);
        }
        else {
            QJsonObject obj = QJsonObject();
            obj["timeStamp"] = timeStamp;
            obj["description"] = historyItem.description;
            obj["imageUrl"] = historyItem.imageUrl;
            if (!historyItem.deleteUrl.isEmpty()) {
                obj["deleteUrl"] = historyItem.deleteUrl;
            }
            history.append(obj);
        }
    }
    historyDocument.setArray(history);

    QFile historyFile = QFile(path() + "history.json");
    historyFile.open(QIODevice::WriteOnly);
    historyFile.write(historyDocument.toJson());
}

void History::save(const QPixmap& pixmap,
                   const QString& description,
                   const QString& imageUrl,
                   const QString& deleteUrl)
{
    history();

    // scale preview only in local disk
    QPixmap pixmapScaled = QPixmap(pixmap);
    if (pixmap.height() / HISTORYPIXMAP_MAX_PREVIEW_HEIGHT >=
        pixmap.width() / HISTORYPIXMAP_MAX_PREVIEW_WIDTH) {
        pixmapScaled = pixmap.scaledToHeight(HISTORYPIXMAP_MAX_PREVIEW_HEIGHT);
    } else {
        pixmapScaled = pixmap.scaledToWidth(HISTORYPIXMAP_MAX_PREVIEW_WIDTH);
    }

    // save preview
    long long timeStamp = QDateTime::currentSecsSinceEpoch();
    QString fileName = QString::number(timeStamp) + ".png";
    QFile file(path() + fileName);
    file.open(QIODevice::WriteOnly);
    pixmapScaled.save(&file, "PNG");
    m_history.insert(timeStamp, HistoryItem(description, fileName, imageUrl, deleteUrl));

    saveJson();
}

void History::remove(const long long timeStamp)
{
    history();

    QFile file(path() + m_history[timeStamp].fileName);
    if (file.exists()) {
        file.remove();
    }
    m_history.remove(timeStamp);

    saveJson();
}

const QMap<long long, HistoryItem>& History::history()
{
    m_history = QMap<long long, HistoryItem>();
    QFile file = QFile(path() + "history.json");
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QByteArray jsonBytes = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(jsonBytes);
        QJsonArray history = doc.array();
        foreach (const QJsonValue& historyValue, history) {
            QJsonObject historyItem = historyValue.toObject();
            long long timeStamp = historyItem["timeStamp"].toVariant().toLongLong();
            m_history.insert(timeStamp,
                             HistoryItem(historyItem["description"].toString(),
                                         QString::number(timeStamp) + ".png",
                                         historyItem["imageUrl"].toString(),
                                         historyItem["deleteUrl"].toString()));
        }
    } else {
        return m_history;
    }

    return m_history;
}
