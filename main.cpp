#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

int main()
{
  Mat frame;
  VideoCapture cap;

  cap.open(0);

  if ( !cap.isOpened() ) {
    cerr << "Could not open the camera" << endl;
    return -1;
  }

  cap.read(frame);

  if (frame.empty()) {
    cerr << "Error! Blank Frame" << endl;
  }

  imshow("Video", frame);

  waitKey(0);


  return 0;
}