#include <iostream>
#include <fstream>
#include <vector>
#include <math.h> 
#include <stdlib.h>
#include <unordered_map>

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
void delauney(vector<vector<Point> > D_mat, int width, int height, Point ar);

int main()
{
    int dens = 50;
    //char filename[50];
    Mat image;

    /*
    do
    {
        cout << "Filename: ";
        cin >> filename;
        image = imread(filename, 1);
    } while (! image.data);
    */

    image = imread("hubble.jpg", 1);
    //namedWindow( "Display window", WINDOW_NORMAL); 
    //imshow( "Display window", image );            

    cout << image.size << "\n";

    int im_width = image.size[1];
    int im_height = image.size[0];

    Point ar = generate_aspect(im_width, im_height, dens);

    cout << "x_size: "<< ar.x << ", y_size: " << ar.y << endl;

    vector<vector<Point> > D_mat = point_generator(im_width, im_height, ar);

    cout << "D_mat x: " << D_mat[0].size() << ", D_mat y: " << D_mat.size() << endl;

    // create white image and draw the triangles on it
    //Mat triangles(im_height + 3 * ar.y, im_width + 3 * ar.x, CV_8UC3, Scalar(255, 255, 255));
    //draw_triangles(triangles, D_mat, im_width, im_height, ar);
    //triangles.release();

    delauney(D_mat, im_width, im_height, ar);
    
    waitKey(0);    
    image.release();

    return 0;
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
    int x_len = floor(width / (double)ar.x) + 3;
    int y_len = floor(height / (double)ar.y) + 3;

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

void delauney(vector<vector<Point> > D_mat, int width, int height, Point ar)
{

    int x_size = D_mat[0].size();
    int y_size = D_mat.size();

    vector<Point> d_points;
    Rect rect(0, 0, width + 3 * ar.x + 1, height + 3 * ar.y + 1);
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

    cout << "# triangles: " << size << endl;

    cout << triangleList[0] << endl;
    cout << typeid(triangleList[0][0]).name() << endl;
    vector<Point> pt(3);
    unordered_map<Point, vector<Vec6f>> d_map;

    for (int i = 0; i < size; i++) {
        Vec6f t = triangleList[i];
        pt[0] = Point(cvRound(t[0]), cvRound(t[1]));
        pt[1] = Point(cvRound(t[2]), cvRound(t[3]));
        pt[2] = Point(cvRound(t[4]), cvRound(t[5]));

        if (d_map.find(pt[0]) == d_map.end()) {
            vector<Vec6f> vect(6, t);
            d_map.insert( {pt[0], vect});
        }
        else {
            vector<Vec6f> vect = d_map[pt[0]];
            vect.push_back(t);
            d_map.insert( {pt[0], vect});
        }

        if (d_map.find(pt[1]) == d_map.end()) {
            vector<Vec6f> vect(6, t);
            d_map.insert( {pt[1], vect});
        }
        else {
            vector<Vec6f> vect = d_map[pt[0]];
            vect.push_back(t);
            d_map.insert( {pt[1], vect});
        }

        if (d_map.find(pt[2]) == d_map.end()) {
            vector<Vec6f> vect(6, t);
            d_map.insert( {pt[2], vect});
        }
        else {
            vector<Vec6f> vect = d_map[pt[0]];
            vect.push_back(t);
            d_map.insert( {pt[2], vect});
        }
    }

}

void draw_triangles(Mat& img, vector<vector<Point> > D_mat, int width, int height, Point ar)
{
    int x_size = D_mat[0].size();
    int y_size = D_mat.size();

    vector<Point> d_points;
    Rect rect(0, 0, width + 3 * ar.x + 1, height + 3 * ar.y + 1);
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






