#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>


using namespace std;
using namespace cv;

typedef struct marker {
  Scalar color;
  Scalar min_color_range;
  Scalar max_color_range;
  Point pen_tip;
} Marker;

Mat img;

/**
 * 1. press (a)dd to select and add markers
 *    - for each marker, open up a window to adjust params.
 *    - store the params in a vector
 *    - press (n)ext to move to next marker
 * 2. start painting
 */

void mouseCallback(int event, int xPos, int yPos, int flags, void* userdata) {
  Point* p = (Point*)userdata;
  if (event == cv::EVENT_LBUTTONDOWN) {
    // Capture x and y coordinates when the left mouse button is pressed
    p->x = xPos;
    p->y = yPos;
    // std::cout << "Mouse Clicked at: (" << x << ", " << y << ")" << std::endl;
  }
}

Marker colorPicker(Mat img, Point mouse_click_pos)
{
  Marker marker;
  int h_min = 0, s_min = 0, v_min = 0;
  int h_max = 179, s_max = 255, v_max = 255;
  int avg_b = 0, avg_g = 0, avg_r = 0;
  int no_pixels = 0;

  Scalar avg_hsv(0,0,0);
  Scalar avg_bgr(0,0,0);

  Mat img_hsv, mask;

  namedWindow("Finetune color", WINDOW_AUTOSIZE);
  createTrackbar("Hue_min", "Finetune color", &h_min, 179);
  createTrackbar("Hue_max", "Finetune color", &h_max, 179);
  createTrackbar("Sat_min", "Finetune color", &s_min, 255);
  createTrackbar("Sat_max", "Finetune color", &s_max, 255);
  createTrackbar("Val_min", "Finetune color", &v_min, 255);
  createTrackbar("Val_max", "Finetune color", &v_max, 255);

  // grow the point to a bounding box
  Rect bbox = Rect(
    mouse_click_pos.x, mouse_click_pos.y, 
    10, 10
  );

  // find average range of color
  for (int x = 0;x < bbox.height; x++) {
    for (int y = 0; y < bbox.width; y++) {
      Vec3b pixel = img(bbox).at<Vec3b>(x,y);
      no_pixels++;
      
      avg_b += pixel[0];
      avg_g += pixel[1];
      avg_r += pixel[2];
    }
  }

  // store bgr values
  if ( no_pixels > 0) {
    if (avg_b > 0) avg_bgr.val[0] = avg_b / no_pixels;
    if (avg_g > 0) avg_bgr.val[1] = avg_g / no_pixels;
    if (avg_r > 0) avg_bgr.val[2] = avg_r / no_pixels;
  }

  // Fine tune hsv values

  int offset = 25;
  while (true) {
    // Conver RGB to HSV
    cvtColor(img, img_hsv, COLOR_BGR2HSV);

    Scalar lower(h_min, s_min, v_min);
    Scalar upper(h_max, s_max, v_max);

    inRange(img_hsv, lower, upper, mask);

    // draw rect of color being picked
    // rectangle(img, Point(offset, 5), Point(25 + offset, 25), avg_bgr, FILLED);

    imshow("Finetune color", mask);

    if ( waitKey(1) == 'n' ) {
      destroyWindow("Finetune color");
      break;
    }
  }

  marker.color = avg_bgr;
  marker.min_color_range = Scalar(h_min, s_min, v_min);
  marker.max_color_range = Scalar(h_max, s_max, v_max);

  return marker;
}

void getPenTip(Marker *marker)
{
  vector<vector<Point>> contours;
  vector<Vec4i> heirarchy;

  Mat mask;
  Mat img_hsv;

  // Conver RGB to HSV
  cvtColor(img, img_hsv, COLOR_BGR2HSV);

  inRange(img_hsv, marker->min_color_range, marker->max_color_range, mask);

  findContours(mask, contours, heirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

  vector<vector<Point>> min_polygon(contours.size());   // Minimum bounding box poligon, used to predict shape
  vector<Rect> bounding_rect(contours.size());
  
  Point pen_tip(0, 0);

  for (int i = 0; i < contours.size(); i++) {

    double area = contourArea(contours[i]);
    double perimeter = arcLength(contours[i], true);

    // skip small contours
    if (area < 1000) {
      marker->pen_tip.x = 0;
      marker->pen_tip.y = 0;
      continue;
    }

    // Find minimum polygon
    approxPolyDP(contours[i], min_polygon[i], 0.02*perimeter, true);
    
    // find minimum bounding rect; can be gotten from contour directly too
    bounding_rect[i] = boundingRect(min_polygon[i]);

    drawContours(img, min_polygon, i, Scalar(0, 0, 255), 2);

    // get pen tip from bounding rect
    marker->pen_tip.x = bounding_rect[i].x + bounding_rect[i].width / 2;  // center of bounding rect width
    marker->pen_tip.y = bounding_rect[i].y;                               // top of bounding rect

  }
}


int main()
{
  bool debug = true;
  int camera_index = 0;
  VideoCapture cap(camera_index);
  vector<Marker> markers;
  vector<Point> pen_tips;
  Point mouse_click_pos;

  namedWindow("Select markers", WINDOW_AUTOSIZE);
  setMouseCallback("Select markers", mouseCallback, &mouse_click_pos);

  while(true) {
    cap.read(img);
    
    if ( mouse_click_pos.x > 0 && mouse_click_pos.y > 0 ) {
      markers.push_back( colorPicker(img, mouse_click_pos) );

      // reset mouse click position
      mouse_click_pos.x = 0;
      mouse_click_pos.y = 0;

      if (debug) {
        cout << "RGB: " << markers[0].color << endl;
        cout << "HSV: " << markers[0].min_color_range << " - " << markers[0].max_color_range << endl;
        cout << "Markers: " << markers.size() << endl;
      }
    }

    if (markers.size() > 0) {
      for (int i = 0; i < markers.size(); i++) {
        getPenTip(&markers[i]);
        if (debug) cout << "Pen tip[" << i << "]: " << markers[i].pen_tip << endl;
      }
    }

    imshow("Select markers", img);
    if ( waitKey(1) == 'n' )
      break;
  }

  return  0;
}