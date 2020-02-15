#include <iostream>
#include <fstream>
#include <vector>
#include <math.h> 
#include <stdlib.h>
#include <algorithm>   

#include <opencv4/opencv2/imgproc/imgproc.hpp>
#include <opencv4/opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

using namespace std;
using namespace cv;

Point generate_aspect(int width, int height, int dens);
vector<Point> point_generator(int width, int height, Point ar);
void draw_triangles(Mat& img, vector<vector<Point> > D_mat, int width, int height, Point ar);
void delauney(vector<Point> D_mat, int width, int height, Point ar, Mat& img);
void basic(int, void*);

int slider = 0;
const int slider_max = 97;
Mat img;
int lines = 1;

int main() {
    char filename[50];

    do
    {
        cout << "Filename: ";
        cin >> filename;
        img = imread(filename, 1);
    } while (! img.data);
    
    namedWindow("Delauney", WINDOW_NORMAL);
    namedWindow("Slider", WINDOW_NORMAL);
    resizeWindow("Delauney", 1920, 1080);
    createTrackbar("Density", "Slider", &slider, slider_max, basic);

    waitKey(0);    
    img.release();

    return 0;
}

void basic(int, void*) {
    int im_width = img.size[1];
    int im_height = img.size[0];

    Point ar = generate_aspect(im_width, im_height, slider);
    vector<Point> D_mat = point_generator(im_width, im_height, ar);

    Mat border;
    copyMakeBorder(img, border, ar.y * 2, ar.y * 2, ar.x * 2, ar.x * 2, BORDER_CONSTANT, Scalar(255,255,255));

    delauney(D_mat, im_width, im_height, ar, border);
}

Point generate_aspect(int width, int height, int dens) {
    int adj_dens = abs(dens - 100);

    int x_size = ceil(width / 1000.0) * adj_dens;
    int y_size = ceil(height / 1000.0) * adj_dens;

    return Point(x_size, y_size);
}

vector<Point> point_generator(int width, int height, Point ar) {
    int rnd_w;
    int rnd_h;
    int n = 0;

    int x_len = floor(width / (double)ar.x) + 4;
    int y_len = floor(height / (double)ar.y) + 4;

    vector<Point> D_mat(y_len * x_len);

    srand(time(0));

    for (int i = 0; i < y_len; i++) {
        for (int j = 0; j < x_len; j++) {
            rnd_w = (rand() % ((j * ar.x + ar.x) - j * ar.x + 1)) + j * ar.x;
            rnd_h = (rand() % ((i * ar.y + ar.y) - i * ar.y + 1)) + i * ar.y;

            D_mat[n] = (Point(rnd_w, rnd_h));
            n++;
        }
    }

    return D_mat;
}

void delauney(vector<Point> D_mat, int width, int height, Point ar, Mat& img) {
    int x_size = floor(width / (double)ar.x) + 4;
    int y_size = floor(height / (double)ar.y) + 4;

    Rect rect(0, 0, width + 5 * ar.x, height + 5 * ar.y);
    Subdiv2D subdiv(rect);
    int sub_size = x_size * y_size;

    for (int i = 0; i < sub_size; i++) {
        subdiv.insert(D_mat[i]);
    }

    vector<Vec6f> triangleList;
    subdiv.getTriangleList(triangleList);  
    int size = triangleList.size();

    int pt0, pt1, pt2, pt3, pt4, pt5;
    Rect roi(2 * ar.x, 2 * ar.y, width, height);

    for (int i = 0; i < size; i++) {
        Vec6f t = triangleList[i];
        pt0 = round(t[0]); pt1 = round(t[1]); pt2 = round(t[2]); 
        pt3 = round(t[3]); pt4 = round(t[4]); pt5 = round(t[5]);
        int red = 0, green = 0, blue = 0, n = 1;

        auto xrange = minmax({pt0, pt2, pt4});
        auto yrange = minmax({pt1, pt3, pt5});
        int maxx = xrange.second;
        int maxy = yrange.second;
        int minx = xrange.first;
        int miny = yrange.first;

        for (int y = miny; y < maxy; y++) {
            for (int x = minx; x < maxx; x++) {     
                if (roi.contains(Point(x,y))){
                    Vec3b intensity = img.at<Vec3b>(y, x);
                    red += (int) intensity[2];
                    green += (int) intensity[1];
                    blue += (int) intensity[0];
                    n += 1;
                } 
            }  
        }

        Point pts[3] = {Point(pt0, pt1), Point(pt2, pt3), Point(pt4, pt5)};
        fillConvexPoly(img, pts, 3, Scalar(floor(blue / n), floor(green / n), floor(red / n)));

        if (lines) {            
            Point points[3];
            points[0] = Point(cvRound(t[0]), cvRound(t[1]));
            points[1] = Point(cvRound(t[2]), cvRound(t[3]));
            points[2] = Point(cvRound(t[4]), cvRound(t[5]));

            Scalar delaunay_color(0, 0, 0);

            line(img, pts[0], pts[1], delaunay_color, 1, 0);
            line(img, pts[1], pts[2], delaunay_color, 1, 0);
            line(img, pts[2], pts[0], delaunay_color, 1, 0);
        }    
        
    }
    
    Mat roi_img = img(roi);

    imshow("Delauney", roi_img);
}






