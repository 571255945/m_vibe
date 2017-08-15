#include"ViBe_M.h"

/*
1. cv::Mat -> IplImage
cv::Mat matimg = cv::imread ("heels.jpg");
IplImage* iplimg;
*iplimg = IplImage(matimg);
2. IplImage -> cv::Mat
IplImage* iplimg = cvLoadImage("heels.jpg");
cv::Mat matimg;
matimg = cv::Mat(iplimg);
*/

const static int mask = 65535;
static int isModelInit = 0;
ViBe_M::~ViBe_M(){
	if (ptrSample != nullptr){

	}
	if (pViBeSampleModel != nullptr){
		free(pViBeSampleModel);
		pViBeSampleModel = nullptr;
	}
	if (pCapture != nullptr){
		cvReleaseCapture(&pCapture);
		pCapture = nullptr;
	}	
	if (pGrayImage != nullptr){
		cvReleaseImage(&pGrayImage);
		pGrayImage = nullptr;
	}	
	if (pFrame != nullptr){
		cvReleaseImage(&pFrame);
		pFrame = nullptr;
	}	
	if (pGrayForeImage != nullptr){
		cvReleaseImage(&pGrayForeImage);
		pGrayForeImage = nullptr;
	}
}
void ViBe_M::initParams2(const cv::Mat &frame,const Config& config){
	//cv::cvtColor(frame, frame, COLOR_RGB2GRAY);
	this->imageChannels = frame.channels();
	this->imageType = frame.type();
	this->imageHeight = frame.rows;
	this->imageWidth = frame.cols;
	this->oneSampleSize = imageWidth*imageHeight;
	//this->oneSampleSize = imageChannels*imageHeight*imageWidth;
	

	this->numOfsample = config.numOfSample;
	this->minMatchCount = config.minMatchCount;
	this->foregroundCount = config.foregroundCount;
	this->radius = config.radius;

	this->lastModelSampleIndex = imageHeight*imageWidth*numOfsample;

	sample = Mat::zeros(imageWidth*imageHeight*(numOfsample+1), 1, CV_8UC1);
	ptrSample = sample.data;
	foreImage.create(imageHeight, imageWidth, CV_8UC1);
}
void ViBe_M::InitSampleModelData2(const cv::Mat &image){
	for (size_t i = 0; i < numOfsample; i++){
		memcpy(ptrSample + oneSampleSize*i, image.data, oneSampleSize);
	}
}
void ViBe_M::update2(const cv::Mat &src, cv::Mat &foreImage){
	size_t nOffset = 0;//ƫ����
	size_t nDistance = 0;//��ǰ֡�뱳��ģ�͵Ĳ�ֵ
	size_t nMatchCount = 0;

	static uchar CountNum = 0;

	uchar* pCurrentImage = src.data;
	uchar* pForegoundImage = foreImage.data;
	CountNum++;
	if (CountNum > 127){
		CountNum = 0;
	}
	//1.��ʼ������ģ�ͣ�û�в���8�������20���������ǲ����˽�������ͬһ֡��8��copy���൱��ÿһ�����ظ�����8����Ϊ�䱳��������
	if (!isModelInit){
		InitSampleModelData2(src);
		isModelInit = 1;
		return;
	}
	//2.ģ������� 0-7
	//int  t = (getTickCount()&mask) % numOfsample;//����������������˶�ǰ�������һ�ѵ����㣬�Ǹ���̫�������
	nOffset = (CountNum >> 4)*oneSampleSize;

	//3.�Ƚ�Ԫ�ض���Ϊǰ�����ٰѵ�ǰԪ���뱳�����������бȽϣ����뱳���������������Ϊ����
	for (size_t i = 0; i < oneSampleSize; i++){
		nMatchCount = 0;
		pForegoundImage[i] = 255;
		for (size_t j = 0; j < numOfsample; j++){
			nDistance = abs(ptrSample[j*oneSampleSize + i] - pCurrentImage[i]);//������������˵��Mat���ڴ���Ҳ���������е�
			if (nDistance <= radius){//radius
				nMatchCount++;
			}
			if (nMatchCount >= minMatchCount){
				pForegoundImage[i] = 0;
				break;
			}
		}
	}
	//4.ǰ������������б���������
	for (size_t i = 0; i < oneSampleSize; i++){
		if (pForegoundImage[i]){//��ǰ���������ǰ�������
			ptrSample[lastModelSampleIndex + i]++;
			if (ptrSample[lastModelSampleIndex + i] > foregroundCount){//foregroundCount
				ptrSample[nOffset + i] = pCurrentImage[i];
			}
		}
		else{//����˵�Ϊ��������Ա���ģ�ͽ��и���
			ptrSample[nOffset + i] = pCurrentImage[i];
			ptrSample[lastModelSampleIndex + i] = 0;
		}
	}

}

void ViBe_M::apply2(const cv::Mat &src, cv::Mat &dst){
	if (src.data != nullptr)
	{
		nFrameNum++;
		if (nFrameNum == 1)//��һ֡ʱ��ʼ������
		{
			initParams2(src, ViBe_M::Config::getGrayConfig());
			if (src.channels() == 3){
				cvtColor(src, grayImage, CV_BGR2GRAY);
			}
			update2(grayImage, foreImage);
		}
		else
		{
			if (src.channels() == 3){
				cvtColor(src, grayImage, CV_BGR2GRAY);
			}
			timeRecord = (double)cvGetTickCount();
			update2(grayImage, foreImage);
			timeRecord = (double)cvGetTickCount() - timeRecord;
			printf("exec time = %gms\n", timeRecord / (cvGetTickFrequency() * 1000));

			dst = foreImage;
			if (dst.data != NULL){
				//cv::imshow("dst", dst);
				//dst = cvarrToMat(pGrayForeImage, false);//true ������������
			}
			else{
				cout << "pGrayForeImage is NULL" << endl;
			}
		}
	}
}
void ViBe_M::initParams(const IplImage *pFrame, const Config& config){
	//CV_Assert(image.cols > 0 && image.rows > 0 && (image.type() == CV_8UC3 || image.type() == CV_8UC1));
	//cout << pFrame->depth<<endl;
	CV_Assert(pFrame->nChannels == 3 && pFrame->height > 0 && pFrame->width > 0 && pFrame->depth == IPL_DEPTH_8U);

	this->imageChannels = pFrame->nChannels;
	this->imageHeight = pFrame->height;
	this->imageWidth = pFrame->width;
	this->oneSampleSize = imageHeight*imageWidth;
	//this->oneSampleSize = imageChannels*imageHeight*imageWidth;
	

	this->numOfsample = config.numOfSample;
	this->minMatchCount = config.minMatchCount;
	this->foregroundCount = config.foregroundCount;
	this->radius = config.radius;

	this->lastModelSampleIndex = imageHeight*imageWidth*numOfsample;

	pGrayImage = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 1);
	//pRGBImage = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);
	//pRGBForeImage = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 3);
	pGrayForeImage = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 1);

	//this->pViBeSampleModel = (uchar *)malloc(imageHeight*imageWidth*(numOfsample + 1)*IPL_DEPTH_8U);
	this->pViBeSampleModel = (uchar *)malloc(imageHeight*imageWidth*(numOfsample + 1)*sizeof(uchar));
}
void ViBe_M::InitSampleModelData(const uchar *pImageData){
	for (size_t i = 0; i < numOfsample; i++){
		memcpy(pViBeSampleModel + oneSampleSize*i, pImageData, oneSampleSize);
	}
}
void ViBe_M::update(const uchar  *pCurrentImage, uchar  *pForegoundImage){
	size_t nOffset = 0;//ƫ����
	size_t nDistance = 0;//��ǰ֡�뱳��ģ�͵Ĳ�ֵ
	size_t nMatchCount = 0;

	static uchar CountNum = 0;
	CountNum++;
	if (CountNum > 127){
		CountNum = 0;
	}

	//1.��ʼ������ģ�ͣ�û�в���8�������20���������ǲ����˽�������ͬһ֡��8��copy���൱��ÿһ�����ظ�����8����Ϊ�䱳��������
	if (!isModelInit){
		InitSampleModelData(pCurrentImage);
		isModelInit = 1;
		return;
	}
	//2.ģ������� 0-7
	//int  t = (getTickCount()&mask) % numOfsample;//����������������˶�ǰ�������һ�ѵ����㣬�Ǹ���̫�������
	
	nOffset = (CountNum>>4)*oneSampleSize;

	//3.�Ƚ�Ԫ�ض���Ϊǰ�����ٰѵ�ǰԪ���뱳�����������бȽϣ����뱳���������������Ϊ����
	for (size_t i = 0; i < oneSampleSize; i++){
		nMatchCount = 0;
		pForegoundImage[i] = 255;//pGrayForeImage = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 1);˵���������ڴ������������е�
		for (size_t j = 0; j < numOfsample; j++){
			nDistance = abs(pViBeSampleModel[j*oneSampleSize + i] - pCurrentImage[i]);
			if (nDistance <=radius ){//radius
				nMatchCount++;
			}
			if (nMatchCount >= minMatchCount){
				pForegoundImage[i] = 0;
				break;
			}
		}
	}
	//4.ǰ������������б���������
	for (size_t i = 0; i < oneSampleSize; i++){
		if (pForegoundImage[i]){//��ǰ���������ǰ�������
			pViBeSampleModel[lastModelSampleIndex + i]++;
			if (pViBeSampleModel[lastModelSampleIndex + i] > foregroundCount){//foregroundCount
				pViBeSampleModel[nOffset + i] = pCurrentImage[i];
			}
		}
		else{//����˵�Ϊ��������Ա���ģ�ͽ��и���
			pViBeSampleModel[nOffset + i] = pCurrentImage[i];
			pViBeSampleModel[lastModelSampleIndex + i] = 0;
		}
	}

}
void ViBe_M::apply(const cv::Mat &src, cv::Mat &dst){
	//IplImage *IplSrc;
	//*pFrame = src;//ֱ��ת���ᱨ���������˹��ˣ�������û�����������
	//IplImage test = src;
	//pFrame = &test;// pFrame��IplImage*����
	pFrame = &(IplImage)src;
	if (pFrame != nullptr){
		nFrameNum++;
		if (nFrameNum == 1){
			initParams(pFrame, ViBe_M::Config::getGrayConfig());
			cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
			update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
		}
		else{
			cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
			timeRecord = (double)cvGetTickCount();
			update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
			timeRecord = (double)cvGetTickCount() - timeRecord;
			printf("exec time = %gms\n", timeRecord / (cvGetTickFrequency() * 1000));
			if (pGrayForeImage != NULL){
				dst = cvarrToMat(pGrayForeImage, false);//true ������������
			}
			else{
				cout << "pGrayForeImage is NULL" << endl;
			}
			//imshow("dst", dst);
			//cvShowImage("video", pGrayImage);
			//cvShowImage("foreground", pGrayForeImage);
		}
	}
}
//void ViBe_M::apply(IplImage *src, IplImage *dst){//�����⣬dstָ���ֵ������ȥ
//	//IplImage *IplSrc;
//	pFrame = src;
//
//	if (pFrame != nullptr){
//		nFrameNum++;
//		if (nFrameNum == 1){
//			initParams(pFrame, ViBe_M::Config::getGrayConfig());
//			cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
//			update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
//		}
//		else{
//			cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
//			timeRecord = (double)cvGetTickCount();
//			update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
//			timeRecord = (double)cvGetTickCount() - timeRecord;
//			printf("exec time = %gms\n", timeRecord / (cvGetTickFrequency() * 1000));
//
//			//cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 1);
//			//�ڴ��ں����ڲ����䣬��������ʱ���ͷ���,���Դ�����ȥֵ
//			dst = cvCreateImage(cvSize((*pGrayForeImage).width, (*pGrayForeImage).height), pGrayForeImage->depth, pGrayForeImage->nChannels);
//			*dst = *pGrayForeImage;//�ı�ָ���ַ��ֵ���Ǵ�����ֵ
//			//IplImage* img=cvLoadImage("");
//			//IplImage *temp=&dst;
//			//temp = pGrayForeImage;
//			//cvShowImage("video", pGrayImage);
//			//cvShowImage("foreground", pGrayForeImage);
//			cvShowImage("foreground2", dst);
//		}
//	}
//}
IplImage* ViBe_M::apply(IplImage *src){//ֱ�ӽ�����pGrayForeImage��ֵ��Ϊ����ֵ
	//IplImage *IplSrc;
	pFrame = src;

	if (pFrame != nullptr){
		nFrameNum++;
		if (nFrameNum == 1){
			initParams(pFrame, ViBe_M::Config::getGrayConfig());
			cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
			update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
		}
		else{
			cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
			timeRecord = (double)cvGetTickCount();
			update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
			timeRecord = (double)cvGetTickCount() - timeRecord;
			printf("exec time = %gms\n", timeRecord / (cvGetTickFrequency() * 1000));

			cvShowImage("video", pGrayImage);
			cvShowImage("foreground", pGrayForeImage);
		}
	}
	return pGrayForeImage;
}
void ViBe_M::applyTest(){
	/*
	1. cv::Mat -> IplImage
	cv::Mat matimg = cv::imread ("heels.jpg");
	IplImage* iplimg;
	*iplimg = IplImage(matimg);
	2. IplImage -> cv::Mat
	IplImage* iplimg = cvLoadImage("heels.jpg");
	cv::Mat matimg;
	matimg = cv::Mat(iplimg);
	*/
	const char *videoName = "D:\\007\\opencv2\\test\\00008_1.mp4";
	double t;
	int nFrameNum = 0;

	if (!(pCapture = cvCaptureFromAVI(videoName))){
		cout << "can not open the video" << endl;
		return ;
	}
	while (pFrame = cvQueryFrame(pCapture))
	{

		nFrameNum++;
		if (nFrameNum == 1){
			initParams(pFrame, ViBe_M::Config::getGrayConfig());
			cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
			update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
		}
		else{
			cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
			t = (double)cvGetTickCount();
			update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
			t = (double)cvGetTickCount() - t;
			printf("exec time = %gms\n", t / (cvGetTickFrequency() * 1000));
	
			cvShowImage("video", pGrayImage);
			cvShowImage("foreground",pGrayForeImage);

			char c = cvWaitKey(5);
			if (c == 27)break;
		}
	}
}