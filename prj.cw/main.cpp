#include <iostream>
#include <vector>
#include <utility>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <opencv2/core/types.hpp>

#include <opencv2/core.hpp>

#include "ParchmentLinker/ParchmentLinker.h"
#include "RefinerSegmentator/RefinerSegmentator.h"

int main()
{
    cv::Mat image = cv::imread("../prj.cw/dataset/7228.png",
            cv::IMREAD_GRAYSCALE);

    RefinerSegmentator rf(image);

    std::cout<<"ok";

    return 0;
}

//
//struct FusedRegion{
//    std::vector<cv::Point> contour; // общий контур слитой области
//    struct Section { // секция, которая хранит точку и ее соседей справа и слева
//        cv::Point point; // точка, которую надо соеденить
//        std::vector<cv::Point> neighborPoints; // соседи слева и справа данной точки
//    };
//    std::vector<Section> points; // массив секций для в котором хранятся точки которые надо соеденить и ее соседи
//};

//////////////////////////////////////////////////////////////////////////
// Дальше думать
//struct Line {
//    Point start;
//    Point end;
//    Point2f direction;
//    float length;
//};
//
//// Функция для расчета направлений и длин между соседними точками
//vector<Line> calculateLines(const vector<Point>& polygon) {
//    vector<Line> lines;
//    for (size_t i = 0; i < polygon.size() - 1; ++i) {
//        Line line;
//        line.start = polygon[i];
//        line.end = polygon[i + 1];
//        Point2f direction = Point2f(line.end - line.start);
//        line.length = sqrt(direction.x * direction.x + direction.y * direction.y);
//        line.direction = direction / line.length; // Нормализация
//        lines.push_back(line);
//    }
//    return lines;
//}
//
//// Вычисление косинуса угла между двумя векторами
//float cosineSimilarity(Point2f v1, Point2f v2) {
//    return v1.dot(v2) / (norm(v1) * norm(v2));
//}
//
//// Функция для проверки направления и корректировки точек p0 и pe
//void checkAndCorrectPoints(Point& p0, Point& pe, const vector<Line>& lines) {
//    Point2f newDirection = Point2f(pe - p0);
//    newDirection = newDirection / norm(newDirection);
//
//    float cosineStart = cosineSimilarity(lines.front().direction, newDirection);
//    float cosineEnd = cosineSimilarity(lines.back().direction, newDirection);
//
//    if (cosineStart < 0.5 && cosineEnd < 0.5) {
//        swap(p0, pe);
//    }
//}
//
//// Функция для построения новой ломаной линии
//vector<Point> constructParallelCurve(Point p0, Point pe, const vector<Line>& lines) {
//    vector<Point> newCurve;
//    newCurve.push_back(p0);
//
//    Point2f totalVector = pe - p0;
//    float newTotalLength = sqrt(totalVector.x * totalVector.x + totalVector.y * totalVector.y);
//    float totalLength = 0;
//    for (const auto& line : lines) {
//        totalLength += line.length;
//    }
//    float scale = newTotalLength / totalLength;
//
//    Point2f lastPoint = p0;
//    for (const auto& line : lines) {
//        Point2f nextPoint = lastPoint + line.direction * line.length * scale;
//        newCurve.push_back(nextPoint);
//        lastPoint = nextPoint;
//    }
//
//    newCurve.back() = pe;
//
//    return newCurve;
//}
//
//vector<Point> createParallelBrokenLine(const vector<Point>& originalPolygon, Point p0, Point pe) {
//    std::vector<cv::Point> approximateCurve;
//    cv::approxPolyDP(originalPolygon, approximateCurve, 1.0, false);
//
//    vector<Line> lines = calculateLines(approximateCurve);
//    checkAndCorrectPoints(p0, pe, lines);
//    return constructParallelCurve(p0, pe, lines);
//}

//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//float computeCost(const cv::Point& p1, const cv::Point& p2) {
//    return cv::norm(p2 - p1);  // Евклидово расстояние
//}

//std::vector<std::vector<float>> buildCostMatrix(const std::vector<cv::Point>& points) {
//    size_t n = points.size();
//    std::vector<std::vector<float>> costMatrix(n, std::vector<float>(n, 0.0f));
//
//    for (size_t i = 0; i < n; ++i) {
//        for (size_t j = i + 1; j < n; ++j) {
//            costMatrix[i][j] = computeCost(points[i], points[j]);
//            costMatrix[j][i] = costMatrix[i][j];  // Симметричная матрица
//        }
//    }
//
//    return costMatrix;
//}
//
//
//std::vector<std::pair<int, int>> primMinimumSpanningTree(const std::vector<std::vector<float>>& costMatrix) {
//    int n = costMatrix.size();
//    std::vector<bool> inTree(n, false);
//    std::vector<float> minCost(n, std::numeric_limits<float>::max());
//    std::vector<int> parent(n, -1);
//
//    minCost[0] = 0.0;
//    for (int _ = 0; _ < n - 1; ++_) {
//        float min = std::numeric_limits<float>::max();
//        int u = -1;
//
//        for (int v = 0; v < n; ++v) {
//            if (!inTree[v] && minCost[v] < min) {
//                min = minCost[v];
//                u = v;
//            }
//        }
//
//        inTree[u] = true;
//
//        for (int v = 0; v < n; ++v) {
//            if (!inTree[v] && costMatrix[u][v] < minCost[v]) {
//                minCost[v] = costMatrix[u][v];
//                parent[v] = u;
//            }
//        }
//    }
//
//    std::vector<std::pair<int, int>> result;
//    for (int v = 1; v < n; ++v) {
//        result.emplace_back(parent[v], v);
//    }
//
//    return result;
//}
//
//void processFusedRegion(FusedRegion& fusedRegion) {
//    if (fusedRegion.points.size() < 2) {
//        return;
//    }
//
//    auto costMatrix = buildCostMatrix(fusedRegion.points);
//    auto mstEdges = primMinimumSpanningTree(costMatrix);
//
//    std::vector<cv::Point> parallelCurve;
//
//    for (const auto &edge: mstEdges) {
//        const auto &p0 = fusedRegion.points[edge.first];
//        const auto &pe = fusedRegion.points[edge.second];
//
//        auto segment = createParallelBrokenLine(fusedRegion.contour, p0, pe);
//        parallelCurve.insert(parallelCurve.end(), segment.begin(), segment.end());
//    }
//}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//int main() {
//    // Исходные данные: координаты ломаной линии (многоугольника)
//    std::vector<cv::Point> originalPolygon = {
//            cv::Point(95, 181),
//            cv::Point(96, 181),
//            cv::Point(96, 182),
//            cv::Point(96, 183),
//            cv::Point(96, 184),
//            cv::Point(97, 184),
//            cv::Point(98, 184),
//            cv::Point(99, 185),
//            cv::Point(100, 186),
//
//            cv::Point(100, 186),
//            cv::Point(100, 187),
//            cv::Point(100, 188),
//            cv::Point(100, 189),
//            cv::Point(100, 190),
//            cv::Point(100, 191),
//            cv::Point(100, 192),
//            cv::Point(100, 193),
//            cv::Point(100, 194),
//
//            cv::Point(101, 194),
//            cv::Point(102, 194),
//            cv::Point(103, 194),
//            cv::Point(104, 194),
//            cv::Point(105, 194),
//            cv::Point(106, 194),
//            cv::Point(107, 194),
//            cv::Point(108, 194),
//            cv::Point(109, 194)
//
//    };
//
//    // Две точки, через которые должна пройти новая кривая
//    cv::Point pe(95, 140), p0(109, 164);
//
//    vector<Point> newCurve = createParallelBrokenLine(originalPolygon, p0, pe);
//
//    // Отображение результатов
//    cout << "Исходная ломаная линия:" << endl;
//    for (const auto& point : originalPolygon) {
//        cout << point << " ";
//    }
//    cout << endl;
//
//    cout << "Новая ломаная линия:" << endl;
//    for (const auto& point : newCurve) {
//        cout << point << " ";
//    }
//    cout << endl;
//
//    // Визуализация (опционально)
//    Mat img(600, 800, CV_8UC3, Scalar(255, 255, 255));
//
//    // Рисование исходной ломаной линии
//    for (size_t i = 0; i < originalPolygon.size() - 1; ++i) {
//        line(img, originalPolygon[i], originalPolygon[i + 1], Scalar(0, 0, 255), 2);
//    }
//
//    // Рисование новой ломаной линии
//    for (size_t i = 0; i < newCurve.size() - 1; ++i) {
//        line(img, newCurve[i], newCurve[i + 1], Scalar(255, 0, 0), 2);
//    }
//
//    // Рисование точек p0 и pe
//    circle(img, p0, 5, Scalar(0, 255, 0), 1);
//    circle(img, pe, 5, Scalar(0, 255, 0), 1);
//
//    imshow("Ломаные линии", img);
//    waitKey(0);
//
//    return 0;
//}


//std::vector<cv::Point> generateParallelCurve(const std::vector<cv::Point>& curve, const cv::Point& p0, const cv::Point& pe) {
//    std::vector<cv::Point> approximateCurve;
//    cv::approxPolyDP(curve, approximateCurve, 1.0, false);
//
//    int n = approximateCurve.size() - 1;
//    double invN = 1.0 / (n);
//
//    cv::Mat temp(500, 500, CV_8UC3);
//
//    std::cout<<"approximateCurve:\n";
//    for (int i = n; i > 0; i--){
//        std::cout<< i << ": "<< approximateCurve[i]<<"\n";
//        cv::arrowedLine(temp, approximateCurve[i], approximateCurve[i-1], cv::Scalar(rand()%255, rand()%255, rand()%255), 1);
//    }
//    std::cout<<"0: "<< approximateCurve[0]<<"\n";
//
//    std::cout<<"--------------------------------------------------------\n";
//
//    cv::imwrite("temp.png", temp);
//
//    cv::Mat P(2 * (n + 0), 1, CV_64FC1);
//    for (int i = 1; i <= n; i++) {
//        P.at<double>(2 * (i - 1), 0) = -approximateCurve[i].x;
//        P.at<double>(2 * (i - 1) + 1, 0) = -approximateCurve[i].y;
//    }
//
//    cv::Mat M(2, 1, CV_64FC1);
//    M.at<double>(0, 0) = pe.x - p0.x;
//    M.at<double>(1, 0) = pe.y - p0.y;
//
//    std::cout<<"M:\n";
//    std::cout<<M<<"\n";
//
//    cv::Mat A(2, 2 * (n + 0), CV_64FC1, cv::Scalar(0));
//    for (int i = 0; i < n; ++i) {
//        A.at<double>(0, 2 * i) = 1;
//        A.at<double>(1, 2 * i + 1) = 1;
//    }
//
//    std::cout<<"A:\n"<<A<<"\n";
//
//    std::cout<<"P:\n" << P <<"\n";
//
//    cv::Mat I = cv::Mat::eye(2 * (n + 0), 2 * (n + 0), CV_64FC1);
//
//    std::cout<<"I:\n"<<I<<"\n";
//
//    std::cout<<"A.t:\n"<<A.t()<<"\n";
//
//    cv::Mat AtA = A.t() * A;
//
//    std::cout<<"--------------------------------------------------------\n";
//
//    std::cout<<"AtA:\n"<<AtA<<"\n";
//
//    cv::Mat term1 = -I + invN * AtA;
//
//    std::cout<<"--------------------------------------------------------\n";
//
//    std::cout<<"term1:\n"<<term1<<"\n";
//    cv::Mat term2 = invN * A.t() * M;
//
//    std::cout<<"--------------------------------------------------------\n";
//
//    std::cout<<"term2:\n"<<term2<<"\n";
//
//    cv::Mat X = term1 * P + term2;
//
//    std::cout<<"--------------------------------------------------------\n";
//
//    std::cout <<"X:\n" << X << "\n";
//
//    std::vector<cv::Point> parallelCurve((n + 1));
//    for (int i = 0; i < n; ++i) {
//        parallelCurve[i].x = X.at<double>(2 * i, 0);
//        parallelCurve[i].y = X.at<double>(2 * i + 1, 0);
//    }
//
//    return parallelCurve;
//}
//
//
//int main(){
//    std::vector<cv::Point> consecutiveLines = {cv::Point(95, 181),
//                                               cv::Point(96, 181),
//                                               cv::Point(96, 182),
//                                               cv::Point(96, 183),
//                                               cv::Point(96, 184),
//                                               cv::Point(97, 184),
//                                               cv::Point(98, 184),
//                                               cv::Point(99, 185),
//                                               cv::Point(100, 186),
//
//                                               cv::Point(100, 186),
//                                               cv::Point(100, 187),
//                                               cv::Point(100, 188),
//                                               cv::Point(100, 189),
//                                               cv::Point(100, 190),
//                                               cv::Point(100, 191),
//                                               cv::Point(100, 192),
//                                               cv::Point(100, 193),
//                                               cv::Point(100, 194),
//
//                                               cv::Point(101, 194),
//                                               cv::Point(102, 194),
//                                               cv::Point(103, 194),
//                                               cv::Point(104, 194),
//                                               cv::Point(105, 194),
//                                               cv::Point(106, 194),
//                                               cv::Point(107, 194),
//                                               cv::Point(108, 194),
//                                               cv::Point(109, 194),
//    };
//
//    cv::Mat resultImage(1000, 1000, CV_8UC3, cv::Scalar(0, 0, 0));
//    for (int i = 0; i < consecutiveLines.size() - 1; ++i) {
//        cv::line(resultImage, consecutiveLines[i], consecutiveLines[i+1], cv::Scalar(0, 0, 255),
//                        1, cv::LineTypes::LINE_8);
//    }
//
//    cv::Point p0(95, 151), pe(109, 164);
//    cv::circle(resultImage, p0, 3, cv::Scalar(255, 0, 255), -1);
//    cv::circle(resultImage, pe, 3, cv::Scalar(255, 0, 255), -1);
//
//    std::vector<cv::Point> parall = generateParallelCurve(consecutiveLines, p0, pe);
//
//    for (int i = 0; i < parall.size(); ++i) {
//        cv::circle(resultImage, parall[i], 3, cv::Scalar(255, 255, 255), -1);
//    }
//
//    cv::imwrite("test_result_image.png", resultImage);
//
//    std::cout<<"ok";
//
//    return 0;
//}