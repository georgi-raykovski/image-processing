#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QTextStream>
#include <QString>

#include <cmath>

using namespace std;

const int INTENS_MIN = 0;
const int INTENS_MAX = 255;
const int DEF_THR = 50;
const QString file_name = "./gray_lenna.png";

void negative(QImage &image)
{
  for (int indx_row = 0; indx_row < image.height(); indx_row++)
  {
    quint8 *ptr_row = (quint8 *)(image.bits() + indx_row * image.bytesPerLine());
    for (int indx_col = 0; indx_col < image.width(); indx_col++)
    {
      // ptr_row[indx_col] = INTENS_MAX - ptr_row[indx_col];
      ptr_row[indx_col] = DEF_THR * log(1 + ptr_row[indx_col]);
    }
  }
} // thresh

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
      negative(image);
    }

    label.setPixmap(QPixmap::fromImage(image));
  }
  else
  {
    QTextStream(stdout) << "Cannot load image: " << file_name << Qt::endl;
  }

  label.show();

  return app.exec();
} // main
