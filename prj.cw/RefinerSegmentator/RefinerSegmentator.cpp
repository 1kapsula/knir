//
// Created by Yuri Bespalov on 27.07.2024.
//
#include "RefinerSegmentator.h"

RefinerSegmentator::RefinerSegmentator(const cv::Mat& inputImage):parchment_(new ParchmentLinker(inputImage)) {
    binaryImage_ = parchment_->getCorrectBinaryImage();

    cv::Mat copyBinaryImage = binaryImage_.clone();
    copyBinaryImage.convertTo(copyBinaryImage, CV_8UC1);

    contourImage_ = cv::Mat::zeros(binaryImage_.size(), CV_8UC1);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy; // улучшение мб потом

    cv::findContours(binaryImage_, contours , hierarchy, cv::RetrievalModes::RETR_TREE,
                     cv::ContourApproximationModes::CHAIN_APPROX_NONE);

    std::vector<std::vector<cv::Point>> contours1 = (contours);

    cv::Mat cnt = cv::Mat::zeros(contourImage_.size(), CV_8UC3);

    for (int i = 0; i < contours1.size(); i++) {
        cv::Scalar curr = cv::Scalar(rand()%255, rand()%255, rand()%255);
        for(const auto & point: contours1[i]){
            cv::Vec3b& pixel = cnt.at<cv::Vec3b>(point);
            pixel[0] = curr[0];
            pixel[1] = curr[1];
            pixel[2] = curr[2];
        }
    }

    contours.erase(std::remove_if(contours.begin(), contours.end(), [](const std::vector<cv::Point>& vec) {
        return vec.size() < 0; // важно
    }), contours.end());

    for (int i = 0; i < contours.size(); i++) {
        cv::drawContours(contourImage_, contours, i, cv::Scalar(255, 255, 255), 1);
    }

    std::vector<cv::Point> junctionPoint = findJunctionPoints(contours, 15, 120);
    std::vector<cv::Point> junctionPoint1 = findJunctionPoints(contours1, 15, 120);

    for(const auto & point: junctionPoint1){

        cv::Vec3b& pixel = cnt.at<cv::Vec3b>(point);
        pixel[0] = 0;
        pixel[1] = 0;
        pixel[2] = 255;
    }

    std::vector<JunctionSection> junctionSections = findJunctionSections(junctionPoint);

    findFirstAndLast(junctionSections);
    cv::Mat markedImage = contourImage_.clone();
    cv::cvtColor(markedImage, markedImage, cv::COLOR_RGBA2RGB);

    std::vector<std::pair<cv::Point, cv::Point>> sections;

    std::vector<std::pair<cv::Point, cv::Point>> lines;

    for (int i = 0; i < junctionSections.size(); ++i) {
        cv::Scalar color = cv::Scalar(rand()%255, rand()%255, rand()%255);
        for (int j = 0; j < junctionSections[i].points.size(); ++j) {
            cv::Vec3b& pixel = markedImage.at<cv::Vec3b>(junctionSections[i].points[j]);
            pixel[0] = color[0];
            pixel[1] = color[1];
            pixel[2] = color[2];
        }

        cv::circle(markedImage, junctionSections[i].firstEndpoint, 3, cv::Scalar(0, 0, 255), 1);
        cv::circle(markedImage, junctionSections[i].lastEndpoint, 3, cv::Scalar(0, 255, 0), 1);

        lines.push_back(drawRay(copyBinaryImage, junctionSections[i].firstEndpoint,
                                junctionSections[i].lastEndpoint));
    }

    for (const auto &line : lines){
        cv::line(copyBinaryImage, line.first, line.second, cv::Scalar(0), 2);
    }

    cv::Mat out2 = cv::Mat::zeros(copyBinaryImage.size(), CV_8UC3), posCont = cv::Mat::zeros(copyBinaryImage.size(), CV_8UC1);

    std::vector<std::vector<cv::Point>> contoursFusedRegion, possibleСontoursFusedRegion;

    cv::findContours(copyBinaryImage, possibleСontoursFusedRegion, cv::RetrievalModes::RETR_TREE,
                     cv::ContourApproximationModes::CHAIN_APPROX_NONE);

    for (int i = 0; i < possibleСontoursFusedRegion.size(); i++) {
        cv::drawContours(out2, possibleСontoursFusedRegion, i, cv::Scalar(rand()%255, rand()%255, rand()%255), 1);
    }

    std::vector<cv::Point> middlePointsJunctionSections, tempMiddlePointsJunctionSections;

    for(auto & point: junctionSections){
        middlePointsJunctionSections.push_back(point.points[int(point.points.size()/2)]);
        point.middlePoint = point.points[int(point.points.size()/2)];
    }

    std::set<int> indexUsedRegion;

    for (const auto & testMiddlePoint : middlePointsJunctionSections){
        for(int i = 0; i < possibleСontoursFusedRegion.size(); ++i){
            if (!indexUsedRegion.count(i) && cv::pointPolygonTest(possibleСontoursFusedRegion[i], testMiddlePoint, 0) == 0){//!=-1 надо проверить как лучше
                contoursFusedRegion.push_back(possibleСontoursFusedRegion[i]);
                indexUsedRegion.insert(i);
//                    tempMiddlePointsJunctionSections.push_back(testMiddlePoint);
            }
        }
    }

    for (int i = 0; i < contoursFusedRegion.size(); i++) {
        cv::drawContours(posCont, contoursFusedRegion, i, cv::Scalar(255), -1);
    }

    cv::morphologyEx(posCont, posCont, cv::MorphTypes::MORPH_DILATE, cv::getStructuringElement(
            cv::MORPH_RECT, cv::Size(3, 3)), cv::Point(-1, -1), 2);

    cv::bitwise_and(binaryImage_, posCont, posCont);

    std::vector<std::vector<cv::Point>> posContContours;

    cv::findContours(posCont, posContContours , hierarchy, cv::RetrievalModes::RETR_TREE,
                     cv::ContourApproximationModes::CHAIN_APPROX_NONE);

    cv::Mat posConContursMat = cv::Mat::zeros(copyBinaryImage.size(), CV_8UC1);;

    for (int i = 0; i < posContContours.size(); i++) {
        cv::drawContours(posConContursMat, posContContours, i, cv::Scalar(255), 1);
    }

    for (const auto & posContContour : posContContours) {
        FusedRegion temp;
        temp.contour = posContContour;
        for (auto & junctionSection : junctionSections) {
            if (cv::pointPolygonTest(posContContour, junctionSection.middlePoint, 0) == 0){
                //tempMiddlePointsJunctionSections.push_back(junctionSection.middlePoint);
                temp.points.push_back(junctionSection.middlePoint);
                temp.junctionSection.push_back(junctionSection);
            }
        }
        fusedRegion.push_back(temp);
    }

    cv::Mat tempPosCont = posCont.clone();

    cv::cvtColor(tempPosCont, tempPosCont, cv::COLOR_GRAY2BGR);

    for (auto &region: fusedRegion){
        for (auto &x: region.points){
            cv::circle(tempPosCont, x, 1, cv::Scalar(0, 0, 255), -1);
            cv::circle(tempPosCont, x, 5, cv::Scalar(0, 0, 255), 1);
        }
    }

//        for (double i = 1.0; i < 1.1; i+=0.1){
//            cv::Mat energy = computeShapeEnergy(posCont, i, 1);
//            cv::imwrite("energy/" + std::to_string(i) + "_energy.png", energy);
//        }

    // временный результат
    cv::imwrite("binaryImage.png", binaryImage_);
    cv::imwrite("pos_con_conturs_mat.png", posConContursMat);
    cv::imwrite("out2.png", out2);
    cv::imwrite("temp_fused_region.png", tempPosCont);
    cv::imwrite("fused_region.png", posCont);
    cv::imwrite("contourImage12.png", cnt);
    cv::imwrite("markedImage22.png", markedImage);
    cv::imwrite("copyBinaryImage.png", copyBinaryImage);
}

std::pair<cv::Point, cv::Point> RefinerSegmentator::drawRay(cv::Mat &image, cv::Point startPoint, cv::Point endPoint){
    cv::Point direction(endPoint - startPoint);
    float phi = std::atan2(direction.y, direction.x);
    int step = 1;

    cv::Point whiteEnd = endPoint, whiteStart = startPoint;
    bool isStart = false, isEnd = false;

    while(!isStart || !isEnd){
        if (whiteEnd.x < 0 || whiteEnd.x >= image.cols || whiteEnd.y < 0 || whiteEnd.y >= image.rows
            || image.at<uint8_t>(whiteEnd) != 255) {
            isEnd = true;
        }
        if (whiteStart.x < 0 || whiteStart.x >= image.cols || whiteStart.y < 0 || whiteStart.y >= image.rows
            || image.at<uint8_t>(whiteStart) != 255) {
            isStart = true;
        }
        if (!isEnd){
            whiteEnd += cv::Point( step * std::cos(phi), step * std::sin(phi));
        }
        if (!isStart){
            whiteStart -= cv::Point( step * std::cos(phi), step * std::sin(phi));
        }
        step++;
    }
    return {whiteStart, whiteEnd};
}

bool RefinerSegmentator::isJunctionPoint(const std::vector<cv::Point> &contour, int i, int n, double thresholdAngle){
    if (contour.size() < 2 * n + 1) return false;

    cv::Point p0 = contour[i];
    cv::Point pn = contour[(i + n) % contour.size()];
    cv::Point pMinusN = contour[(i - n + contour.size()) % contour.size()];

    cv::Point vecP0toPn = pn - p0;
    cv::Point vecP0toPMinusN = pMinusN - p0;

    if (norm(vecP0toPn) < 1e-6 || norm(vecP0toPMinusN) < 1e-6) return false;

    double cosTheta = vecP0toPn.dot(vecP0toPMinusN) / (norm(vecP0toPn) * norm(vecP0toPMinusN));
    double angle = acos(cosTheta) * 180 / CV_PI;

    return angle < thresholdAngle;
}

std::vector<cv::Point>
RefinerSegmentator::findJunctionPoints(const std::vector<std::vector<cv::Point>> &contours, int n,
                                       double thresholdAngle) {
    std::vector<cv::Point> junctionPoints;
    for (const std::vector<cv::Point>& contour : contours) {
        for (int i = 0; i < contour.size(); ++i) {
            if (isJunctionPoint(contour, i, n, thresholdAngle)) {
                junctionPoints.push_back(contour[i]);
            }
        }
    }
    return junctionPoints;
}

int RefinerSegmentator::getIndexOfPoint(const std::vector<cv::Point> &points, const cv::Point &point) {
    for (int i = 0; i < points.size(); ++i) {
        if (points[i] == point) {
            return i;
        }
    }
    return -1;
}

std::vector<RefinerSegmentator::JunctionSection> RefinerSegmentator::findJunctionSections(const std::vector<cv::Point> &junctionPoint) {
    std::vector<JunctionSection> junctionSections;
    std::vector<bool> used(junctionPoint.size(), false);
    for (int i = 0; i < junctionPoint.size(); ++i) {
        if (!used[i]) {
            JunctionSection component;

            component.points.push_back(junctionPoint[i]);
            component.used.insert({junctionPoint[i].x, junctionPoint[i].y});

            used[i] = true;
            std::queue<cv::Point> q;
            q.push(junctionPoint[i]);
            while (!q.empty()) {
                cv::Point currPoint = q.front();
                q.pop();
// надо добавить в used
                used[getIndexOfPoint(junctionPoint, currPoint)] = true;
                for (const auto &offset: offsets_) {
                    cv::Point neighbor(currPoint.x + offset.first, currPoint.y + offset.second);
                    if (!used[getIndexOfPoint(junctionPoint, neighbor)]) {

                        component.points.push_back(neighbor);
                        component.used.insert({neighbor.x, neighbor.y});

                        used[getIndexOfPoint(junctionPoint, neighbor)] = true;
                        q.push(neighbor);
                    }
                }
            }
            if (component.points.size() > 0) {
                junctionSections.push_back(component);
            }
        }
    }
    return junctionSections;
}

void RefinerSegmentator::findFirstAndLast(std::vector<JunctionSection> &junctionSections) {
    for (auto & currentSection: junctionSections){
        bool first_found = false;
        bool last_found = false;
        for (int i = 0; i < currentSection.points.size(); ++i) {
            int currentNeighbors = 0;
            for (auto &offset: offsets_) {
                int nextX = currentSection.points[i].x + offset.first;
                int nextY = currentSection.points[i].y + offset.second;
                if (currentSection.used.count({nextX, nextY})) {
                    currentNeighbors++;
                }
            }
//                if (currentNeighbors == 1) {
//                    if (currentNeighbors == 1) {
//                        if (currentSection.firstEndpoint == cv::Point(-1, -1)) {
//                            currentSection.firstEndpoint = currentSection.points[i];
//                        }
//                        else if (currentSection.lastEndpoint == cv::Point(-1, -1)){
//                            currentSection.lastEndpoint = currentSection.points[i];
//                        }
//                    }
//                }
            if (currentNeighbors == 1) {
                if (currentSection.firstEndpoint == cv::Point(-1, -1)) {
                    currentSection.firstEndpoint = currentSection.points[i];
                    first_found = true;
                }
                else if (currentSection.lastEndpoint == cv::Point(-1, -1)) {
                    currentSection.lastEndpoint = currentSection.points[i];
                    last_found = true;
                }
            }
        }

        // Если первая и последняя точки не найдены, вычисляем центр масс
        if (!first_found && !last_found) {
            int sumX = 0;
            int sumY = 0;
            for (const auto& point : currentSection.points) {
                sumX += point.x;
                sumY += point.y;
            }
            int centerX = sumX / currentSection.points.size();
            int centerY = sumY / currentSection.points.size();
            cv::Point centerMass(centerX, centerY);

            currentSection.firstEndpoint = centerMass;
            currentSection.lastEndpoint = centerMass;
        }
    }
}