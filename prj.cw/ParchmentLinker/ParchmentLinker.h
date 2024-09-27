//
// Created by Yuri Bespalov on 07.04.2024.
//

#ifndef BESPALOV_Y_V_PARCHMENTLINKER_H
#define BESPALOV_Y_V_PARCHMENTLINKER_H


#include <opencv2/opencv.hpp>
#include <vector>
#include <queue>
#include <utility>

class ParchmentLinker {
public:
    explicit ParchmentLinker(const cv::Mat& inputImage);

    cv::Mat getTempImage(); // const
    cv::Mat getCorrectBinaryImage();
    cv::Mat getOriginImage();
    cv::Mat getSkeletonImage();
    std::vector<cv::Point> getEndpoints();
    // потом удалить
    void tempMethodShowEndpoints(const std::vector<cv::Point>& endpoints_);

private:
    void binarizationImage();
    void correctedSkeleton();
    void findEndPoints(); // bfs вылезает за границы
    float calculateCost(cv::Point p1, cv::Point p2);
    int findThickness(const cv::Point& endPoint);
    void linkBrokenLayers();

private:
    cv::Mat binaryImage_;
    cv::Mat inputImage_;
    cv::Mat skeleton_;
    cv::Mat tempp; // tempp удалить

    std::set<std::pair<int, int>> used_; // used убрать из полей класса
    std::vector<cv::Point> endpoints_;

    std::vector<std::pair<int, int>> offsets_= {{-1, 0},
                                                {1, 0},
                                                {-1, 1},
                                                {1, 1},
                                                {0, -1},
                                                {0, 1},
                                                {1, -1},
                                                {-1, -1}};
};


#endif //BESPALOV_Y_V_PARCHMENTLINKER_H
