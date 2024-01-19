#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QTextStream>
#include <QString>
#include <vector>
#include <algorithm>

const int WINDOW_SIZE = 3;
const QString file_name = "../gray_lenna.png";

void medianFilter(QImage &image, int windowSize)
{
  int halfWindowSize = windowSize / 2;
  int direction = 1;
  std::vector<quint8> windowValues(windowSize);

  for (int y = 0; y < image.height(); ++y)
  {
    for (int x = (direction == 1) ? 0 : (image.width() - 1); (direction == 1) ? (x < image.width()) : (x >= 0); x += direction)
    {

      windowValues.clear();

      for (int wx = 0; wx < windowSize; ++wx)
      {
        int currX = qBound(0, x + wx - halfWindowSize, image.width() - 1);
        windowValues[wx] = *image.scanLine(y) + currX;
      }

      std::sort(windowValues.begin(), windowValues.end());

      *(image.scanLine(y) + x) = windowValues[halfWindowSize];
    }

    direction *= -1;
  }
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
      medianFilter(image, WINDOW_SIZE);
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
