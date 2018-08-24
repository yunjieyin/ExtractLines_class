#include "extractLines.h"

ExtractLines::ExtractLines(uchar* pImg, mSize imgSize)
{
	assert(pImg != NULL);

	srcImgSize = imgSize;
	rows = srcImgSize.rows;
	cols = srcImgSize.cols;
	pIndex = pImg;
}

ExtractLines::~ExtractLines()
{
	free(pSrcImg);
	free(pGrdImg);
	free(pCpyImg);
	free(pThinImg);
	free(pChar);
}

void ExtractLines::gradGraph()
{
	mPoint pt;
	int nStride = 2;
	int dx;
	int dy;
	int grade;

	for (int x = 1; x < cols - 1; x = x + 1)
	{
		for (int y = 1; y < rows - 1; y = y + 1)
		{
			pt.x = x;
			pt.y = y;

			if (pt.y + nStride <= rows - 1)
			{
				dy = pSrcImg[cols * (pt.y + nStride) + pt.x] - pSrcImg[cols * pt.y + pt.x];
			}
			else
			{
				dy = 0;
			}

			if (pt.x + nStride <= cols - 1)
			{
				dx = pSrcImg[cols * pt.y + pt.x + nStride] - pSrcImg[cols * pt.y + pt.x];
			}
			else
			{
				dx = 0;
			}

			if (pSrcImg[cols * (pt.y + nStride) + pt.x] > pSrcImg[cols * pt.y + pt.x])
			{
				grade = int(dx / 5) + dy;
			}
			else
			{
				grade = 0;
			}

			if (grade < GRAD_THRESH_VAL)
				grade = 0;

			pGrdImg[y * cols + x] = grade;
		}
	}
}

void ExtractLines::wipe_singular_points(uchar* pThin)
{
	//del illegal points
	int upNum;
	int botNum;
	unsigned int val0;
	unsigned int val1;
	unsigned int val2;
	unsigned int val3;
	unsigned int val4;
	const int lagVal = 3;

	for (int y = 1; y < rows - 2; ++y)
	{
		for (int x = 1; x < cols - 2; ++x)
		{
			val0 = pThin[y * cols + x];
			val1 = pThin[y * cols + x - 1];
			val2 = pThin[y * cols + x + 1];
			val3 = pThin[(y - 1) * cols + x];
			val4 = pThin[(y + 1) * cols + x];

			if ((val1 == 255 && val3 == 255 && val2 == 0 && val4 == 0 && val0 == 255)
				|| (val1 == 0 && val2 == 255 && val3 == 255 && val4 == 0 && val0 == 255)
				|| (val1 == 0 && val2 == 0 && val3 == 255 && val4 == 0 && val0 == 255))
			{
				pThin[y * cols + x] = 0;
			}

			//3 connected point
			if (val1 + val2 + val3 + val4 == 255 * 3)
			{
				if (pThin[(y - 1) * cols + x] == 0 && pThin[(y + 1) * cols + x] == 255)
				{
					pThin[(y + 1) * cols + x] = 0;
				}
				else if (pThin[(y + 1) * cols + x] == 0 && pThin[(y - 1) * cols + x] == 255)
				{
					pThin[(y - 1) * cols + x] = 0;
				}
				else
				{
					pThin[y * cols + x] = 0;

					upNum = 0;
					botNum = 0;;
					for (int j = -3; j <= 3; j++)
					{
						if (x + j > 0 && x + j < cols && pThin[(y - 1) * cols + x + j] == 255)
						{
							upNum++;
						}
						if (x + j > 0 && x + j < cols && pThin[(y + 1) * cols + x + j] == 255)
						{
							botNum++;
						}
					}


					if (upNum > botNum)
					{
						pThin[(y + 1) * cols + x] = 0;
					}
					else
					{
						pThin[(y - 1) * cols + x] = 0;
					}

				}

			}

			if (y - lagVal > 0)
			{
				val0 = pThin[(y - lagVal) * cols + x];
				val1 = pThin[(y - lagVal) * cols + x - 1];
				val2 = pThin[(y - lagVal) * cols + x + 1];
				val3 = pThin[(y - 1 - lagVal) * cols + x];
				val4 = pThin[(y + 1 - lagVal) * cols + x];
				if ((val1 == 0 && val2 == 0 && val3 == 0 && val4 == 255 && val0 == 255)
					|| (val1 == 0 && val2 == 0 && val3 == 255 && val4 == 0 && val0 == 255))
				{
					pThin[(y - lagVal) * cols + x] = 0;
				}
			}

		}
	}


	for (int y = 1; y < rows - 2; ++y)
	{
		for (int x = 1; x < cols - 2; ++x)
		{
			//del corner point
			//  1          1        1
			//1 1 0		 0 1 1    0 1 0
			//  0          0        0

			val0 = pThin[y * cols + x];
			val1 = pThin[y * cols + x - 1];
			val2 = pThin[y * cols + x + 1];
			val3 = pThin[(y - 1) * cols + x];
			val4 = pThin[(y + 1) * cols + x];

			if ((val1 == 255 && val3 == 255 && val2 == 0 && val4 == 0 && val0 == 255)
				|| (val1 == 0 && val2 == 255 && val3 == 255 && val4 == 0 && val0 == 255)
				|| (val1 == 0 && val2 == 0 && val3 == 255 && val4 == 0 && val0 == 255))
			{
				pThin[y * cols + x] = 0;
			}

			//connect ..101...
			if (val0 == 0 && val1 == 255 && val2 == 255)
			{
				pThin[y * cols + x] = 255;
			}
		}
	}
}

void ExtractLines::mark_lines()
{
	makeImgThinner();
	wipe_singular_points(pCpyImg);

	memcpy(pThinImg, pCpyImg, rows * cols * sizeof(uchar));

	mPoint seed;
	std::deque<mPoint> linePts;
	while (findFirstPoint(pCpyImg, seed))
	{
		linePts.clear();
		mark_connect_region(pCpyImg, seed, linePts);

		if (linePts.size() > LINE_LEAST_POINTS)
		{
			markedLines.push_back(linePts);
		}
	}

	//convert data
	for (int i = 0; i < markedLines.size(); i++)
	{
		linesSet.linesSize[i] = markedLines[i].size();

		for (int j = 0; j < markedLines[i].size(); j++)
		{
			linesSet.linesSets[i * LINE_MAX_POINTS + j] = markedLines[i][j];
		}
	}

	linesSet.linesNum = markedLines.size();
}

void ExtractLines::linesData()
{
	pSrcImg = (uchar*)malloc((rows * cols) * sizeof(uchar));
	memcpy(pSrcImg, pIndex, (rows * cols) * sizeof(uchar));

	pGrdImg = (uchar*)malloc((rows * cols) * sizeof(uchar));
	memset(pGrdImg, 0, rows*cols * sizeof(uchar));

	pCpyImg = (uchar*)malloc(rows * cols * sizeof(uchar));
	pChar = (uchar**)malloc(rows * cols * sizeof(uchar*));

	pThinImg = (uchar*)malloc(rows * cols * sizeof(uchar));
	assert(pIndex && pSrcImg && pGrdImg && pCpyImg && pChar && pThinImg);

	gradGraph();
	grad_graph_binary();
	padding_points();
	//filterPixels();
	mark_lines();
}

void ExtractLines::mark_connect_region(uchar* pThin, mPoint pt, std::deque<mPoint>& linePts)
{
	linePts.clear();
	mPoint p1, p2, p3, p4, p5, p6, p7, p8;

	p1.x = -1; p1.y = -1;
	p2.x = 0; p2.y = -1;
	p3.x = 1; p3.y = -1;
	p4.x = -1; p4.y = 0;
	p5.x = 1; p5.y = 0;
	p6.x = -1; p6.y = 1;
	p7.x = 0; p7.y = 1;
	p8.x = 1; p8.y = 1;
	static mPoint connects[8] = { p1, p2, p3, p4, p5, p6, p7, p8 };

	std::stack<mPoint> seeds;
	seeds.push(pt);

	linePts.push_back(pt);

	mPoint seed;
	mPoint tmpPt;
	while (!seeds.empty())
	{
		seed = seeds.top();
		seeds.pop();

		for (size_t i = 0; i < 8; i++)
		{
			int tmpx = seed.x + connects[i].x;
			int tmpy = seed.y + connects[i].y;

			if (tmpx < 0 || tmpy < 0 || tmpx >= cols || tmpy >= rows)
				continue;

			if (pThin[tmpy * cols + tmpx] != 0)
			{
				tmpPt.x = tmpx;
				tmpPt.y = tmpy;

				if (tmpPt.x >= linePts.back().x )
				{
					linePts.push_back(tmpPt);
				}
				else
				{
					linePts.push_front(tmpPt);
				}
				
				pThin[tmpy * cols + tmpx] = 0;

				seeds.push(tmpPt);

			}

		}
	}

	pThin[pt.y * cols + pt.x] = 0;
}

int ExtractLines::pixelGrade(uchar* pImg, mPoint pt)
{
	/***compute a pixel's grade at point 'pt'***/
	assert(pImg != NULL && pt.x > 0 && pt.y > 0);
	int nStride = 2;
	int dx;
	int dy;
	int grade;

	if (pt.y + nStride <= rows - 1)
	{
		dy = pImg[cols * (pt.y + nStride) + pt.x] - pImg[cols * pt.y + pt.x];
	}
	else
	{
		dy = 0;
	}
	
	if (pt.x + nStride <= cols - 1)
	{
		dx = pImg[cols * pt.y + pt.x + nStride] - pImg[cols * pt.y + pt.x];
	}
	else
	{
		dx = 0;
	}


	if (pImg[cols * (pt.y + nStride) + pt.x] > pImg[cols * pt.y + pt.x])
	{
		grade = int(dx / 5) + dy;
	}
	else
	{
		grade = 0;
	}

	if (grade < GRAD_THRESH_VAL)
		grade = 0;

	return grade;
}

void ExtractLines::grad_graph_binary()
{
	int sumNeig_9 = 0;
	const unsigned int neig9SumVal = 30;

	for (int y = 1; y < rows - 1; ++y)
	{
		for (int x = 1; x < cols - 1; ++x)
		{
			//消除4邻域为空的噪点
			if (pGrdImg[y * cols + x] > 0 && pGrdImg[(y - 1) * cols + x] == 0 &&
				pGrdImg[(y + 1) * cols + x] == 0 && pGrdImg[y * cols + x - 1] == 0 &&
				pGrdImg[y * cols + x + 1] == 0)
			{
				pGrdImg[y * cols + x] = 0;
			}

			//根据八邻域点的平均像素值把当前像素设为背景或前景点
			sumNeig_9 = pGrdImg[y * cols + x] + pGrdImg[(y - 1) * cols + x - 1] +
				pGrdImg[(y - 1) * cols + x] + pGrdImg[(y - 1) * cols + x + 1] +
				pGrdImg[y * cols + x - 1] + pGrdImg[y * cols + x + 1] +
				pGrdImg[(y + 1) * cols + x - 1] + pGrdImg[(y + 1) * cols + x] +
				pGrdImg[(y + 1) * cols + x + 1];

			if (sumNeig_9 / 9 < neig9SumVal)
			{
				pGrdImg[y * cols + x] = 0;
			}
			else if (pGrdImg[y * cols + x] > 0)
			{
				pGrdImg[y * cols + x] = 255;
			}
		}
	}
}

void ExtractLines::padding_points()
{
	int bkPtFlag = 0;
	mPoint pt;
	mPoint bpt;
	mPoint holePt;
	mPoint tmp;

	for (int y = 1; y < rows - 1; ++y)
	{
		for (int x = 1; x < cols - 1; ++x)
		{
			pt.x = x;
			pt.y = y;

			//消除满足左右像素点均为背景点的若干点（3）  
			if (isNisolatePoints(pGrdImg, pt) == 1)
			{
				pGrdImg[pt.y * cols + pt.x] = 0;
			}
			else if (isNisolatePoints(pGrdImg, pt) == 2)
			{
				pGrdImg[pt.y * cols + pt.x] = 0;
				pGrdImg[pt.y * cols + pt.x + 1] = 0;
			}
			else if (isNisolatePoints(pGrdImg, pt) == 3)
			{
				pGrdImg[pt.y * cols + pt.x] = 0;
				pGrdImg[pt.y * cols + pt.x + 1] = 0;
				pGrdImg[pt.y * cols + pt.x + 2] = 0;
			}

			/*补水平方向0的点...1***1...*/
			if (x + 4 < cols && pGrdImg[pt.y * cols + pt.x] == 255 &&
				(pGrdImg[pt.y * cols + pt.x + 1] == 0 || pGrdImg[pt.y * cols + pt.x + 2] == 0 ||
					pGrdImg[pt.y * cols + pt.x + 3] == 0) && pGrdImg[pt.y * cols + pt.x + 4] == 255)
			{
				pGrdImg[pt.y * cols + pt.x + 1] = 255;
				pGrdImg[pt.y * cols + pt.x + 2] = 255;
				pGrdImg[pt.y * cols + pt.x + 3] = 255;
			}

			//连接水平段的断点
			bpt.x = x;
			bpt.y = y - 1;

			bkPtFlag = BreakPoint(pGrdImg, bpt);

			if (bkPtFlag == 1 && x + NUM_BRK_PTS_HORIZ < cols)
			{
				//判断该点（1->0）右边 NUM_BK_PTS_HORIZ 个点内是否有0->1断点
				for (int i = 1; i <= NUM_BRK_PTS_HORIZ; i++)
				{
					bpt.x = x + i;
					bpt.y = y - 1;
					if (BreakPoint(pGrdImg, bpt) == 2)
					{
						for (int j = x; j <= bpt.x; j++)
						{
							tmp.x = j;
							tmp.y = y - 1;
							pGrdImg[tmp.y * cols + tmp.x] = 255;
						}
						break;
					}
				}
			}

			//填补孔洞
			holePt.x = x;
			holePt.y = y - 3;
			if (holePt.y > 0)
				pad_hole(pGrdImg, holePt);
		}
	}
}

void ExtractLines::filterPixels()
{
	assert(pGrdImg != NULL);

	int sumNeig_9 = 0;
	int bkPtFlag = 0;
	const unsigned int neig9SumVal = 30;
	mPoint pt;
	mPoint bpt;
	mPoint holePt;
	mPoint tmp;

	for (int y = 1; y < rows - 1; ++y)
	{
		for (int x = 1; x < cols - 1; ++x)
		{
			//消除4邻域为空的噪点
			if (pGrdImg[y * cols + x] > 0 && pGrdImg[(y - 1) * cols + x] == 0 &&
				pGrdImg[(y + 1) * cols + x] == 0 && pGrdImg[y * cols + x - 1] == 0 &&
				pGrdImg[y * cols + x + 1] == 0)
			{
				pGrdImg[y * cols + x] = 0;
			}

			//根据八邻域点的平均像素值把当前像素设为背景或前景点
			sumNeig_9 = pGrdImg[y * cols + x] + pGrdImg[(y - 1) * cols + x - 1] +
				pGrdImg[(y - 1) * cols + x] + pGrdImg[(y - 1) * cols + x + 1] +
				pGrdImg[y * cols + x - 1] + pGrdImg[y * cols + x + 1] +
				pGrdImg[(y + 1) * cols + x - 1] + pGrdImg[(y + 1) * cols + x] +
				pGrdImg[(y + 1) * cols + x + 1];

			if (sumNeig_9 / 9 < neig9SumVal)
			{
				pGrdImg[y * cols + x] = 0;
			}
			else if (pGrdImg[y * cols + x] > 0)
			{
				pGrdImg[y * cols + x] = 255;
			}
		}
	}


	for (int y = 1; y < rows - 1; ++y)
	{
		for (int x = 1; x < cols - 1; ++x)
		{
			pt.x = x;
			pt.y = y;

			//消除满足左右像素点均为背景点的若干点（3）  
			if (isNisolatePoints(pGrdImg, pt) == 1)
			{
				pGrdImg[pt.y * cols + pt.x] = 0;
			}
			else if (isNisolatePoints(pGrdImg, pt) == 2)
			{
				pGrdImg[pt.y * cols + pt.x] = 0;
				pGrdImg[pt.y * cols + pt.x + 1] = 0;
			}
			else if (isNisolatePoints(pGrdImg, pt) == 3)
			{
				pGrdImg[pt.y * cols + pt.x] = 0;
				pGrdImg[pt.y * cols + pt.x + 1] = 0;
				pGrdImg[pt.y * cols + pt.x + 2] = 0;
			}

			/*补水平方向0的点...1***1...*/
			if (x + 4 < cols && pGrdImg[pt.y * cols + pt.x] == 255 &&
				(pGrdImg[pt.y * cols + pt.x + 1] == 0 || pGrdImg[pt.y * cols + pt.x + 2] == 0 ||
					pGrdImg[pt.y * cols + pt.x + 3] == 0) && pGrdImg[pt.y * cols + pt.x + 4] == 255)
			{
				pGrdImg[pt.y * cols + pt.x + 1] = 255;
				pGrdImg[pt.y * cols + pt.x + 2] = 255;
				pGrdImg[pt.y * cols + pt.x + 3] = 255;
			}

			//连接水平段的断点
			bpt.x = x;
			bpt.y = y - 1;

			bkPtFlag = BreakPoint(pGrdImg, bpt);

			if (bkPtFlag == 1 && x + NUM_BRK_PTS_HORIZ < cols)
			{
				//判断该点（1->0）右边 NUM_BK_PTS_HORIZ 个点内是否有0->1断点
				for (int i = 1; i <= NUM_BRK_PTS_HORIZ; i++)
				{
					bpt.x = x + i;
					bpt.y = y - 1;
					if (BreakPoint(pGrdImg, bpt) == 2)
					{
						for (int j = x; j <= bpt.x; j++)
						{
							tmp.x = j;
							tmp.y = y - 1;
							pGrdImg[tmp.y * cols + tmp.x] = 255;
						}
						break;
					}
				}
			}

			//填补孔洞
			holePt.x = x;
			holePt.y = y - 3;
			if (holePt.y > 0)
				pad_hole(pGrdImg, holePt);
		}
	}

}

int ExtractLines::isNisolatePoints(uchar* pImg, mPoint pt)
{
	assert(pImg != NULL);

	int bFlag = 0;
	const int conNum = 10;

	mPoint tmpPt1 = { pt.x, pt.y - 1 };
	mPoint tmpPt2 = { pt.x, pt.y + 1 };
	mPoint tmpPt3 = { pt.x + 1, pt.y - 1 };
	mPoint tmpPt4 = { pt.x + 1, pt.y + 1 };
	mPoint tmpPt5 = { pt.x + 2, pt.y - 1 };
	mPoint tmpPt6 = { pt.x + 2, pt.y + 1 };
	mPoint tmpPt7 = { pt.x + 2, pt.y - 2 };
	mPoint tmpPt8 = { pt.x + 2, pt.y + 2 };

	bool bPt1Left = isNleftPts_1(pImg, conNum, tmpPt1, true, true);
	bool bPt1Righ = isNleftPts_1(pImg, conNum, tmpPt1, false, true);
	bool bPt2Left = isNleftPts_1(pImg, conNum, tmpPt2, true, true);
	bool bPt2righ = isNleftPts_1(pImg, conNum, tmpPt2, false, true);
	bool bPt3Left = isNleftPts_1(pImg, conNum, tmpPt3, true, true);
	bool bPt4Left = isNleftPts_1(pImg, conNum, tmpPt4, true, true);
	bool bPt5Left = isNleftPts_1(pImg, conNum, tmpPt5, true, true);
	bool bPt6Left = isNleftPts_1(pImg, conNum, tmpPt6, true, true);
	bool bPt7Left = isNleftPts_1(pImg, conNum, tmpPt7, true, true);
	bool bPt8Left = isNleftPts_1(pImg, conNum, tmpPt8, true, true);


	//...010...
	if (pImg[pt.y * cols + pt.x] == 255 && pImg[pt.y * cols + pt.x - 1] == 0 &&
		pImg[pt.y * cols + pt.x + 1] == 0)
	{
		if (bPt1Left || bPt1Righ || bPt2Left || bPt2righ || bPt7Left || bPt8Left)
		{
			bFlag = 1;
		}
	}

	//...0110...
	if (pImg[pt.y * cols + pt.x] == 255 && pImg[pt.y * cols + pt.x + 1] == 255 &&
		pImg[pt.y * cols + pt.x - 1] == 0 && pImg[pt.y * cols + pt.x + 2] == 0)
	{
		if (bPt1Left || bPt1Righ || bPt2righ || bPt2Left || bPt3Left || bPt4Left || bPt7Left || bPt8Left)
		{
			bFlag = 2;
		}
	}

	//...01110...
	if (pImg[pt.y * cols + pt.x] == 255 && pImg[pt.y * cols + pt.x + 1] == 255 &&
		pImg[pt.y * cols + pt.x + 2] == 255 &&
		pImg[pt.y * cols + pt.x - 1] == 0 && pImg[pt.y * cols + pt.x + 3] == 0)
	{
		if (bPt1Left || bPt1Righ || bPt2righ || bPt2Left || bPt3Left || bPt4Left || bPt5Left || bPt6Left
			|| (pt.y - 2 >= 0 && pt.y + 2 < cols && bPt7Left || bPt8Left))
		{
			bFlag = 3;
		}
	}

	return bFlag;
}

bool ExtractLines::isNleftPts_1(uchar* pImg, int num, mPoint pt, bool bLeft, bool bFrontVal)
{
	/***make sure have at least 'num' points on the left or right direction at the point 'pt' ***/
	assert(pImg != NULL);

	bool bRes = true;
	int index = 1;
	int pixelVal = bFrontVal ? 255 : 0;
	int invVal = abs(pixelVal - 255);

	int x;
	while (index <= num)
	{
		if (bLeft)
			x = pt.x - index;
		else
			x = pt.x + index;

		if (x < cols && x >= 0
			&& pt.y >= 0 && pt.y < rows
			&& pImg[pt.y * cols + x] == invVal)
		{
			bRes = false;
			return bRes;
		}
		index++;
	}

	return bRes;
}

int ExtractLines::BreakPoint(uchar* pImg, mPoint pt)
{
	assert(pImg != NULL && pt.y < rows  && pt.x < cols);

	int nRes = 0;//0:not break point; 1:left break point 2:right break point

	if (pt.x + 1 < cols - 1 && pt.y < rows - 1 &&
		pImg[pt.y * cols + pt.x] == 255 &&
		pImg[pt.y * cols + pt.x + 1] == 0)
	{
		//1 -> 0
		if (pt.x - NUM_CON_PTS_HORIZ > 0)
		{
			//judge on the left at least NUM_CON_PTS_HORIZ points are 1 and on the right at most NUM _BR_PTS_HORIZ points are 0
			if (isNleftPts_1(pImg, NUM_CON_PTS_HORIZ, pt, true, true) &&
				atLeastOnebreakPts(pImg, NUM_BRK_PTS_HORIZ, pt, false, false))
			{
				nRes = 1;
			}
		}
	}
	else if (pt.x + 1 < cols - 1 && pt.y < rows - 1 &&
		pImg[pt.y * cols + pt.x] == 0 &&
		pImg[pt.y * cols + pt.x + 1] == 255)
	{
		//0 -> 1
		if (pt.x + NUM_CON_PTS_HORIZ < cols - 1)
		{
			// judge on the right at least NUM_CON_PTS_HORIZ points are 1 and  on the left at most NUM _BR_PTS_HORIZ points are 0
			if (isNleftPts_1(pImg, NUM_CON_PTS_HORIZ, pt, false, true) &&
				atLeastOnebreakPts(pImg, NUM_BRK_PTS_HORIZ, pt, true, false))
			{
				nRes = 2;
			}
		}

	}

	return nRes;
}

bool ExtractLines::atLeastOnebreakPts(uchar* pImg, int num, mPoint pt, bool bLeft, bool bFrontVal)
{
	/***detect num points, make sure at least one point is background***/
	assert(pImg != NULL);
	bool bRes = false;
	int index = 1;
	int pixelVal = bFrontVal ? 255 : 0;
	int nBrkPts = 0;
	int x;

	while (index <= num)
	{
		if (bLeft)
			x = pt.x - index;
		else
			x = pt.x + index;

		if (x < 0)
			break;

		if (x < cols - 1 && pt.y < rows - 1 && pImg[pt.y * cols + x] == pixelVal)
		{
			nBrkPts = index;
			bRes = true;
		}
		index++;
	}

	return bRes;
}

void ExtractLines::pad_hole(uchar* pImg, mPoint pt)
{
	unsigned int maxConBrkNum = 5;
	bool bRes = true;;
	bool bUp = true;
	bool bDown = true;
	unsigned int brkNum = 0;

	if (pImg[pt.y * cols + pt.x] == 255 && pImg[pt.y * cols + pt.x + 1] == 0)
	{
		for (; brkNum < maxConBrkNum; brkNum++)
		{
			if (pt.x + brkNum < cols && pImg[pt.y * cols + pt.x + brkNum + 1] != 0)
				break;
		}
	}
	else
	{
		return;
	}

	if (brkNum >= 1)
	{
		for (int i = 1; i <= brkNum; ++i)
		{
			bUp = pImg[(pt.y - 1) * cols + pt.x + i] == 255 ? true : false;
			bDown = pImg[(pt.y + 1) * cols + pt.x + i] == 255 ? true : false;
			bRes = bUp && bDown;

			if (!bRes)
			{
				return;
			}
		}
	}

	if (bRes)
	{
		for (int i = 0; i < brkNum; ++i)
		{
			pImg[pt.y * cols + pt.x + i + 1] = 255;
		}
	}

}

void ExtractLines::makeImgThinner(const int maxIterations)
{
	/***Extract a binarized image's skeleton***/

	int width = cols;
	int height = rows;

	memcpy(pCpyImg, pGrdImg, rows * cols * sizeof(uchar));
	for (int i = 0; i < rows * cols; ++i)
	{
		pCpyImg[i] /= 255;
	}


	int count = 0;
	uchar p1, p2, p3, p4, p5, p6, p7, p8, p9;

	//uchar** pChar = NULL;

	int ptrIndex = 0;
	while (true)
	{
		count++;
		if (maxIterations != -1 && count > maxIterations)
			break;

		for (int i = 0; i < height; ++i)
		{
			uchar* p = &(pCpyImg[i * cols]);
			assert(p != NULL);
			for (int j = 0; j < width; ++j)
			{
				p1 = p[j];
				if (p1 != 1)
					continue;

				p4 = (j == width - 1) ? 0 : pCpyImg[i* cols + j + 1];
				p8 = (j == 0) ? 0 : pCpyImg[i * cols + j - 1];
				p2 = (i == 0) ? 0 : pCpyImg[(i - 1) * cols + j];
				p3 = (i == 0 || j == width - 1) ? 0 : pCpyImg[(i - 1) * cols + j + 1];
				p9 = (i == 0 || j == 0) ? 0 : pCpyImg[(i - 1) * cols + j - 1];
				p6 = (i == height - 1) ? 0 : pCpyImg[(i + 1) * cols + j];
				p5 = (i == height - 1 || j == width - 1) ? 0 : pCpyImg[(i + 1) * cols + j + 1];
				p7 = (i == height - 1 || j == 0) ? 0 : pCpyImg[(i + 1) * cols + j - 1];

				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
				{
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1 && p2 * p4 * p6 == 0 && p4 * p6 * p8 == 0)
					{
						pChar[ptrIndex++] = p + j;
					}
				}
			}
		}

		for (int i = 0; i < ptrIndex; i++)
		{
			*pChar[i] = 0;
		}

		if (ptrIndex == 0)
		{
			break;
		}
		else
		{
			ptrIndex = 0;
		}


		for (int i = 0; i < height; ++i)
		{
			uchar * p = &(pCpyImg[i * cols]);
			for (int j = 0; j < width; ++j)
			{
				//  p9 p2 p3  
				//  p8 p1 p4  
				//  p7 p6 p5  
				p1 = p[j];
				if (p1 != 1) continue;
				p4 = (j == width - 1) ? 0 : pCpyImg[i * cols + j + 1];
				p8 = (j == 0) ? 0 : pCpyImg[i * cols + j - 1];
				p2 = (i == 0) ? 0 : pCpyImg[(i - 1) * cols + j];
				p3 = (i == 0 || j == width - 1) ? 0 : pCpyImg[(i - 1) * cols + j + 1];
				p9 = (i == 0 || j == 0) ? 0 : pCpyImg[(i - 1) * cols + j - 1];
				p6 = (i == height - 1) ? 0 : pCpyImg[(i + 1) * cols + j];
				p5 = (i == height - 1 || j == width - 1) ? 0 : pCpyImg[(i + 1) * cols + j + 1];
				p7 = (i == height - 1 || j == 0) ? 0 : pCpyImg[(i + 1) * cols + j - 1];

				if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) >= 2 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) <= 6)
				{
					int ap = 0;
					if (p2 == 0 && p3 == 1) ++ap;
					if (p3 == 0 && p4 == 1) ++ap;
					if (p4 == 0 && p5 == 1) ++ap;
					if (p5 == 0 && p6 == 1) ++ap;
					if (p6 == 0 && p7 == 1) ++ap;
					if (p7 == 0 && p8 == 1) ++ap;
					if (p8 == 0 && p9 == 1) ++ap;
					if (p9 == 0 && p2 == 1) ++ap;

					if (ap == 1 && p2 * p4 * p8 == 0 && p2 * p6 * p8 == 0)
					{
						pChar[ptrIndex++] = p + j;
					}
				}
			}
		}

		for (int i = 0; i < ptrIndex; i++)
		{
			*pChar[i] = 0;
		}

		if (ptrIndex == 0)
		{
			break;
		}
		else
		{
			ptrIndex = 0;
		}
	}


	for (int i = 0; i < rows * cols; ++i)
	{
		pCpyImg[i] *= 255;
	}
}

void ExtractLines::findLines(uchar* pImg, mLines& sLines)
{
	assert(pImg != NULL);

	mPoint p1, p2, p3, p4, p5, p6, p7, p8;
	p1.x = -1; p1.y = -1;
	p2.x = 0; p2.y = -1;
	p3.x = 1; p3.y = -1;
	p4.x = 1; p4.y = 0;
	p5.x = 1; p5.y = 1;
	p6.x = 0; p6.y = 1;
	p7.x = -1; p7.y = 1;
	p8.x = -1; p8.y = 0;
	mPoint neighborPoints[8] = { p1, p2, p3, p4, p5, p6, p7, p8 };

	mPoint first_point;
	mPoint next_point;
	mPoint this_point;
	int this_flag;
	mPoint backArr[1024];
	mPoint frontArr[1024];

	mPoint linePoints[LINE_MAX_POINTS];
	mPoint tmpPoint;
	tmpPoint.x = -1;
	tmpPoint.y = -1;
	unsigned int backCnt = 0;
	unsigned int frontCnt = 0;
	int num = 0;

	while (findFirstPoint(pImg, first_point))
	{
		backArr[backCnt++] = first_point;
		this_point = first_point;
		this_flag = 0;

		int next_flag;
		//find in one direction
		while (findNextPoint(neighborPoints, pImg,
			this_point, this_flag, next_point, next_flag))
		{
			backArr[backCnt++] = next_point;
			this_point = next_point;
			this_flag = next_flag;
		}
		//search in the other direction
		this_point = first_point;
		this_flag = 0;
		while (findNextPoint(neighborPoints, pImg,
			this_point, this_flag, next_point, next_flag))
		{
			frontArr[frontCnt++] = next_point;
			this_point = next_point;
			this_flag = next_flag;
		}

		//merge front and back array
		if (backArr[0].x <= frontArr[0].x && backArr[backCnt - 1].x <= frontArr[0].x)
		{
			for (int i = 0; i < backCnt; ++i)
			{
				if (backArr[backCnt - 1].x < backArr[0].x)
					linePoints[i] = backArr[backCnt - 1 - i];
				else
					linePoints[i] = backArr[i];
			}

			for (int i = 0; i < frontCnt; ++i)
			{
				if (frontArr[0].x < frontArr[frontCnt - 1].x)
					linePoints[backCnt + i] = frontArr[i];
				else
					linePoints[backCnt + i] = frontArr[frontCnt - 1 - i];
			}
		}
		else
		{
			for (int i = 0; i < frontCnt; ++i)
			{
				if (frontArr[0].x < frontArr[frontCnt - 1].x)
					linePoints[i] = frontArr[i];
				else
					linePoints[i] = frontArr[frontCnt - 1 - i];
			}

			for (int i = 0; i < backCnt; ++i)
			{
				if (backArr[backCnt - 1].x < backArr[0].x)
					linePoints[frontCnt + i] = backArr[backCnt - 1 - i];
				else
					linePoints[frontCnt + i] = backArr[i];
			}
		}


		if (frontCnt + backCnt > LINE_LEAST_POINTS)
		{
			for (int i = 0; i < LINE_MAX_POINTS; ++i)
			{
				if (i < frontCnt + backCnt)
				{
					sLines.linesSets[num * LINE_MAX_POINTS + i] = linePoints[i];
				}
				else
				{
					sLines.linesSets[num * LINE_MAX_POINTS + i] = tmpPoint;
				}

			}

			sLines.linesSize[num] = frontCnt + backCnt;
			num++;
		}

		backCnt = 0;
		frontCnt = 0;
	}

	sLines.linesNum = num;
}

bool ExtractLines::findFirstPoint(uchar* pImg, mPoint &outputPoint)
{
	assert(pImg != NULL);
	uchar* data = NULL;

	bool success = false;
	for (int i = 0; i < rows; i++)
	{
		data = &pImg[i * cols];
		for (int j = 0; j < cols; j++)
		{
			if (data[j] == 255)
			{
				success = true;
				outputPoint.x = j;
				outputPoint.y = i;
				data[j] = 0;
				break;
			}
		}
		if (success)
			break;
	}

	return success;
}

bool ExtractLines::findNextPoint(mPoint _neighbor_points[], uchar* pImg,
	mPoint _inpoint, int flag, mPoint& _outpoint, int &_outflag)
{
	assert(pImg != NULL);

	int i = flag;
	int count = 1;
	bool success = false;
	mPoint tmppoint;

	while (count <= 7)
	{
		tmppoint.x = _inpoint.x + _neighbor_points[i].x;
		tmppoint.y = _inpoint.y + _neighbor_points[i].y;

		if (tmppoint.x > 0 && tmppoint.y > 0 && tmppoint.x < cols &&tmppoint.y < rows)
		{
			if (pImg[tmppoint.y * cols + tmppoint.x] == 255)
			{
				_outpoint = tmppoint;
				_outflag = i;
				success = true;
				pImg[tmppoint.y * cols + tmppoint.x] = 0;
				break;
			}
		}
		if (count % 2)
		{
			i += count;
			if (i > 7)
			{
				i -= 8;
			}
		}
		else
		{
			i += -count;
			if (i < 0)
			{
				i += 8;
			}
		}
		count++;
	}
	return success;
}

