#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QTextStream>
#include <QString>

const int INTENS_MIN = 0;
const int INTENS_MAX = 255;
const int DEF_THR = 100;
const QString file_name = "gray_cat.jpeg";

void thresh(QImage &image, int thr)
{
    for (int indx_row = 0; indx_row < image.height(); indx_row++)
    {
        quint8* ptr_row = (quint8*)(image.bits() 
                + indx_row * image.bytesPerLine());
        for (int indx_col = 0; indx_col < image.width(); indx_col++)
        {
            ptr_row[indx_col] = 
                (ptr_row[indx_col] < thr) ? INTENS_MIN : INTENS_MAX;
        }
    }
}//thresh

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QImage image;
    QLabel label;
    if (image.load(file_name))
    {
        QTextStream(stdout) << "Image loaded: " << file_name << Qt::endl;
        QTextStream(stdout) << "Format: " << image.format() << Qt::endl;
        QTextStream(stdout) << "Threshold: ";
        int thr;
        QTextStream(stdin) >> thr;
        thr = (thr >= INTENS_MIN && thr <= INTENS_MAX) ? thr : DEF_THR;

        if (image.format() == QImage::Format_Grayscale8)
        {
            thresh(image, thr);   
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
