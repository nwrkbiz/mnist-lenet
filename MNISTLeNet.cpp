/**
 * @file MNISTLeNet.cpp
 * @brief Class to train a LeNet for MNIST and to predict digits found on a jpeg picture.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#include "MNISTLeNet.h"
#include <iostream>
#include <dlib/data_io.h>
#include <dlib/image_io.h>
#include <dlib/gui_widgets.h>
#include <dlib/opencv/cv_image.h>
#include <dlib/opencv/to_open_cv.h>
#include <opencv2/imgproc/types_c.h>
#include <jpeglib.h>

using namespace giri;
using namespace std;
using namespace dlib;

MNISTLeNet::MNISTLeNet(const std::filesystem::path& path) : m_DataSetPath(path),  m_Images(path), m_Labels(path), m_TestImages(path), m_TestLabels(path), m_NetworkFile(path), m_SyncFile(path) {
    m_Images.append("t10k-images-idx3-ubyte");
    m_Labels.append("t10k-labels-idx1-ubyte");
    m_TestImages.append("train-images-idx3-ubyte");
    m_TestLabels.append("train-labels-idx1-ubyte");
    m_NetworkFile.append("mnist_network.dat");
    m_SyncFile.append("mnist_sync");
    if(!std::filesystem::exists(m_NetworkFile)){
        train();
    }
    else{
        deserialize(m_NetworkFile.string()) >> m_Net;
    }
}

void MNISTLeNet::train(){

    // check for mnist dataset
    if(!std::filesystem::exists(m_Images) || 
       !std::filesystem::exists(m_Labels) ||
       !std::filesystem::exists(m_TestImages) ||
       !std::filesystem::exists(m_TestLabels) 
    ){
        throw MNISTLeNetException(std::string("MNIST dataset not found within: ") + m_DataSetPath.string());
    }

    // delete existing network if existent
    if(std::filesystem::exists(m_NetworkFile)){
        std::filesystem::remove(m_NetworkFile);
    }

    try
    {
        std::vector<matrix<unsigned char>> training_images;
        std::vector<unsigned long>         training_labels;
        std::vector<matrix<unsigned char>> testing_images;
        std::vector<unsigned long>         testing_labels;
        // convinience function provided by dlib
        load_mnist_dataset(m_DataSetPath.string(), training_images, training_labels, testing_images, testing_labels);


        // train LeNet it using the MNIST data. The code below uses mini-batch stochastic
        // gradient descent with an initial learning rate of 0.01 to accomplish this.
        dnn_trainer<LeNet> trainer(m_Net);
        trainer.set_learning_rate(0.01);
        trainer.set_min_learning_rate(0.00001);
        trainer.set_mini_batch_size(128);
        trainer.be_verbose(); // print progress to std::cout

        // store snapshots every 20 seconds in case of programm interruption,
        // training can be resumed if snapshot is found.
        trainer.set_synchronization_file(m_SyncFile.string(), std::chrono::seconds(20));
        trainer.train(training_images, training_labels);

        // save trained network to disk
        m_Net.clean();
        serialize(m_NetworkFile.string()) << m_Net;
    }
    catch(std::exception& e)
    {
        throw MNISTLeNetException(e.what());
    }
}

// See: https://answers.opencv.org/question/75510/how-to-make-auto-adjustmentsbrightness-and-contrast-for-image-android-opencv-image-correction/?answer=75797#post-id-75797)
void MNISTLeNet::BrightnessAndContrastAuto(const cv::Mat &src, cv::Mat &dst, float clipHistPercent)
{

    CV_Assert(clipHistPercent >= 0);
    CV_Assert((src.type() == CV_8UC1) || (src.type() == CV_8UC3) || (src.type() == CV_8UC4));

    int histSize = 256;
    float alpha, beta;
    double minGray = 0, maxGray = 0;

    //to calculate grayscale histogram
    cv::Mat gray;
    if (src.type() == CV_8UC1) gray = src;
    else if (src.type() == CV_8UC3) cvtColor(src, gray, CV_BGR2GRAY);
    else if (src.type() == CV_8UC4) cvtColor(src, gray, CV_BGRA2GRAY);
    if (clipHistPercent == 0)
    {
        // keep full available range
        cv::minMaxLoc(gray, &minGray, &maxGray);
    }
    else
    {
        cv::Mat hist; //the grayscale histogram

        float range[] = { 0, 256 };
        const float* histRange = { range };
        bool uniform = true;
        bool accumulate = false;
        calcHist(&gray, 1, 0, cv::Mat (), hist, 1, &histSize, &histRange, uniform, accumulate);

        // calculate cumulative distribution from the histogram
        std::vector<float> accumulator(histSize);
        accumulator[0] = hist.at<float>(0);
        for (int i = 1; i < histSize; i++)
        {
            accumulator[i] = accumulator[i - 1] + hist.at<float>(i);
        }

        // locate points that cuts at required value
        float max = accumulator.back();
        clipHistPercent *= (max / 100.0); //make percent as absolute
        clipHistPercent /= 2.0; // left and right wings
        // locate left cut
        minGray = 0;
        while (accumulator[minGray] < clipHistPercent)
            minGray++;

        // locate right cut
        maxGray = histSize - 1;
        while (accumulator[maxGray] >= (max - clipHistPercent))
            maxGray--;
    }

    // current range
    float inputRange = maxGray - minGray;

    alpha = (histSize - 1) / inputRange;   // alpha expands current range to histsize range
    beta = -minGray * alpha;             // beta shifts current range so that minGray will go to 0

    // Apply brightness and contrast normalization
    // convertTo operates with saurate_cast
    src.convertTo(dst, -1, alpha, beta);

    // restore alpha channel from source 
    if (dst.type() == CV_8UC4)
    {
        int from_to[] = { 3, 3};
        cv::mixChannels(&src, 4, &dst,1, from_to, 1);
    }
    return;
}

Blob MNISTLeNet::to_jpeg(array2d<rgb_pixel>& img, size_t quality, const std::string& comment) {
    if(img.size() == 0)
        throw MNISTLeNetException("Cannot convert empty image.");
    if(quality <= 0 || quality >= 100)
        throw MNISTLeNetException("Invalid quality value.");
	Blob b(img.size());
    struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	cinfo.image_width = img.nc();
	cinfo.image_height = img.nr();
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
    long unsigned int siz = img.size();
    unsigned char* buf = (unsigned char*)b.data();
	jpeg_mem_dest(&cinfo, &buf, &siz);
	jpeg_start_compress(&cinfo, TRUE);
    jpeg_write_marker(&cinfo, JPEG_COM, (const JOCTET*)comment.c_str(), comment.size());
	while (cinfo.next_scanline < cinfo.image_height) {
		JSAMPROW row_pointer = (JSAMPROW)&img[cinfo.next_scanline][0];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
    return b;
}

Blob MNISTLeNet::to_jpeg(array2d<unsigned char>& img, size_t quality, const std::string& comment) {
    if(img.size() == 0)
        throw MNISTLeNetException("Cannot convert empty image.");
    if(quality <= 0 || quality >= 100)
        throw MNISTLeNetException("Invalid quality value.");
    Blob b(img.size());
    struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	cinfo.image_width = img.nc();
	cinfo.image_height = img.nr();
	cinfo.input_components = 1;
	cinfo.in_color_space = JCS_GRAYSCALE;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
    long unsigned int siz = img.size();
    unsigned char* buf = (unsigned char*)b.data();
	jpeg_mem_dest(&cinfo, &buf, &siz);
	jpeg_start_compress(&cinfo, TRUE);
    jpeg_write_marker(&cinfo, JPEG_COM, (const JOCTET*)comment.c_str(), comment.size());
	while (cinfo.next_scanline < cinfo.image_height) {
		JSAMPROW row_pointer = (JSAMPROW)&img[cinfo.next_scanline][0];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
    return b;
}

cv::Mat MNISTLeNet::translateImg(cv::Mat &img, int offsetx, int offsety){
    cv::Mat trans_mat = (cv::Mat_<double>(2,3) << 1, 0, offsetx, 0, 1, offsety);
    warpAffine(img,img,trans_mat,img.size());
    return img;
}

json::JSON MNISTLeNet::predict(const Blob& b){
    // load image as rgb from blob
    array2d<rgb_pixel> img;
    load_jpeg(img, (unsigned char*)b.data(), b.size());

    // load image as greyscale from blob
    array2d<unsigned char> img_gray;
    load_jpeg(img_gray, (unsigned char*)b.data(), b.size());

    // ----------------------------------
    // ------ opencv manipulations ------
    cv::Mat ocv_bin;
    cv::Mat ocv_auto;
    std::vector<std::vector<cv::Point>> cnt;
    std::vector<cv::Vec4i> hier;
    BrightnessAndContrastAuto(toMat(img_gray), ocv_auto, 1); // better contrast
    cv::threshold(ocv_auto, ocv_bin, 150, 255, cv::THRESH_BINARY_INV); // convert to inverted binary
    cv::findContours(ocv_bin, cnt, hier, cv::RETR_TREE, cv::CHAIN_APPROX_NONE); // find contours


    // create bounding rectangles around contours
    std::vector<cv::Rect> rcts;
    for( size_t i = 0; i < cnt.size(); i++ )
    {
        // skip if cnt is no parent (->3), 
        // only use uppermost hierachy, skips for instance two zeros within 8
        if(hier[i][3] != -1) continue;
        cv::Rect rct = cv::boundingRect(cnt[i]);
        // only use rectangles with at least 250 pixels and also filter way too big ones (caused by shadows etc.)
        if(rct.area() > 250 && rct.width < ocv_bin.cols * 0.80 && rct.height < ocv_bin.rows * 0.80)
            rcts.push_back(rct);
    }


    // sort rectangles from top left to bottm right
    std::sort(rcts.begin(), rcts.end(), [](const cv::Rect& l, const cv::Rect& r){
        if(l.y == r.y) return l.x > r.x; return (l.y < r.y);
    });

    // create subimages that match the mnist examples
    std::vector<matrix<unsigned char>> digits;
    for(auto const& curRct : rcts) {
        // create cropped squared subimage
        size_t n = std::max(curRct.width, curRct.height);
        cv::Mat cro = ocv_bin(curRct);
        cv::Mat squ = cv::Mat::zeros(n, n, ocv_bin.type());
        cro.copyTo(squ(cv::Rect((n - curRct.width) / 2, (n - curRct.height) / 2, cro.cols, cro.rows)));

        // downscale squared image
        cv::Mat squ_small;
        cv::resize(squ, squ_small, cv::Size(m_DigitSize, m_DigitSize));

        // add border
        cv::Mat squ_border = cv::Mat::zeros(m_ImgSize, m_ImgSize, ocv_bin.type());
        squ_small.copyTo(squ_border(cv::Rect(m_ImgSize - m_DigitSize, m_ImgSize - m_DigitSize , squ_small.cols, squ_small.rows)));

        // move to center of mass
        cv::Moments mu = cv::moments(squ_border, true);
        cv::Point com; com.x = mu.m10 / mu.m00; com.y = mu.m01 / mu.m00;
        cv::Mat fin = translateImg(squ_border, m_ImgSize / 2 - com.x, m_ImgSize / 2 - com.y);

        // store resulting image as dlib matrix
        matrix<unsigned char> dlibmtrx;
        assign_image(dlibmtrx, cv_image<unsigned char>(cvIplImage(fin)));
        digits.push_back(dlibmtrx);
    }
    // ------ end opencv manipulations ------
    // --------------------------------------

    // do prediction
    json::JSON retVal;
    retVal["predictions"] = json::Array();

    // use softmax layer to access label probability
    softmax<LeNet::subnet_type> sNet;
    sNet.subnet() = m_Net.subnet();
    for(auto const & curDigit : digits) {
        matrix<float, 1, 10> p = mat(sNet(curDigit));
        unsigned long highest = index_of_max(p);
        json::JSON pred;
        pred["label"] = highest;
        pred["probability"] = p(highest);
        retVal["predictions"].append(pred);
    }

    // draw green rectangles into original picture
    for(auto const& curRct : rcts)
        cv::rectangle(toMat(img), curRct, cv::Scalar(0, 255, 0), 2);
    retVal["result_picture"] = to_jpeg(img).toBase64();
    return retVal;
}

