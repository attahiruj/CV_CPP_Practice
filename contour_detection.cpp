#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

void detectShapes(Mat input, Mat output)
{
  vector<vector<Point>> contours;
  vector<Vec4i> heirarchy;

  findContours(input, contours, heirarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

  vector<vector<Point>> min_polygon(contours.size());   // Minimum bounding box poligon, used to predict shape
  vector<Rect> bounding_rect(contours.size());
  
  string shape_type;
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

    // infer shape type
    size_t vertex_count = min_polygon[i].size();

    if (vertex_count == 3) { shape_type = "triangle"; }
    if (vertex_count == 4) {
      float aspect_ratio = (float) bounding_rect[i].width / (float) bounding_rect[i].height;
      if (aspect_ratio > 0.9f && aspect_ratio < 1.1f)
        shape_type = "square"; 
      else
        shape_type = "rectangle"; 
    }
    if (vertex_count > 4) { shape_type = "circle"; }

    // drawContours(output, contours, i, Scalar(255, 0, 255), 2);
    drawContours(output, min_polygon, i, Scalar(0, 0, 255), 2);
    rectangle(output, bounding_rect[i], Scalar(255, 0, 255), 1);
    putText(
      output, 
      shape_type, 
      { bounding_rect[i].x, bounding_rect[i].y - 2 }, 
      FONT_HERSHEY_PLAIN, 
      1, 
      Scalar(255, 0, 0),
      1
    );
  }
}

int main()
{
  string path = "./Resources/shapes.png";
  Mat img = imread(path);
  Mat gray, blur, canny, dilated;
  Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));

  // Image processing
  cvtColor(img, gray, COLOR_BGR2GRAY);        // rgb to grayscale
  GaussianBlur(gray, blur, Size(3, 3), 3);    // blur
  Canny(blur, canny, 25, 75);                 // edge detection
  dilate(canny, dilated, kernel);             // dilatio

  detectShapes(dilated, img);
  imshow("Image", img);
  // imshow("Image Dilate", dilated);

  waitKey(0);

  return 0;
}