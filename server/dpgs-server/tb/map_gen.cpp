#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <limits.h>
using namespace std;
using namespace cv;

// Global storage for parking spot ROIs (each ROI is 4 points)
static vector<vector<Point>> spots;
static vector<Point> curr;

// Mouse callback to collect exactly 4 clicks per spot
void onMouse(int event, int x, int y, int, void*) {
    if (event != EVENT_LBUTTONDOWN) return;
    curr.emplace_back(x, y);
    if (curr.size() == 4) {
        spots.emplace_back(curr);
        cout << "Defined spot #" << spots.size()-1 << endl;
        curr.clear();
    }
}



// Save spots to YAML file
bool saveSpots(const string &filename) {
    FileStorage fs(filename, FileStorage::WRITE);
    if (!fs.isOpened()) return false;
    fs << "spots" << spots;
    fs.release();
    cout << "Saved " << spots.size() << " spots to " << filename << endl;
    return true;
}

// Draw existing spots and current selection points
void drawSpots(Mat &img) {
    // Draw completed ROIs
    for (size_t i = 0; i < spots.size(); ++i) {
        polylines(img, spots[i], true, Scalar(0,255,0), 2);
        Moments m = moments(spots[i]);
        if (m.m00 > 0) {
            int cx = int(m.m10/m.m00);
            int cy = int(m.m01/m.m00);
            putText(img, to_string((int)i), Point(cx, cy), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0,255,0), 1);
        }
    }
    // Draw current in-progress clicks
    for (auto &p : curr) {
        circle(img, p, 5, Scalar(0,0,255), -1);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " <video>" << endl;
        return 1;
    }

    string video_file = argv[1];
    VideoCapture capture(video_file);
    if (!capture.isOpened()) {
        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        cerr << "Error: Fail to open video source, " << cwd << "\n";
        return false;
    }

    Mat img;
    capture.read(img);
    if (img.empty()) {
        cerr << "Failed to load image\n";
        return 1;
    }


    const string win = "ROI Selector";
    namedWindow(win);
    setMouseCallback(win, onMouse);
    cout << "Click 4 corners per spot. Press 'q' to finish and save." << endl;

    while (true) {
        Mat disp = img.clone();
        drawSpots(disp);
        imshow(win, disp);
        char c = (char)waitKey(1);
        if (c == 'q') break;
    }
    destroyWindow(win);

    if (!spots.empty()) {
        saveSpots("spots.yml");
    } else {
        cout << "No spots defined, nothing saved." << endl;
    }
    return 0;
}
