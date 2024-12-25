#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>

using namespace cv;
using namespace std;

void detectFaces(int camera_index, string cascade_path)
{
  VideoCapture cap(camera_index);
  Mat img;
  
  while(1) {
    cap.read(img);

    if (img.empty())
      return;
    
    // load cascade
    CascadeClassifier faceCascade;
    faceCascade.load(cascade_path);
    if ( faceCascade.empty() ) {
      cout << "Could not load cascade: " << cascade_path << endl;
      return;
    }

    // detect faces
    vector<Rect> faces;
    faceCascade.detectMultiScale(img, faces, 1.1, 1);

    // draw bounding box
    for (int i = 0; i < faces.size(); i++) {
      string text =  "Face " + to_string(i+1);
      rectangle(img, faces[i].tl(), faces[i].br(), Scalar(0, 255, 0), 1);
      putText(
        img, 
        text,
        { faces[i].x, faces[i].y - 2 }, 
        FONT_HERSHEY_PLAIN, 
        1.2,
        Scalar(0, 255, 0),
        1
      );
    }

    imshow("Video", img);
    waitKey(1);
  }

}

int main()
{
  string cascade_path   = "./Resources/haarcascade_frontalface_default.xml";
  string image_path     = "./Resources/test.png";

  detectFaces(0, cascade_path);

  return 0;
}