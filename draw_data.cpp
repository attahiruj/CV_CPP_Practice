#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;

int main()
{
  // Blank image
  Mat img(512, 512, CV_8UC3, Scalar(255, 255, 255));

  // Circle outline
  circle(img, Point(100, 100), 40, Scalar(0, 69, 255), 2);
  
  // Circle filed
  circle(img, Point(200, 100), 40, Scalar(0, 69, 255), FILLED);

  // Rectangle
  rectangle(img, Point(250, 50), Point(350, 150), Scalar(0, 69, 255), 2);

  // Line
  line(img, Point(0, 256), Point(512, 256), Scalar(0, 69, 255), 2);

  // Put text
  putText(img, "OpenCV", Point(200, 256), FONT_HERSHEY_PLAIN, 2.0, Scalar(0, 69, 255), 2);

  imshow("Image", img);
  waitKey(0);

  return 0;
}

