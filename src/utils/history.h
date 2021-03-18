#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_MAX_SIZE 25

#define HISTORYPIXMAP_MAX_PREVIEW_WIDTH 160
#define HISTORYPIXMAP_MAX_PREVIEW_HEIGHT 90

#include <QJsonObject>
#include <QMap>
#include <QPixmap>
#include <QString>

class HistoryItem
{
public:
    HistoryItem();
    HistoryItem(QString imageUrl);
    HistoryItem(QString imageUrl, QString deleteUrl);

    QString imageUrl;
    QString deleteUrl;
};

class History
{
public:
    void save(const QPixmap& pixmap,
              const QString& fileName,
              const QString& imageUrl,
              const QString& deleteUrl);
    void remove(const QString& fileName);
    const QMap<QString, HistoryItem>& history();
    const QString& path();
    static History* getInstance();

private:
    History();

    void saveJson();
    QString m_historyPath;
    QJsonObject m_historyMetadata;
    QMap<QString, HistoryItem> m_history;
    static History* m_instance;
};

#endif // HISTORY_H
