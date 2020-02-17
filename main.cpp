#include <iostream>
#include <vector> 
#include <algorithm>   

#include <opencv4/opencv2/imgproc/imgproc.hpp>
#include <opencv4/opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>

#include <math.h>
#include <omp.h>

using namespace std;
using namespace cv;

Point generate_aspect(int dens);
vector<Point> point_generator(Point ar);
void delauney(vector<Point> D_mat, Point ar, Mat& img);
void basic(int, void*);

int slider = 0;
const int slider_max = 99;
const int lines = 0;

Mat img;
Mat padded;
Mat roi_img; 

Scalar black(0, 0, 0);
int width;
int height;

int main() {
    char filename[50];

    do
    {
        cout << "Filename: ";
        cin >> filename;
        img = imread(filename, 1);
    } while (! img.data);

    width = img.size[1];
    height = img.size[0];
    
    srand(time(0));

    // create windows for slider and output
    namedWindow("Delauney", WINDOW_NORMAL);
    namedWindow("Slider", WINDOW_AUTOSIZE);
    resizeWindow("Delauney", 1920, 1080);
    createTrackbar("Density:", "Slider", &slider, slider_max, basic);
    imshow("Slider", Mat(1, 400, 1, black));

    waitKey(0);    
    img.release();

    return 0;
}

// main loop for slider
void basic(int, void*) {
    Point ar = generate_aspect(slider);
    vector<Point> D_mat = point_generator(ar);

    copyMakeBorder(img, padded, ar.y * 2, ar.y * 2, ar.x * 2, ar.x * 2, BORDER_CONSTANT, black);
    delauney(D_mat, ar, padded);
}
// calculates cell size
Point generate_aspect(int dens) {
    int x_size = ceil(width / 1000.0) * abs(dens - 100);
    int y_size = ceil(height / 1000.0) * abs(dens - 100);

    return Point(x_size, y_size);
}

// generates points for delauney triangulation
vector<Point> point_generator(Point ar) {
    int rnd_w;
    int rnd_h;
    int n = 0;

    // number of cell in x, y direction + border
    int x_len = (int) (width / ar.x) + 4;
    int y_len = (int) (height / ar.y) + 4;

    vector<Point> D_mat(y_len * x_len);

    // pick random point in each cell
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

// computes delauney, finds avg color of each triangle and draws the triangle
void delauney(vector<Point> D_mat, Point ar, Mat& img) {
    Rect rect(0, 0, width + 5 * ar.x, height + 5 * ar.y);
    Subdiv2D subdiv(rect);
    int d_size = D_mat.size();

    // compute delauney triangulation
    for (int i = 0; i < d_size; i++) {
        subdiv.insert(D_mat[i]);
    }

    vector<Vec6f> triangleList;
    subdiv.getTriangleList(triangleList);  
    int size = triangleList.size();

    int pt0, pt1, pt2, pt3, pt4, pt5;
    // img without borders
    Rect roi(2 * ar.x, 2 * ar.y, width, height);

    #pragma omp parallel for private(pt0, pt1, pt2, pt3, pt4, pt5) schedule(auto)
    for (int i = 0; i < size; i++) {
        Vec6f t = triangleList[i];
        pt0 = (int) t[0]; pt1 = (int) t[1]; pt2 = (int) t[2]; 
        pt3 = (int) t[3]; pt4 = (int) t[4]; pt5 = (int) t[5];
        float r = 0, g = 0, b = 0;
        int n = 1;

        // compute bounding box around triangle
        auto xrange = minmax({pt0, pt2, pt4});
        auto yrange = minmax({pt1, pt3, pt5});
        int minx = xrange.first;
        int miny = yrange.first;
        int maxx = xrange.second;
        int maxy = yrange.second;

        // compute average color of bounding box
        for (int y = miny; y < maxy; y++) {
            for (int x = minx; x < maxx; x++) {     
                if (roi.contains(Point(x,y))){
                    Vec3b intensity = img.at<Vec3b>(y, x);
                    r += intensity[2];
                    g += intensity[1];
                    b += intensity[0];
                    n += 1;
                } 
            }  
        }

        Point pts[3] = {Point(pt0, pt1), Point(pt2, pt3), Point(pt4, pt5)};
        fillConvexPoly(img, pts, 3, Scalar((int) (b / n), (int) (g / n), (int) (r / n)));
        
        // draws vertices of triangle
        if (lines) {            
            line(img, pts[0], pts[1], black, 1, 0);
            line(img, pts[1], pts[2], black, 1, 0);
            line(img, pts[2], pts[0], black, 1, 0);
        }       
    }
    
    // crop image to without borders
    imshow("Delauney", img(roi));
}