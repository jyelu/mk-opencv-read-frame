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

void
read_all(int fd, void * buf, size_t count)
{
    unsigned char * p = (unsigned char *) buf;
    while (count > 0) {
        ssize_t len = read(fd, p, count);
        if (len < 0) {
            perror("frame load");
            exit(1);
        }
        count -= len;
        p += len;
    }
}

int
read_int(int fd)
{
    int d;
    read_all(fd, &d, sizeof(d));
    return d;
}

int
main()
{
    // open image-store as frames buffer
    int fd = open("../image-store", O_RDONLY);
    if (fd == -1) {
        std::cerr << "fail to open image store" << std::endl;
        return 1;
    }

    // load the metadata of Mat
    const int type = read_int(fd);
    const int rows = read_int(fd);
    const int cols = read_int(fd);
    const int step = read_int(fd);

    const int bufSize = read_int(fd);
    // padding
    read_int(fd);
    read_int(fd);
    read_int(fd);

    // load frames
    size_t count = 0;

    // size limit
    const size_t FILE_LIMIT = 1500000000; // 1.5 GB
    // create buffer
    unsigned char * b = new unsigned char [bufSize];
    // create Mat by loading data from 'b'
    cv::Mat mat(rows, cols, type, b, step);
    
    // timing
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    int testCount = 0;

    while (count < FILE_LIMIT) {
        read_all(fd, b, bufSize);
        count += bufSize;
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
        << testCount << " frames" << std::endl;

    // clear buffer
    delete [] b;

    // close fd
    close(fd);

    return 0;
}
