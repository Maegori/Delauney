#include <iostream>
#include <fstream>
#include <vector>
#include <math.h> 
#include <stdlib.h>
#include <algorithm>   

#include <opencv4/opencv2/imgproc/imgproc.hpp>
#include <opencv4/opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv4/opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/types.hpp>

using namespace std;
using namespace cv;

Point generate_aspect(int width, int height, int dens);
vector<vector<Point> > point_generator(int width, int height, Point ar);
void draw_triangles(Mat& img, vector<vector<Point> > D_mat, int width, int height, Point ar);
void delauney(vector<vector<Point> > D_mat, int width, int height, Point ar, Mat& img);
void basic(int, void*);

int slider;
const int slider_max = 97;
Mat img;
int lines = 1;

int main()
{
    char filename[50];

    do
    {
        cout << "Filename: ";
        cin >> filename;
        img = imread(filename, 1);
    } while (! img.data);

    //img = imread("mona.jpg", 1); 

    slider = 0;
    
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
    vector<vector<Point> > D_mat = point_generator(im_width, im_height, ar);

    // create white image and draw the triangles on it
    //Mat triangles(im_height + 4 * ar.y, im_width + 4 * ar.x, CV_8UC3, Scalar(255, 255, 255));
    Mat border;
    copyMakeBorder(img, border, ar.y * 2, ar.y * 2, ar.x * 2, ar.x * 2, BORDER_CONSTANT, Scalar(255,255,255));
    //draw_triangles(triangles, D_mat, im_width, im_height, ar);
    //triangles.release();

    delauney(D_mat, im_width, im_height, ar, border);
}

Point generate_aspect(int width, int height, int dens)
{
    int adj_dens = abs(dens - 100);

    int x_size = ceil(width / 1000.0) * adj_dens;
    int y_size = ceil(height / 1000.0) * adj_dens;

    return Point(x_size, y_size);
}

vector<vector<Point> > point_generator(int width, int height, Point ar)
{
    int x_len = floor(width / (double)ar.x) + 4;
    int y_len = floor(height / (double)ar.y) + 4;

    vector<vector<Point> > D_mat((y_len), vector<Point> (x_len));

    srand(time(0));

    for (int i = 0; i < y_len; i++)
    {
        for (int j = 0; j < x_len; j++)
        {
            // int num = (rand() % (upper - lower + 1)) + lower

            int rnd_w = (rand() % ((j * ar.x + ar.x) - j * ar.x + 1)) + j * ar.x;
            int rnd_h = (rand() % ((i * ar.y + ar.y) - i * ar.y + 1)) + i * ar.y;

            D_mat[i][j] = Point(rnd_w, rnd_h);
        }
    }

    return D_mat;
}

void delauney(vector<vector<Point> > D_mat, int width, int height, Point ar, Mat& img)
{
    int x_size = D_mat[0].size();
    int y_size = D_mat.size();

    vector<Point> d_points;
    Rect rect(0, 0, width + 5 * ar.x, height + 5 * ar.y);
    Subdiv2D subdiv(rect);

    for (int i = 0; i < y_size; i++) {
        for (int j = 0; j < x_size; j++) {
            subdiv.insert(D_mat[i][j]);
            d_points.push_back(D_mat[i][j]);
        }
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
                Vec3b intensity = img.at<Vec3b>(y, x);
                if (roi.contains(Point(x,y))){
                    red += (int) intensity[2];
                    green += (int) intensity[1];
                    blue += (int) intensity[0];
                    n += 1;
                } 
            }  
        }

        Point pts[6] = {Point(pt0, pt1), Point(pt2, pt3), Point(pt4, pt5)};
        fillConvexPoly(img, pts, 3, Scalar(floor(blue / n), floor(green / n), floor(red / n),  255));

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
    //for (int i = 0; i < size; i++) {
        //cout << "triangle #:  " << i <<" r g b n" << (int) triangles[i].red << triangles[i].green << triangles[i].blue << triangles[i].n << endl; 
    //}

}

void draw_triangles(Mat& img, vector<vector<Point> > D_mat, int width, int height, Point ar)
{
    int x_size = D_mat[0].size();
    int y_size = D_mat.size();

    vector<Point> d_points;
    Rect rect(0, 0, width + 4 * ar.x, height + 4 * ar.y);
    Subdiv2D subdiv(rect);

    for (int i = 0; i < y_size; i++) {
        for (int j = 0; j < x_size; j++) {
            subdiv.insert(D_mat[i][j]);
            d_points.push_back(D_mat[i][j]);
        }
    }

    vector<Vec6f> triangleList;
    subdiv.getTriangleList(triangleList);
    vector<Point> pt(3);
    int size = triangleList.size();

    Scalar delaunay_color(0, 0, 0);

    for (int i = 0; i < size; i++) {
        Vec6f t = triangleList[i];
        pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
        pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
        pt[2] = Point(cvRound(t[4]), cvRound(t[5]));

        if ( rect.contains(pt[0]) && rect.contains(pt[1]) && rect.contains(pt[2])) {
            line(img, pt[0], pt[1], delaunay_color, 1, 0);
            line(img, pt[1], pt[2], delaunay_color, 1, 0);
            line(img, pt[2], pt[0], delaunay_color, 1, 0);
        }
    }

    Rect roi(ar.x, ar.y, width, height);
    Mat roi_img = img(roi);
    cout << img.size() << endl;
    cout << roi_img.size() << endl;

    namedWindow( "Delauney mesh", WINDOW_NORMAL);
    imshow( "Delauney mesh", img);

    namedWindow( "Normalised delauney mesh", WINDOW_NORMAL);
    imshow( "Normalised delauney mesh", roi_img);
  
}






