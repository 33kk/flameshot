// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "ocrtool.h"
#include "src/utils/screenshotsaver.h"
#include "src/utils/tesseractutils.h"
#include "tesseract/baseapi.h"
#include "leptonica/allheaders.h"
#include <QPainter>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

OcrTool::OcrTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool OcrTool::closeOnButtonPressed() const
{
    return true;
}

QIcon OcrTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "content-copy.svg");
}
QString OcrTool::name() const
{
    return tr("OCR");
}

ToolType OcrTool::nameID() const
{
    return ToolType::OCR;
}

QString OcrTool::description() const
{
    return tr("Copy text into the clipboard");
}

CaptureTool* OcrTool::copy(QObject* parent)
{
    return new OcrTool(parent);
}

void OcrTool::pressed(const CaptureContext& context)
{
    PIX *image = TesseractUtils::qImageToPIX(context.selectedScreenshotArea().toImage());
    QString text = TesseractUtils::OCR("/home/marko/Downloads/tessdata", "eng", image);
    delete image;

    QApplication::clipboard()->setText(text);
    qDebug() << text;
    emit requestAction(REQ_CAPTURE_DONE_OK);
}


