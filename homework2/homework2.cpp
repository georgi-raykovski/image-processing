#include <QApplication>
#include <QImage>
#include <QLabel>
#include <algorithm>
#include <iostream>

using namespace std;
const QString file_name = "../gray_lenna.png";

QImage applyAdaptiveMedianFilter(const QImage &inputImage, int maxSize)
{
    int width = inputImage.width();
    int height = inputImage.height();

    QImage outputImage(width, height, QImage::Format_Grayscale8);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int currentSize = 3;
            int zxy = inputImage.pixelColor(x, y).red();

            while (currentSize <= maxSize)
            {
                int halfSize = currentSize / 2;
                QList<int> intensityValues;

                for (int j = -halfSize; j <= halfSize; ++j)
                {
                    for (int i = -halfSize; i <= halfSize; ++i)
                    {
                        int nx = qBound(0, x + i, width - 1);
                        int ny = qBound(0, y + j, height - 1);
                        intensityValues.append(inputImage.pixelColor(nx, ny).red());
                    }
                }

                std::sort(intensityValues.begin(), intensityValues.end());

                int zmin = intensityValues.first();
                int zmax = intensityValues.last();
                int zmed = intensityValues.at(intensityValues.size() / 2);

                // Level A
                if (zmin < zmed && zmed < zmax)
                {
                    // Level B
                    if (zmin < zxy && zxy < zmax)
                    {
                        outputImage.setPixelColor(x, y, QColor(zxy, zxy, zxy));
                    }
                    else
                    {
                        outputImage.setPixelColor(x, y, QColor(zmed, zmed, zmed));
                    }
                    break;
                }
                else
                {
                    // Increase size of the neighborhood
                    currentSize += 2;
                    if (currentSize > maxSize)
                    {
                        outputImage.setPixelColor(x, y, QColor(zmed, zmed, zmed));
                        break;
                    }
                }
            }
        }
    }

    return outputImage;
}

QImage applyBlur(const QImage &inputImage, int radius)
{
    QImage blurredImage = inputImage;

    for (int y = 0; y < inputImage.height(); ++y)
    {
        for (int x = 0; x < inputImage.width(); ++x)
        {
            int sum = 0;
            int count = 0;

            for (int j = -radius; j <= radius; ++j)
            {
                for (int i = -radius; i <= radius; ++i)
                {
                    int nx = qBound(0, x + i, inputImage.width() - 1);
                    int ny = qBound(0, y + j, inputImage.height() - 1);

                    sum += qRed(inputImage.pixel(nx, ny));
                    ++count;
                }
            }

            int average = sum / count;
            blurredImage.setPixel(x, y, qRgb(average, average, average));
        }
    }

    return blurredImage;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QImage inputImage;
    QLabel label1;
    QLabel label2;

    if (inputImage.load(file_name))
    {
        QTextStream(stdout) << "Image loaded: " << file_name << Qt::endl;
        QTextStream(stdout) << "Format: " << inputImage.format() << Qt::endl;

        int blurRadius = 5;
        QImage blurredImage = applyBlur(inputImage, blurRadius);
        label1.setPixmap(QPixmap::fromImage(blurredImage));

        int maxSize = 50;

        if (inputImage.format() == QImage::Format_Grayscale8)
        {
            QTextStream(stdout) << "Here" << Qt::endl;
            QImage filteredImage = applyAdaptiveMedianFilter(blurredImage, maxSize);
            label2.setPixmap(QPixmap::fromImage(filteredImage));
        }
    }
    else
    {
        QTextStream(stdout) << "Cannot load image: " << file_name << Qt::endl;
    }

    label1.show();
    label2.show();

    return a.exec();
}