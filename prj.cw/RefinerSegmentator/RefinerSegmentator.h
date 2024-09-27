//
// Created by Yuri Bespalov on 27.07.2024.
//

#ifndef BESPALOV_Y_V_REFINERSEGMENTATOR_H
#define BESPALOV_Y_V_REFINERSEGMENTATOR_H

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <utility>
#include <set>
#include <stack>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc.hpp>

#include <opencv2/core/types.hpp>

#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include "../ParchmentLinker/ParchmentLinker.h"


class RefinerSegmentator {
private:
    struct JunctionSection {
        std::vector<cv::Point> points;
        std::set<std::pair<int, int>> used;
        cv::Point firstEndpoint = cv::Point(-1, -1);
        cv::Point lastEndpoint = cv::Point(-1, -1);
        cv::Point middlePoint = cv::Point(-1, -1);
    };

    struct FusedRegion {
        std::vector<cv::Point> points;
        std::vector<cv::Point> contour;
        std::vector<JunctionSection> junctionSection;
    };

    cv::Mat binaryImage_;
    ParchmentLinker* parchment_;
    cv::Mat contourImage_;
    int conturSize = 5;
    std::vector<std::pair<int, int>> offsets_ = {{-1, 0}, {1, 0}, {-1, 1}, {1, 1}, {0, -1}, {0, 1}, {1, -1}, {-1, -1}};

public:
    std::vector<FusedRegion> fusedRegion;

    RefinerSegmentator(const cv::Mat& inputImage);

private:
    std::pair<cv::Point, cv::Point> drawRay(cv::Mat& image, cv::Point startPoint, cv::Point endPoint);
    bool isJunctionPoint(const std::vector<cv::Point>& contour, int i, int n, double thresholdAngle);
    std::vector<cv::Point> findJunctionPoints(const std::vector<std::vector<cv::Point>>& contours, int n, double thresholdAngle);
    int getIndexOfPoint(const std::vector<cv::Point>& points, const cv::Point& point);
    std::vector<JunctionSection> findJunctionSections(const std::vector<cv::Point>& junctionPoint);
    void findFirstAndLast(std::vector<JunctionSection>& junctionSections);
};

#endif //BESPALOV_Y_V_REFINERSEGMENTATOR_H
