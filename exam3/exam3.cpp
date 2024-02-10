#include <QTextStream>
#include <QApplication>
#include <QImage>
#include <QLabel>
#include <fftw3.h>
#include <iostream>
#include <complex>

const QString file_name = "char_A.png";

std::vector<std::pair<int, int>> extractContour(const QImage &image)
{
  int width = image.width();
  int height = image.height();

  std::vector<std::pair<int, int>> contourPoints;

  for (int y = 0; y < height; ++y)
  {
    for (int x = 0; x < width; ++x)
    {
      QColor color(image.pixel(x, y));
      int intensity = color.red() * 0.299 + color.green() * 0.587 + color.blue() * 0.114;
      if (intensity < 128)
      {
        contourPoints.push_back(std::make_pair(x, y));
      }
    }
  }

  return contourPoints;
}

std::vector<std::complex<double>> fft(const std::vector<std::pair<int, int>> &contour)
{
  int N = contour.size();

  fftw_complex *in = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);
  fftw_complex *out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * N);

  fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

  for (int i = 0; i < N; ++i)
  {
    in[i][0] = contour[i].first;
    in[i][1] = contour[i].second;
  }

  fftw_execute(plan);

  std::vector<std::complex<double>> spectrum(N);
  for (int i = 0; i < N; ++i)
  {
    spectrum[i] = std::complex<double>(out[i][0], out[i][1]);
  }

  fftw_destroy_plan(plan);
  fftw_free(in);
  fftw_free(out);

  return spectrum;
}

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QImage image;

  if (image.load(file_name))
  {
    QTextStream(stdout) << "Image loaded: " << file_name << Qt::endl;

    std::vector<std::pair<int, int>> contour = extractContour(image);
    std::vector<std::complex<double>> fftCoefficients = fft(contour);

    std::cout << "Contours:" << std::endl;
    for (const auto &point : contour)
    {
      std::cout << "(" << point.first << ", " << point.second << ")" << std::endl;
    }

    std::cout << "FFT Coefficients: \n";
    for (auto coef : fftCoefficients)
    {
      std::cout << coef << "\n";
    }
    std::cout << "\n";
  }
  else
  {
    QTextStream(stdout) << "Cannot load image: " << file_name << Qt::endl;
  }

  return 0;
}
