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

#include "commanduploader.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include "src/widgets/imagelabel.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QDesktopServices>
#include <QDrag>
#include <QHBoxLayout>
#include <QLabel>
#include <QMimeData>
#include <QProcess>
#include <QPushButton>
#include <QShortcut>
#include <QVBoxLayout>
#include <cstdio>

CommandUploader::CommandUploader(const QPixmap& capture, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(capture)
{
    setWindowTitle(tr("Command Uploader"));
    setWindowIcon(QIcon(":img/app/flameshot.svg"));

    m_spinner = new LoadSpinner(this);
    m_spinner->setColor(ConfigHandler().uiMainColorValue());
    m_spinner->start();

    m_infoLabel = new QLabel(tr("Uploading Image"));

    m_vLayout = new QVBoxLayout();
    setLayout(m_vLayout);
    m_vLayout->addWidget(m_spinner, 0, Qt::AlignHCenter);
    m_vLayout->addWidget(m_infoLabel);

    setAttribute(Qt::WA_DeleteOnClose);

    upload();
}

void CommandUploader::startDrag()
{
    QMimeData* mimeData = new QMimeData;
    mimeData->setUrls(QList<QUrl>{ m_imageURL });
    mimeData->setImageData(m_pixmap);

    QDrag* dragHandler = new QDrag(this);
    dragHandler->setMimeData(mimeData);
    dragHandler->setPixmap(m_pixmap.scaled(
      256, 256, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    dragHandler->exec();
}

void CommandUploader::upload()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    m_pixmap.save(&buffer, "PNG");

    process = new QProcess();
    process->start(ConfigHandler().uploadCommandValue());
    process->write(byteArray);

    process->closeWriteChannel();

    connect(
      process,
      QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
      [=](int exitCode) {
          m_spinner->deleteLater();
          QByteArray resultByteArr = process->readAll();
          process->close();
          process = nullptr;
          QString result = QString::fromStdString(resultByteArr.toStdString());
          if (exitCode == 0) {
              QStringList lines = result.split(QRegExp(R"((\r\n|\r|\n))"));
              if (lines.count() > 0 && !lines[0].isEmpty()) {
                  m_imageURL.setUrl(lines[0]);
                  bool showDelete = false;
                  if (ConfigHandler().copyAndCloseAfterUploadEnabled()) {
                      QApplication::clipboard()->setText(m_imageURL.toString());
                      SystemNotification().sendMessage(
                        QObject::tr("URL copied to clipboard."));
                      close();
                  } else if (lines.count() > 1 && !lines[1].isEmpty()) {
                      m_deleteImageURL.setUrl(lines[1]);
                      showDelete = true;
                  }
                  onUploadOk(showDelete);
              } else {
                  close();
                  SystemNotification().sendMessage(
                    QObject::tr("Image upload failed."));
              }
          } else {
              close();
              SystemNotification().sendMessage(
                QObject::tr("Image upload failed."));
          }
      });
}

void CommandUploader::onUploadOk(bool showDelete = false)
{
    m_infoLabel->deleteLater();

    m_notification = new NotificationWidget();
    m_vLayout->addWidget(m_notification);

    ImageLabel* imageLabel = new ImageLabel();
    imageLabel->setScreenshot(m_pixmap);
    imageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(imageLabel,
            &ImageLabel::dragInitiated,
            this,
            &CommandUploader::startDrag);
    m_vLayout->addWidget(imageLabel);

    m_hLayout = new QHBoxLayout();
    m_vLayout->addLayout(m_hLayout);

    m_copyUrlButton = new QPushButton(tr("Copy URL"));
    m_hLayout->addWidget(m_copyUrlButton);
    connect(
      m_copyUrlButton, &QPushButton::clicked, this, &CommandUploader::copyURL);

    m_openUrlButton = new QPushButton(tr("Open URL"));
    m_hLayout->addWidget(m_openUrlButton);
    connect(
      m_openUrlButton, &QPushButton::clicked, this, &CommandUploader::openURL);

    if (showDelete) {
        m_openDeleteUrlButton = new QPushButton(tr("Delete image"));
        m_hLayout->addWidget(m_openDeleteUrlButton);
        connect(m_openDeleteUrlButton,
                &QPushButton::clicked,
                this,
                &CommandUploader::openDeleteURL);
    }

    m_toClipboardButton = new QPushButton(tr("Image to Clipboard."));
    m_hLayout->addWidget(m_toClipboardButton);
    connect(m_toClipboardButton,
            &QPushButton::clicked,
            this,
            &CommandUploader::copyImage);
}

void CommandUploader::openURL()
{
    bool successful = QDesktopServices::openUrl(m_imageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void CommandUploader::copyURL()
{
    QApplication::clipboard()->setText(m_imageURL.toString());
    m_notification->showMessage(tr("URL copied to clipboard."));
}

void CommandUploader::openDeleteURL()
{
    bool successful = QDesktopServices::openUrl(m_deleteImageURL);
    if (!successful) {
        m_notification->showMessage(tr("Unable to open the URL."));
    }
}

void CommandUploader::copyImage()
{
    QApplication::clipboard()->setPixmap(m_pixmap);
    m_notification->showMessage(tr("Screenshot copied to clipboard."));
}
