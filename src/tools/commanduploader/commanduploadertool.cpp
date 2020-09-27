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
    return tr("Upload Using Command");
}

ToolType CommandUploaderTool::nameID() const
{
    return ToolType::COMMANDUPLOADER;
}

QString CommandUploaderTool::description() const
{
    return tr("Upload using command specified in settings");
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
