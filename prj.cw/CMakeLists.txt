add_executable(main main.cpp
        ParchmentLinker/ParchmentLinker.h
        ParchmentLinker/ParchmentLinker.cpp
        RefinerSegmentator/RefinerSegmentator.h
        RefinerSegmentator/RefinerSegmentator.cpp)
target_link_libraries(main PRIVATE opencv_core opencv_imgproc opencv_highgui opencv_ximgproc)