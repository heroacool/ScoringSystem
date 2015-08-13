/*
Copyright(c) 2015, Fang Li
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met :

*Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and / or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


anthor:	Fangli
e-mail: fangli0620@gmail.com

*/


#ifndef _SCORE_ENGINE_H_
#define  _SCORE_ENGINE_H_


#include <iostream>
#include <vector>
#include <fstream>
#include <io.h>
#include <stdlib.h>
#include <ctime>
#include <stdio.h>
#include <set>


#include <direct.h>
#include <opencv2/opencv.hpp>


//#include "opencv2/line_descriptor.hpp"

#include "opencv2/core/utility.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>


using namespace std;
using namespace cv;
//using namespace cv::line_descriptor;



class ScoreEngine{

public:

	ScoreEngine();
	~ScoreEngine();


	/*
	author: FL, fangli0620@163.com
	time: 2015年6月22日14:46:28
	version: 1.1
	log:解决x64下不能用的bug

	function: 拿到指定文件夹下的文件
	*/
	void getAllFiles(string path, vector<string>& files);


	/*
	func:	识别试卷考号

	input_src: 待识别考号的试卷
	search_rect: 包含考号区域的矩形
	input_len: 学生考号的位数
	result_id: 识别出的考号结果
	debug_flag: 调试标志位

	return:	0，正常, result_id 为正常值
			-1，图片异常，包含图片错误，识别出的id数字个数和传入的input_len不相等，
			-2，有一列考号没有涂抹，result_id = "U"， 请外部人工check
			-3，有一列考号，被涂抹了多次, result_id = "M"， 请外部分工check

	version: 1.0	2015年7月6日12:59:05
	log: 这个版本的每个数字的比率没有归一化，用单纯的阈值虚警率有点高

	version: 1.1	2015年7月11日18:35:30
	log: 

	*/
	int scanSpottedID(const Mat & input_src, const Rect & search_rect, int input_len, char result_id[32], int * output_len, int debug_flag);



	/*
	function: 识别选择题目
	result：各题目的识别结果，计算方式如下：
	A选择为1，不选择为0；
	B选择为2，不选择为0；
	C选择为4，不选择为0；
	D选择为8，不选择为0
	result = A + B + C + D
	例如：result = 8，表示选择了D；result = 6，表示选择了B和C。

	return value: 0, normal, 
				  -1, please check!	
	
	*/
	//int scanScoreOptionpanel_H(const Mat & input_src, const Rect & search_rect, int item_num, vector<int> & result, int debug_flag);

	int scanScoreOptionpanel_H(const Mat & input_src, const Rect & search_rect, int exclusiveRegion[4], int item_num, int result[256], int *ques_num, int multi_flag = 0, int debug_flag = 1);



// 	int scanScoreOptionPanel_H(
// 	unsigned char* grayImgLine, //源图像
// 	int width, //源图像宽
// 	int height, //源图像高
// 	int roiRegion[4], //感兴趣区域
// 	int exclusiveRegion0[4], //暂且不用
// 	int itemNum, //总共有多少题
// 	int *result, //如上
// 	int *quesNum //总共识别出了多少题
// 	);

	/*
	function: 识别分数框的分数

	input_src: 输入的图片
	search_rect: 分数框必须在此矩形中
	result_score: 考虑到调试，返回值确认为vector<int> 应该为22位，每一位为0时，代表没有被划中，若为1，则代表被划中; 若为-1，则代表无法判断属于
	debug_flag: 中间结果显示

	return value: 0，正常
				  -1， 图片异常，或者没有找到22位数字框
				  -2, 需要人工check
	*/
	int scanScoreSquare(const Mat & input_src, const Rect & search_rect, int result_score[22], int debug_flag);

// 	int  scanScoreBar
// 		(
// 		unsigned char* grayImgLine, //源图像
// 		int width, //源图像宽
// 		int height, //源图像高
// 		int roiRegion[4], //感兴趣区域
// 		short barType, //数字框的类型，暂且不用
// 		short* headMark, //输出识别的第一个空格有没有划过
// 		short* outputPrime,//输出识别结果整数位
// 		short* outputDecimal //输出识别结果小数位（5）
// 		);

	int scanScoreBar
		(
		const Mat & input_src, 
		const Rect & search_rect, 
		short bar_type,		//数字框的类型，暂且不用
		short * head_mark,	//输出识别的第一个空格有没有划过, 0,没有划过；1划过了
		short * output_prime, //输出识别结果整数位, 测试期间可能会越界
		short * output_decimal,//输出识别结果小数位（5），0，没有；5，表示0.5被划过
		short * prime_num
		);

	


	/*
	func:	找到4个标记，返回4，
			找到3个标记，放回3，
			找到2个标记，返回2，以此类推到1、0
			初始化失败（没有找到标准的标记点），或者输入图片不是灰度图，返回-1
	input_src: 输入图片，必须是单通道，即灰度图，若该不是则返回-1
	out_dst: 输出图片，也是单通道
	
	*/
	int alignImage(const Mat & input_src, Mat & output_dst);

	
	
	//用最小二乘法拟合,opencv 的矩阵运算比较慢，不过我们这个矩阵比较小，
	//暂时不用考虑效率，实在要求，可以用armodillo替代矩阵预算
	int getHomoMatrix(const Point2f * src_points, const Point2f * dst_points, const int n, Mat & homo);
	

	string recognizeBarcode(const Mat & src);

	Point2f getStandardPoint(int i);


private:

	//定位模板
	const int template_r = 6;
	Mat templ;

	bool init_is_ready ;

	//标准位置，所有的标记点都要对齐到这个坐标
	Point2f standard_mark_points[4];

	// 当坐标为负数时，表示没有定位成功，表示废弃的点
	Point2f marked_points[4];

	//在使用这个类之前要初始化这个
	int initStandardMarkedPoints(const Mat & input_src);

	//得到标记点的坐标,顺序如下
	//0  1
	//3  2
	int getMarkedPoints(const Mat & input_src, const int search_x, const int search_y);



	bool checkDelta(const vector<double> & value, double precision);
	static double compareRect(const Rect & r1, const Rect & r2);
	double compareRect1(const Rect & r1, const Rect & r2);
	static bool sort_y_rect(const vector<Rect> & rects1, const vector<Rect> & rects2);
	static bool sort_x_rect(const Rect & rect1, const Rect & rect2);


	//返回0，代表成功
	//返回1，代表
	//返回2，代表
	int eraseOutline(const Mat & src, Mat & dst, const int gray_threshold, int debug_flag);

	int getStandardRect(const Mat & input_src, Rect & standard_rect);

	vector<Vec6i> getConnectedRegion(const Mat & src, int y_index);

	void removeConnectedRegion(vector<Vec6i> & connect_region, double xs[23], double radius = 1.0);
	void setFlag(vector<Vec6i> & connect_region, double xs[23], bool flag[22]);
	bool contourConfirm(const Mat & src_input, int margin_threshold = 1);

	int getUpDownRect(const Mat & input_src, int gray_threshold, Rect & first_rect, Rect & second_rect);	//return 0, normal, -1,abnormal

	int myOtsu(const Mat & src);
	int myOtsu(const IplImage *frame);


	//2015年8月10日10:30:45
	int getStandardRect1(const Mat & input_src, Rect & standard_rect);


	//没有补偿返回0
	//补偿返回1
	int compensateRect(vector<Rect> & rects, const Rect  & current_rect, const Rect & standard_rect, const double & delta, const Mat & src);

	
};



#endif // !_SCORE_ENGINE_H_
