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

using namespace std;
using namespace cv;

bool debug = false;

Mat preprocess(Mat input)
{
  Mat gray, blur, canny, dilated;
  Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));

  cvtColor(input, gray, COLOR_BGR2GRAY);      // rgb to grayscale
  GaussianBlur(gray, blur, Size(3, 3), 3);    // blur
  Canny(blur, canny, 25, 75);                 // edge detection
  dilate(canny, dilated, kernel);              // dilation

  return dilated;
}

vector<Point> getDocBounds(Mat input)
{
  vector<vector<Point>> contours;
  vector<Vec4i> heirarchy;
  vector<Point> doc_bounds;

  Mat processed = preprocess(input);
  findContours(processed, contours, heirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

  vector<vector<Point>> min_polygon(contours.size());   // Minimum bounding box poligon, used to predict shape
  
  for (int i = 0; i < contours.size(); i++) {
    // skip small contours
    double area         = contourArea(contours[i]);
    if (area < 1000)
      continue;

    // Find minimum polygon
    double perimeter    = arcLength(contours[i], true);
    approxPolyDP(contours[i], min_polygon[i], 0.02*perimeter, true);

    // skip non-quadrilaterals
    size_t vertex_count = min_polygon[i].size();
    if (vertex_count != 4) 
      continue;
    
    if (debug)
      drawContours(input, min_polygon, i, Scalar(0, 0, 255), 2);

    doc_bounds = min_polygon[i];
  }

  return doc_bounds;
}

vector<Point> sortDocBounds(vector<Point> doc_bounds)
{
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

Mat wrapDoc(Mat input, vector<Point> doc_bounds, float width=0, float height=0, bool crop=true)
{
  if (doc_bounds.size() != 4) {
    cout << "Invalid document bounds" << endl;
    return input;
  }

  // default to input size if width and height are not provided
  if (width <= 0 || height <= 0) {
    float scale = 0.6;
    width = (float) input.cols * scale;
    height = (float) input.rows * scale;;
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

int main()
{
  string path = "./Resources/paper.jpg";
  vector<Point> doc_bounds;
  Mat doc_original = imread(path);

  // resize(doc_original, doc_original, Size(), 0.5, 0.5);
  
  doc_bounds = getDocBounds(doc_original);
  doc_bounds = sortDocBounds(doc_bounds);

  if (debug) {
    for (int i = 0; i < doc_bounds.size(); i++) {
      putText(doc_original, to_string(i), doc_bounds[i], FONT_HERSHEY_PLAIN, 2, Scalar(255, 255, 0), 2);
    }
  }

  Mat doc_scanned = wrapDoc(doc_original, doc_bounds);
  // imshow("Image", doc_original);
  imshow("Scanned", doc_scanned);

  waitKey(0);
  
  return 0;
}