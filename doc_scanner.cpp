/**
 * @file doc_scanner.cpp
 * @author Attahiru Jibril (attahiruj@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2024-12-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <optional>

using namespace std;
using namespace cv;

bool debug = true;

const Scalar CYAN = Scalar(182, 196, 46);
const Scalar RED = Scalar(84, 0, 255);

// Invalid points type
vector<Point> invalid_points = {Point(-1, -1)};


/**
 * @brief preprocess function to preprocess image
 * 
 * @param input input image
 * @return Mat preprocessed image
 */
Mat preprocess(Mat input)
{
  Mat gray, blur, canny, dilated;
  Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));

  int blur_size = 7;

  cvtColor(input, gray, COLOR_BGR2GRAY);        // rgb to grayscale
  GaussianBlur(gray, blur, Size(blur_size, blur_size), 3);      // blur
  Canny(blur, canny, 25, 75);                   // edge detection
  dilate(canny, dilated, kernel);               // dilation

  return dilated;
}

/**
 * @brief getDocBounds function to get document bounds
 * 
 * @param input input image
 * @return vector<Point> document bounds
 */
vector<Point> getDocBounds(Mat input)
{
  vector<vector<Point>> contours;
  vector<Vec4i> heirarchy;
  static vector<Point> prev_doc_identified;
  int likely_doc = -1;

  Mat processed = preprocess(input);
  findContours(processed, contours, heirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

  vector<vector<Point>> min_polygon(contours.size());   // Minimum bounding box poligon, used to predict shape

  for (int i = 0; i < contours.size(); i++) {

    // draw contours
    double area = contourArea(contours[i]);

    if (area > 1000) {      // skip small contours
      // Find minimum polygon, assume quadrilateral is document
      double perimeter    = arcLength(contours[i], true);
      approxPolyDP(contours[i], min_polygon[i], 0.02*perimeter, true);

      if (min_polygon[i].size() == 4 && isContourConvex(min_polygon[i])) {
        likely_doc = i;
        prev_doc_identified = min_polygon[i];
      }
    }
  }

  if (likely_doc >= 0) {
    drawContours(input, min_polygon, likely_doc, CYAN, 2);
  } else if (prev_doc_identified.size() > 0) {
    vector<vector<Point>> prev_doc_contour = {prev_doc_identified};
    drawContours(input, prev_doc_contour, 0, CYAN, 2); 
  } else {
    // draw  small X mid screen
    int x = input.cols / 2;
    int y = input.rows / 2;
    line(input, Point(x-50, y-50), Point(x+50, y+50), RED, 2);
    line(input, Point(x-50, y+50), Point(x+50, y-50), RED, 2);

    return invalid_points;
  }

  return prev_doc_identified;
}

/**
 * @brief sortDocBounds function to sort document bounds
 * 
 * @param doc_bounds document bounds
 * @return vector<Point> sorted document bounds
 */
vector<Point> sortDocBounds(vector<Point> doc_bounds)
{
  if (doc_bounds.size() != 4) {
    cout << "Invalid document bounds" << endl;
    return invalid_points;
  }

  vector<Point> sorted_bounds;
  vector<int> sums, diffs;
  int tl, tr, br, bl;

  for (int i = 0; i < doc_bounds.size(); i++) {
    sums.push_back( doc_bounds[i].x + doc_bounds[i].y );
    diffs.push_back( doc_bounds[i].x - doc_bounds[i].y );
  }

  tl = min_element(sums.begin(), sums.end()) - sums.begin();
  tr = max_element(diffs.begin(), diffs.end()) - diffs.begin();
  bl = min_element(diffs.begin(), diffs.end()) - diffs.begin();
  br = max_element(sums.begin(), sums.end()) - sums.begin();

  sorted_bounds.push_back(doc_bounds[tl]);
  sorted_bounds.push_back(doc_bounds[tr]);
  sorted_bounds.push_back(doc_bounds[bl]);
  sorted_bounds.push_back(doc_bounds[br]);

  return sorted_bounds;
}

/**
 * @brief wrapDoc function to wrap document image
 * 
 * @param input input image
 * @param doc_bounds document bounds
 * @param width width of the document
 * @param height height of the document
 * @param crop crop the image
 * @return Mat wrapped document image
 */
Mat wrapDoc(Mat input, vector<Point> doc_bounds, float width=0, float height=0, bool crop=true)
{
  if (doc_bounds.size() != 4) {
    cout << "Invalid document bounds" << endl;
    return input;
  }

  // estimate width and height from bounds
  if (width <= 0 || height <= 0) {
    int max_width = max(doc_bounds[1].x - doc_bounds[0].x, doc_bounds[3].x - doc_bounds[2].x);
    int max_height = max(doc_bounds[2].y - doc_bounds[0].y, doc_bounds[3].y - doc_bounds[1].y);
    width = (float) max_width;
    height = (float) max_height;
  }

  Point doc_dim((int)width, (int)height);
  Mat matrix, img_warp;

  Point2f src_coord[4] = {doc_bounds[0], doc_bounds[1], doc_bounds[2], doc_bounds[3]};

  Point2f dest_coord[4] = {
    {0.0f, 0.0f},
    {width, 0.0f},
    {0.0f, height},
    {width, height}
  };
  
  // get transformation matrix and warp image
  matrix = getPerspectiveTransform(src_coord, dest_coord);
  warpPerspective(input, img_warp, matrix, doc_dim);

  // crop image
  if (crop) {
    int crop_amount = 10;
    Rect crop_region(crop_amount, crop_amount, (int) width-crop_amount*2, (int) height-crop_amount*2);
    img_warp = img_warp(crop_region);
  }

  return img_warp;
}

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


int main()
{
  bool from_camera = true;
  bool running = true;

  string window = "Doc Scanner";
  namedWindow(window, WINDOW_AUTOSIZE);

  Point mouse_click_pos;
  setMouseCallback(window, mouseCallback, &mouse_click_pos);

  Mat doc_original;
  Mat doc_scanned;
  vector<Point> doc_bounds;

  if (from_camera) {
    VideoCapture cap(1);
    while(1) {
      cap.read(doc_original);

      // wait for mouse click to capture image
      if ( mouse_click_pos.x > 0 && mouse_click_pos.y > 0 )
        break;
      
      if (doc_original.empty())
        continue;
      
      doc_bounds = getDocBounds(doc_original);
      imshow(window, doc_original);
      if (waitKey(10) == 'q') {
        running = false;
        break;
      }
    }
  
  } else {
    string path = "./Resources/paper.jpg";
    doc_original = imread(path);
  }

  if (doc_bounds == invalid_points) {
    cout << "Invalid document bounds" << endl;
    return -1;
  }

  doc_bounds = sortDocBounds(doc_bounds);
  doc_scanned = wrapDoc(doc_original, doc_bounds);

  if (debug) {
    for (int i = 0; i < doc_bounds.size(); i++) {
      putText(doc_original, to_string(i), doc_bounds[i], FONT_HERSHEY_PLAIN, 2, Scalar(255, 255, 0), 2);
    }
  }

  imshow(window, doc_scanned);
  waitKey(0);

  
  return 0;
}