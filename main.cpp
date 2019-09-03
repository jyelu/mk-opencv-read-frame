#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>


int
main()
{
    // open image-store as frames buffer
    int fd = open("../image-store", O_RDONLY);
    if (fd == -1) {
        std::cerr << "fail to open image store" << std::endl;
        return 1;
    }

    off_t filesize = lseek(fd, 0, SEEK_END);
    if (filesize == (off_t) -1) {
        perror("store size");
        return 1;
    }

    void * map = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("map store");
        return 1;
    }
    // close fd when mmap done
    close(fd);

    // load the metadata of Mat
    const int * p = (const int *) map;
    const int type = *p++;
    const int rows = *p++;
    const int cols = *p++;
    const int step = *p++;

    const int bufSize = *p++;
    // padding
    ++p; ++p; ++p;

    // load frames
    size_t count = 0;
    unsigned char * b = (unsigned char *) p;

    // size limit
    const size_t FILE_LIMIT = 1500000000; // 1.5 GB
    // create Mat by loading data from 'b'
    cv::Mat mat(rows, cols, type, b, step);
    
    // timing
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    int testCount = 0;
    long sum = 0;

    while (count < FILE_LIMIT) {
        mat.data = b;
        count += bufSize;
        for (int i = 0; i < bufSize; ++i) {
            sum += *b; // read dummy
            ++b;
        }
        // cv::imshow("frames", mat);
        // b += bufSize;
        // unsigned char dummy = *b; // read at first
        // cv::waitKey(1);
        ++testCount;
    }

    auto end = high_resolution_clock::now();
    duration<double> timeSpent = duration_cast<duration<double> >(end - start);
    std::cout << "average load time: "
        << timeSpent.count() * 1000. / testCount << " ms"
        << std::endl
        << testCount << " frames, sum = " << sum << std::endl;

    return 0;
}
