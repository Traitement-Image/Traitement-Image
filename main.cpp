#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
   cv::Mat img = cv::imread("DVI1/gi01.jpeg");
   if (img.empty()) {
      std::cerr << "Image not found!" << std::endl;
      return -1;
   }
   cv::imshow("Display window", img);
   cv::waitKey(0);
   return 0;
}
