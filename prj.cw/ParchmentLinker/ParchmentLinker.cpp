//
// Created by Yuri Bespalov on 07.04.2024.
//

#include "ParchmentLinker.h"

#include <limits>
#include <algorithm>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ximgproc.hpp>

// показывает промежуточный результат
void ParchmentLinker::tempMethodShowEndpoints(const std::vector<cv::Point>& endpoints_){
    cv::Mat temp;

    temp = skeleton_.clone();

    cv::cvtColor(temp, temp, cv::COLOR_RGBA2RGB);

    for(const auto& p : endpoints_){
        cv::circle(temp, p, 5, cv::Scalar(0, 0, 255), 1);
    }

    cv::imshow("endpoints_", temp);
}

ParchmentLinker::ParchmentLinker(const cv::Mat& inputImage):inputImage_(inputImage.clone()) {
    //inputImage_ = inputImage.clone();
    binarizationImage();
    linkBrokenLayers();
}

cv::Mat ParchmentLinker::getTempImage() {
    cv::Mat tempImage = tempp.clone();
    return tempImage;
}

cv::Mat ParchmentLinker::getCorrectBinaryImage() {
    cv::Mat correctBinaryImage = binaryImage_.clone();
    return correctBinaryImage;
}

cv::Mat ParchmentLinker::getOriginImage() {
    cv::Mat originImage = inputImage_.clone();
    return originImage;
}

cv::Mat ParchmentLinker::getSkeletonImage() {
    cv::Mat skeletonImage = skeleton_.clone();
    return skeletonImage;
}

std::vector<cv::Point> ParchmentLinker::getEndpoints(){
    std::vector<cv::Point> endpoints = endpoints_;
    return endpoints;
}

void ParchmentLinker::binarizationImage() {
    cv::threshold(inputImage_, binaryImage_, 0, 255, cv::THRESH_OTSU);

    cv::Mat tskeleton_;
    cv::ximgproc::thinning(binaryImage_, tskeleton_, cv::ximgproc::THINNING_ZHANGSUEN);

    cv::imwrite("skeleton.png", tskeleton_);
}

void ParchmentLinker::correctedSkeleton(){
    used_.clear();
    std::vector<cv::Point> differencePoints;
    for (int y = 0; y < skeleton_.rows; ++y) {
        for (int x = 0; x < skeleton_.cols; ++x) {
            if (skeleton_.at<uint8_t>(y, x) != 0 && !used_.count({y, x})) {
                std::queue<std::pair<int, int>> q;
                q.push({y, x});
                used_.insert({y, x});
                while (!q.empty()) {
                    std::pair<int, int> currPoint = q.front();
                    q.pop();
                    used_.insert({currPoint.first, currPoint.second});

                    if (skeleton_.at<uint8_t>(currPoint.first, currPoint.second) == 255 &&
                        binaryImage_.at<uint8_t>(currPoint.first, currPoint.second) == 0)
                    {
                        differencePoints.push_back(cv::Point(currPoint.second, currPoint.first));
                    }

                    for (const auto &offset: offsets_) {

                        int nextX = currPoint.second + offset.second;
                        int nextY = currPoint.first + offset.first;

                        if (skeleton_.at<uint8_t>(nextY, nextX) == 255 &&
                            binaryImage_.at<uint8_t>(nextY, nextX) == 0)
                        {
                            differencePoints.push_back(cv::Point(nextX, nextY));
                        }
                        else continue;

                        if (nextX >= 0 && nextX < skeleton_.cols &&
                            nextY >= 0 && nextY < skeleton_.rows &&
                            !used_.count({nextY, nextX}))
                        {
                            q.push({nextY, nextX});
                            used_.insert({nextY, nextX});
                        }

                    }
                }
            }
        }
    }
    for (const auto & point : differencePoints){
        skeleton_.at<uint8_t>(point) = 0;
    }

    cv::imwrite("sk.png", skeleton_);
    //used_.clear();
}

// мб стоит вынести в отдельный класс обработчик
void ParchmentLinker::findEndPoints() {
    used_.clear();
    //std::vector<cv::Point> endpoints;
    // наоборот потом мб фиксить так как x y перепутаны лучше через cv::Point делать, но тогда менять все местами
    for (int y = 0; y < skeleton_.rows; ++y) {
        for (int x = 0; x < skeleton_.cols; ++x) {
            if (skeleton_.at<uint8_t>(y, x) != 0 && !used_.count({y, x})) {
                std::queue<std::pair<int, int>> q;
                q.push({y, x});
                used_.insert({y, x});

                while (!q.empty()) {
                    std::pair<int, int> currPoint = q.front(); q.pop();
                    used_.insert({currPoint.first, currPoint.second});
                    int currLineNeighbour = 0;

                    for (const auto& offset : offsets_) {

                        int nextX = currPoint.second + offset.second;
                        int nextY = currPoint.first + offset.first;

                        if(skeleton_.at<uint8_t>(cv::Point(nextX, nextY)) != 0){ // вылезет за матрицу спокойно потом фикси
                            currLineNeighbour++;
                        }
                        else continue;

                        if (nextX >= 0 && nextX < skeleton_.cols &&
                            nextY >= 0 && nextY < skeleton_.rows &&
                            !used_.count({nextY, nextX})){

                            q.push({nextY, nextX});
                            used_.insert({nextY, nextX});
                        }

                    }
                    if (currLineNeighbour == 1) {
                        // наоборот потом мб фиксить
                        endpoints_.push_back(cv::Point(currPoint.second, currPoint.first));
                    }
                }
            }
        }
    }
    //return endpoints;
}

float ParchmentLinker::calculateCost(cv::Point p1, cv::Point p2) {
    cv::Point neighbourP1(-1, -1), neighbourP2(-1, -1);

    for (const auto& offset: offsets_) {
        int tempXP1 = p1.x + offset.first, tempYP1 = p1.y + offset.second;
        int tempXP2 = p2.x + offset.first, tempYP2 = p2.y + offset.second;

        if (tempXP1 >= 0 && tempYP1 >= 0 && tempXP1 < skeleton_.cols && tempYP1 < skeleton_.rows
            && neighbourP1 == cv::Point(-1, -1)) {
            if (skeleton_.at<uint8_t>(cv::Point( tempXP1, tempYP1)) != 0) {
                neighbourP1 = cv::Point(tempXP1, tempYP1);
            }
        }

        if (tempXP2 >= 0 && tempYP2 >= 0 && tempXP2 < skeleton_.cols && tempYP2 < skeleton_.rows
            && neighbourP2 == cv::Point(-1, -1)) {
            if (skeleton_.at<uint8_t>(cv::Point(tempXP2, tempYP2)) != 0) {
                neighbourP2 = cv::Point(tempXP2, tempYP2);
            }
        }
    }

    if (neighbourP1 == cv::Point(-1, -1) || neighbourP2 == cv::Point(-1, -1)){
        return std::numeric_limits<float>::infinity();
    }

    cv::Point v1 = p1 - neighbourP1;
    cv::Point v2 = p2 - neighbourP2;

    float angle = std::acos(v1.dot(v2) / (cv::norm(v1) * cv::norm(v2)));

    if (angle < 60 * CV_PI / 180) {
        //std::cout << "angel " << "\n";
        return std::numeric_limits<float>::infinity();
    }

    float distance = cv::norm(p2 - p1);

    // поставить ограничение на min расстояние вроде норм идея, но часто плохо и LineIterator не справляется
    if (distance > 1000 || distance < 10) {
        //std::cout << "distance " << "\n";
        return std::numeric_limits<float>::infinity();
    }

    // надо лучше подумать над проходом коробочкой так как сейчас не очень хорошо все
    cv::LineIterator it(skeleton_, p1, p2, 8);
    for (int i = 1; i < it.count - 1; ++i, ++it) {
        cv::Point pt = it.pos();
        bool flag = 0;
        for(const auto point : endpoints_){
            if (point == pt) {
                flag = 1;
                continue;
            }
        }

        if (skeleton_.at<uint8_t>(pt) == 255 && !flag){
            //std::cout << "LineIterator " << "\n";
            return std::numeric_limits<float>::infinity();
        }
//            for(int k = -1; k <= 1; ++k){
//                for(int j = -1; j <= 1; ++j ){
//                    int tempXPt = pt.x + k, tempYPt = pt.y + j;
//                    if(tempXPt >= 0 && tempYPt >= 0 && tempXPt < skeleton_.cols && tempYPt < skeleton_.rows){
//                        if (skeleton_.at<uint8_t>(cv::Point (tempXPt, tempYPt)) == 255) {
//                            return std::numeric_limits<float>::infinity();
//                        }
//                    }
//                }
//            }
    }
    //std::cout << "norm" << "\n";
    return distance;
}

int ParchmentLinker::findThickness(const cv::Point& endPoint){
    cv::Mat gradX, gradY;
    cv::Sobel(binaryImage_, gradX, CV_16S, 1, 0, 3);
    cv::Sobel(binaryImage_, gradY, CV_16S, 0, 1, 3);

    float angle = std::atan2(gradY.at<uint8_t>(endPoint.y, endPoint.x),
                             gradX.at<uint8_t>(endPoint.y, endPoint.x));

    int thickness = 0;
    for (int i = 1; ; ++i) {
        cv::Point p1 = endPoint + cv::Point(i * cos(angle), i * sin(angle));
        cv::Point p2 = endPoint - cv::Point(i * cos(angle), i * sin(angle));
        if (binaryImage_.at<uchar>(p1.y, p1.x) == 0) {
            thickness = i;
            break;
        }
        if (binaryImage_.at<uchar>(p2.y, p2.x) == 0) {
            thickness = i;
            break;
        }
    }
    return thickness;
}

void ParchmentLinker::linkBrokenLayers() {
    cv::ximgproc::thinning(binaryImage_, skeleton_, cv::ximgproc::THINNING_ZHANGSUEN);

    //cv::imshow("skeleton", skeleton_);

    correctedSkeleton();

    findEndPoints();

    //cv::Mat temps = skeleton_.clone();

    // tempp удалить
    tempp = skeleton_.clone();

    cv::cvtColor(tempp, tempp, cv::COLOR_RGBA2RGB);
    //cv::cvtColor(temps, temps, cv::COLOR_RGBA2RGB);

    //tempMethodShowEndpoints(endpoints_);

    int numEndpoints = endpoints_.size();
    std::vector<std::vector<float>> costMatrix(numEndpoints,
                                               std::vector<float>(numEndpoints,std::numeric_limits<float>::infinity()));
    for (int i = 0; i < numEndpoints; ++i) {
        for (int j = i + 1; j < numEndpoints; ++j) {
            float cost = calculateCost(endpoints_[i], endpoints_[j]);
            costMatrix[i][j] = cost;
            costMatrix[j][i] = cost;
        }
    }

    while (true) {
        int u = 0, v = 1;
        float minCost = std::numeric_limits<float>::infinity();
        for (int i = 0; i < numEndpoints; ++i) {
            for (int j = i + 1; j < numEndpoints; ++j) {
                if (costMatrix[i][j] < minCost) {
                    minCost = costMatrix[i][j];
                    u = i;
                    v = j;
                }
            }
        }

        if (minCost == std::numeric_limits<float>::infinity()) {
            break;
        }

        cv::line(tempp, endpoints_[u], endpoints_[v], cv::Scalar(0,0,255), 1);

        cv::line(skeleton_, endpoints_[u], endpoints_[v], cv::Scalar(255), 1);

        int thickness1 = findThickness(endpoints_[u]);
        int thickness2 = findThickness(endpoints_[v]);

//            int lineLength = cv::norm(endpoints_[v] - endpoints_[u]);
//
//            for (int i = 0; i < lineLength; ++i) {
//                float t = i / (float)lineLength;
//                int thickness = (1 - t) * thickness1 + t * thickness2;
//                cv::Point p = endpoints_[u] + t * (endpoints_[v] - endpoints_[u]);
//                cv::line(binaryImage_, p, p + cv::Point(1, 0), cv::Scalar(255), thickness);
//            }

        int thickness = (thickness2 + thickness1) / 2;

        cv::line(binaryImage_, endpoints_[u], endpoints_[v], cv::Scalar(255), thickness);

        for (int i = 0; i < numEndpoints; i++) {
            costMatrix[u][i] = std::numeric_limits<float>::infinity();
            costMatrix[i][u] = std::numeric_limits<float>::infinity();
            costMatrix[v][i] = std::numeric_limits<float>::infinity();
            costMatrix[i][v] = std::numeric_limits<float>::infinity();
        }
    }
}