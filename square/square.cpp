#include <QtGui>
#include <QApplication>
#include <QImage>
#include <QLabel>
#include <QString>
#include <QPainter>

struct PCoord
{
  int row;
  int col;
};

PCoord center(QImage &image)
{
  const int RECT_SIZE = 20;

  QPainter painter(&image);
  painter.setBrush(Qt::NoBrush);
  painter.setPen(Qt::green);
  PCoord cnt;
  cnt.row = image.height() / 2;
  cnt.col = image.width() / 2;
  painter.drawRect(cnt.row, cnt.col, RECT_SIZE, RECT_SIZE);
  painter.end();

  return cnt;
} // center

int main(int argc, char *argv[])
{

  const QString file_name = "cat.jpeg";
  QApplication app(argc, argv);
  QImage image;
  QLabel label;

  if (image.load(file_name))
  {
    QTextStream(stdout) << "Loaded: " << file_name << Qt::endl;
    PCoord cnt = center(image);
    QTextStream(stdout) << "(" << cnt.row << ", " << cnt.col << ")" << Qt::endl;
    label.setPixmap(QPixmap::fromImage(image));
  }
  else
  {
    QTextStream(stdout) << "Failed: " << file_name << Qt::endl;
  }

  label.show();

  return app.exec();
} // main