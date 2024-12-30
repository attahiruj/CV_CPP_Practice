/**
 * @file virtual_paint.cpp
 * @author Attahiru Jibril (attahiruj@gmail.com)
 * @brief A virtual paint application that uses color detection to draw on a virtual canvas
 *        Uses OpenCV to capture video feed from camera.
 *      Usage:
 *      1. click on a marker to pick color and fine tune color range
 *      2. start painting
 *      3. to add another marker, click on a different color
 *      4. press 'q' to quit
 * 
 * @date 2024-12-28
 * 
 * 
 */

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>


using namespace std;
using namespace cv;

bool debug = true;

typedef struct marker {
  Scalar color;
  Scalar min_color_range;
  Scalar max_color_range;
  vector<Point> pen_tip;
} Marker;

Mat img;      // Global image variable for persistence across functions

/**
 * @brief mouseCallback function to capture mouse click position
 * 
 * @param event event type
 * @param xPos 
 * @param yPos 
 * @param flags 
 * @param userdata the point object to store the mouse click position
 */
void mouseCallback(int event, int xPos, int yPos, int flags, void* userdata) {
  Point* p = (Point*)userdata;
  if (event == cv::EVENT_LBUTTONDOWN) {
    // Capture x and y coordinates when the left mouse button is pressed
    p->x = xPos;
    p->y = yPos;
    if (debug) std::cout << "Mouse Clicked at: (" << p->x << ", " << p->y << ")" << std::endl;
  }
}

/**
 * @brief colorPicker function to pick color from image
 * 
 * @param img image to pick color from
 * @param mouse_click_pos mouse click position
 * @return Marker 
 */
Marker colorPicker(Mat& img, Point mouse_click_pos)
{
  Marker marker;
  int h_min = 0, s_min = 0, v_min = 0;
  int h_max = 179, s_max = 255, v_max = 255;
  int avg_b = 0, avg_g = 0, avg_r = 0;
  int no_pixels = 0;

  Scalar avg_bgr(0,0,0);

  Mat img_hsv, mask;

  namedWindow("Finetune color", WINDOW_AUTOSIZE);

  // Create trackbars to fine tune color range
  createTrackbar("Hue_min", "Finetune color", &h_min, 179);
  createTrackbar("Hue_max", "Finetune color", &h_max, 179);
  createTrackbar("Sat_min", "Finetune color", &s_min, 255);
  createTrackbar("Sat_max", "Finetune color", &s_max, 255);
  createTrackbar("Val_min", "Finetune color", &v_min, 255);
  createTrackbar("Val_max", "Finetune color", &v_max, 255);

  // grow the point to a bounding box
  int bbox_size = 10;
  Rect bbox = Rect(
    max(mouse_click_pos.x - bbox_size / 2, 0), 
    max(mouse_click_pos.y - bbox_size / 2, 0), 
    bbox_size, bbox_size
  );

  bbox &= Rect(0, 0, img.cols, img.rows); // Ensure bbox is within image bounds

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

/**
 * @brief getPenTip function to get pen tip from image
 * 
 * @param marker marker object
 */
void getPenTip(Marker *marker)
{
  vector<vector<Point>> contours;
  vector<Vec4i> heirarchy;

  Mat mask;
  Mat img_hsv;

  // Conver RGB to HSV and get contours, then mask, then bounding rect, then pen tip
  cvtColor(img, img_hsv, COLOR_BGR2HSV);

  inRange(img_hsv, marker->min_color_range, marker->max_color_range, mask);

  findContours(mask, contours, heirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
  if (contours.size() == 0) return;

  vector<vector<Point>> min_polygon(contours.size());   // Minimum bounding box poligon, used to predict shape
  vector<Rect> bounding_rect(contours.size());
  
  Point pen_tip(0, 0);

  for (int i = 0; i < contours.size(); i++) {

    double area = contourArea(contours[i]);
    double perimeter = arcLength(contours[i], true);

    // skip small contours
    if (area < 1000)
      continue;
    
    // Find minimum polygon
    approxPolyDP(contours[i], min_polygon[i], 0.02*perimeter, true);
    
    // find minimum bounding rect; can be gotten from contour directly too
    bounding_rect[i] = boundingRect(min_polygon[i]);

    if (debug) drawContours(img, min_polygon, i, Scalar(0, 0, 255), 2);

    // get pen tip from bounding rect
    pen_tip.x = bounding_rect[i].x + bounding_rect[i].width / 2;  // center of bounding rect width
    pen_tip.y = bounding_rect[i].y;                               // top of bounding rect

    // draw crossair at pen tip
    line(img, Point(pen_tip.x - 10, pen_tip.y), Point(pen_tip.x + 10, pen_tip.y), Scalar(0, 255, 0), 1);
    line(img, Point(pen_tip.x, pen_tip.y - 10), Point(pen_tip.x, pen_tip.y + 10), Scalar(0, 255, 0), 1);

    marker->pen_tip.push_back(pen_tip);
  }
}

void drawPaint(Marker marker)
{
  if (marker.pen_tip.size() == 0) return;

  for (int i = 0; i < marker.pen_tip.size(); i++) {
    // pop empty pen tip
    if (marker.pen_tip[i].x == 0 && marker.pen_tip[i].y == 0) {
      marker.pen_tip.pop_back();
      continue;
    }
    
    // draw line from previous pen tip to current pen tip
    if (i > 0 && (marker.pen_tip[i-1].x > 0 && marker.pen_tip[i].x > 0))
      line(img, marker.pen_tip[i-1], marker.pen_tip[i], marker.color, 2);
  }
}

int main()
{
  int camera_index = 0;
  VideoCapture cap(camera_index);
  vector<Marker> markers;
  vector<Point> pen_tips;
  Point mouse_click_pos;

  namedWindow("Virtual canvas", WINDOW_AUTOSIZE);
  setMouseCallback("Virtual canvas", mouseCallback, &mouse_click_pos);

  while(true) {
    cap.read(img);
    
    // Add markers
    if ( mouse_click_pos.x > 0 && mouse_click_pos.y > 0 ) {
      markers.push_back( colorPicker(img, mouse_click_pos) );

      // reset mouse click position
      mouse_click_pos.x = 0;
      mouse_click_pos.y = 0;
    }

    // Paint on canvas
    for (int i = 0; i < markers.size(); i++) {
      getPenTip(&markers[i]);
      drawPaint(markers[i]);

      if (debug) cout << "Pen tip[" << i << "]: " << markers[i].pen_tip << endl;
    }

    imshow("Virtual canvas", img);

    if ( waitKey(1) == 'q' )
      break;
  }

  return  0;
}