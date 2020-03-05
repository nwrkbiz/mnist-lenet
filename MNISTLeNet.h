/**
 * @file MNISTLeNet.h
 * @brief Class to train a LeNet for MNIST and to predict digits found on a jpeg picture.
 * @author Daniel Giritzer
 * @copyright "THE BEER-WARE LICENSE" (Revision 42):
 * <giri@nwrk.biz> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Daniel Giritzer
 */
#ifndef MNISTLENET_H
#define MNISTLENET_H

#include <Object.h>
#include <filesystem>
#include <Blob.h>
#include <JSON.h>
#include <Exception.h>
#include <dlib/dnn.h>
#include <dlib/image_transforms.h>
#include <opencv2/imgproc/imgproc.hpp>

/**
 * @brief Exception to be thrown on errors within MNISTLeNet.
 */
class MNISTLeNetException final : public giri::ExceptionBase
{
public:
  MNISTLeNetException(const std::string &msg) : giri::ExceptionBase(msg) {}; 
  using SPtr = std::shared_ptr<MNISTLeNetException>;
  using UPtr = std::unique_ptr<MNISTLeNetException>;
  using WPtr = std::weak_ptr<MNISTLeNetException>;
};

/**
 * @brief Class to train a LeNet for MNIST and 
 * to predict digits found on a jpeg picture.
 */
class MNISTLeNet : public giri::Object<MNISTLeNet>
{
public:
    /**
     * @brief Will load a existing network if existent in path, will train a new newtork 
     * if no network exists. Network will be stored to path.
     * @param path Folder containing the mnist dataset and trained network if existent. (defaults to ./mnist)
     * @note The mnist dataset can be downloaded from: http://yann.lecun.com/exdb/mnist/
     */
    MNISTLeNet(const std::filesystem::path& path = "mnist");
    ~MNISTLeNet() = default;

    /**
     * @brief Will train a new newtork, result will be stored to the path set by the CTor.
     */
    void train();

    /**
     * @brief Find digits on jpeg and does prediction using the trained network.
     * @param b Blob containing jpeg with handwritten digits.
     * @returns JSON containing result with following structure:
     * {
     * "predictions" : [{ "label" : 0, "probability" : 0.9 }],
     * "result_picture" : "base-64-encoded-jpeg"
     * }
     */
    giri::json::JSON predict(const giri::Blob& b);


    // LeNet definition
    using LeNet = dlib::loss_multiclass_log<
                                dlib::fc<10,        
                                dlib::relu<dlib::fc<84,   
                                dlib::relu<dlib::fc<120,  
                                dlib::max_pool<2,2,2,2,dlib::relu<dlib::con<16,5,5,1,1,
                                dlib::max_pool<2,2,2,2,dlib::relu<dlib::con<6,5,5,1,1,
                                dlib::input<dlib::matrix<unsigned char>> 
                                >>>>>>>>>>>>;

private:

    /**
     *  @brief Automatic brightness and contrast optimization with optional histogram clipping.
     *  @param src [out] Input image GRAY or BGR or BGRA
     *  @param dst [in] Destination image 
     *  @param clipHistPercent cut wings of histogram at given percent tipical=>1, 0=>Disabled
     *  @note In case of BGRA image, we won't touch the transparency
     */
    void BrightnessAndContrastAuto(const cv::Mat &src, cv::Mat &dst, float clipHistPercent=0);

    /**
     * @brief Converts an dlib RGB image to jpeg
     * @param img dlib rgb image
     * @returns blob containig jpeg
     */
    giri::Blob to_jpeg( dlib::array2d<dlib::rgb_pixel>& img, 
                        size_t quality = 85, 
                        const std::string& comment = "Pic from giri's MNIST LeNet example."
                      );
    /**
     * @brief Converts an dlib grayscale image to jpeg
     * @param img dlib grayscale image
     * @returns blob containig jpeg
     */
    giri::Blob to_jpeg( dlib::array2d<unsigned char>& img, 
                        size_t quality = 85, 
                        const std::string& comment = "Pic from giri's MNIST LeNet example."
                      );

    /**
     * @brief Allows moving/recentering image
     * @param img Image to use
     * @param offsetx x offset
     * @param offsety y offset
     * @returns New moved image.
     */
    cv::Mat translateImg(cv::Mat &img, int offsetx, int offsety);

    std::filesystem::path m_DataSetPath;
    std::filesystem::path m_Images;
    std::filesystem::path m_Labels;
    std::filesystem::path m_TestImages;
    std::filesystem::path m_TestLabels;
    std::filesystem::path m_NetworkFile;
    std::filesystem::path m_SyncFile;
    MNISTLeNet::LeNet m_Net;

    // MNIST image size
    size_t m_ImgSize = 28;
    size_t m_DigitSize = 20;
};
#endif // MNISTLENET_H