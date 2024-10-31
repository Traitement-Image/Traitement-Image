#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

vector<Mat> images;
vector<string> imagePaths = {
    "DVI1/gi05.jpeg",
    "DVI1/gi02.jpeg"
};
Mat display, result;

// Global variables to store selected points
vector<Point2f> points1, points2;

// Function to update the display with reordered images
void updateDisplay() {
    hconcat(images, display);
    imshow("Image Order", display);
}

// Function to swap images for reordering
void swapImages(int idx1, int idx2) {
    if (idx1 >= 0 && idx1 < images.size() && idx2 >= 0 && idx2 < images.size()) {
        Mat temp = images[idx1];
        images[idx1] = images[idx2];
        images[idx2] = temp;
    }
}

// Mouse callback function for reordering images
void onMouseReorder(int event, int x, int y, int, void* userdata) {
    static int firstImageIdx = -1;


    if (event == EVENT_LBUTTONDOWN) {
        int imageWidth = display.cols / images.size();
        int clickedImageIdx = x / imageWidth;

        if (firstImageIdx == -1) {
            firstImageIdx = clickedImageIdx;
            cout << "Selected image " << firstImageIdx + 1 << " for swapping" << endl;
        } else {
            cout << "Swapping image " << firstImageIdx + 1 << " with image " << clickedImageIdx + 1 << endl;
            swapImages(firstImageIdx, clickedImageIdx);
            firstImageIdx = -1;
            updateDisplay();
        }
    }
}

// Mouse callback function for point selection
void onMouseSelectPoints(int event, int x, int y, int, void* userdata) {
    if (event == EVENT_LBUTTONDOWN) {
        if (points1.size() < 4) {
            points1.push_back(Point2f(x, y));
            cout << "Point for base image: (" << x << ", " << y << ")" << endl;
        } else if (points2.size() < 4) {
            points2.push_back(Point2f(x, y));
            cout << "Point for current image: (" << x << ", " << y << ")" << endl;
        }
    }
}

int main() {
    // Load images
    for (const auto& path : imagePaths) {
        Mat img = imread(path);
        if (img.empty()) {
            cout << "Could not open image: " << path << endl;
            return -1;
        }
        images.push_back(img);
    }

    // Initial display of images for reordering
    if (images.size() > 1) {
        hconcat(images, display);
    } else {
        display = images[0].clone();
    }

    namedWindow("Image Order", WINDOW_NORMAL);
    imshow("Image Order", display);
    setMouseCallback("Image Order", onMouseReorder, nullptr);

    cout << "Click on two images sequentially to swap their positions." << endl;
    waitKey(0);

    destroyWindow("Image Order");

    // Start assembly with the first image as the base result
    result = images[0].clone();

    // Process each pair of images iteratively
    for (int i = 1; i < images.size(); i++) {
        points1.clear();
        points2.clear();
        Mat img2 = images[i];

        // Select corresponding points between the current result and the next image
        cout << "Select 4 matching points on the base image and the current image." << endl;

        namedWindow("Base Image (current result)", WINDOW_AUTOSIZE);
        imshow("Base Image (current result)", result);
        setMouseCallback("Base Image (current result)", onMouseSelectPoints, nullptr);

        // Wait until 4 points are selected
        while (points1.size() < 4) {
            waitKey(1);
        }

        namedWindow("Current Image to Align", WINDOW_AUTOSIZE);
        imshow("Current Image to Align", img2);
        setMouseCallback("Current Image to Align", onMouseSelectPoints, nullptr);

        // Wait until 4 points are selected
        while (points2.size() < 4) {
            waitKey(1);
        }

        // Remove mouse callbacks
        setMouseCallback("Base Image (current result)", nullptr, nullptr);
        setMouseCallback("Current Image to Align", nullptr, nullptr);

        // Compute homography and warp the current image to align with the result
        Mat H = findHomography(points2, points1, RANSAC);
        Mat warped;
        warpPerspective(img2, warped, H, Size(result.cols + img2.cols, result.rows));

        // Combine the current result and warped image
        Mat temp(Size(result.cols + warped.cols, result.rows), result.type());
        result.copyTo(temp(Rect(0, 0, result.cols, result.rows)));
        warped.copyTo(temp(Rect(result.cols, 0, warped.cols, warped.rows)));

        result = temp.clone();

        // Show intermediate assembly result
        imshow("Assembled Image", result);
        waitKey(0);

        destroyWindow("Base Image (current result)");
        destroyWindow("Current Image to Align");
    }

    // Final display of the assembled image
    namedWindow("Final Assembled Image", WINDOW_NORMAL);
    imshow("Final Assembled Image", result);
    waitKey(0);
    destroyAllWindows();

    return 0;
}