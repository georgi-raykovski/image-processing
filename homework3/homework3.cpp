#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>
#include <QPainter>
#include <QPen>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <fftw3.h>
#include <unistd.h>
#include <sys/resource.h>

constexpr int INTENS_MIN = 0;

std::vector<std::vector<int>> readMNIST(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};
    }

    file.seekg(16);

    const int rows = 28;
    const int cols = 28;
    std::vector<std::vector<int>> images;

    while (!file.eof())
    {
        std::vector<int> image(rows * cols);
        file.read(reinterpret_cast<char *>(image.data()), rows * cols);
        if (!file.eof())
            images.push_back(image);
    }

    file.close();
    return images;
}

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
            if (intensity > 128)
            { // Assuming white if lightness is greater than 128
                contourPoints.push_back(std::make_pair(x, y));
            }
        }
    }

    return contourPoints;
}

std::vector<std::vector<double>> applyFFT(const std::vector<std::pair<int, int>> &contour, int fftSize)
{
    const int N = contour.size();
    std::vector<std::vector<double>> fftCoefficients(fftSize, std::vector<double>(2));

    // Allocate memory for FFTW input and output arrays
    fftw_complex *in = fftw_alloc_complex(fftSize);
    fftw_complex *out = fftw_alloc_complex(fftSize);

    if (!in || !out)
    {
        std::cerr << "Error: Failed to allocate memory for FFTW arrays." << std::endl;
        return {}; // Return an empty vector to indicate failure
    }

    // Populate the input array with contour coordinates
    for (int i = 0; i < N; ++i)
    {
        in[i][0] = contour[i].first;
        in[i][1] = contour[i].second;
    }

    // Create FFTW plan
    fftw_plan p = fftw_plan_dft_1d(fftSize, in, out, FFTW_FORWARD, FFTW_MEASURE);

    // Check if plan creation succeeded
    if (!p)
    {
        std::cerr << "Error: Failed to create FFTW plan." << std::endl;
        fftw_free(in);
        fftw_free(out);
        return {}; // Return an empty vector to indicate failure
    }

    // Execute FFT
    fftw_execute(p);

    // Copy FFT result to output vector
    for (int i = 0; i < fftSize; ++i)
    {
        fftCoefficients[i][0] = out[i][0]; // Real part
        fftCoefficients[i][1] = out[i][1]; // Imaginary part
    }

    // Clean up
    fftw_destroy_plan(p);
    fftw_free(in);
    fftw_free(out);

    return fftCoefficients;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    std::vector<std::vector<int>> mnistImages = readMNIST("train-images-idx3-ubyte");

    // Apply FFT for all images
    int fftSize = 512; // Choose appropriate FFT size
    // std::vector<std::vector<std::vector<double>>> allFftCoefficients;

    for (const auto &image : mnistImages)
    {
        QImage qimage(28, 28, QImage::Format_Grayscale8);

        for (int i = 0; i < 28; ++i)
        {
            for (int j = 0; j < 28; ++j)
            {
                qimage.setPixel(j, i, image[i * 28 + j]);
            }
        }

        std::vector<std::pair<int, int>> contour = extractContour(qimage);
        std::vector<std::vector<double>> fftCoefficients = applyFFT(contour, fftSize);
    }
    std::cout << "Processing complete." << std::endl;
    return 0;
}
