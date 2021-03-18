// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#pragma once

#include <QWidget>
#include <QUrl>

class QNetworkReply;
class QNetworkAccessManager;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class LoadSpinner;
class QPushButton;
class QUrl;
class QProcess;
class NotificationWidget;
class History;

class CommandUploader : public QWidget
{
    Q_OBJECT
public:
    explicit CommandUploader(const QPixmap& capture, QWidget* parent = nullptr);

private slots:
    void startDrag();

    void openURL();
    void copyURL();
    void openDeleteURL();
    void copyImage();
    void processExited(int exitCode);

private:
    QPixmap m_pixmap;
    QNetworkAccessManager* m_NetworkAM;

    QVBoxLayout* m_vLayout;
    QHBoxLayout* m_hLayout;
    // loading
    QLabel* m_infoLabel;
    LoadSpinner* m_spinner;
    // uploaded
    QPushButton* m_openUrlButton;
    QPushButton* m_openDeleteUrlButton;
    QPushButton* m_copyUrlButton;
    QPushButton* m_toClipboardButton;
    QUrl m_imageURL;
    QUrl m_deleteImageURL;
    NotificationWidget* m_notification;
    History* m_history;

    void upload();
    void onUploadOk(bool showDelete);
    QProcess* m_process;
};
