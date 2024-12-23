/* basic_operations.cpp
 * 
 * Basic Image Processing in openCV
 */


#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>


using namespace std;
using namespace cv;

int main()
{
  string path = "./Resources/shapes.png";
  Mat img = imread(path);
  Mat gray, blur, canny, dilated, eroded, resized, scaled, cropped;
  Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));

  cvtColor(img, gray, COLOR_BGR2GRAY);      // rgb to grayscale
  GaussianBlur(img, blur, Size(3, 3), 3);   // blur
  Canny(img, canny, 25, 75);                // edge detection
  dilate(canny, dilated, kernel);           // dilation
  erode(dilated, eroded, kernel);           // erosion

  resize(img, resized, Size(320, 320));
  resize(img, scaled, Size(), 0.5, 0.5);

  Rect roi(100, 100, 200, 200);
  cropped = img(roi);


  // Display images
  imshow("image", img);
  imshow("Gray", gray);
  imshow("Blur", blur);
  imshow("Edge", canny);
  imshow("Edge Dilate", dilated);
  imshow("Edge Erode", eroded);

  imshow("Resized", resized);
  imshow("Scaled", scaled);

  imshow("Cropped", cropped);

  waitKey(0);

  return 0;
}