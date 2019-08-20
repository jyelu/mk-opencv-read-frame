#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

int
main()
{
    cv::VideoCapture cap;
    cap.open("../FILE190620-070756.MP4");

    cv::Mat mat;
    // discard 50 (unstable) head frames
    for (int i = 0; i < 50; ++i) {
        if (!cap.read(mat)) {
            std::cerr << "fail to read frames" << std::endl;
            return 1;
        }
    }

    // timing
    using namespace std::chrono;

    auto start = high_resolution_clock::now();
    const int testCount = 2000;
    for (int i = 0; i < testCount; ++i) {
        if (!cap.read(mat)) {
            std::cerr << "read frame error" << std::endl;
            return 2;
        }
        // turn on following 2 lines to see frames
        cv::imshow("frames", mat);
        cv::waitKey(1);
    }
    auto end = high_resolution_clock::now();
    duration<double> timeSpent = duration_cast<duration<double> >(end - start);
    std::cout << "average load time: "
        << timeSpent.count() * 1000. / testCount << " ms"
        << std::endl;
    return 0;
}
