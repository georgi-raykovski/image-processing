#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QTextStream>
#include <QString>

const int INTENS_MIN = 0;
const int INTENS_MAX = 255;

const QString file_name = "../gray_lenna.png";

void calcHisto(QImage &image, double histo[])
{
    for (int i = 0; i <= INTENS_MAX; i++)
    {
        histo[i] = 0;
    }
    for (int indx_row = 0; indx_row < image.height(); indx_row++)
    {
        quint8* ptr_row = (quint8*)(image.bits() 
                + indx_row * image.bytesPerLine());
        for (int indx_col = 0; indx_col < image.width(); indx_col++)
        {
            histo[ptr_row[indx_col]]++;
        }
    }
    int numb_pix = image.height() * image.width();
    for (int i = 0; i <= INTENS_MAX; i++)
    {
        histo[i] /= numb_pix;
    }
}//thresh

void printHisto(double histo[])
{
    for (int i = 0; i <= INTENS_MAX; i++)
    {
        QTextStream(stdout) << i << ": " << histo[i] << Qt::endl;   
    }
}//printHisto

int main(int argc, char* argv[])
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
            double histo[INTENS_MAX + 1];
            calcHisto(image, histo);  
            printHisto(histo);
        } 
             
        label.setPixmap(QPixmap::fromImage(image));
    }
    else
    {
        QTextStream(stdout) << "Cannot load image: " << file_name << Qt::endl;
    }

    label.show();

    return app.exec();
}//main

