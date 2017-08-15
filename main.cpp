#include"ViBe_M.h"
#include<iostream>

int main(){

	//ViBe_M vibe;
	//vibe.applyTest();
	//if (!videoTest.open(videoName)){
	//	QMessageBox::information(this, tr("ERROR!"), tr("open video fail!"));
	//	return;
	//}

	/*
	
	(1) IplImage ת Mat:

	IplImage* image = cvLoadImage( "lena.jpg");  
	Mat mat=cvarrToMat(image);

	(2)MatתIplImage:

	IplImage img = IplImage(mat);
	*/
	string fileName = "D:\\007\\opencv2\\test\\00008.mp4";
	cv::VideoCapture capture(fileName);
	cv::Mat frame;
	if (!capture.open(fileName)){
		cout << "can not open the video" << endl;
		return -1;
	}
	//	Mat dst = Mat::zeros(detectROI.size(), detectROI.type());
	Mat dst;
	IplImage *density=nullptr;
	//Mat test,aaa;
	//imshow("aa", aaa);
	ViBe_M vibe;
	//cvNamedWindow("video", 1);
	//cvNamedWindow("foreground", 1);
	while (1){
	
		capture >> frame;
		//IplImage *dst=nullptr;
		//IplImage src = IplImage(frame);
		//*src = IplImage(frame);
		//dst = Mat::zeros(frame.size(), frame.type());
		//dst=vibe.apply(frame);

		//����1 ok
		//dst = cvCreateImage(cvSize((*pGrayForeImage).width, (*pGrayForeImage).height), pGrayForeImage->depth, pGrayForeImage->nChannels);
		//density = cvCreateImage(cvSize(frame.cols, frame.rows), IPL_DEPTH_8U, 1);
		
		//����3 ok
		//density=vibe.apply(&(IplImage(frame)));//������density��ֵ��������,����Ϊ֮ǰdensity���ڴ�����apply�ڲ�����ľֲ��ڴ棬��������ʱ���ڴ汻�ͷ���
		//cvShowImage("456", density);
		//test = dst;

	     //����2
		//vibe.apply(frame, dst);
		////vibe.apply(frame, dst);//ok
		//if (dst.data != NULL){//�ж��ǲ���null,�жϣ�������
		//	cv::imshow("dst", dst);
		//	//cout << "null" << endl;
		//}

		//����4 ok
		vibe.apply2(frame, dst);
		if (dst.data != NULL){//�ж��ǲ���null,�жϣ�������
			cv::imshow("dst", dst);
			//cout << "null" << endl;
		}

		//density = vibe.pGrayForeImage;
		//cvShowImage("123", vibe.pGrayForeImage);
		//cvShowImage("456", density);
		//cvShowImage("foreground3", density);
		//if (density != NULL){
		//	cvShowImage("foreground3", density);
		//}
		//waitKey(30);
		//cv::imshow("foreground", dst);
		waitKey(30);
	}
	//Mat dst;

	//int i = 0;
	//while (i<1000){
	//	i++;
	//	capture >> frame;
	//	//dst = Mat::zeros(frame.size(), frame.type());
	//	//vibe.apply(frame, dst);
	//	cv::imshow("frame", frame);
	//	//cv::imshow("foreground", dst);
	//	cv::waitKey(10);
	//}
	system("pause");
}

//int main1()
//{
//	const char *videoName = "D:\\007\\opencv2\\test\\00005.mp4";
//	IplImage *pFrame = nullptr;
//	IplImage *pGrayImage = nullptr;
//	IplImage *pGrayForeImage = nullptr;
//
//	CvCapture *pCapture = nullptr;
//	////��������
//	//cvNamedWindow("video", 1);
//	//cvNamedWindow("foreground", 1);
//
//	////ʹ������������
//	//cvMoveWindow("video", 300, 50);
//	//cvMoveWindow("foreground", 680, 50);
//	double t;
//	int nFrameNum = 0;
//
//	if (!(pCapture = cvCaptureFromAVI(videoName))){
//		cout << "can not open the video" << endl;
//		return -1;
//	}
//	ViBe_M vibeGray;
//	
//	while (pFrame = cvQueryFrame(pCapture))
//	{
//		
//		nFrameNum++;
//		if (nFrameNum == 1){
//			vibeGray.initParams(pFrame, ViBe_M::Config::getGrayConfig());
//			
//			cvCvtColor(pFrame, vibeGray.pGrayImage, CV_BGR2GRAY);
//			//pGrayImage = cvCreateImage(cvSize(vibeGray.imageWidth, vibeGray.imageHeight), IPL_DEPTH_8U, 1);
//			//cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
//			//pGrayForeImage = cvCreateImage(cvSize(vibeGray.imageWidth, vibeGray.imageHeight), IPL_DEPTH_8U, 1);
//			//vibeGray.update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
//			vibeGray.update((uchar *)vibeGray.pGrayImage->imageData, (uchar *)vibeGray.pGrayForeImage->imageData);
//			//vibeGray.update((uchar *)pFrame->imageData, (uchar *)vibeGray.pRGBForeImage->imageData);
//		}
//		else{
//			cvCvtColor(pFrame, vibeGray.pGrayImage, CV_BGR2GRAY);
//			//cvCvtColor(pFrame, pGrayImage, CV_BGR2GRAY);
//			t = (double)cvGetTickCount();
//			//vibeGray.update((uchar *)pGrayImage->imageData, (uchar *)pGrayForeImage->imageData);
//			vibeGray.update((uchar *)vibeGray.pGrayImage->imageData, (uchar *)vibeGray.pGrayForeImage->imageData);
//			//vibeGray.update((uchar *)pFrame->imageData, (uchar *)vibeGray.pRGBForeImage->imageData);
//
//			t = (double)cvGetTickCount() - t;
//			printf("exec time = %gms\n", t / (cvGetTickFrequency() * 1000));
//
//			//cvShowImage("video", pGrayImage);
//			//cvShowImage("foreground", pGrayForeImage);		
//			cvShowImage("video", vibeGray.pGrayImage);
//			cvShowImage("foreground", vibeGray.pGrayForeImage);
//
//			char c = cvWaitKey(5);
//			if (c == 27)break;
//		}
//	}
//
//	//���ٴ���
//	//cvDestroyWindow("video");
//	//cvDestroyWindow("foreground");
//
//	//cvReleaseImage(&pGrayForeImage);
//	//cvReleaseImage(&pGrayImage);
//	cvReleaseImage(&pFrame);
//	cvReleaseCapture(&pCapture);
//
//	system("pause");
//
//	return 0;
//}