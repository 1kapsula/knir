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
