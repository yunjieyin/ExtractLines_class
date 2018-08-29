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

	for (int x = 0; x < cols; x = x + 1)
	{
		for (int y = 0; y < rows; y = y + 1)
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

			//if (pt.x + nStride <= cols - 1)
			//{
			//	dx = 0;// pSrcImg[cols * pt.y + pt.x + nStride] - pSrcImg[cols * pt.y + pt.x];
			//}
			//else
			//{
			//	dx = 0;
			//}

			if (pt.y + nStride <= rows - 1 && pSrcImg[cols * (pt.y + nStride) + pt.x] > pSrcImg[cols * pt.y + pt.x])
			{
				grade =  dy;
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

void ExtractLines::del_biforked_lines(uchar* pThin, int x, int y, bool bLeft)
{
	assert(x >= 0 && y >= 0);
	const int nSize = 1024;
	mPoint p1, p2;
	mPoint p1LU, p1LM, p1LB;
	mPoint p2LU, p2LM, p2LB;
	mPoint arr1[nSize], arr2[nSize];
	mPoint *ptr1 = arr1, *ptr2 = arr2;
	bool bStop1 = true;
	bool bStop2 = true;
	bool bF1 = false, bF2 = false, bF3 = false;
	int r = bLeft ? 1 : -1;

	p1.x = x, p1.y = y - 1;
	p2.x = x, p2.y = y + 1;
	*(ptr1++) = p1;
	*(ptr2++) = p2;

	p1LU.x = p1.x - 1 * (r), p1LU.y = p1.y - 2;
	p1LM.x = p1.x - 1 * (r), p1LM.y = p1.y - 1;
	p1LB.x = p1.x - 1 * (r), p1LB.y = p1.y;

	p2LU.x = p2.x - 1 * (r), p2LU.y = p2.y;
	p2LM.x = p2.x - 1 * (r), p2LM.y = p2.y + 1;
	p2LB.x = p2.x - 1 * (r), p2LB.y = p2.y + 2;

	while ((p1LU.y >= 0 && p1LB.y < rows && p1LU.x >= 0 && p1LU.x < cols)
		&&(pThin[p1LU.y * cols + p1LU.x] == 255
		|| pThin[p1LM.y * cols + p1LM.x] == 255
		|| pThin[p1LB.y * cols + p1LB.x] == 255))
	{
		if (bStop1)
		{
			bF1 = false, bF2 = false, bF3 = false;
			
			if (pThin[p1LB.y * cols + p1LB.x] == 255)
			{
				p1 = p1LB;
				*(ptr1++) = p1;
				bF3 = true;
			}

			if (pThin[p1LM.y * cols + p1LM.x] == 255)
			{
				p1 = p1LM;
				*(ptr1++) = p1;
				bF2 = true;
			}
			

			if (pThin[p1LU.y * cols + p1LU.x] == 255)
			{
				p1 = p1LU;
				*(ptr1++) = p1;
				bF1 = true;
			}

			if(!bF1 && !bF2 && !bF3)
			{
				bStop1 = false;
			}
		}

		p1LU.x = p1.x - 1 * (r), p1LU.y = p1.y - 1;
		p1LM.x = p1.x - 1 * (r), p1LM.y = p1.y ;
		p1LB.x = p1.x - 1 * (r), p1LB.y = p1.y + 1;
	}

	while ((p2LU.y >= 0 && p2LB.y < rows && p2LU.x >=0 && p2LB.x < cols)
		&& (pThin[p2LU.y * cols + p2LU.x] == 255
		|| pThin[p2LM.y * cols + p2LM.x] == 255
		|| pThin[p2LB.y * cols + p2LB.x] == 255))
	{
		if (bStop2)
		{
			bF1 = false, bF2 = false, bF3 = false;
			if (pThin[p2LU.y * cols + p2LU.x] == 255)
			{
				p2 = p2LU;
				*(ptr2++) = p2;
				bF1 = true;
			}
			if (pThin[p2LM.y * cols + p2LM.x] == 255)
			{
				p2 = p2LM;
				*(ptr2++) = p2;
				bF2 = true;
			}
			if (pThin[p2LB.y * cols + p2LB.x] == 255)
			{
				p2 = p2LB;
				*(ptr2++) = p2;
				bF3 = true;
			}

			if(!bF1 && !bF2 && !bF3)
			{
				bStop2 = false;
			}
		}

		p2LU.x = p2.x - 1 * (r), p2LU.y = p2.y - 1;
		p2LM.x = p2.x - 1 * (r), p2LM.y = p2.y ;
		p2LB.x = p2.x - 1 * (r), p2LB.y = p2.y + 1;
	}

	if (abs(ptr1 - arr1) > abs(ptr2 - arr2))
	{
		for (mPoint* ptr = arr2; ptr != ptr2; ++ptr)
		{
			pThin[(*ptr).y * cols + (*ptr).x] = 0;
		}
	}
	else
	{
		for (mPoint* ptr = arr1; ptr != ptr1; ++ptr)
		{
			pThin[(*ptr).y * cols + (*ptr).x] = 0;
		}
	}
}

void ExtractLines::wipe_singular_points(uchar* pThin)
{
	//del illegal points
	int upNum;
	int botNum;
	int sumNei = 0;
	unsigned int val0, val1, val2, val3, val4, val5, val6, val7, val8;
	unsigned int idxCen, idxUL, idxU, idxUR, idxL, idxR, idxBL, idxB, idxBR;
	bool f1, f2, f3, f4;
	const int lagVal = 3;
	int nLeft = 0;
	int nRigh = 0;

	for (int y = 1; y < rows - 2; ++y)
	{
		for (int x = 1; x < cols - 2; ++x)
		{
			idxCen = y * cols + x;
			idxUL = (y - 1) * cols + x - 1;
			idxU = (y - 1) * cols + x;
			idxUR = (y - 1) * cols + x + 1;
			idxL = y * cols + x - 1;
			idxR = y * cols + x + 1;
			idxBL = (y + 1) * cols + x - 1;
			idxB = (y + 1) * cols + x;
			idxBR = (y + 1) * cols + x + 1;

			val0 = pThin[idxCen];
			val1 = pThin[idxUL];
			val2 = pThin[idxU];
			val3 = pThin[idxUR];
			val4 = pThin[idxR];
			val5 = pThin[idxBR];
			val6 = pThin[idxB];
			val7 = pThin[idxBL];
			val8 = pThin[idxL];
			sumNei = val1 + val2 + val3 + val4 +
					 val5 + val6 + val7 + val8;

			//del corner point
			//  1          1       0          0
			//1 1 0		 0 1 1   0 1 1      1 1 0
			//  0          0       1          1
			f1 = (val0 == 255 && val2 == 255 && val8 == 255)
				   && (val4 == 0 && val5 == 0 && val6 == 0);
			f2 = (val0 == 255 && val2 == 255 && val4 == 255)
				   && (val6 == 0 && val7 == 0 && val8 == 0);
			f3 = (val0 == 255 && val4 == 255 && val6 == 255)
				   && (val2 == 0 && val8 == 0 && val1 == 0);
			f4 = (val0 == 255 && val6 == 255 && val8 == 255)
				   && (val2 == 0 && val4 == 0 && val3 == 0);
		
			if (f1 || f2 || f3 || f4)
			{
				pThin[idxCen] = 0;
			}
		}
	}

	for (int y = 1; y < rows - 2; ++y)
	{
		for (int x = 1; x < cols - 2; ++x)
		{
			idxCen = y * cols + x;
			idxUL = (y - 1) * cols + x - 1;
			idxU = (y - 1) * cols + x;
			idxUR = (y - 1) * cols + x + 1;
			idxL = y * cols + x - 1;
			idxR = y * cols + x + 1;
			idxBL = (y + 1) * cols + x - 1;
			idxB = (y + 1) * cols + x;
			idxBR = (y + 1) * cols + x + 1;

			val0 = pThin[idxCen];
			val1 = pThin[idxUL];
			val2 = pThin[idxU];
			val3 = pThin[idxUR];
			val4 = pThin[idxR];
			val5 = pThin[idxBR];
			val6 = pThin[idxB];
			val7 = pThin[idxBL];
			val8 = pThin[idxL];
			sumNei = val1 + val2 + val3 + val4 +
				val5 + val6 + val7 + val8;

			// break forked path
			if (val0 == 255 && sumNei >= 255 * 3)
			{
				if (val2 == 0 && val4 == 255 && val6 == 255 && val8 == 255)
				{
					pThin[idxB] = 0;
				}
				else if (val6 == 0 && val2 == 255 && val4 == 255 && val8 == 255)
				{
					pThin[idxU] = 0;
				}

				nLeft = 0;
				nRigh = 0;
				for (int m = x - 2; m <= x + 2; m++)
				{
					if (m <= x - 1)
					{
						for (int n = y - 5; n <= y + 5; n++)
						{
							if (m > 0 && m < cols && n > 0 && n < rows && pThin[n * cols + m] == 255)
								nLeft++;
						}
					}

					if (m >= x + 1)
					{
						for (int n = y - 5; n <= y + 5; n++)
						{
							if (m > 0 && m < cols && n > 0 && n < rows && pThin[n * cols + m] == 255)
								nRigh++;
						}
					}
				}


				if (nLeft >= nRigh)
				{
					//left direction				
					del_biforked_lines(pThin, x, y, true);
				}
				else
				{
					//right direction
					del_biforked_lines(pThin, x, y, false);
				}
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
	unsigned int idxFront = 0;
	unsigned int idxBack = 0;
	unsigned int lineNum = 0;
	while (findFirstPoint(pCpyImg, seed))
	{
		mPoint lineDataArr[LINE_MAX_POINTS];
		mark_connect_region(pCpyImg, seed, lineDataArr, idxFront, idxBack);

		if (idxBack - idxFront + 1 >= LINE_LEAST_POINTS)
		{
			int cnt = 0;
			for (int idx = idxFront; idx <= idxBack; ++idx)
			{
				linesSet.linesSets[lineNum * LINE_MAX_POINTS + cnt] = lineDataArr[idx];
				cnt++;
			}
			linesSet.linesSize[lineNum] = cnt;
			lineNum++;
		}
	}
	
	linesSet.linesNum = lineNum;

	//convert test data
	for (int i = 0; i < linesSet.linesNum; ++i)
	{
		std::deque<mPoint> lineVec;
		for (int j = 0; j < linesSet.linesSize[i]; ++j)
		{
			lineVec.push_back(linesSet.linesSets[i * LINE_MAX_POINTS + j]);
		}

		markedLines.push_back(lineVec);
	}
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
	mark_lines();
}

void ExtractLines::mark_connect_region(uchar* pThin, mPoint pt, mPoint* pArr, unsigned int& idxFront, unsigned int& idxBack)
{
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

	idxBack = idxFront = int(LINE_MAX_POINTS / 2);
	pArr[idxBack] = pt;

	mPoint seed;
	mPoint tmpPt;
	int tmpx;
	int tmpy;

	while (!seeds.empty())
	{
		seed = seeds.top();
		seeds.pop();

		for (size_t i = 0; i < 8; i++)
		{
			tmpx = seed.x + connects[i].x;
			tmpy = seed.y + connects[i].y;

			if (tmpx < 0 || tmpy < 0 || tmpx >= cols || tmpy >= rows)
				continue;

			if (pThin[tmpy * cols + tmpx] != 0)
			{
				tmpPt.x = tmpx;
				tmpPt.y = tmpy;

				if (tmpPt.x >= pArr[idxBack].x && (++idxBack) < LINE_MAX_POINTS)
				{
					pArr[idxBack] = tmpPt;
				}
				else
				{
					assert(--idxFront >= 0);
					pArr[idxFront] = tmpPt;
				}

				pThin[tmpy * cols + tmpx] = 0;
				seeds.push(tmpPt);
			}

		}
	}


	pThin[pt.y * cols + pt.x] = 0;
}

void ExtractLines::grad_graph_binary()
{
	int sumNeig_9 = 0;
	const unsigned int neig9SumVal = 30;
	unsigned int idxCen, idxUL, idxU, idxUR, idxL, idxR, idxBL, idxB, idxBR;

	for (int y = 1; y < rows - 1; ++y)
	{
		for (int x = 1; x < cols - 1; ++x)
		{
			idxCen = y * cols + x;
			idxUL = (y - 1) * cols + x - 1;
			idxU = (y - 1) * cols + x;
			idxUR = (y - 1) * cols + x + 1;
			idxL = y * cols + x - 1;
			idxR = y * cols + x + 1;
			idxBL = (y + 1) * cols + x - 1;
			idxB = (y + 1) * cols + x;
			idxBR = (y + 1) * cols + x + 1;

			//消除4邻域为空的噪点 
			if (pGrdImg[idxCen] > 0 && pGrdImg[idxU] == 0
				&& pGrdImg[idxB] == 0 && pGrdImg[idxL] == 0 
				&& pGrdImg[idxR])
			{
				pGrdImg[idxCen] = 0;
			}

			//根据八邻域点的平均像素值把当前像素设为背景或前景点
			sumNeig_9 = pGrdImg[idxCen] + pGrdImg[idxUL] +
				pGrdImg[idxU] + pGrdImg[idxUR] +
				pGrdImg[idxL] + pGrdImg[idxR] +
				pGrdImg[idxBL] + pGrdImg[idxB] +
				pGrdImg[idxBR];

			if (sumNeig_9 / 9 < neig9SumVal)
			{
				pGrdImg[idxCen] = 0;
			}
			else if (pGrdImg[idxCen] > 0)
			{
				pGrdImg[idxCen] = 255;
			}
		}
	}
}

void ExtractLines::padding_points()
{
	int bkPtFlag = 0;
	int idxPt;
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
			idxPt = pt.y * cols + pt.x;

			//消除满足左右像素点均为背景点的若干点（3）  
			if (isNisolatePoints(pGrdImg, pt) == 1)
			{
				pGrdImg[idxPt] = 0;
			}
			else if (isNisolatePoints(pGrdImg, pt) == 2)
			{
				pGrdImg[idxPt] = 0;
				pGrdImg[idxPt + 1] = 0;
			}
			else if (isNisolatePoints(pGrdImg, pt) == 3)
			{
				pGrdImg[idxPt] = 0;
				pGrdImg[idxPt + 1] = 0;
				pGrdImg[idxPt + 2] = 0;
			}

			/*补水平方向0的点...1***1...*/
			if ((x + 4 < cols && pGrdImg[idxPt] == 255 && pGrdImg[idxPt + 4] == 255)
				&&(pGrdImg[idxPt + 1] == 0 || pGrdImg[idxPt + 2] == 0 || pGrdImg[idxPt + 3] == 0))
			{
				pGrdImg[idxPt + 1] = 255;
				pGrdImg[idxPt + 2] = 255;
				pGrdImg[idxPt + 3] = 255;
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
	int idxPt = pt.y * cols + pt.x;

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
	if (pImg[idxPt] == 255 && pImg[idxPt - 1] == 0 && pImg[idxPt + 1] == 0)
	{
		if (bPt1Left || bPt1Righ || bPt2Left || bPt2righ || bPt7Left || bPt8Left)
		{
			bFlag = 1;
		}
	}

	//...0110...
	if (pImg[idxPt] == 255 && pImg[idxPt + 1] == 255
		&& pImg[idxPt - 1] == 0 && pImg[idxPt + 2] == 0)
	{
		if (bPt1Left || bPt1Righ || bPt2righ || bPt2Left || bPt3Left || bPt4Left || bPt7Left || bPt8Left)
		{
			bFlag = 2;
		}
	}

	//...01110...
	if (pImg[idxPt] == 255 && pImg[idxPt + 1] == 255 && pImg[idxPt + 2] == 255
		&& pImg[idxPt - 1] == 0 && pImg[idxPt + 3] == 0)
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
	int idxPt = pt.y * cols + pt.x;

	if (pt.x + 1 < cols - 1 && pt.y < rows - 1 &&
		pImg[idxPt] == 255 && pImg[idxPt + 1] == 0)
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
			pImg[idxPt] == 0 && pImg[idxPt + 1] == 255)
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


