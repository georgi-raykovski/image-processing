#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QString>
#include <QTextStream>


struct DRange
{
  int min;
  int max;
};

DRange dynamicRange(const QImage &image)
{
  DRange result;
  result.min = 0;
  result.max = 0;

  for (int indx_row = 0; indx_row < image.height(); indx_row++)
  {
    quint8 *ptr_row = (quint8 *)(image.bits() + indx_row * image.bytesPerLine());

    for (int indx_col = 0; indx_col < image.height(); indx_col++)
    {
      if (ptr_row[indx_col] < result.min)
      {
        result.min = ptr_row[indx_col];
      }
      if (ptr_row[indx_col] > result.max)
      {
        result.max = ptr_row[indx_col];
      }
    }
  }

  return result;
}

void toGray(QImage &image)
{
  const double red_coef = 0.2989;
  const double grn_coef = 0.5870;
  const double blu_coef = 0.1140;

  for (int indx_row = 0; indx_row < image.height(); indx_row++)
  {
    QRgb *pnt_row = (QRgb *)image.scanLine(indx_row);

    for (int indx_col = 0; indx_col < image.height(); indx_col++)
    {
      int red = qRed(pnt_row[indx_col]);
      int green = qGreen(pnt_row[indx_col]);
      int blue = qBlue(pnt_row[indx_col]);

      int gray = red_coef * red + grn_coef * green + blu_coef * blue;
      pnt_row[indx_col] = qRgb(gray, gray, gray);
    }
  }
}

int main(int argc, char *argv[])
{

  const QString file_name = "cat.jpeg";
  QApplication app(argc, argv);
  QImage image;
  QLabel label;

  if (image.load(file_name))
  {
    QTextStream(stdout) << "Loaded: " << file_name << Qt::endl;

    if (image.format() == QImage::Format_RGB32)
    {
      toGray(image);
      if (image.allGray())
      {
        QImage gray_image = image.convertToFormat(QImage::Format_Grayscale8);
        gray_image.save("gray_" + file_name);

        DRange dr = dynamicRange(gray_image);
        QTextStream(stdout) << "Dynamic range: " << dr.min << " " << dr.max << Qt::endl;
        QTextStream(stdout) << "Contast: " << (dr.max - dr.min) << Qt::endl;
      }
    }

    label.setPixmap(QPixmap::fromImage(image));
  }
  else
  {
    QTextStream(stdout) << "Failed: " << file_name << Qt::endl;
  }

  label.show();

  return app.exec();
} // main