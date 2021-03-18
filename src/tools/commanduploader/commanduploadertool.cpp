// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "commanduploadertool.h"
#include "commanduploader.h"
#include <QPainter>

CommandUploaderTool::CommandUploaderTool(QObject* parent)
  : AbstractActionTool(parent)
{}

bool CommandUploaderTool::closeOnButtonPressed() const
{
    return true;
}

QIcon CommandUploaderTool::icon(const QColor& background, bool inEditor) const
{
    Q_UNUSED(inEditor);
    return QIcon(iconPath(background) + "cloud-upload.svg");
}
QString CommandUploaderTool::name() const
{
    return tr("Image Uploader");
}

ToolType CommandUploaderTool::nameID() const
{
    return ToolType::IMAGEUPLOADER;
}

QString CommandUploaderTool::description() const
{
    return tr("Upload the selection using command");
}

QWidget* CommandUploaderTool::widget()
{
    return new CommandUploader(capture);
}

CaptureTool* CommandUploaderTool::copy(QObject* parent)
{
    return new CommandUploaderTool(parent);
}

void CommandUploaderTool::pressed(const CaptureContext& context)
{
    capture = context.selectedScreenshotArea();
    emit requestAction(REQ_CAPTURE_DONE_OK);
    emit requestAction(REQ_ADD_EXTERNAL_WIDGETS);
}
