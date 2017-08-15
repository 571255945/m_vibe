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
	size_t nOffset = 0;//偏移量
	size_t nDistance = 0;//当前帧与背景模型的差值
	size_t nMatchCount = 0;

	static uchar CountNum = 0;

	uchar* pCurrentImage = src.data;
	uchar* pForegoundImage = foreImage.data;
	CountNum++;
	if (CountNum > 127){
		CountNum = 0;
	}
	//1.初始化背景模型，没有采用8邻域随机20采样，而是采用了仅采用了同一帧的8个copy，相当于每一个像素复制了8份作为其背景样本集
	if (!isModelInit){
		InitSampleModelData2(src);
		isModelInit = 1;
		return;
	}
	//2.模拟随机数 0-7
	//int  t = (getTickCount()&mask) % numOfsample;//这样产生随机数，运动前景后会拖一堆的闪点，是更新太随机了吗？
	nOffset = (CountNum >> 4)*oneSampleSize;

	//3.先将元素都置为前景，再把当前元素与背景样本集进行比较，将与背景样本集相近的设为背景
	for (size_t i = 0; i < oneSampleSize; i++){
		nMatchCount = 0;
		pForegoundImage[i] = 255;
		for (size_t j = 0; j < numOfsample; j++){
			nDistance = abs(ptrSample[j*oneSampleSize + i] - pCurrentImage[i]);//可以这样操作说明Mat在内存中也是连续排列的
			if (nDistance <= radius){//radius
				nMatchCount++;
			}
			if (nMatchCount >= minMatchCount){
				pForegoundImage[i] = 0;
				break;
			}
		}
	}
	//4.前景点计数，更行背景样本集
	for (size_t i = 0; i < oneSampleSize; i++){
		if (pForegoundImage[i]){//是前景，则进行前景点计数
			ptrSample[lastModelSampleIndex + i]++;
			if (ptrSample[lastModelSampleIndex + i] > foregroundCount){//foregroundCount
				ptrSample[nOffset + i] = pCurrentImage[i];
			}
		}
		else{//如果此点为背景，则对背景模型进行更新
			ptrSample[nOffset + i] = pCurrentImage[i];
			ptrSample[lastModelSampleIndex + i] = 0;
		}
	}

}

void ViBe_M::apply2(const cv::Mat &src, cv::Mat &dst){
	if (src.data != nullptr)
	{
		nFrameNum++;
		if (nFrameNum == 1)//第一帧时初始化参数
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
				//dst = cvarrToMat(pGrayForeImage, false);//true 复制整个矩阵
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
	size_t nOffset = 0;//偏移量
	size_t nDistance = 0;//当前帧与背景模型的差值
	size_t nMatchCount = 0;

	static uchar CountNum = 0;
	CountNum++;
	if (CountNum > 127){
		CountNum = 0;
	}

	//1.初始化背景模型，没有采用8邻域随机20采样，而是采用了仅采用了同一帧的8个copy，相当于每一个像素复制了8份作为其背景样本集
	if (!isModelInit){
		InitSampleModelData(pCurrentImage);
		isModelInit = 1;
		return;
	}
	//2.模拟随机数 0-7
	//int  t = (getTickCount()&mask) % numOfsample;//这样产生随机数，运动前景后会拖一堆的闪点，是更新太随机了吗？
	
	nOffset = (CountNum>>4)*oneSampleSize;

	//3.先将元素都置为前景，再把当前元素与背景样本集进行比较，将与背景样本集相近的设为背景
	for (size_t i = 0; i < oneSampleSize; i++){
		nMatchCount = 0;
		pForegoundImage[i] = 255;//pGrayForeImage = cvCreateImage(cvSize(imageWidth, imageHeight), IPL_DEPTH_8U, 1);说明数据在内存中是线性排列的
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
	//4.前景点计数，更行背景样本集
	for (size_t i = 0; i < oneSampleSize; i++){
		if (pForegoundImage[i]){//是前景，则进行前景点计数
			pViBeSampleModel[lastModelSampleIndex + i]++;
			if (pViBeSampleModel[lastModelSampleIndex + i] > foregroundCount){//foregroundCount
				pViBeSampleModel[nOffset + i] = pCurrentImage[i];
			}
		}
		else{//如果此点为背景，则对背景模型进行更新
			pViBeSampleModel[nOffset + i] = pCurrentImage[i];
			pViBeSampleModel[lastModelSampleIndex + i] = 0;
		}
	}

}
void ViBe_M::apply(const cv::Mat &src, cv::Mat &dst){
	//IplImage *IplSrc;
	//*pFrame = src;//直接转换会报错，真是奇了怪了，可能是没有重载运算符
	//IplImage test = src;
	//pFrame = &test;// pFrame是IplImage*类型
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
				dst = cvarrToMat(pGrayForeImage, false);//true 复制整个矩阵
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
//void ViBe_M::apply(IplImage *src, IplImage *dst){//有问题，dst指针的值传不出去
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
//			//内存在函数内部分配，函数返回时被释放了,所以传不出去值
//			dst = cvCreateImage(cvSize((*pGrayForeImage).width, (*pGrayForeImage).height), pGrayForeImage->depth, pGrayForeImage->nChannels);
//			*dst = *pGrayForeImage;//改变指针地址的值还是传不出值
//			//IplImage* img=cvLoadImage("");
//			//IplImage *temp=&dst;
//			//temp = pGrayForeImage;
//			//cvShowImage("video", pGrayImage);
//			//cvShowImage("foreground", pGrayForeImage);
//			cvShowImage("foreground2", dst);
//		}
//	}
//}
IplImage* ViBe_M::apply(IplImage *src){//直接将采用pGrayForeImage的值作为返回值
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