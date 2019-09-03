#include <opencv2/opencv.hpp>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


void
write_all(int fd, const void * buf, size_t count)
{
    const unsigned char * p = (const unsigned char *) buf;
    while (count > 0) {
        ssize_t len = count;
        if (len > 8192) len = 8192;
        len = write(fd, p, len);
        if (len < 0) {
            perror("frame save");
            exit(1);
        }
        count -= len;
        p += len;
    }
}

int
main()
{
    // open image-store as frames buffer
    int fd = open("../image-store", O_CREAT | O_TRUNC | O_RDWR, 0666);
    if (fd == -1) {
        std::cerr << "fail to open image store" << std::endl;
        return 1;
    }

    cv::VideoCapture cap;
    cap.open("../FILE190620-070756.MP4");

    cv::Mat mat;
    if (!cap.read(mat)) {
        std::cerr << "fail to read frames" << std::endl;
        return 1;
    }
    // save the metadata of Mat
    const int type = mat.type();
    write_all(fd, &type, sizeof(type));
    const int rows = mat.rows;
    write_all(fd, &rows, sizeof(rows));
    const int cols = mat.cols;
    write_all(fd, &cols, sizeof(cols));
    const int step = mat.step;
    write_all(fd, &step, sizeof(step));

    const int bufSize = step * rows;
    write_all(fd, &bufSize, sizeof(bufSize));
    const int pad = 0;
    write_all(fd, &pad, sizeof(pad));
    write_all(fd, &pad, sizeof(pad));
    write_all(fd, &pad, sizeof(pad));

    size_t count = 0;
    // save frame
    write_all(fd, mat.data, bufSize);
    count += bufSize;

    // size limit
    const size_t FILE_LIMIT = 1500000000; // 1.5 GB
    while (count < FILE_LIMIT) {
        if (!cap.read(mat)) {
            std::cerr << "fail to read frames" << std::endl;
            return 1;
        }

        write_all(fd, mat.data, bufSize);
        count += bufSize;
    }

    // close image-store
    close(fd);
    return 0;
}
