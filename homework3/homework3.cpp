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

class SVM
{
private:
    double learningRate;
    std::vector<std::vector<double>> weights;
    std::vector<double> biases;

public:
    SVM(double lr) : learningRate(lr) {}

    void train(const std::vector<std::vector<std::vector<double>>> &features, const std::vector<int> &labels, int epochs)
    {
        int numClasses = *std::max_element(labels.begin(), labels.end()) + 1;
        weights.resize(numClasses, std::vector<double>(features[0].size() * features[0][0].size(), 0.0));
        biases.resize(numClasses, 0.0);

        // One-vs-all training
        for (int c = 0; c < numClasses; ++c)
        {
            std::vector<int> binaryLabels(labels.size(), -1);
            for (size_t i = 0; i < labels.size(); ++i)
            {
                if (labels[i] == c)
                    binaryLabels[i] = 1;
            }
            trainOneClass(features, binaryLabels, c, epochs);
        }
    }

    void trainOneClass(const std::vector<std::vector<std::vector<double>>> &features, const std::vector<int> &labels, int classIndex, int epochs)
    {
        for (int epoch = 0; epoch < epochs; ++epoch)
        {
            for (size_t i = 0; i < features.size(); ++i)
            {
                double prediction = predict(features[i], classIndex);
                double label = labels[i];
                double error = label * prediction;

                if (error < 1)
                {
                    for (size_t j = 0; j < weights[classIndex].size(); ++j)
                    {
                        weights[classIndex][j] += learningRate * (label * features[i][j / features[i][0].size()][j % features[i][0].size()] - weights[classIndex][j]);
                    }
                    biases[classIndex] += learningRate * label;
                }
            }
        }
    }

    double predict(const std::vector<std::vector<double>> &feature, int classIndex)
    {
        double prediction = 0.0;
        for (size_t i = 0; i < feature.size(); ++i)
        {
            for (size_t j = 0; j < feature[i].size(); ++j)
            {
                prediction += weights[classIndex][i * feature[i].size() + j] * feature[i][j];
            }
        }
        prediction += biases[classIndex];
        return prediction;
    }

    int predictClass(const std::vector<std::vector<double>> &feature)
    {
        int numClasses = weights.size();
        double maxPrediction = -std::numeric_limits<double>::infinity(); // Initialize with negative infinity
        int predictedClass = -1;
        for (int c = 0; c < numClasses; ++c)
        {
            double prediction = predict(feature, c);
            if (prediction > maxPrediction)
            {
                maxPrediction = prediction;
                predictedClass = c;
            }
        }
        return predictedClass;
    }
};

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

    // while (!file.eof())
    // {
    //     std::vector<int> image(rows * cols);
    //     file.read(reinterpret_cast<char *>(image.data()), rows * cols);
    //     if (!file.eof())
    //         images.push_back(image);
    // }

    int count = 0; // Counter to track the number of images read
    while (count < 10000 && !file.eof())
    {
        std::vector<int> image(rows * cols);
        file.read(reinterpret_cast<char *>(image.data()), rows * cols);
        if (!file.eof())
        {
            images.push_back(image);
            count++; // Increment the counter after reading an image
        }
    }

    file.close();
    return images;
}

std::vector<int> readMNISTLabels(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};
    }

    file.seekg(8);

    std::vector<int> labels;

    int count = 0;
    while (count < 10000 && !file.eof())
    {
        unsigned char label;
        file.read(reinterpret_cast<char *>(&label), 1);
        if (!file.eof())
        {
            labels.push_back(label);
            count++;
        }
    }

    file.close();
    return labels;
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

struct Complex
{
    double real;
    double imag;

    // Multiplication operator for complex numbers
    Complex operator*(const Complex &other) const
    {
        Complex result;
        result.real = real * other.real - imag * other.imag;
        result.imag = real * other.imag + imag * other.real;
        return result;
    }

    Complex operator-(const Complex &other) const
    {
        Complex result;
        result.real = real - other.real;
        result.imag = imag - other.imag;
        return result;
    }

    Complex operator+(const Complex &other) const
    {
        Complex result;
        result.real = real + other.real;
        result.imag = imag + other.imag;
        return result;
    }
};

// Helper function to perform FFT recursively
void fft(std::vector<Complex> &x, bool inverse)
{
    int n = x.size();
    if (n <= 1)
        return;

    std::vector<Complex> even, odd;
    for (int i = 0; i < n; i += 2)
    {
        even.push_back(x[i]);
        odd.push_back(x[i + 1]);
    }

    fft(even, inverse);
    fft(odd, inverse);

    double angle = (inverse ? 2 : -2) * M_PI / n;
    Complex w = {1, 0};
    Complex wn = {cos(angle), sin(angle)};

    for (int i = 0; i < n / 2; ++i)
    {
        Complex temp = odd[i] * w; // Corrected multiplication here
        x[i] = even[i] + temp;
        x[i + n / 2] = even[i] - temp;
        if (inverse)
        {
            x[i].real /= 2;
            x[i].imag /= 2;
            x[i + n / 2].real /= 2;
            x[i + n / 2].imag /= 2;
        }
        w = w * wn;
    }
}

// Function to apply FFT to a contour
std::vector<std::vector<double>> applyFFT(const std::vector<std::pair<int, int>> &contour, int fftSize)
{
    int N = contour.size();
    std::vector<Complex> x(N);

    // Populate the input array with contour coordinates
    for (int i = 0; i < N; ++i)
    {
        x[i].real = contour[i].first;
        x[i].imag = contour[i].second;
    }

    // Pad with zeros if input size is less than fftSize
    if (N < fftSize)
    {
        x.resize(fftSize);
        for (int i = N; i < fftSize; ++i)
        {
            x[i] = {0, 0};
        }
    }

    // Apply FFT
    fft(x, false);

    // Extract FFT coefficients
    std::vector<std::vector<double>> fftCoefficients(fftSize, std::vector<double>(2));
    for (int i = 0; i < fftSize; ++i)
    {
        fftCoefficients[i][0] = x[i].real;
        fftCoefficients[i][1] = x[i].imag;
    }

    return fftCoefficients;
}
std::vector<std::vector<double>> cutHighFrequencyCoefficients(const std::vector<std::vector<double>> &fftCoefficients, double cutoffPercentage)
{
    std::vector<std::vector<double>> reducedCoefficients;

    // Iterate over each set of FFT coefficients
    for (const auto &coeffs : fftCoefficients)
    {
        // Calculate the total energy of all coefficients
        double totalEnergy = 0.0;
        for (size_t i = 0; i < coeffs.size(); i += 2)
        {
            double real = coeffs[i];
            double imag = coeffs[i + 1];
            totalEnergy += real * real + imag * imag;
        }

        // Calculate the cutoff threshold
        double cutoffThreshold = totalEnergy * cutoffPercentage;

        // Accumulate the energy until it reaches the cutoff threshold
        double accumulatedEnergy = 0.0;
        size_t cutoffIndex = 0;
        for (size_t i = 0; i < coeffs.size(); i += 2)
        {
            double real = coeffs[i];
            double imag = coeffs[i + 1];
            double energy = real * real + imag * imag;
            accumulatedEnergy += energy;
            if (accumulatedEnergy > cutoffThreshold)
            {
                // We reached the cutoff threshold, so break
                cutoffIndex = i;
                break;
            }
        }

        // Keep the coefficients up to the cutoff index
        std::vector<double> reducedCoeffs(coeffs.begin(), coeffs.begin() + cutoffIndex + 2);
        reducedCoefficients.push_back(reducedCoeffs);
    }

    return reducedCoefficients;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Load training images
    std::vector<std::vector<int>> trainImages = readMNIST("train-images-idx3-ubyte");
    std::vector<int> trainLabels = readMNISTLabels("train-labels-idx1-ubyte");

    double cutoffPercentage = 0.15;

    int fftSize = 512;
    std::vector<std::vector<std::vector<double>>> trainCoefficients;

    for (const auto &image : trainImages)
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
        trainCoefficients.push_back(fftCoefficients);
    }

    // Train the classifier
    SVM classifier(0.01);                                  // Adjust the learning rate as needed
    classifier.train(trainCoefficients, trainLabels, 100); // Adjust the number of epochs as needed

    // Load test images
    std::vector<std::vector<int>> testImages = readMNIST("t10k-images-idx3-ubyte");
    std::vector<int> testLabels = readMNISTLabels("t10k-labels-idx1-ubyte");

    // Apply FFT for test images
    std::vector<std::vector<std::vector<double>>> testCoefficients;

    for (const auto &image : testImages)
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
        testCoefficients.push_back(fftCoefficients);
    }

    // Predict classes for test images
    size_t numCorrectPredictions = 0;

    for (size_t i = 0; i < testImages.size(); ++i)
    {
        int predictedClass = classifier.predictClass(testCoefficients[i]);
        int actualClass = testLabels[i];

        if (predictedClass == actualClass)
        {
            ++numCorrectPredictions;
        }

        std::cout << "Predicted class for image " << i + 1 << ": " << predictedClass << std::endl;
    }

    double accuracy = static_cast<double>(numCorrectPredictions) / testImages.size() * 100.0;
    std::cout << "Accuracy: " << accuracy << "%" << std::endl;

    std::cout << "Prediction complete." << std::endl;

    return 0;
}
