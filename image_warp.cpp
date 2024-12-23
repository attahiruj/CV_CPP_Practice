#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>


using namespace std;
using namespace cv;

int main()
{
  string path = "Resources/cards.jpg";

  Mat img = imread(path);

  float width = 250, height = 350;
  Mat matrix, img_warp;

  Point2f src_coord[4] = {
    {529, 142},
    {771, 190},
    {405, 395},
    {674, 457}
  };

  Point2f dest_coord[4] = {
    {0.0f, 0.0f},
    {width, 0.0f},
    {0.0f, height},
    {width, height}
  };
  
  // get transformation matrix
  matrix = getPerspectiveTransform(src_coord, dest_coord);
  warpPerspective(img, img_warp, matrix, Point(width, height));

  for (int i = 0; i < 4; i++) {
    circle(img, src_coord[i], 10, Scalar(0, 0, 255), FILLED);
  }

  imshow("Image", img);
  imshow("Image warp", img_warp);
  waitKey(0);
  
  return 0;
}