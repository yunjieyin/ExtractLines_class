#define _SCL_SECURE_NO_WARNINGS
#include "extractLines.h"
#include <fstream>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <highgui.hpp>
#include <vector>


///***********************************test function begin******************************************************/
//Scalar random_color(RNG& _rng)
//{
//	int icolor = (unsigned)_rng;
//	return Scalar(icolor & 0xFE, (icolor >> 8) & 0xFE, (icolor >> 16) & 0xFE);
//}
//
uchar* MatToArr(cv::Mat img)
{
	/***convert a Mat to a 1D array***/
	assert(!img.empty());

	uchar *dataPtr = NULL;
	int row = img.rows;
	int col = img.cols;

	dataPtr = (uchar *)malloc((row * col) * sizeof(uchar));
	assert(dataPtr != NULL);

	//copy pixels's elements
	unsigned int cnt = 0;
	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < col; j++)
		{
			dataPtr[cnt++] = img.at<uchar>(i, j);
		}
	}
	return dataPtr;
}


//
cv::Mat arryToMat(uchar *ptr, unsigned int rows, unsigned int cols)
{
	assert(ptr != NULL && rows * cols != 0);

	cv::Mat MatImg = cv::Mat(rows, cols, CV_8UC1);
	uchar *pTmp = NULL;

	for (int i = 0; i < rows; ++i)
	{
		pTmp = MatImg.ptr<uchar>(i);
		for (int j = 0; j < cols; ++j)
		{
			pTmp[j] = ptr[i * cols + j];
		}
	}

	return MatImg;
}

///***********************************test function end******************************************************/


/***************************test function***********************************/


cv::Scalar random_color(cv::RNG& _rng)
{
	int icolor = (unsigned)_rng;
	return cv::Scalar(icolor & 0xFE, (icolor >> 8) & 0xFE, (icolor >> 16) & 0xFE);
}

void drawLinesOnSrcImg(cv::Mat& img, std::vector<std::deque<cv::Point>> lines, std::string picName)
{
	if (1)
	{
		std::vector<std::vector<cv::Point>> linesVec;
		for (int i = 0; i < lines.size(); ++i)
		{
			std::vector<cv::Point> points;
			for (int j = 0; j < lines[i].size(); ++j)
			{
				cv::Point p;
				p.x = lines[i][j].x;
				p.y = lines[i][j].y;
				points.push_back(p);
			}

			linesVec.push_back(points);

		}

		cv::Mat draw_img = img.clone();
		cv::RNG rng(123);
		cv::Scalar color;
		for (int i = 0; i < linesVec.size(); i++)
		{
			color = random_color(rng);
			for (int j = 0; j < linesVec[i].size(); j++)
			{
				img.at<cv::Vec3b>(linesVec[i][j]) = cv::Vec3b(color[0], color[1], color[2]);
			}
		}

		int nLineNum = linesVec.size();
		std::string str = "lines number = " + std::to_string(nLineNum);
		cv::putText(img, str, cv::Point(20, 20), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 255));

		/*the path of to be write*/
		//cv::imwrite("E:\\pictures\\coil\\testResult_1D\\" + picName, img);
		//cv::imwrite("E:\\pictures\\coil\\rawTest\\" + picName, img);
		//cv::imwrite("E:\\pictures\\coil\\0630Test\\" + picName, img);
		cv::imwrite("E:\\pictures\\coil\\allTest\\" + picName, img);
	}
}

void batch_test()
{
	//std::string dir_path = "E:\\pictures\\coil\\roi_20180820\\";
	//std::string dir_path = "E:\\pictures\\coil\\roi(1)\\";
	//std::string dir_path = "E:\\pictures\\coil\\raw\\";
	//std::string dir_path = "E:\\pictures\\coil\\0630\\";
	std::string dir_path = "E:\\pictures\\coil\\all\\";


	Directory dir;
	std::vector<std::string> picNames = dir.GetListFiles(dir_path, "*.bmp", false);

	std::vector<cv::String> fileNames;
	//cv::glob("E:\\pictures\\coil\\roi_20180820\\*.bmp", fileNames, true);
	//cv::glob("E:\\pictures\\coil\\roi(1)\\*.bmp", fileNames, true);
	//cv::glob("E:\\pictures\\coil\\raw\\*.bmp", fileNames, true);
	//cv::glob("E:\\pictures\\coil\\0630\\*.bmp", fileNames, true);
	cv::glob("E:\\pictures\\coil\\all\\*.bmp", fileNames, true);


	cv::Mat imgSrc;
	cv::Mat imgGray;
	for (int i = 0; i < fileNames.size(); ++i)
	{
		std::string fileName = fileNames[i];
		std::cout << "file name:" << fileName << std::endl;
		imgSrc = cv::imread(fileName);
		cv::cvtColor(imgSrc, imgGray, cv::COLOR_BGR2GRAY);

		uchar* pGray = MatToArr(imgGray);
		mSize imgSize = { imgSrc.rows, imgSrc.cols };

		std::string picName = picNames[i];
		//linesData_test(imgSrc, pGray, imgSize, picName);
		ExtractLines line(pGray, imgSize);
		line.linesData();

		//convert to cv points data
		std::vector<std::deque<cv::Point>> cv_markedLines;
		std::deque<cv::Point> cv_line;
		for (int i = 0; i < line.markedLines.size(); i++)
		{
			cv_line.clear();
			for (int j = 0; j < line.markedLines[i].size(); j++)
			{
				cv::Point cv_pt(line.markedLines[i][j].x, line.markedLines[i][j].y);
				if (cv_line.size() > 0 && abs(cv_pt.x - cv_line.back().x) > 1
					|| cv_line.size() > 0 && (cv_pt.x - cv_line.back().x) < 0)
				{
					std::cout << "x index strid:" << cv_pt.x << "," << cv_pt.y  << "index:" << i << std::endl;
				}

				cv_line.push_back(cv_pt);
			}

			cv_markedLines.push_back(cv_line);
		}

		drawLinesOnSrcImg(imgSrc, cv_markedLines, picName);
	}
}



int main()
{
	if (1)
	{
		//cv::Mat img = cv::imread("E:\\pictures\\coil\\roi_20180820\\46-7.bmp", 0);
		/*cv::Mat img = cv::imread("E:\\pictures\\coil\\all\\132-1.bmp", 0);
		uchar* pImg = MatToArr(img);
		mSize imgSize = { img.rows, img.cols };*/

		/*write array data to file*/
		/*if (0)
		{
			FILE *fp = NULL;
			if ((fp = fopen("E:\\pictures\\coil\\data.txt", "w")) == NULL)
			{
				printf("open error!");
				exit(0);
			}

			for (int i = 0; i < imgSize.rows * imgSize.cols; i++)
			{
				fprintf(fp, "%d ", pImg[i]);
			}
			fclose(fp);
		}*/

		/*read file data to array*/
		/*if (0)
		{
			uchar* pImgTxt = (uchar *)malloc((imgSize.rows * imgSize.cols) * sizeof(uchar));
			if (pImgTxt == NULL)
			{
				return 0;
			}
			uchar* p = pImgTxt;


			uchar data;
			FILE* fp = NULL;
			fp = (fp = fopen("E:\\pictures\\coil\\data.txt", "r"));
			if (!fp)
			{
				return 0;
			}

			int i = 0;
			while (!feof(fp))
			{
				fscanf(fp, "%d", &data);
				pImgTxt[i++] = data;
			}
		}*/


		//主入口函数
		/*ExtractLines line(pImg, imgSize);
		line.linesData();

		cv::Mat thin = arryToMat(line.pThinImg, img.rows, img.cols);
		cv::Mat grd = arryToMat(line.pGrdImg, img.rows, img.cols);*/

		batch_test();
		
	}

	std::getchar();
	return 0;
}
