
#include <stdio.h>
#include <opencv/cv.h>
#include <opencv2/opencv.hpp>
#include <highgui.hpp>
#include <vector>
#include <assert.h>
#include <numeric>
#include "directory.h"
#include <stack>

#define NUM_BRK_PTS_HORIZ 10
#define NUM_CON_PTS_HORIZ 15
#define LINE_MAX_POINTS 2048
#define LINE_MAX_NUMS 64
#define LINE_LEAST_POINTS 10
#define GRAD_THRESH_VAL 15

typedef unsigned char uchar;
typedef struct _imgSize
{
	unsigned int rows;
	unsigned int cols;
} mSize;

typedef struct _point
{
	int x;
	int y;
} mPoint;

typedef struct _lines
{
	mPoint linesSets[LINE_MAX_POINTS * LINE_MAX_NUMS];
	unsigned int linesNum = 0;
	int linesSize[LINE_MAX_NUMS] = { 0 };
} mLines;


class ExtractLines
{
public:
	ExtractLines(uchar* pImg, mSize imgSize);
	~ExtractLines();
	void linesData();

private:
	void filterPixels();
	void makeImgThinner(const int maxIterations = -1);

	void findLines(uchar* pImg, mLines& sLines);
	bool findNextPoint(mPoint _neighbor_points[], uchar* pImg,
		mPoint _inpoint, int flag, mPoint& _outpoint, int &_outflag);
	bool findFirstPoint(uchar* pImg, mPoint &outputPoint);

	bool isNleftPts_1(uchar* pImg, int num, mPoint pt, bool bLeft, bool bFrontVal);
	bool atLeastOnebreakPts(uchar* pImg, int num, mPoint pt, bool bLeft, bool bFrontVal);
	int BreakPoint(uchar* pImg, mPoint pt);
	int isNisolatePoints(uchar* pImg, mPoint pt);
	void pad_hole(uchar* pImg, mPoint pt);
	void mark_connect_region(uchar* pThin, mPoint pt, mPoint* pArr, unsigned int& idxFront, unsigned int& idxBack);

	int pixelGrade(uchar* pImg, mPoint pt);
	void gradGraph();
	void mark_lines();
	void grad_graph_binary();
	void padding_points();
	void wipe_singular_points(uchar* pThin);

public:
	mSize srcImgSize;
	mLines linesSet;
	unsigned int rows;
	unsigned int cols;

	std::vector<std::deque<mPoint>> markedLines;

public:
	uchar* pIndex = NULL;
	uchar* pSrcImg = NULL;
	uchar* pGrdImg = NULL;
	uchar* pCpyImg = NULL;
	uchar* pThinImg = NULL;
	uchar** pChar = NULL;

};
