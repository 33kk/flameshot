#pragma once

#include <QPainter>
#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

// Copypasted from https://github.com/flameshot-org/flameshot/pull/1239/files#diff-ba599ebf38d0a6a0fbb4f66e3949ed60bfdfbe82411cfe127e3c5617f0938460
// Thanks to Stackoverflow user "user898678": https://stackoverflow.com/a/10019508
// https://github.com/zdenop/qt-box-editor/blob/master/src/TessTools.cpp
// LICENSE: https://github.com/zdenop/qt-box-editor/blob/master/LICENSE
// modified version

class TesseractUtils
{
public:
    static PIX* qImageToPIX(const QImage& qImage)
    {
        PIX* pixs;
        l_uint32* lines;

        QImage qImageCopy = qImage.copy();

        qImageCopy = qImageCopy.rgbSwapped();
        int width = qImageCopy.width();
        int height = qImageCopy.height();
        int depth = qImageCopy.depth();
        int wpl = qImageCopy.bytesPerLine() / 4;

        pixs = pixCreate(width, height, depth);
        pixSetWpl(pixs, wpl);
        pixSetColormap(pixs, NULL);
        l_uint32* datas = pixs->data;

        for (int y = 0; y < height; y++) {
            lines = datas + y * wpl;
            QByteArray a((const char*)qImageCopy.scanLine(y),
                         qImageCopy.bytesPerLine());
            for (int j = 0; j < a.size(); j++) {
                *((l_uint8*)lines + j) = a[j];
            }
        }
        return pixEndianByteSwapNew(pixs);
    }

    static QString OCR(const char *data, const char *language, PIX* image) {
        tesseract::TessBaseAPI *tesseractApi = new tesseract::TessBaseAPI();

        if (tesseractApi->Init(data, language)) {
            return "";
        }

        tesseractApi->SetImage(image);

        char *outText = tesseractApi->GetUTF8Text();
        const QString qString = outText;

        tesseractApi->End();
        delete tesseractApi;
        delete [] outText;

        return qString;
    }
};
