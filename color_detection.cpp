#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

int main()
{
  string path = "./Resources/lambo.png";
  Mat img = imread(path);
  Mat img_hsv, mask;
  int h_min = 0, s_min = 0, v_min = 0;
  int h_max = 179, s_max = 255, v_max = 255;

  namedWindow("Trackbars", (640, 200));
  createTrackbar("Hue_min", "Trackbars", &h_min, 179);
  createTrackbar("Hue_max", "Trackbars", &h_max, 179);
  createTrackbar("Sat_min", "Trackbars", &s_min, 255);
  createTrackbar("Sat_max", "Trackbars", &s_max, 255);
  createTrackbar("Val_min", "Trackbars", &v_min, 255);
  createTrackbar("Val_max", "Trackbars", &v_max, 255);

  while(true) {

    // Conver RGB to HSV
    cvtColor(img, img_hsv, COLOR_BGR2HSV);

    Scalar lower(h_min, s_min, v_min);
    Scalar upper(h_max, s_max, v_max);

    inRange(img_hsv, lower, upper, mask);

    imshow("Image", img);
    imshow("Image HSV", img_hsv);
    imshow("Image mask", mask);

    waitKey(1);
  }


  return 0;
}