#include <QApplication>
#include <QImage>
#include <QLabel>
#include <fftw3.h>
#include <cmath>

const QString file_name = "../gray_lenna.png";

void calculateDFT(QImage &image)
{
    int width = image.width();
    int height = image.height();

    fftw_complex *in, *out;
    fftw_plan plan;

    in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * width * height);
    out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * width * height);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            QRgb pixel = image.pixel(x, y);
            in[y * width + x][0] = qRed(pixel);
            in[y * width + x][1] = 0.0;
        }
    }

    plan = fftw_plan_dft_2d(height, width, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);

    double maxVal = 0.0;
    for (int i = 0; i < width * height; ++i)
    {
        double val = log(1 + sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]));
        if (val > maxVal)
            maxVal = val;
    }

    for (int i = 0; i < width * height; ++i)
    {
        double val = log(1 + sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]));
        val = (val / maxVal) * 255.0;
        image.setPixel(i % width, i / width, qRgb(val, val, val));
    }

    fftw_destroy_plan(plan);
    fftw_free(in);
    fftw_free(out);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QImage image;
    QLabel label;

    if (image.load(file_name))
    {
        QTextStream(stdout) << "Image loaded: " << file_name << Qt::endl;
        QTextStream(stdout) << "Format: " << image.format() << Qt::endl;

        if (image.format() == QImage::Format_Grayscale8)
        {
            calculateDFT(image);
        }

        label.setPixmap(QPixmap::fromImage(image));
    }
    else
    {
        QTextStream(stdout) << "Cannot load image: " << file_name << Qt::endl;
    }

    label.show();

    return app.exec();
}
