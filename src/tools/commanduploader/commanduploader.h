// Copyright(c) 2017-2019 Alejandro Sirgo Rica & Contributors
//
// This file is part of Flameshot.
//
//     Flameshot is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Flameshot is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Flameshot.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <QProcess>
#include <QUrl>
#include <QWidget>

class QNetworkReply;
class QNetworkAccessManager;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class LoadSpinner;
class QPushButton;
class QUrl;
class NotificationWidget;

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

private:
    QPixmap m_pixmap;

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

    void upload();
    void onUploadOk();
		QProcess* process;
};
