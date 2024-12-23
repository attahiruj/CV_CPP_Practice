#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>


using namespace std;
using namespace cv;

void readImage(string path)
{
  Mat img = imread(path);
  imshow("Image", img);
  waitKey(0);
}

void readVideo(string path)
{
  VideoCapture cap(path);
  Mat img;
  
  while(1) {
    cap.read(img);

    if (img.empty())
      return;
    
    imshow("Video", img);
    waitKey(1);
  }

}

void readCamera(int camera_index)
{
  VideoCapture cap(camera_index);
  Mat img;
  
  while(1) {
    cap.read(img);

    if (img.empty())
      return;
    
    imshow("Camera", img);
    waitKey(1);
  }

}


int main()
{
  // readImage("Resources/test.png");
  // readVideo("Resources/test_video.mp4");
  readCamera(0);
  
  return 0;
}