#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>


using namespace std;
using namespace cv;

// vector<vector<int>> colors {
//   {}
// }

void colorPicker(Mat img)
{
  int avg_h = 0, avg_s = 0, avg_v = 0;
  vector<Rect> bboxes;
  Scalar avg_hsv(0,0,0);

  // draw bounding box over color
  // Rect2d bbox = selectROI("Select ROI and press enter", img);
  selectROIs("Select ROI and press enter", img, bboxes);

  for (int i = 0; i < bboxes.size(); i++) {
    avg_h = 0; avg_s = 0; avg_v = 0;
    int no_pixels = 0;

    // convert RGB to hsv (cropped to roi)
    Mat img_hsv;
    cvtColor(img(bboxes[i]), img_hsv, COLOR_BGR2HSV);

    // find average range of color
    for (int x = 0;x < bboxes[i].height; x++) {
      for (int y = 0; y < bboxes[i].width; y++) {
        Vec3b pixel = img_hsv.at<Vec3b>(x,y);
        no_pixels++;

        avg_h += pixel[0];
        avg_s += pixel[1];
        avg_v += pixel[2];
      }
    }

    // store hsv values
    if ( no_pixels > 0) {
      if (avg_h > 0) avg_hsv.val[0] = avg_h / no_pixels;
      if (avg_s > 0) avg_hsv.val[1] = avg_s / no_pixels;
      if (avg_v > 0) avg_hsv.val[2] = avg_v / no_pixels;
    }

    // Create single pixel HSV Mat with the average values 

    Scalar hsv_color(avg_hsv.val[0], avg_hsv.val[1], avg_hsv.val[2]);
    Mat hsv_mat(1, 1, CV_8UC3, hsv_color);

    // Convert to BGR
    Mat bgr_mat;
    cvtColor(hsv_mat, bgr_mat, COLOR_HSV2BGR);

    // Draw rectangle with converted color
    Scalar bgr_color = Scalar(bgr_mat.at<Vec3b>(0,0));

    int offset = 0;
    if (i == 0) offset = (i+1);
    else offset = (i+1) * 25;
    rectangle(img, Point(5 + offset, 5), Point(25 + offset, 25), bgr_color, FILLED);

  }

  imshow("Press Enter to continue", img);
  waitKey(0);
}

int main()
{
  int camera_index = 0;
  VideoCapture cap(camera_index);
  Mat img;
  Scalar avg_hsv;

  while(true) {
    cap.read(img);
    imshow("Press n to continue", img);

    if ( waitKey(1) == 'n' )
      break;
  }


  colorPicker(img);

  // while(1) {

  //   if (img.empty())
  //     return 1;
    
  // }

  return  0;
}