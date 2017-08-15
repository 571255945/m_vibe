#ifndef ViBe_M_H
#define ViBe_M_H

#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv/cv.h>
#include<cv.h>
#include<core.hpp>
#include<opencv/cxcore.h>

using namespace std;
using namespace cv;

#define uchar unsigned char
class ViBe_M
{
public:
	ViBe_M(){};
	~ViBe_M();

	struct Config{
		//struct中的元素默认是public
		//static Config getRGBConfig(void){
		//	//return Config("[rgb]", numOfSample, radius, minMatchCount, foregroundCount);
		//	return Config("[rgb]", 8, 40, 2, 100);
		//}
		static Config getGrayConfig(void){
			//return Config("[gray]", numOfSample, radius, minMatchCount, foregroundCount);
			return Config("[gray]", 8, 40, 2, 100);
		}
		Config(const string& label_, int numOfSamle_, int radius_, int minMatchCount_, int foregroundCount_) :
			label(label_),
			numOfSample(numOfSamle_),
			radius(radius_),
			minMatchCount(minMatchCount_),
			foregroundCount(foregroundCount_)
		{}
		string label;
		int numOfSample;
		int radius;
		int minMatchCount;
		int foregroundCount;
	};
	void initParams2(const cv::Mat &frame, const ViBe_M::Config &config);
	void update2(const cv::Mat &src, cv::Mat &foreImage);
	void InitSampleModelData2(const cv::Mat &image);
	void apply2(const cv::Mat &src,cv::Mat &dst);

	//void update(const cv::Mat &frame, cv::Mat &foregroundImage);
	void initParams(const IplImage *pFrame, const ViBe_M::Config &config);
	void InitSampleModelData(const uchar *pImageData);
	//void update(const IplImage *pFrame, IplImage *dst);
	void update(const uchar *currentImage, uchar *foregroundImage);
	void apply(const cv::Mat &src, cv::Mat &dst);
	IplImage* apply(IplImage *src);
	void applyTest();

public:
	size_t imageWidth;
	size_t imageHeight;
	size_t imageType;
	size_t imageChannels;

	size_t numOfsample;
	size_t radius;
	size_t minMatchCount;
	size_t foregroundCount;

	size_t oneSampleSize;
	size_t lastModelSampleIndex;
	uchar *pViBeSampleModel=0;
	IplImage *pGrayImage = nullptr;
	//IplImage *pRGBImage = nullptr;
	//IplImage *pRGBForeImage = nullptr;
	IplImage *pGrayForeImage = nullptr;
	CvCapture *pCapture = nullptr;  
	IplImage *pFrame = nullptr;
	int nFrameNum = 0;
	double timeRecord = 0.0;

	cv::Mat sample;
	uchar *ptrSample=nullptr;

	cv::Mat  grayImage;
	//uchar *ptrGrayImage = nullptr;

	cv::Mat foreImage;
	//uchar *ptrForeImage = nullptr;
};

#endif