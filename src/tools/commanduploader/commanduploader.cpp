// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "commanduploader.h"
#include "src/utils/confighandler.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/history.h"
#include "src/utils/systemnotification.h"
#include "src/widgets/imagelabel.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QCursor>
#include <QDateTime>
#include <QDesktopServices>
#include <QDrag>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMimeData>
#include <QProcess>
#include <QPushButton>
#include <QRect>
#include <QRegExp>
#include <QScreen>
#include <QShortcut>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>

CommandUploader::CommandUploader(const QPixmap& capture, QWidget* parent)
  : QWidget(parent)
  , m_pixmap(capture)
{
    setWindowTitle(tr("Image Uploader"));
    setWindowIcon(QIcon(":img/app/flameshot.svg"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    QRect position = frameGeometry();
    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    position.moveCenter(screen->availableGeometry().center());
    move(position.topLeft());
#endif

    m_spinner = new LoadSpinner(this);
    m_spinner->setColor(ConfigHandler().uiMainColorValue());
    m_spinner->start();

    m_infoLabel = new QLabel(tr("Uploading Image"));

    m_vLayout = new QVBoxLayout();
    setLayout(m_vLayout);
    m_vLayout->addWidget(m_spinner, 0, Qt::AlignHCenter);
    m_vLayout->addWidget(m_infoLabel);

    m_history = History::getInstance();

    setAttribute(Qt::WA_DeleteOnClose);

    upload();
    // QTimer::singleShot(2000, this, &CommandUploader::onUploadOk); // testing
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

    m_process = new QProcess();

    connect(m_process, SIGNAL(finished(int)), this, SLOT(processExited(int)));

    m_process->start(ConfigHandler().uploaderCommandValue());
    m_process->write(byteArray);

    m_process->closeWriteChannel();
}

void CommandUploader::processExited(int exitCode)
{
    m_spinner->deleteLater();
    QByteArray resultByteArr = m_process->readAllStandardOutput();
    QString error =
      QString::fromStdString(m_process->readAllStandardError().toStdString());
    if (exitCode == 0) {
        QString description = "";
        QString imageUrl = "";
        QString deleteUrl = "";

        QJsonDocument doc = QJsonDocument::fromJson(resultByteArr);
        if (!doc.isNull()) {
            QJsonObject obj = doc.object();
            if (obj["description"].isString()) {
                description = obj["description"].toString();
            }
            if (obj["imageUrl"].isString()) {
                imageUrl = obj["imageUrl"].toString();
            }
            if (obj["deleteUrl"].isString()) {
                deleteUrl = obj["deleteUrl"].toString();
            }
        } else {
            QString result =
              QString::fromStdString(resultByteArr.toStdString());
            QStringList lines = result.split(QRegExp(R"((\r\n|\r|\n))"));
            if (lines.count() > 0) {
                if (lines.count() > 1) {
                    imageUrl = lines[0];
                    deleteUrl = lines[1];
                } else {
                    imageUrl = lines[0];
                }
            } else {
                m_infoLabel->setText(error);
            }
        }
        if (!imageUrl.isEmpty()) {
            m_imageURL.setUrl(imageUrl);

            History* history = History::getInstance();
            bool showDelete = false;

            if (description.isEmpty()) {
                description = FileNameHandler().parsedPattern();
            }
            if (!deleteUrl.isEmpty()) {
                m_deleteImageURL.setUrl(deleteUrl);
                showDelete = true;
            }

            history->save(m_pixmap, description, imageUrl, deleteUrl);

            if (ConfigHandler().copyAndCloseAfterUploadEnabled()) {
                QApplication::clipboard()->setText(imageUrl);
                SystemNotification().sendMessage(
                  QObject::tr("URL copied to clipboard."));
                close();
            }
            onUploadOk(showDelete);
        } else {
            m_infoLabel->setText(error);
        }
    } else {
        m_infoLabel->setText(error);
    }

    m_process->close();
    m_process = nullptr;
}

void CommandUploader::onUploadOk(bool showDelete)
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
    m_openUrlButton = new QPushButton(tr("Open URL"));

    m_toClipboardButton = new QPushButton(tr("Image to Clipboard."));
    m_hLayout->addWidget(m_copyUrlButton);
    m_hLayout->addWidget(m_openUrlButton);

    if (showDelete) {
        m_openDeleteUrlButton = new QPushButton(tr("Delete image"));
        connect(m_openDeleteUrlButton,
                &QPushButton::clicked,
                this,
                &CommandUploader::openDeleteURL);
        m_hLayout->addWidget(m_openDeleteUrlButton);
    }

    m_hLayout->addWidget(m_toClipboardButton);

    connect(
      m_copyUrlButton, &QPushButton::clicked, this, &CommandUploader::copyURL);
    connect(
      m_openUrlButton, &QPushButton::clicked, this, &CommandUploader::openURL);
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
