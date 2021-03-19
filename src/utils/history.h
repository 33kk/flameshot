#ifndef HISTORY_H
#define HISTORY_H

#define HISTORYPIXMAP_MAX_PREVIEW_WIDTH 250
#define HISTORYPIXMAP_MAX_PREVIEW_HEIGHT 100

#include <QMap>
#include <QPixmap>
#include <QString>

class HistoryItem
{
public:
    HistoryItem();
    HistoryItem(QString description, QString fileName, QString imageUrl, QString deleteUrl);

    QString description;
    QString fileName;
    QString imageUrl;
    QString deleteUrl;
};

class History
{
public:
    void save(const QPixmap& pixmap,
              const QString& description,
              const QString& imageUrl,
              const QString& deleteUrl);
    void remove(const long long timeStamp);
    const QMap<long long, HistoryItem>& history();
    const QString& path();
    static History* getInstance();

private:
    History();

    void saveJson();
    QString m_historyPath;
    QMap<long long, HistoryItem> m_history;
    static History* m_instance;
};

#endif // HISTORY_H
