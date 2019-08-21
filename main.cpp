#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>

using namespace std;

// producer / consumer

list<int> mats; // stores frames to be processed
list<int> free_mats; // empty frames
mutex mtx;
mutex free_mtx;
condition_variable cond;
condition_variable free_cond;
cv::Mat matBuf[3];

// consumer free
void
mats_free(int id)
{
    unique_lock<mutex> lock(free_mtx);
    free_mats.push_back(id);
    free_cond.notify_one();
}

// producer alloc
int
mats_alloc()
{
    unique_lock<mutex> lock(free_mtx);
    while (free_mats.size() <= 0) {
        free_cond.wait(lock);
    }
    const int id = free_mats.front();
    free_mats.pop_front();
    return id;
}

// producer add
void
mats_add(int id)
{
    unique_lock<mutex> lock(mtx);
    mats.push_back(id);
    cond.notify_one();
}

// consume next
int
mats_next()
{
    unique_lock<mutex> lock(mtx);
    while (mats.size() <= 0) {
        cond.wait(lock);
    }
    const int id = mats.front();
    mats.pop_front();
    return id;
}

void
mats_init()
{
    mats_free(0);
    mats_free(1);
    mats_free(2);
}

// frames grabber thread, producer

volatile bool isTerm = false;

void
grabber()
{
    cv::VideoCapture cap;
    cap.open("../FILE190620-070756.MP4");

    cv::Mat mat;
    // discard first 50 (unstable) frames
    for (int i = 0; i < 50; ++i) {
        if (!cap.read(mat)) {
            std::cerr << "fail to discard frames" << std::endl;
            exit(1);
        }
    }

    // main job here

    while (!isTerm) {
        const int id = mats_alloc();
        if (id < 0) {
            continue;
        }
        // read next frame
        if (!cap.read(matBuf[id])) {
            std::cerr << "fail to read frames" << std::endl;
            exit(2);
        }
        // put frame to the queue
        mats_add(id);
    }
}


int
main()
{
    mats_init();

    // new thread to grab frames
    thread th(grabber);

    // timing
    using namespace std::chrono;

    auto start = high_resolution_clock::now();
    const int testCount = 1000;
    for (int i = 0; i < testCount; ++i) {
        const int id = mats_next();
        // turn on following 2 lines to see frames
        cv::imshow("frames", matBuf[id]);
        cv::waitKey(1);
        mats_free(id);
    }
    auto end = high_resolution_clock::now();
    duration<double> timeSpent = duration_cast<duration<double> >(end - start);
    cout << "average load time: "
        << timeSpent.count() * 1000. / testCount << " ms"
        << std::endl;

    // stop the thread
    isTerm = true;
    mats_free(-1); // add a invalid handle to wakeup thread
    th.join();

    return 0;
}

// 16.62 ms: only imshow and waitKey
// 5.35 ms: only grab and retrieve
// 22.73 ms: multithread grab frames
