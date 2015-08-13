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



#include "ScoreEngine.h"



int bar_table[22] = { -1, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 5 };

typedef struct  t_Rects
{
	vector<Rect> rects;
	double average = 0;
}Rects;


//构造函数
ScoreEngine::ScoreEngine(){

	templ.create(template_r * 2, template_r * 2, CV_8UC1);
	templ.setTo(Scalar::all(255));

	Rect roi_rect(0, 0, template_r, template_r);
	templ(roi_rect).setTo(Scalar::all(0));

	roi_rect.x = template_r;
	roi_rect.y = template_r;

	templ(roi_rect).setTo(Scalar::all(0));

	init_is_ready = false;

}



//析构函数
ScoreEngine::~ScoreEngine(){

}


//先按照个数排序
//个数相等的情形下，按照y的坐标排序
bool ScoreEngine::sort_y_rect(const vector<Rect> & rects1, const vector<Rect> & rects2){

	double sum1 = 0;
	double sum2 = 0;

	for (int i = 0; i < rects1.size(); i++)
		sum1 += rects1[i].y;

	for (int i = 0; i < rects2.size(); i++)
		sum2 += rects2[i].y;

	if (rects1.size() == 0){
		sum1 = 0;
	}
	else{
		sum1 = sum1 / rects1.size();
	}


	if (rects2.size() == 0){
		sum2 = 0;
	}
	else {
		sum2 = sum2 / rects2.size();
	}

	if (rects1.size() > rects2.size()){
		return true;
	}
	else if (rects1.size() == rects2.size()){
		if (sum1 > sum2){
			return true;
		}

	}

	return false;

}


bool ScoreEngine::sort_x_rect(const Rect & rect1, const Rect & rect2){

	return rect1.x < rect2.x;

}




void ScoreEngine::getAllFiles(string path, vector<string>& files){
	_finddata_t fileDir;
	string dir = path;
	dir = dir + "*.jpg";
	//char* dir = "./fangli/*.txt";
	intptr_t  lfDir;

	if ((lfDir = _findfirst(dir.c_str(), &fileDir)) == -1L)
		printf("No file is found\n");
	else{
		//printf("file list:\n");
		do{
			//printf("%s\n", fileDir.name);
			char buffer[256];
			sprintf_s(buffer, "%s", fileDir.name);
			string ss(buffer);
			files.push_back(ss);
		} while (_findnext(lfDir, &fileDir) == 0);
	}
	_findclose(lfDir);
}

/*
	function: erase the outline of the input image

	src: input image, gray, the square is white, the other is black
	dst: output image, gray
	gray_threshold: binay threshold
	debug_flag: display debug image

	return: 0, no outline, just binary; -1, find outline and erase; -2, src is empty or channel error

	version: 1.2,
	log: 将二值化的功能改为了opencv自带的大津法

	
*/
int ScoreEngine::eraseOutline(const Mat & src, Mat & dst, const int gray_threshold, int debug_flag){

	if (src.empty()){
		return -2;
	}

	Mat src_binary;

	if (src.channels() == 1){
		src_binary = src.clone();
	}
	else if (src.channels() == 3){
		cvtColor(src, src_binary, CV_BGR2GRAY);
	}
	else{
		return -2;
	}

	dst = src_binary.clone();

	threshold(src_binary, src_binary, gray_threshold, 255, CV_THRESH_BINARY);

// 	blur(src_binary, src_binary, Size(2, 2));
// 
// 	threshold(src_binary, src_binary, 2, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);

	//imshow("src_binary", src_binary);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(src_binary, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));

	bool outline_flag = false;
	for (int j = 0; j < contours.size(); j++){
		Rect bound_rect = boundingRect(contours[j]);

		if (bound_rect.area() > 1600 || bound_rect.area() < 150 || bound_rect.width > 50 || bound_rect.height > 50){

			//cout << "come into" << endl;
			drawContours(dst, contours, j, Scalar::all(0), 3);	//擦除矩形边框
			outline_flag = true;
		}
	}

	if (outline_flag == true){
		return -1;
	}
	else{
		return 0;
	}

}


/*
	return : -1 error, 0 normal

	输入必须是单通道

*/
int ScoreEngine::getStandardRect(const Mat & input_src, Rect & standard_rect){
	if (!input_src.data){
		return -1;
	}
	Mat src;


	if (input_src.channels() == 1){
		//cvtColor(input_src, src, CV_GRAY2BGR);
		src = input_src.clone();
	}
	else if (input_src.channels() == 3){
		//src = input_src.clone();
		cvtColor(input_src, src, CV_BGR2GRAY);
	}
	else {
		return -1;
	}

	
	int gray_threshold = myOtsu(src) * 1.4;

	if (gray_threshold > 255)
		gray_threshold = 200;




	threshold(src, src, 200, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);

	Mat src_temp = src.clone();

	//将src_temp中的方框给搞掉
	eraseOutline(src_temp, src_temp, gray_threshold, 0);


	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(src_temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));


	//RNG rng(clock());

	//Mat disp(src_temp.size(), CV_8UC3, Scalar::all(0));
	Mat disp_wh(src_temp.size(), CV_8UC1, Scalar::all(0));

	for (int j = 0; j < contours.size(); j++){
		Rect bound_rect = boundingRect(contours[j]);

		Point temp_point(bound_rect.width, bound_rect.height);

		//width_height.push_back(temp_point);

		if (temp_point.x < 10 || temp_point.y < 10)
			continue;

		disp_wh.at<uchar>(temp_point.y, temp_point.x) += 1;

	}



	double minVal;
	double maxVal;
	Point minLoc;
	Point maxLoc;
	Point matchLoc;

	minMaxLoc(disp_wh, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	circle(disp_wh, maxLoc, 2, Scalar::all(255), -1);


	standard_rect = Rect(0, 0, maxLoc.x - 1, maxLoc.y - 1);

	if (standard_rect.width < 1 || standard_rect.height < 1){
		return -1;
	}

	return 0;


}



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

version: 1.2 2015年7月26日08:58:43
log: 每一列只有两种情况 空或者选出最可能被涂抹的那个



*/

int ScoreEngine::scanSpottedID(const Mat & input_src, const Rect & search_rect, int input_len, char result_id[32], int * output_len, int debug_flag){

	if (!input_src.data){
		return -1;
	}

	if (search_rect.x < 0 || search_rect.y < 0 || search_rect.height <= 0 || search_rect.width <= 0)
		return -1;

	Rect standard_rect(0, 0, 0, 0);

	if (getStandardRect(input_src(search_rect), standard_rect) < 0){
		cout << "no find standard rect" << endl;
		return -1;
	}

	const double standard_width = 35;
	double zoom_ratio = standard_width * 1.0 / standard_rect.width;
	
	standard_rect.width *= zoom_ratio;
	standard_rect.height *= zoom_ratio;

	cout << "zoom_ratio = " << zoom_ratio << endl;



	Mat src = input_src(search_rect).clone();

	imshow("srcccccccc", src);

	
	resize(src, src, Size(src.cols * zoom_ratio, src.rows * zoom_ratio));


	if (zoom_ratio < 1.2){
		blur(src, src, Size(5, 5));
	}



	Mat src_copy, src_copy1;

	if (src.channels() == 1){
		cvtColor(src, src_copy, CV_GRAY2BGR);
	}
	else if (src.channels() == 3){
		src_copy = src.clone();
		cvtColor(src, src, CV_BGR2GRAY);
	}
	else {
		return -1;
	}

	//src 是gray， src_copy是三通道的, src_copy1 是单通道的

	
	src_copy1 = src.clone();



	int gray_threshold = myOtsu(src) * 1.4;


	if (gray_threshold > 255)
		gray_threshold = 200;


	threshold(src, src, 2, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);

	if (debug_flag){
		//imshow("1_src", src);
	}
	

	Mat src_temp = src.clone();

	//这个地方有优化的地方，可以将这个函数拿到这里来实现
	for (int j = 0; j < 1; j++){
		if (!eraseOutline(src_temp, src_temp, gray_threshold, 0)){
			break;
		}
	}

	

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(src_temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));


	RNG rng(clock());

	Mat disp(src_temp.size(), CV_8UC3, Scalar::all(0));
	Mat disp_wh(src_temp.size(), CV_8UC1, Scalar::all(0));

	for (int j = 0; j < contours.size(); j++){
		Rect bound_rect = boundingRect(contours[j]);

		Point temp_point(bound_rect.width, bound_rect.height);

		//width_height.push_back(temp_point);

		if (temp_point.x < 10 || temp_point.y < 10)
			continue;

		disp_wh.at<uchar>(temp_point.y, temp_point.x) += 1;

	}

	

	double minVal; 
	double maxVal; 
	Point minLoc; 
	Point maxLoc;
	Point matchLoc;

	minMaxLoc(disp_wh, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	circle(disp_wh, maxLoc, 2, Scalar::all(255), -1);


	cout << "befor, standard_rect = " << standard_rect << endl;
	standard_rect = Rect(0, 0, maxLoc.x -1 , maxLoc.y -1 );
	cout << "after, standard_rect = " << standard_rect << endl;
	
	
	
// 	if (zoom_ratio > 1.2){
// 		//blur(src_copy1, src, Size(1, 1));
// 		src = src_copy1.clone();
// 	}
// 	else{
// 		blur(src_copy1, src, Size(5, 5));
// 	}


// 	if (standard_rect.width >= 30){
// 		blur(src_copy1, src, Size(5, 5));
// 	}
// 	else if (standard_rect.width >= 25){
// 		blur(src_copy1, src, Size(2, 2));
// 	}
// 	else{
// 		blur(src_copy1, src, Size(3, 1));
// 	}


	//threshold(src, src, 200, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);

	//cout << "standard_rect.width ============================================= " << standard_rect.width << endl;

	if (debug_flag){
		//imshow("2_src", src);
	}

	rectangle(disp_wh, standard_rect, Scalar::all(255), 1);

	vector<Point> approx;


	vector<Rect> rects;


	for (int j = 0; j < contours.size(); j++){


		// 			approx.clear();
		// 			approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.1, true);

		// 			if (!(approx.size() == 4 && isContourConvex(Mat(approx))))
		// 				continue;

		Rect bound_rect = boundingRect(contours[j]);
		
		if (bound_rect.height * 1.0 / bound_rect.width >0.96)
			continue;

		Point temp_point(bound_rect.width, bound_rect.height);


		if (temp_point.x < 10 || temp_point.y < 10)
			continue;

		if (compareRect(bound_rect, standard_rect) > 0.9){

			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			//drawContours(disp, contours, j, color, 1);
			//drawContours(disp, contours, j, color, 1);

			rects.push_back(bound_rect);

		}
	}

	vector<vector<Rect>> y_rects, x_rects;
	

	//对列操作
	for (int j = 0; j < rects.size(); j++){

		bool is_new_rect = true;

		for (int k = 0; k < x_rects.size(); k++){
			double sum = 0;

			for (int q = 0; q < x_rects[k].size(); q++){

				sum += x_rects[k][q].x;
			}
			double average = sum / x_rects[k].size();

			if (abs(rects[j].x - average) <= 5.0){
				x_rects[k].push_back(rects[j]);
				is_new_rect = false;
				break;
			}
		}

		if (is_new_rect){
			vector<Rect> temp_rects;
			temp_rects.push_back(rects[j]);
			x_rects.push_back(temp_rects);
		}
	}

	//擦除聚类太少的个数
	for (int i = 0; i < x_rects.size(); i++){
		if (x_rects[i].size() < 3){
			x_rects.erase(x_rects.begin() + i);
			i--;
		}
			
	}

	rects.clear();
	


	//重新转载rects
	for (int j = 0; j < x_rects.size(); j++){
		for (int k = 0; k < x_rects[j].size(); k++){
			rects.push_back(x_rects[j][k]);
		}
	}


	//for y，对每行处理
	for (int j = 0; j < rects.size(); j++){

		bool is_new_rect = true;

		for (int k = 0; k < y_rects.size(); k++){
			double sum = 0;

			for (int q = 0; q < y_rects[k].size(); q++){

				sum += y_rects[k][q].y;
			}
			double average = sum / y_rects[k].size();

			if (abs(rects[j].y - average) <= 5.0){
				y_rects[k].push_back(rects[j]);
				is_new_rect = false;
				break;
			}
		}

		if (is_new_rect){
			vector<Rect> temp_rects;
			temp_rects.push_back(rects[j]);
			y_rects.push_back(temp_rects);
		}
	}

	//这里可以改为选择题那样的方式
	//判断起点和终点，还有间隙
	if (y_rects.size() < 10){
		cout << "fail to find " << endl;
		return -1;
	}

	sort(y_rects.begin(), y_rects.end(), sort_y_rect);



	vector<double> ys, xs;

	for (int i = 0; i < 10; i++){
		double sum = 0;

		for (int j = 0; j < y_rects[i].size(); j++){
			sum += y_rects[i][j].y;
		}

		if (y_rects[i].size() == 0){
			sum = 0;
		}
		else{
			sum = sum / (y_rects[i].size());
		}


		ys.push_back(sum);


	}

	sort(ys.begin(), ys.end());

// 	if (!checkDelta(ys, 4)){
// 
// 		for (int i = 0; i < ys.size(); i++){
// 			cout << ys[i] << endl;
// 		}
// 		cout << "y delta wrong!" << endl;
// 		return -1;
// 	}


	for (int i = 0; i < x_rects.size(); i++){
		double sum = 0;

		for (int j = 0; j < x_rects[i].size(); j++){
			sum += x_rects[i][j].x;
		}

		if (x_rects[i].size() == 0){
			sum = 0;
		}
		else{
			sum = sum / (x_rects[i].size());
		}


		xs.push_back(sum);


	}

	sort(xs.begin(), xs.end());

// 	if (!checkDelta(xs, 4)){
// 		cout << "x delta wrong!" << endl;
// 		return -1;
// 	}

	for (int j = 0; j < xs.size(); j++){
		line(src_copy, Point(xs[j], 0), Point(xs[j], 1000), CV_RGB(255, 0, 0));
	}


	for (int j = 0; j < ys.size(); j++){
		//cout << ys[j] << endl;
		line(src_copy, Point(0, ys[j]), Point(1000, ys[j]), CV_RGB(255, 0, 0));
	}

	for (int j = 0; j < y_rects.size(); j++){

		//rects_num += y_rects[j].size();

		Scalar color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

		for (int k = 0; k < y_rects[j].size(); k++){
			rectangle(disp, y_rects[j][k], color, 1);
		}

	}


	string result_id1;

	bool legal_id = true;

	int return_value = 0;

	const int MAXCOL = 10;

	//申请一维数据并将其转成二维数组指针  
	double *pp_arr = new double[xs.size() * MAXCOL];

	double(*binary_ratio)[MAXCOL] = (double(*)[MAXCOL])pp_arr;


	
	vector<vector<double>> ratios(xs.size());	//里面的vector的size为10，外面的vector size 为xs.size



	for (int j = 0; j < xs.size(); j++){

		vector<int> x_col;
		//cout << j + 1 << " th col" << endl;

		double first_value = 0;
		double second_value = 0;

		int first_index = 0;
		int second_index = 0;

// 		Mat temp_src = src_copy1.clone();
// 		int border = 5;
// 		Rect rect_bin(xs[j] - border, 0, standard_rect.width + border * 2, temp_src.rows);
// 
// 
// 		if (rect_bin.x < 0)
// 			rect_bin.x = 0;
// 		if (rect_bin.y < 0)
// 			rect_bin.y = 0;
// 
// 		if (rect_bin.br().y >= src.rows)
// 			rect_bin.y = src.rows - rect_bin.height;
// 		if (rect_bin.br().x >= src.cols){
// 			rect_bin.x = src.cols - rect_bin.width;
// 		}
// 
// 
// 		Mat src_roi_bin = temp_src(rect_bin); 


// 		blur(src_roi_bin, src_roi_bin, Size(3, 3));
// 		adaptiveThreshold(src_roi_bin, src_roi_bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY_INV, 3, 5);

//		blur(src_roi_bin, src_roi_bin, Size(3, 3));

//		threshold(src_roi_bin, src_roi_bin, 2, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);
//		imshow("edsfa", temp_src);

		for (int k = 0; k < ys.size(); k++){
			Rect roi_rect(xs[j], ys[k], standard_rect.width, standard_rect.height);

			if (roi_rect.x < 0)
				roi_rect.x = 0;
			if (roi_rect.y < 0)
				roi_rect.y = 0;

			if (roi_rect.br().y >= src.rows)
				roi_rect.y = src.rows - roi_rect.height;
			if (roi_rect.br().x >= src.cols){
				roi_rect.x = src.cols - roi_rect.width;
			}

			Mat roi(src, roi_rect);
		//	Mat roi(temp_src, roi_rect);


			double no_zero = countNonZero(roi);

			double ratio = no_zero / standard_rect.area();

			ratios[j].push_back(ratio);



			//cout << ratio << endl;

			if (ratio > first_value){

				second_value = first_value;
				second_index = first_index;

				first_value = ratio;
				first_index = k;
			}
			else if (ratio > second_value){
				second_value = ratio;
				second_index = k;
			}

		}

//		waitKey(0);

		//cout << endl;

		//double threshold = 0.66;
// 		double threshold = 0.6;
// 
// 		cout << "first_value = " << first_value << ",  second value = " << second_value << endl;
// 
// 		if (first_value < threshold){
// 			//cout << j << " th col has no num" << endl;
// 			//result_id1 = "U";
// 			result_id[0] = 'U';
// 			result_id[1] = '\0';
// 			*output_len = 1;
// 			putText(src_copy, "U", Point(xs[j], ys[0]), CV_FONT_HERSHEY_SIMPLEX, 1.0, CV_RGB(255, 0, 255), 2);
// 			return_value = -2;
// 			goto return_positon;
// 			//return -2;
// 		}
// 		//Muti 
// 		else if (first_value >= threshold && second_value > 0.70){
// 			cout << "one col has two num" << endl;
// 
// 			result_id[0] = 'M';
// 			result_id[1] = '\0';
// 			*output_len = 1;
// 			putText(src_copy, "M", Point(xs[j], ys[0]), CV_FONT_HERSHEY_SIMPLEX, 1.0, CV_RGB(255, 0, 255), 2);
// 			return_value = -3;
// 			goto return_positon;
// 		}
// 		else{
// 			string temp;
// 			temp.push_back('0' + first_index);
// 			//result_id1.push_back('0' + first_index);
// 			result_id[j] = '0' + first_index;
// 			putText(src_copy, temp, Point(xs[j], ys[0]), CV_FONT_HERSHEY_SIMPLEX, 1.0, CV_RGB(255, 0, 255), 2);
// 		}


	}

	
	//sprintf_s(result_id, "%s", result_id1.c_str());
	*output_len = xs.size();
	result_id[xs.size()] = '\0';
	//cout << result_id << endl;

	double not_choose_threshold = 0.66;

	vector<double> temp_ratio;

	bool is_undefined = false;

	for (int i = 0; i < ratios.size(); i++){

		temp_ratio = ratios[i];
		sort(temp_ratio.begin(), temp_ratio.end());

// 		for (int j = temp_ratio.size() - 1; j >= 0; j--){
// 			cout << temp_ratio[j] << " ";
// 		}
// 		cout << endl;

		int max_index = 0;
		double max_value = temp_ratio[temp_ratio.size() - 1];

		for (int j = 0; j < ratios[i].size(); j++){
			if (ratios[i][j] == max_value){
				max_index = j;
				break;
			}
		}

		if (max_value < not_choose_threshold || max_value - temp_ratio[6] < 0.1){
			putText(src_copy, "U", Point(xs[i], ys[0]), CV_FONT_HERSHEY_SIMPLEX, 1.0, CV_RGB(0, 0, 255), 2);
			is_undefined = true;
		}
		else{
			string temp;
			temp.push_back('0' + max_index);
			result_id[i] = '0' + max_index;
			putText(src_copy, temp, Point(xs[i], ys[0]), CV_FONT_HERSHEY_SIMPLEX, 1.0, CV_RGB(255, 0, 255), 2);
		}

	}

	return_value = 0;
	result_id[xs.size()] = '\0';

	if (is_undefined){
		result_id[0] = 'U';
		result_id[1] = '\0';
		return_value = -1;
		*output_len = 1;
	}


return_positon:
	if (debug_flag){
		resize(src_copy, src_copy, Size(src_copy.cols / zoom_ratio, src_copy.rows / zoom_ratio));

		imshow("src_copy", src_copy);
		//resize(src, src, src.size() / 2);
		//imshow("disp", disp);

		//imshow("wh", disp_wh);
	}
	
	delete[] pp_arr;

	return return_value;
}



double ScoreEngine::compareRect(const Rect & r1, const Rect & r2){
	if (r2.width <= 0 || r2.height <= 0)
		return -1;

	double ratio = 0;
	ratio = 0.5 * (1 - abs(r1.height - r2.height) * 1.0 / r2.height) + 0.5 * (1 - abs(r1.width - r2.width) * 1.0 / r2.width);

	return ratio;
}

double ScoreEngine::compareRect1(const Rect & r1, const Rect & r2){
	if (r2.width <= 0 || r2.height <= 0)
		return -1;

	double ratio = 0;
	ratio = 0.5 * (1 - (r1.height - r2.height) * 1.0 / r2.height) + 0.5 * (1 - (r1.width - r2.width) * 1.0 / r2.width);

	return ratio;
}




bool ScoreEngine::checkDelta(const vector<double> & value, double precision){

	if (value.size() < 2)
		return false;

	vector<double> deltas;
	for (int i = 0; i < value.size() - 1; i++){
		deltas.push_back(value[i + 1] - value[i]);
	}


	for (int i = 0; i < deltas.size(); i++){
		for (int j = i + 1; j < deltas.size(); j++){

			if (abs(deltas[j] - deltas[i]) > precision){
				return false;
			}

		}
	}

	return true;

}


//return 0, normal, -1,abnormal
int ScoreEngine::getUpDownRect(const Mat & input_src, int gray_threshold, Rect & first_rect, Rect & second_rect){


	if (!input_src.data){
		return -1;
	}


	//必须为单通道的数据
	//CV_Assert(input_src.channels() == 1);
	if (input_src.channels() != 1){
		cout << "channels != 1" << endl;
		return -1;
	}

	Mat src = input_src.clone();

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;


	//int gray_threshold = 100;
	
	threshold(src, src, gray_threshold, 255, CV_THRESH_BINARY_INV);

	Mat src_temp = src.clone();
	Mat hough_src = src.clone();


	//去除小的轮廓误差
	findContours(src_temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));

	for (int i = 0; i < contours.size(); i++){

		if (contourArea(contours[i]) < 100){
			drawContours(hough_src, contours, i, Scalar::all(0), -1);
			continue;
		}
		if (boundingRect(contours[i]).height >= src_temp.rows - 1){

			cout << "=========================" << endl;
			return -1;
		}
	}

	
	std::vector<cv::Vec4i> lines, vertical_lines, horizon_lines;
	//void HoughLinesP(InputArray image, OutputArray lines, double rho, double theta, int threshold, double minLineLength=0, double maxLineGap=0 )
	cv::HoughLinesP(hough_src, lines, 1, CV_PI / 360, 70, 50, 5);

	Mat horizon_src = Mat::zeros(hough_src.size(), CV_8UC1);
	//Mat vertical_src = horizon_src.clone();


	//水平和竖直方向的阈值
	const double vertical_angle_thresh = 85;
	const double horizon_angle_thresh = 2;
	const double k_v_thresh = tan(vertical_angle_thresh / 180.0 * CV_PI);
	const double k_h_thresh = tan(horizon_angle_thresh / 180.0 * CV_PI);


	//区分水平和竖直的直线
	for (int i = 0; i < lines.size(); i++){
		double x1, x2, y1, y2;
		x1 = lines[i][0];
		y1 = lines[i][1];
		x2 = lines[i][2];
		y2 = lines[i][3];
		//线段的长度
		//double distance = sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)* (y2 - y1));

		//水平或者竖直，
		bool good_line_flag = false;

		//竖直的情况
		if (abs(x1 - x2) <= 1){
			//good_line_flag = true;
			//vertical_lines.push_back(lines[i]);

		}
		else{
			double k;
			k = abs((y2 - y1) / (x2 - x1));

			if (k < k_h_thresh){
				//good_line_flag = true;
				horizon_lines.push_back(lines[i]);

			}
			else if (k > k_v_thresh){
				//good_line_flag = true;
				//vertical_lines.push_back(lines[i]);
			}
		}
	}


	//将找到水平和竖直的直线，用不同颜色的线段画出来


	for (int i = 0; i < horizon_lines.size(); i++){
		line(horizon_src, Point2f(horizon_lines[i][0], horizon_lines[i][1]), Point2f(horizon_lines[i][2], horizon_lines[i][3]), Scalar::all(255), 2);
	}


	//imshow("horizon_src", horizon_src);

	


	findContours(horizon_src, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));


	first_rect = Rect(0, 0, 0, 0);
	second_rect = Rect(0, 0, 0, 0);

	//找到最长和第二长的两条直线，是矩形的上下框

	for (int i = 0; i < contours.size(); i++){

		Rect temp_rect = boundingRect(contours[i]);

		if (temp_rect.width > first_rect.width ){
			second_rect = first_rect;
			first_rect = temp_rect;
		}
		else if (temp_rect.width > second_rect.width){
			second_rect = temp_rect;
		}

	}

	//说明没有找到
	if (first_rect.width == 0 || second_rect.width == 0){
		cout << "no first or second rect" << endl;
		return -1;
	}
		
	if (first_rect.width >= src.cols * 0.98){
		cout << "too large" << endl;
		return -1;
	}


	double r1, r2;
	r1 = second_rect.width * 1.0 / src.cols;

	r2 = (first_rect.width - second_rect.width) * 1.0 / first_rect.width;
	//cout << "r1 = " << r1 << endl;
	//cout << "r2 = " << r2 << endl;

	double max_height = first_rect.height > second_rect.height ? first_rect.height : second_rect.height;

	if (/*r1 < 0.8 || */r2 >0.1 || max_height > 30)
		return -1;

	return 0;

}



/*
function: 识别分数框的分数

input_src: 输入的图片
search_rect: 分数框必须在此矩形中
result_score: 考虑到调试，返回值确认为vector<int> 应该为22位，每一位为0时，代表没有被划中，若为1，则代表被划中
debug_flag: 中间结果显示

return value: 0，正常
-1， 图片异常，或者没有找到22位数字框

*/

int ScoreEngine::scanScoreSquare(const Mat & input_src, const Rect & search_rect, int result_score[22], int debug_flag){


	

	if (!input_src.data){
		return -1;
	}

	if (search_rect.x < 0 || search_rect.y < 0 || search_rect.height <= 0 || search_rect.width <= 0)
		return -1;

	for (int i = 0; i < 22; i++){
		result_score[i] = 0;
	}


	Mat src = input_src(search_rect).clone();

	Mat blurImage;

	cv::blur(src, blurImage, cv::Size(2, 2));
	int blockSize = 51;
	int constValue = 20;
	cv::Mat reverseImage;
	cv::adaptiveThreshold(blurImage, src, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, blockSize, constValue);




// 	blur(src, src, Size(2, 2));
// 
// 	//getUpDownRect(src, gray_threshold, first_rect, second_rect);
// 
// 
// 	//threshold(src, src, 2, 255, CV_THRESH_OTSU | CV_THRESH_BINARY);
// 
// 	cv::adaptiveThreshold(src, src, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 51, 20);


// 	pyrUp(src, src);
// 	pyrDown(src, src);
	//medianBlur(src, src, 3);
	//bilateralFilter(src, src, 7, 7 * 2, 7 / 2);
	

	Mat src_copy;

	if (src.channels() == 1){
		cvtColor(src, src_copy, CV_GRAY2BGR);
	}
	else if (src.channels() == 3){
		src_copy = src.clone();
		cvtColor(src, src, CV_BGR2GRAY);
	}
	else {
		return -1;
	}

	if (debug_flag){
		imshow("1111", src);
	}

	//cout << src << endl;


	int vertical_zoom = 4;
	int horizon_zoom = 2;
	resize(src, src, Size(src.cols * horizon_zoom, src.rows * vertical_zoom));

	
	
	//src 是gray

	int gray_threshold = 200;

	Rect first_rect(0, 0, 0, 0), second_rect(0, 0, 0, 0);
	int step = 30;

	int i = 0;

	int iteration_times = 6;

	//cout << "otsu thresh = " << myOtsu(src) << endl;;
// 	gray_threshold = myOtsu(src) * 1.4;
// 	if (gray_threshold > 255){
// 		gray_threshold = 200;
// 	}


// 	int divide_num = 2;
// 	double divide_step = src.cols *1.0 / divide_num; 
// 	//cout << "myOtsu : " << endl;
// 	for (int i = 0; i < divide_num; i++){
// 		Rect temp_rect(i*divide_step, 0, divide_step, src.rows);
// 		Mat roi = src(temp_rect);
// 		//cout << myOtsu(roi) << " ";
// 
// 	}
	//cout << endl;


// 	for (i = 0; i < iteration_times; i++){
// 		gray_threshold = gray_threshold - i * step;
// 		if (gray_threshold < 10)
// 			gray_threshold = 10;
// 
// 
// 		//cout << i << " th iteration" << endl;
// 		if (getUpDownRect(src, gray_threshold, first_rect, second_rect) == 0){
// 			break;
// 		}
// 	}
// 
// 	if (i >= iteration_times){
// 		cout << "超出迭代次数" << endl;
// 		return -1;	//人工check
// 	}


	if (getUpDownRect(src, gray_threshold, first_rect, second_rect) != 0){

		return -1;
	}


	threshold(src, src, gray_threshold, 255, CV_THRESH_BINARY_INV);

// 	blur(src, src, Size(3, 3));
// 
// 	getUpDownRect(src, gray_threshold, first_rect, second_rect);
// 
// 
// 	threshold(src, src, 2, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);





	Mat disp1;
	cvtColor(src, disp1, CV_GRAY2BGR);

	rectangle(src, first_rect, Scalar::all(0), -1);
	rectangle(src, second_rect, Scalar::all(0), -1);

	int x1, x2;

	//取右边的值
	x1 = first_rect.x > second_rect.x ? first_rect.x : second_rect.x;
	//取左边的值
	x2 = first_rect.br().x < second_rect.br().x ? first_rect.br().x : second_rect.br().x;

	//补偿一下
	int compensation = 2;
	x1 += compensation;
	x2 -= compensation;

	first_rect.x = x1;
	first_rect.width = x2 - x1 + 1;

	second_rect.x = x1;
	second_rect.width = first_rect.width;

	if (first_rect.y > second_rect.y){
		Rect temp = first_rect;
		first_rect = second_rect;
		second_rect = temp;
	}

	Rect left_rect(0, 0, x1, src.rows);
	Rect right_rect(x2 + 1, 0, src.cols - x2, src.rows);
	//擦除左边和右边的矩形
	rectangle(src, left_rect, Scalar::all(0), -1);
	rectangle(src, right_rect, Scalar::all(0), -1);



	//cout << src.row(first_rect.y -1) << endl;

	//cout << "row: " << src.row(right_rect.br().y) << endl;

	double x_step = first_rect.width / 22.0;

	Rect standard_rect(0, first_rect.br().y, round(x_step), second_rect.y - first_rect.br().y );
	

	double xs[23];

	for (int i = 0; i < 23; i++){
		xs[i] = first_rect.x + i*x_step;
	}
	xs[22] = first_rect.br().x;


	for (int i = 0; i < 22; i++){
		standard_rect.x = round(first_rect.x + i* x_step);
		rectangle(disp1, standard_rect, CV_RGB(255, 0, 0), 1);
	}
	
	vector<Vec6i> up, inner_up, inner_down, down;

	up = getConnectedRegion(src, first_rect.y -1);
	inner_up = getConnectedRegion(src, first_rect.br().y);

	inner_down = getConnectedRegion(src, second_rect.y -1);
	down = getConnectedRegion(src, second_rect.br().y);


	removeConnectedRegion(up, xs, 1.0);
	removeConnectedRegion(inner_up, xs, 2.0);
	removeConnectedRegion(inner_down,xs,  2.0);
	removeConnectedRegion(down, xs, 1.0);


	bool out_flag[22];
	bool inner_flag[22];

	for (int i = 0; i < 22; i++){
		out_flag[i] = inner_flag[i] = false;
	}


	//是或的关系，不能每次做之前清零
	setFlag(up, xs, out_flag);
	setFlag(down, xs, out_flag);

	setFlag(inner_up, xs, inner_flag);
	setFlag(inner_down, xs, inner_flag);

	for (int i = 0; i < 22; i++){

		if (out_flag[i] && inner_flag[i]){

			Rect temp_rect(xs[i], standard_rect.y, xs[i + 1] - xs[i], standard_rect.height);

			if (contourConfirm(src(temp_rect), 5) == false){
				result_score[i] = 0;
				continue;
			}

			putText(disp1, "M", Point(xs[i], first_rect.y ), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255, 0, 0), 1);
			putText(src_copy, "M", Point(xs[i]/horizon_zoom, first_rect.y *1.0/ vertical_zoom), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255, 0, 0), 1);
			result_score[i] = 1;
		}
		else if (inner_flag[i]){
			Rect temp_rect(xs[i], standard_rect.y, xs[i + 1] - xs[i], standard_rect.height);

			if (contourConfirm(src(temp_rect), 5) == false){
				result_score[i] = 0;
				continue;
			}
		}
		else{
			result_score[i] = 0;
		}
	}


	for (int i = 0; i < up.size(); i++){
		circle(disp1, Point(up[i][0], up[i][1]), 3, CV_RGB(0, 0, 255), -1);
	}

	for (int i = 0; i < inner_up.size(); i++){
		circle(disp1, Point(inner_up[i][0], inner_up[i][1]), 3, CV_RGB(0, 0, 255), -1);
	}

	for (int i = 0; i < inner_down.size(); i++){
		circle(disp1, Point(inner_down[i][0], inner_down[i][1]), 3, CV_RGB(0, 0, 255), -1);
	}

	for (int i = 0; i < down.size(); i++){
		circle(disp1, Point(down[i][0], down[i][1]), 3, CV_RGB(0, 0, 255), -1);
	}
	
	if (debug_flag){
		//line(src, Point(0, first_rect.y + 2), Point(2000, first_rect.y + 2), Scalar::all(255), 1);
		
		imshow("disp1", disp1);
	}

	imshow("src_co", src_copy);


	return 0;

}

/*
func: 得到被切割的区域

return: start.x start.y center.x center.y end.x end.y
*/
vector<Vec6i> ScoreEngine::getConnectedRegion(const Mat & src, int y_index){
	vector<Vec6i> connect_region;
	Vec6i cr;
	if (y_index < 0 || y_index >= src.rows){
		return connect_region;
	}

	Mat src1 = src.clone();
	if (src.channels() > 1){
		cvtColor(src, src1, CV_BGR2GRAY);
	}

	uchar last = 0, current = 0;

	cr[1] = cr[3] = cr[5] = y_index;

	for (int i = 0; i < src1.cols; i++){
		current = src1.at<uchar>(y_index, i);
		if (last == 0 && current > 0){
			cr[0] = i;
		}
		else if (last > 0 && current == 0){
			cr[4] = i - 1;
			cr[2] = (cr[0] + cr[4]) / 2;
			connect_region.push_back(cr);
		}
		last = current;
	}
	return connect_region;
}


void ScoreEngine::removeConnectedRegion(vector<Vec6i> & connect_region, double xs[23], double radius){

	
	double delta[3];
	for (int i = 0; i < connect_region.size(); i++){

		int j = 0;
		for (j = 0; j < 23; j++){
			//xs[j]包含于coonect_region
			if (connect_region[i][0] <= xs[j] && connect_region[i][4] >= xs[j]){
				break;
			}
			//
			else{
				int k;
				for (k = 0; k < 3; k++){
					delta[k] = abs(connect_region[i][k * 2] - xs[j]);
					if (delta[k] <= radius){
						break;
					}
				}

				if (k<3)
					break;
				
			}
			
		}

		if (j < 23){
			connect_region.erase(connect_region.begin() + i);
			i--;
		}
	}
}


//不能去碰flag
void ScoreEngine::setFlag(vector<Vec6i> & connect_region, double xs[23], bool flag[22]){


	for (int i = 0; i < connect_region.size(); i++){
		
		int j = 0;
		
		for (j = 0; j < 22; j++){
			if (connect_region[i][2] > xs[j] && connect_region[i][2] <= xs[j + 1]){
				flag[j] = true;
				break;
			}
		}

// 		if (j >= 22){
// 			if (connect_region[i][2] <= xs[0]){
// 				flag[0] = true;
// 			}
// 			else if (connect_region[i][2] >= xs[22]){
// 				flag[21] = true;
// 			}
// 		}

	}
}


int ScoreEngine::scanScoreBar(
	const Mat & input_src,
	const Rect & search_rect,
	short bar_type,		//数字框的类型，暂且不用
	short * head_mark,	//输出识别的第一个空格有没有划过, 0,没有划过；1划过了
	short * output_prime, //输出识别结果整数位, 测试期间可能会越界
	short * output_decimal, //输出识别结果小数位（5），0，没有；5，表示0.5被划过
	short * prime_num
	){
	
	int result_score[22];




// 	Mat blurImage;
// 
// 	cv::blur(input_src(search_rect), blurImage, cv::Size(2, 2));
// 	int blockSize = 51;
// 	int constValue = 20;
// 	cv::Mat reverseImage;
// 	cv::adaptiveThreshold(blurImage, reverseImage, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, blockSize, constValue);
// 
// 
// 	int flag = scanScoreSquare(reverseImage, Rect(0, 0, search_rect.width, search_rect.height), result_score, 1);

	int flag = scanScoreSquare(input_src, search_rect, result_score, 1);

	if (flag != 0){
		return flag;
	}

	if (result_score[0] == 1)
		*head_mark = 1;

	if (result_score[21] == 1){
		*output_decimal = 5;
	}

	*output_prime = 0;


	long int sum = 0;


	for (int i = 1; i <= 20; i++){
		if (result_score[i] != 0){
			*output_prime = *output_prime * 10 + bar_table[i];
			sum = sum * 10 + bar_table[i];
		}

	}

	//cout << "sum = " << sum << endl;
	*output_prime = sum;

	return 0;
}




bool ScoreEngine::contourConfirm(const Mat & src_input, int margin_threshold){

	Mat src;
	if (src_input.channels() == 1){
		src = src_input.clone();
	}
	else if (src_input.channels() == 3){
		cvtColor(src_input, src, CV_BGR2GRAY);
	}

	//imshow("src_confirm", src);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;


	
	//擦除小的点
	findContours(src, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));
	for (int i = 0; i < contours.size(); i++){

// 		if (contourArea(contours[i]) < 10)
// 			continue;

		Rect bound_rect = boundingRect(contours[i]);

		if (bound_rect.height < 20)
			continue;

		if (bound_rect.y <= 1 || bound_rect.br().y >= src.rows - 2){
			double col_center = src.cols / 2.0;
			if (bound_rect.x <= col_center && bound_rect.br().x >= col_center){
				return true;
			}
			else {
				double margin = 0;
				double x1, x2;
				x1 = bound_rect.x;
				x2 = src.cols - bound_rect.br().x;

				margin = x1 <= x2 ? x1 : x2;

				margin = margin + bound_rect.width;
				if (margin >= margin_threshold){
					return true;
				}
			}
		}
		

		

	}


	return false;



}


int ScoreEngine::myOtsu(const IplImage *frame) //大津法求阈值
{
#define GrayScale 256	//frame灰度级
	int width = frame->width;
	int height = frame->height;
	int pixelCount[GrayScale] = { 0 };
	float pixelPro[GrayScale] = { 0 };
	int i, j, pixelSum = width * height, threshold = 0;
	uchar* data = (uchar*)frame->imageData;

	//统计每个灰度级中像素的个数
	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width; j++)
		{
			pixelCount[(int)data[i * width + j]]++;
		}
	}

	//计算每个灰度级的像素数目占整幅图像的比例
	for (i = 0; i < GrayScale; i++)
	{
		pixelPro[i] = (float)pixelCount[i] / pixelSum;
	}

	//遍历灰度级[0,255],寻找合适的threshold
	float w0, w1, u0tmp, u1tmp, u0, u1, deltaTmp, deltaMax = 0;
	for (i = 0; i < GrayScale; i++)
	{
		w0 = w1 = u0tmp = u1tmp = u0 = u1 = deltaTmp = 0;
		for (j = 0; j < GrayScale; j++)
		{
			if (j <= i)   //背景部分
			{
				w0 += pixelPro[j];
				u0tmp += j * pixelPro[j];
			}
			else   //前景部分
			{
				w1 += pixelPro[j];
				u1tmp += j * pixelPro[j];
			}
		}
		u0 = u0tmp / w0;
		u1 = u1tmp / w1;
		deltaTmp = (float)(w0 *w1* pow((u0 - u1), 2));
		if (deltaTmp > deltaMax)
		{
			deltaMax = deltaTmp;
			threshold = i;
		}
	}
	return threshold;
}

int ScoreEngine::myOtsu(const Mat & src){

	IplImage pI = src;
	return myOtsu(&pI);
}

int ScoreEngine::getStandardRect1(const Mat & input_src, Rect & standard_rect){

	if (!input_src.data){
		cout << "!input_src.data" << endl;
		return -1;
	}

	if (input_src.channels() != 1){
		return -1;
	}

	Mat src = input_src.clone();




	threshold(src, src, 2, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);

	Mat src_temp = src.clone();

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	//默认是没有外方框的
	findContours(src_temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));

	//Mat disp(src_temp.size(), CV_8UC3, Scalar::all(0));
	Mat disp_wh(src_temp.size(), CV_8UC1, Scalar::all(0));

	RNG rng(clock());

	for (int j = 0; j < contours.size(); j++){
		Rect bound_rect = boundingRect(contours[j]);

		if (bound_rect.width < bound_rect.height)
			continue;


		Point temp_point(bound_rect.width, bound_rect.height);

		Scalar color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));


		if (temp_point.x < 10 || temp_point.y < 10)
			continue;

		disp_wh.at<uchar>(temp_point.y, temp_point.x) += 1;

		//drawContours(disp, contours, j, color, 1);

	}


	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;

	minMaxLoc(disp_wh, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

	if (maxVal <= 0){
		return -1;
	}

	//circle(disp_wh, maxLoc, 2, Scalar::all(255), -1);

	standard_rect = Rect(0, 0, maxLoc.x - 1, maxLoc.y - 1);

	//rectangle(disp_wh, standard_rect, Scalar::all(255), 1);



	return 0;




}

//没有补偿返回0
//补偿返回1
int ScoreEngine::compensateRect(vector<Rect> & rects, const Rect  & current_rect, const Rect & standard_rect, const double & delta, const Mat & src){


	static int counter11 = 0;
	counter11++;

	//cout << "counter == " << counter11 << endl;
	Rect last_rect, next_rect;

	last_rect = standard_rect;
	next_rect = standard_rect;

	last_rect.x = current_rect.x - delta - standard_rect.width;
	last_rect.y = current_rect.y;


	if (last_rect.y < 0){
		last_rect.y = 0;
	}

	if (last_rect.br().y >= src.rows){
		last_rect.y = src.rows - last_rect.height;
	}


	next_rect.x = current_rect.br().x + delta;
	next_rect.y = current_rect.y;


	if (next_rect.y < 0){
		next_rect.y = 0;
	}

	if (next_rect.br().y >= src.rows){
		next_rect.y = src.rows - next_rect.height;
	}

	//cout << "last_rect = " << last_rect << " , next_rect = " << next_rect << endl;

	if (last_rect.x < 0 || next_rect.br().x >= src.cols){
		//cout << "000000000000000000000" << endl;
		return 0;
	}


	Mat roi_last = src(last_rect);
	Mat roi_next = src(next_rect);

	double roi_last_ratio = countNonZero(roi_last) * 1.0 / (roi_last.rows * roi_last.cols);
	double roi_next_ratio = countNonZero(roi_next) * 1.0 / (roi_next.rows * roi_next.cols);


	//cout << "roi_last_ratio = " << roi_last_ratio << ", roi_next_ratio = " << roi_next_ratio << endl;

	for (int i = 0; i < rects.size(); i++){

		Point2f last_delta_point = last_rect.tl() - rects[i].tl();
		Point2f next_delta_point = next_rect.tl() - rects[i].tl();

		double d1, d2;
		d1 = sqrt(last_delta_point.x * last_delta_point.x + last_delta_point.y * last_delta_point.y);
		d2 = sqrt(next_delta_point.x * next_delta_point.x + next_delta_point.y * next_delta_point.y);

		if (d1 < standard_rect.height){
			roi_last_ratio = 0;
		}

		if (d2 < standard_rect.height){
			roi_next_ratio = 0;
		}

	}

	int flag = 0;

	if (roi_last_ratio >= 0.2){

		//cout << "roi_last_ratio >= 0.2" << endl;
		rects.push_back(last_rect);
		compensateRect(rects, last_rect, standard_rect, delta, src);
	}

	if (roi_next_ratio >= 0.2){
		//cout << "roi_next_ratio >= 0.2" << endl;

		rects.push_back(next_rect);
		compensateRect(rects, next_rect, standard_rect, delta, src);
	}

	return 0;
}


/*
	function: 识别选择题目
	result：各题目的识别结果，计算方式如下：
	A选择为1，不选择为0；
	B选择为2，不选择为0；
	C选择为4，不选择为0；
	D选择为8，不选择为0
	result = A + B + C + D
	例如：result = 8，表示选择了C；result = 6，表示选择了B和C。

	return value: 0, normal,
	-1, please check!

	version: 1.2
	log: 1，排除异常，没有检查project_xs的size
		 2，解决汉字干扰
		 3，用大津法自适应阈值


*/

//int ScoreEngine::scanScoreOptionpanel_H(const Mat & input_src, const Rect & search_rect, int item_num, vector<int> & result, int debug_flag){
//int ScoreEngine::scanScoreOptionpanel_H(const Mat & input_src, const Rect & search_rect, int item_num, vector<int> & result, int debug_flag){
int ScoreEngine::scanScoreOptionpanel_H(const Mat & input_src, const Rect & search_rect, int exclusiveRegion[4], int item_num, int result[256], int *ques_num, int multi_flag, int debug_flag){


	if (!input_src.data){
		cout << " empty " << endl;
		return -1;
	}

	if (search_rect.x < 0 || search_rect.y < 0 || search_rect.height <= 0 || search_rect.width <= 0){
		cout << "rect illegal" << endl;
		return -1;
	}



	Mat src = input_src(search_rect).clone();


	//imshow("11111", src);

	const double standard_width = 35;	//单位一

	Rect standard_rect;

	if (getStandardRect1(src, standard_rect) != 0){
		cout << "standard rect illegal " << endl;
		return -1;
	}

	cout << "standard_rect11111  = " << standard_rect << endl;

	double zoom_ratio = standard_rect.width * 1.0 / standard_width;

	cout << "zoom_ratio = " << zoom_ratio << endl;



	resize(src, src, Size(src.cols / zoom_ratio, src.rows / zoom_ratio));

	// 	Mat kernel = (Mat_<float>(3, 3) << 0, -1, 0, -1, 5, -1, 0, -1, 0);  // 92.0
	// 	filter2D(src, src, src.depth(), kernel);

	if (zoom_ratio > 0.9){
		blur(src, src, Size(3, 3));
	}


	//equalizeHist(src, src);

	//imshow("srcccccc", src);




	Mat src_copy;

	if (src.channels() == 1){
		cvtColor(src, src_copy, CV_GRAY2BGR);
	}
	else if (src.channels() == 3){
		src_copy = src.clone();
		cvtColor(src, src, CV_BGR2GRAY);
	}
	else {
		return -1;
	}


	Mat src_gray = src.clone();

	//int gray_threshold = 200;
	//int gray_threshold = myOtsu(src)* 1.4;



	//cout << "gray_threshold = " << gray_threshold << endl;

	threshold(src, src, 2, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);

	//adaptiveThreshold(src, src, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 5, 5);
	//adaptiveThreshold(src, src, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 5, 5);

	//threshold(src, src, CV_THRESH_OTSU, 255, CV_THRESH_BINARY_INV);
	//threshold(src, src, gray_threshold, 255, CV_THRESH_BINARY_INV);



	Mat src_temp = src.clone();

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	//默认是没有外方框的
	findContours(src_temp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE, Point(0, 0));


	Mat disp(src_temp.size(), CV_8UC3, Scalar::all(0));
	Mat disp_wh(src_temp.size(), CV_8UC1, Scalar::all(0));

	RNG rng(clock());

	for (int j = 0; j < contours.size(); j++){
		Rect bound_rect = boundingRect(contours[j]);

		Point temp_point(bound_rect.width, bound_rect.height);

		Scalar color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));


		if (temp_point.x < 30 || temp_point.y < 10)
			continue;

		disp_wh.at<uchar>(temp_point.y, temp_point.x) += 1;

		//drawContours(disp, contours, j, color, 1);

	}


	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;

	minMaxLoc(disp_wh, &minVal, &maxVal, &minLoc, &maxLoc, Mat());


	circle(disp_wh, maxLoc, 2, Scalar::all(255), -1);

	standard_rect = Rect(0, 0, maxLoc.x - 1, maxLoc.y - 1);
	cout << "standard_rect = " << standard_rect << endl;

	rectangle(disp_wh, standard_rect, Scalar::all(255), 1);


	// 	imshow("disp_wh", disp_wh);
	// 	imshow("disp", disp);


	vector<Rect> rects;
	vector<Point> approx;

	vector<Rects> row_rects;
	vector<int> projection_xs;

	//默认下方是没有的干扰的，简直了

	for (int j = 0; j < contours.size(); j++){

		Rect bound_rect = boundingRect(contours[j]);

		Point temp_point(bound_rect.width, bound_rect.height);


		//不允许太小
		if (temp_point.x < 30 || temp_point.y < 10){
			drawContours(src, contours, j, Scalar::all(0), -1);
			continue;
		}


		//不允许，高比宽大
		if (bound_rect.height * 1.0 / bound_rect.width > 0.95)	//这个值是观测出来的,避免用除法
		{
			if (bound_rect.height - bound_rect.width > 3){
				cout << "h > w" << endl;

				if (bound_rect.area() < standard_rect.area() * 0.8)
					drawContours(src, contours, j, Scalar::all(0), -1);
			}

			continue;
		}




		double b_ratio = compareRect(bound_rect, standard_rect);

		if (b_ratio >= 0.85){

			//cout << bound_rect.height *1.0 / bound_rect.width << endl;

			//这个检测貌似没什么鸟用
			approx.clear();
			approxPolyDP(Mat(contours[j]), approx, arcLength(Mat(contours[j]), true)*0.12, true);
			if (approx.size() != 4 || !isContourConvex(Mat(approx)))
				continue;

			Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
			//drawContours(disp, contours, j, color, 1);
			//imshow("disp", disp);
			//waitKey(0);
			rects.push_back(bound_rect);

			// 			projection_xs.push_back(bound_rect.x);
			// 			projection_xs.push_back(bound_rect.br().x);


			//还是空的时候
			//legal rect
			if (row_rects.size() == 0){
				Rects temp_rects;
				temp_rects.rects.push_back(bound_rect);
				temp_rects.average = bound_rect.y;
				row_rects.push_back(temp_rects);
			}
			else{
				double y_delta;
				int n1 = row_rects.size();
				int n2 = row_rects[n1 - 1].rects.size();

				//默认上升方式
				y_delta = abs(bound_rect.y - row_rects[n1 - 1].average);

				if (y_delta > standard_rect.width + bound_rect.height)
					continue;

				//同一行的y应该要小于等于3.0个像素
				if (y_delta <= 8.0){
					row_rects[n1 - 1].rects.push_back(bound_rect);
					row_rects[n1 - 1].average = (row_rects[n1 - 1].average * n2 + bound_rect.y) / (n2 + 1);
				}
				else{
					Rects temp_rects;
					temp_rects.rects.push_back(bound_rect);
					temp_rects.average = bound_rect.y;
					row_rects.push_back(temp_rects);
				}
			}

		}
	}

	if (debug_flag){
		imshow("1_src", src);
	}


	vector<Rect> x_rects;


	//擦除，剩下的全是合法的rect
	for (int i = 0; i < row_rects.size(); i++){
		//擦除不合法的rect
		if (row_rects[i].rects.size() < 2){
			row_rects.erase(row_rects.begin() + i);
			i--;
			continue;
		}
	}

	//cout << "row_rects.size = " << row_rects.size() << endl;


	//找到delta
	vector<double> row_delta;
	const int array_length = 50;
	int array_delta[array_length];

	memset(array_delta, 0, sizeof(int)* array_length);

	for (int i = 0; i < row_rects.size(); i++){

		//先将row_rects 排序
		sort(row_rects[i].rects.begin(), row_rects[i].rects.end(), sort_x_rect);

		for (int j = 0; j < row_rects[i].rects.size() - 1; j++){
			int temp_delta = row_rects[i].rects[j + 1].x - row_rects[i].rects[j].br().x;
			if (temp_delta < standard_rect.width){
				row_delta.push_back(temp_delta);

				array_delta[temp_delta]++;

			}

		}
	}

	sort(row_delta.begin(), row_delta.end());

	for (int i = 0; i < row_delta.size(); i++){
		cout << row_delta[i] << "===============" << endl;
	}


	double max_counter_delta = 0;
	double max_index = 0;

	for (int i = 0; i < array_length; i++){
		if (array_delta[i] > max_counter_delta){
			max_counter_delta = array_delta[i];
			max_index = i;
		}
	}

	int counter = 0;
	double delta_sum = 0;

	for (int i = 0; i < row_delta.size(); i++){

		if (abs(row_delta[i] - max_index) <= 1.0){
			counter++;
			delta_sum += row_delta[i];
		}
	}


	double delta1 = 0;

	if (counter == 0){
		cout << "delta counter == 0" << endl;
		return -1;
	}

	delta1 = delta_sum / counter;

	cout << "delta1 = " << delta1 << endl;


	Mat disp_before(src.size(), CV_8UC3, Scalar::all(0));
	Mat disp_after = disp_before.clone();


	for (int i = 0; i < row_rects.size(); i++){
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		for (int j = 0; j < row_rects[i].rects.size(); j++){
			rectangle(disp_before, row_rects[i].rects[j], color, 1);
		}
		//cout << endl;
	}


	//修补矩形
	for (int i = 0; i < row_rects.size(); i++){

		for (int j = 0; j < row_rects[i].rects.size(); j++){
			compensateRect(row_rects[i].rects, row_rects[i].rects[j], standard_rect, delta1, src);
		}
	}



	for (int i = 0; i < row_rects.size(); i++){
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		for (int j = 0; j < row_rects[i].rects.size(); j++){
			rectangle(disp_after, row_rects[i].rects[j], color, 1);
		}
		//cout << endl;
	}


	//imshow("before", disp_before);
	//imshow("after ", disp_after);


	for (int i = 0; i < row_rects.size(); i++){
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		for (int j = 0; j < row_rects[i].rects.size(); j++){
			rectangle(disp, row_rects[i].rects[j], color, 1);

			//cout << row_rects[i].rects[j].height * 1.0 / row_rects[i].rects[j].width << " ";

			x_rects.push_back(row_rects[i].rects[j]);


			projection_xs.push_back(row_rects[i].rects[j].x);
			projection_xs.push_back(row_rects[i].rects[j].br().x);
		}
		//cout << endl;
	}



	if (projection_xs.size() < 12){
		cout << "projection_xs.size() = " << projection_xs.size() << endl;
		cout << "projection_xs.size error" << endl;
		return -1;
	}
	//cout << "projection_xs.size ======================= " << projection_xs.size() << endl;


	sort(x_rects.begin(), x_rects.end(), sort_x_rect);

	vector<vector<Rect>> col_rects;

	double x_threshold = standard_rect.width * 1.6;
	//double x_threshold = standard_rect.width;

	//cout << "x_threshold = " << x_threshold << endl;

	for (int i = 0; i < x_rects.size() - 1; i++){

		//cout << "abs(x_rects[i + 1].x - x_rects[i].br().x) = " << abs(x_rects[i + 1].x - x_rects[i].br().x) << endl;

		if (abs(x_rects[i + 1].x - x_rects[i].br().x) < x_threshold){
			if (col_rects.size() == 0){
				vector<Rect> temp_rects;
				temp_rects.push_back(x_rects[i]);
				col_rects.push_back(temp_rects);
			}
			else{
				col_rects[col_rects.size() - 1].push_back(x_rects[i]);
			}
		}
		else{
			// 
			// 			if (col_rects.size() == 0){
			// 				vector<Rect> temp_rects;
			// 				temp_rects.push_back(x_rects[i]);
			// 				col_rects.push_back(temp_rects);
			// 			}
			if (col_rects.size() == 0){
				vector<Rect> temp_rects;
				//temp_rects.push_back(x_rects[i]);
				col_rects.push_back(temp_rects);
				col_rects[col_rects.size() - 1].push_back(x_rects[i]);

			}
			else{
				col_rects[col_rects.size() - 1].push_back(x_rects[i]);

				vector<Rect> temp_rects;
				//temp_rects.push_back(x_rects[i]);
				col_rects.push_back(temp_rects);

			}



		}
	}

	if (col_rects.size() == 0){
		vector<Rect> temp_rects;
		temp_rects.push_back(x_rects[x_rects.size() - 1]);
		col_rects.push_back(temp_rects);
	}
	else{
		col_rects[col_rects.size() - 1].push_back(x_rects[x_rects.size() - 1]);
	}




	//cout << "col_rect.size = " << col_rects.size() << endl;
	vector<int> col_rects_num;


	Mat disp1 = disp.clone();
	disp1.setTo(Scalar::all(0));

	for (int i = 0; i < col_rects.size(); i++){

		sort(col_rects[i].begin(), col_rects[i].end(), sort_x_rect);


		int current_num = col_rects[i].size();

		double current_w = col_rects[i][current_num - 1].br().x - col_rects[i][0].x - 2.5* delta1;

		double num = current_w / standard_rect.width;
		col_rects_num.push_back(int(num));

		cout << "current rect num = " << num << endl;

		Scalar color(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));

		for (int j = 0; j < col_rects[i].size(); j++){
			rectangle(disp1, col_rects[i][j], color, 1);
		}
	}

	//imshow("disp_col", disp1);


	// 	sort(projection_xs.begin(), projection_xs.end());
	// 	vector<int> delta_xs;
	// 
	// 	//cout << "standard.size " << standard_rect.size() << endl;
	// 
	// 	for (int i = 0; i < projection_xs.size() -1; i++){
	// 		//cout << projection_xs[i] << " ";
	// 		int delta = projection_xs[i + 1] - projection_xs[i];
	// 
	// 		//两个之间的间隙应该要大于2，但是当矩形框不水平，就糙了。
	// 		if (delta < 2.0)
	// 			continue;
	// 
	// 		delta_xs.push_back(delta);
	// 	}
	// 
	// 
	// 	sort(delta_xs.begin(), delta_xs.end());
	// 
	// 
	// 	//每列选择题之间的距离
	// 	vector<int> cols_delta;
	// 	
	// 	
	// 	for (int i = delta_xs.size() - 1; i >= 0; i--){
	// 		
	// 		//初始化cols_delta
	// 		if (cols_delta.size() == 0){
	// 			cols_delta.push_back(delta_xs[i]);
	// 			continue;
	// 		}
	// 
	// 		double  average = 0;
	// 		for (int j = 0; j < cols_delta.size(); j++){
	// 			average += cols_delta[j];
	// 		}
	// 		average = average / cols_delta.size();
	// 
	// 		
	// 		if (abs(delta_xs[i] - average) <= 3.0){
	// 			cols_delta.push_back(delta_xs[i]);
	// 		}
	// 		else{
	// 			break;
	// 		}
	// 	}

	//代表了col数-1
	// 	for (int i = 0; i < cols_delta.size(); i++){
	// 		cout << cols_delta[i] << " ";
	// 	}
	// 	cout << endl;

	// 	double x_start = 0;
	// 	double x_end = 0;
	// 
	// 	for (int i = 0; i < projection_xs.size(); i++){
	// 		if (abs(projection_xs[i] - projection_xs[0]) <= 3.0){
	// 			x_start += projection_xs[i];
	// 		}
	// 		else{
	// 			x_start = x_start / (i);
	// 			break;
	// 		}
	// 	}
	// 
	// 	
	// 	for (int i = projection_xs.size() - 1; i < projection_xs.size(); i--){
	// 		if (abs(projection_xs[i] - projection_xs[projection_xs.size() - 1]) <= 3.0){
	// 			x_end += projection_xs[i];
	// 		}
	// 		else{
	// 			x_end = x_end / (projection_xs.size() - i );
	// 			break;
	// 		}
	// 	}

	double xx_step1 = 0;

	double total_length = 0;
	double total_rects_num = 0;

	for (int i = 0; i < col_rects.size(); i++){

		int current_num = col_rects[i].size();

		total_rects_num += col_rects_num[i];
		total_length += (col_rects[i][current_num - 1].br().x - col_rects[i][0].x);
	}

	//cout << "total_rects_num = " << total_rects_num << endl;

	xx_step1 = (total_length - standard_rect.width * total_rects_num) / (total_rects_num - col_rects.size());

	cout << "xx_step1 = " << xx_step1 << endl;

	//xx_step1 = delta1;


	//这里的距离还可以优化
	//
	//double x_length = projection_xs[projection_xs.size() - 1] - projection_xs[0];
	Rect last_rect, first_rect;
	last_rect = *((*(col_rects.end() - 1)).end() - 1);
	first_rect = *((*col_rects.begin()).begin());

	vector<double> col_steps;
	for (int i = 0; i < col_rects.size() - 1; i++){
		double step = 0;
		step = col_rects[i + 1][0].x - col_rects[i][col_rects[i].size() - 1].br().x;
		col_steps.push_back(step);
	}

	sort(col_steps.begin(), col_steps.end());

	for (int i = 0; i < col_steps.size(); i++){
		cout << col_steps[i] << endl;
	}

	double average_col_step = 0;

	for (int i = 0; i < col_steps.size(); i++){
		//cout << col_steps[i] << endl;
		if (i == 0){
			average_col_step = col_steps[i];
		}
		else{
			if (abs(col_steps[i] - average_col_step) <= 6.0){
				average_col_step = ((average_col_step * i) + col_steps[i]) / (i + 1);
			}
			else{
				break;
			}

		}
	}


	//cout << "average_col_step = " << average_col_step <<endl;

	if (col_rects_num[0] < 4){

		//cout << "1" << endl;
		if (col_rects_num.size() < 2){
			cout << "col_rects_num.size too small" << endl;
			return -1;
		}

		if (col_rects_num[1] < 4){
			cout << " col_rects_num[1] < 4" << endl;
			return -1;
		}

		//小于，说明头上那个没有了，要补偿

		double 	temp_delta = col_rects[1][0].x - (*(col_rects[0].end() - 1)).br().x;
		double temp_threshold = standard_rect.width*0.5 + average_col_step;

		//cout << "temp_delta = " << temp_delta << ", temp_threshold = " << temp_threshold << endl;

		if (temp_delta < temp_threshold){
			first_rect.x -= (standard_rect.width + xx_step1);
			if (first_rect.x < 0)
				first_rect.x = 0;
			//cout << "补偿111" << endl;
		}
	}

	//最后一行
	if (*(col_rects_num.end() - 1) < 4){
		if (col_rects_num.size() < 2){
			cout << "col_rects_num.size too small" << endl;
			return -1;
		}
		int n = col_rects_num.size();

		if (col_rects_num[n - 2] < 4){
			cout << " col_rects_num[n - 2] < 4" << endl;
			return -1;
		}

		int col_rects_n = col_rects.size();

		if (col_rects[col_rects_n - 1][0].x - col_rects[col_rects_n - 2][col_rects[col_rects_n - 2].size() - 1].br().x < standard_rect.width * 0.5 + average_col_step){
			last_rect.x += (standard_rect.width + xx_step1);
			if (last_rect.br().x > src.cols - 1){
				last_rect.x = src.cols - last_rect.width;
			}
		}
	}

	double x_length = last_rect.br().x - first_rect.x;


	//double x_length = x_end - x_start;

	if (x_length < 0){
		cout << "x_lenght error" << endl;
		return -1;
	}


	// 	for (int i = 0; i < cols_delta.size(); i++){
	// 		x_length = x_length - cols_delta[i];
	// 	}
	// 
	// 	x_length = x_length - 4 * (cols_delta.size() + 1) * standard_rect.width;
	// 	double xx_step = x_length  / (3 * (cols_delta.size() + 1));
	// 
	// 	cout << "xx_step = " << xx_step << endl;

	double xx_step = xx_step1;



	// 	line(src_copy, Point(round(projection_xs[0]), 0), Point(projection_xs[0], 1000), CV_RGB(0, 0, 255), 1);
	// 	line(src_copy, Point(round(projection_xs[projection_xs.size() -1]), 0), Point(projection_xs[projection_xs.size() - 1], 1000), CV_RGB(0, 0, 255), 1);

	vector<double> xs, ys;

	for (int i = 0; i < col_rects.size(); i++){

		//double x_start = projection_xs[0] + i * (4 * (standard_rect.width + xx_step) - xx_step + cols_delta[0]);
		double x_start = first_rect.x + i * (4 * (standard_rect.width + xx_step) - xx_step + average_col_step);



		for (int j = 0; j < 4; j++){
			double x1 = x_start + j * (standard_rect.width + xx_step);
			line(src_copy, Point(round(x1), 0), Point(x1, 1000), CV_RGB(255, 0, 0), 1);
			xs.push_back(x1);
		}
	}

	for (int i = 0; i < row_rects.size(); i++){
		double sum = 0;

		for (int j = 0; j < row_rects[i].rects.size(); j++){
			sum = sum + row_rects[i].rects[j].y;
		}

		sum = sum / row_rects[i].rects.size();
		ys.push_back(sum);


	}

	sort(ys.begin(), ys.end());

	for (int i = 0; i < ys.size(); i++){
		line(src_copy, Point(0, round(ys[i])), Point(1000, ys[i]), CV_RGB(0, 0, 255), 1);
	}

	double ratio_threshold = 0.70;

	//double ratio_threshold2 = 0.8;

	double single_choise_threshold = 0.55;

	//题目总数判断错误
	if (ys.size() * col_rects.size() < item_num){
		cout << "detected item less than item_num, error" << endl;
		return -1;
	}


	int item_counter = 0;

	//result.clear();

	vector<int> results;

	Mat temp_gray = src_gray.clone();

	//开始数答案了
	for (int i = 0; i < ys.size(); i++){


		for (int j = 0; j < col_rects.size(); j++){
			int current_result = 0;

			//double x_start = projection_xs[0] + j * (4 * (standard_rect.width + xx_step) - xx_step + cols_delta[0]);
			double x_start = first_rect.x + j * (4 * (standard_rect.width + xx_step) - xx_step + average_col_step);

			double ratios[4];
			int indice[4];
			double no_ratios[4];


			Rect temp_current_abcd_rect(round(x_start) - 2, ys[i] - 2, standard_rect.width * 4 + xx_step1 * 3 + 4, standard_rect.height + 4);
			if (temp_current_abcd_rect.x < 0)
				temp_current_abcd_rect.x = 0;
			if (temp_current_abcd_rect.y < 0)
				temp_current_abcd_rect.y = 0;

			if (temp_current_abcd_rect.br().x >= src.cols){
				temp_current_abcd_rect.x = src.cols - temp_current_abcd_rect.width;
			}

			if (temp_current_abcd_rect.br().y >= src.rows){
				temp_current_abcd_rect.y = src.rows - temp_current_abcd_rect.height;
			}




			Mat temp_roi = temp_gray(temp_current_abcd_rect);


			threshold(temp_roi, temp_roi, 2, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);

			// 			imshow("sfad", temp_gray);
			// 			waitKey(0);

			for (int k = 0; k < 4; k++){
				double x1 = x_start + k * (standard_rect.width + xx_step);


				Rect roi_rect(round(x1), ys[i], standard_rect.width + 1, standard_rect.height + 1);

				if (roi_rect.x < 0)
					roi_rect.x = 0;
				if (roi_rect.br().x >= src.cols){
					roi_rect.x = src.cols - roi_rect.width;
				}
				if (roi_rect.y < 0)
					roi_rect.y = 0;

				if (roi_rect.br().y >= src.rows){
					roi_rect.y = src.rows - roi_rect.height;
				}

				const int distance_threshold = 10;
				for (int i1 = 0; i1 < col_rects[j].size(); i1++){
					Point2f p1 = col_rects[j][i1].tl();
					Point2f p2 = roi_rect.tl();

					Point2f delta = p1 - p2;
					double distance = sqrt(delta.x * delta.x + delta.y * delta.y);

					if (distance < distance_threshold){
						//cout << "find proper rect" << endl;
						//roi_rect = col_rects[j][i1];
						break;
					}
				}


				rectangle(src_copy, roi_rect, CV_RGB(255, 0, 0), 1);

				Mat roi(src, roi_rect);
				//Mat roi(temp_gray, roi_rect);

				Mat roi1(src_gray, roi_rect);
				//cout << roi1 << endl;
				double sum = 0;

				int counter = 0;
				for (int r = 0; r < roi1.rows; r++){
					for (int c = 0; c < roi1.cols; c++){
						sum += (int)roi1.at<uchar>(r, c);
						counter++;
					}
				}

				//cout << "counter = " << counter << ", w*h = " << roi1.rows * roi1.cols << ", area = " << roi_rect.area() << endl;

				double ratio1 = 1.0 - (sum) / (255 * roi1.cols * roi1.rows);
				//cout << ratio1 << " ";

				ratios[k] = ratio1;
				indice[k] = k;

				double no_zero = countNonZero(roi);

				double ratio = no_zero / roi_rect.area();
				no_ratios[k] = ratio;

				if (ratio >= ratio_threshold){

					//cout << "ratio >= ratio_threshold" << endl;
					//current_result |= (1 << k);
					//circle(src_copy, Point(x1, ys[i]),5 , CV_RGB(255, 0, 0), -1);
				}
			}



			//cout << "========";


			//排序
			for (int q = 0; q < 4; q++){

				for (int p = q + 1; p < 4; p++){

					if (no_ratios[p] > no_ratios[q]){

						double temp_ratio = no_ratios[q];
						no_ratios[q] = no_ratios[p];
						no_ratios[p] = temp_ratio;

						int temp_index = indice[q];
						indice[q] = indice[p];
						indice[p] = temp_index;
					}
				}
			}


			// 			for (int q = 0; q < 4; q++){
			// 				cout << ratios[q] << " ";
			// 			}
			// 
			// 			for (int q = 0; q < 4; q++){
			// 				cout << indice[q] << " ";
			// 			}
			//			cout << endl;

			if (multi_flag == 0){

				const int single_choice_threshold = 0.6;


				//cout << endl;

				if (no_ratios[0] >= single_choise_threshold){
					current_result = current_result | (1 << indice[0]);

					double x1 = x_start + indice[0] * (standard_rect.width + xx_step);

					circle(src_copy, Point(x1, ys[i]), 5, CV_RGB(255, 0, 0), -1);
				}

				for (int q = 0; q < 4; q++){
					//cout << no_ratios[q] << " ";

					int temp_int = no_ratios[q] * 1000;
					char buffer[10];
					sprintf_s(buffer, "%d", temp_int);
					string temp_str(buffer);

					//cout << temp_str << endl;
					double x1 = x_start + indice[q] * (standard_rect.width + xx_step);


					putText(src_copy, temp_str, Point(x1, ys[i]), CV_FONT_HERSHEY_PLAIN, 1, CV_RGB(0, 0, 255), 1);

				}


			}

			// 			for (int q = 0; q < 3; q++){
			// 				if (/*ratios[q] - ratios[q + 1] > 0.1 || */no_ratios[indice[q]] > ratio_threshold){
			// 					
			// 					current_result = current_result | (1 << indice[q]);
			// 					double x1 = x_start + indice[q] * (standard_rect.width + xx_step);
			// 					circle(src_copy, Point(x1, ys[i]), 5, CV_RGB(255, 0, 0), -1);
			// 				}
			// 			}
			// 
			// 			if (no_ratios[indice[3]] > ratio_threshold){
			// 				current_result = current_result | (1 << indice[3]);
			// 				double x1 = x_start + indice[3] * (standard_rect.width + xx_step);
			// 				circle(src_copy, Point(x1, ys[i]), 5, CV_RGB(255, 0, 0), -1);
			// 			}
			// 
			// 			if (current_result == 0){
			// 
			// 				//cout << "current = 0" << endl; 
			// 				double max_value = 0;
			// 				int max_index = 0;
			// 
			// 				for (int i = 0; i < 4; i++){
			// 					if (no_ratios[i] > max_value){
			// 						max_value = no_ratios[i];
			// 						max_index = i;
			// 					}
			// 				}
			// 
			// 				//cout << "max_value =  " << max_value << ", index = " << max_index << endl;
			// 				if (max_value >= single_choise_threshold){
			// 					current_result = current_result | (1 << max_index);
			// 					double x1 = x_start + max_index * (standard_rect.width + xx_step);
			// 					circle(src_copy, Point(x1, ys[i]), 5, CV_RGB(255, 0, 0), -1);
			// 				}
			// 			}



			item_counter++;
			results.push_back(current_result);
			if (item_counter >= item_num){
				goto return_positon;
			}
		}
		//cout << endl;
	}



return_positon:
	if (debug_flag){
		imshow("src_copy", src_copy);
		//resize(src, src, src.size() / 2);
		//imshow("disp", disp);
	}

	(*ques_num) = item_counter;

	if (item_counter > 256){
		cout << "item num is too large!" << endl;
		return -1;

	}


	for (int i = 0; i < item_counter; i++){
		result[i] = results[i];
	}


	return 0;

}


//输入必须是单通道，即灰度数据，若不是，则返回-1
//必须找到三个以上（包含三个）的定位点，否则即为定位失败， 返回-1
//找到3个点返回3
//找到4个点返回4
int ScoreEngine::getMarkedPoints(const Mat & input_src, const int search_x, const int search_y){

	if (input_src.empty() || input_src.channels() != 1){
		cout << "input_src is empty or channel error!" << endl;
		return -1;
	}

	if (search_x <0 || search_x > input_src.cols || search_y <0 || search_y > input_src.rows)
		return -1;


	Rect search_rects[4];
	search_rects[0] = Rect(0, 0, search_x, search_y);
	search_rects[1] = Rect(input_src.cols - search_x, 0, search_x, search_y);
	search_rects[2] = Rect(input_src.cols - search_x, input_src.rows - search_y, search_x, search_y);
	search_rects[3] = Rect(0, input_src.rows - search_y, search_x, search_y);

	Mat result;

	int result_cols = search_x - templ.cols + 1;
	int result_rows = search_y - templ.rows + 1;

	result.create(result_cols, result_rows, CV_32FC1);

	int match_method = 0;

	int threshold = 1000000;
	int counter = 0;
	for (int i = 0; i < 4; i++){
		Mat img = input_src(search_rects[i]);
		/// 进行匹配和标准化
		matchTemplate(img, templ, result, match_method);
		//normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());

		/// 通过函数 minMaxLoc 定位最匹配的位置
		double minVal; 
		double maxVal; 
		Point minLoc; 
		Point maxLoc;
		Point matchLoc;

		minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
		//normalize(result, result, 0, 1, NORM_MINMAX, -1, Mat());


		/// 对于方法 SQDIFF 和 SQDIFF_NORMED, 越小的数值代表更高的匹配结果. 而对于其他方法, 数值越大匹配越好
		if (match_method == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED){
			matchLoc = minLoc;
		}
		else{
			matchLoc = maxLoc;
		}

		if (minVal < threshold){
			//threshold 为10^6
			marked_points[i].x = search_rects[i].x + matchLoc.x + template_r - 1;
			marked_points[i].y = search_rects[i].y + matchLoc.y + template_r - 1;
			counter++;
			//cout << "match value = " << minVal << "===============================" << endl;
		}
		else{
			marked_points[i].x = -1;
			marked_points[i].y = -1;
		}

// 		//threshold 为10^6
// 		marked_points[i].x = search_rects[i].x + matchLoc.x + template_r - 1;
// 		marked_points[i].y = search_rects[i].y + matchLoc.y + template_r - 1;
		
	}
	
	return counter;
}


//用最小二乘法拟合，好像矩阵运算有可以优化的地方

int ScoreEngine::getHomoMatrix(const Point2f * src_points, const Point2f * dst_points, const int n, Mat & homo){
	clock_t t1, t2;
	t1 = clock();
	if (n < 3){
		return -1;
	}

	Mat X(2 * n, 6, CV_32FC1, Scalar::all(0));
	Mat Y(2 * n, 1, CV_32FC1);

	//赋值
	for (int i = 0; i < n; i++){
		X.at<float>(i * 2, 0) = src_points[i].x;
		X.at<float>(i * 2, 1) = src_points[i].y;
		X.at<float>(i * 2, 2) = 1;

		X.at<float>(i * 2 + 1, 3) = src_points[i].x;
		X.at<float>(i * 2 + 1, 4) = src_points[i].y;
		X.at<float>(i * 2 + 1, 5) = 1;

		Y.at<float>(i * 2, 0) = dst_points[i].x;
		Y.at<float>(i * 2 + 1, 0) = dst_points[i].y;
	}

	homo.create(2, 3, CV_32FC1);
	Mat temp(6, 1, CV_32FC1);
	Mat Xt = X.t();
	temp = (Xt* X).inv() * Xt * Y;
	
	//cout << temp << endl;

	for (int i = 0; i < 6; i++){
		homo.at<float>(i / 3, i % 3) = temp.at<float>(i, 0);
	}

	t2 = clock();
	//cout << "time11111111 = " << t2 - t1 << endl;
		
	return 0;
}


//初始化标准
int ScoreEngine::initStandardMarkedPoints(const Mat & input_src){
	
	if (getMarkedPoints(input_src, 500, 500) != 4){
		cout << "failed to initialize locating the standard marked points" << endl;
		return -1;
	}

	standard_mark_points[0] = marked_points[0];

	double dx;
	Point2f dx_point = marked_points[1] - marked_points[0];
	dx = sqrt(dx_point.x * dx_point.x + dx_point.y * dx_point.y);

	standard_mark_points[1].x = standard_mark_points[0].x + dx;
	standard_mark_points[1].y = standard_mark_points[0].y;

	double dy; 
	Point2f dy_point = marked_points[3] - marked_points[0];
	dy = sqrt(dy_point.x * dy_point.x + dy_point.y* dy_point.y);
	
	standard_mark_points[3].x = standard_mark_points[0].x;
	standard_mark_points[3].y = standard_mark_points[0].y + dy;

	standard_mark_points[2].x = standard_mark_points[1].x;
	standard_mark_points[2].y = standard_mark_points[3].y;

	init_is_ready = true;

	return 0;

}


//找到四个点，用透射变化，
//找到三个点用仿射变化
//找到两个点用旋转，暂时不考虑
//找到的点小于等于2时，返回-1
//正常时返回0
int ScoreEngine::alignImage(const Mat & input_src, Mat & output_dst){

	if (input_src.empty()){
		return -1;
	}
	if (input_src.channels() != 1){
		return -1;
	}
	if (init_is_ready == false){

		//在使用这个类之前要初始化这个
		int flag  = initStandardMarkedPoints(input_src);
		if (flag < 0){
			cout << "initial is not ready, please check!" << endl;
			return -1;
		}

	}

	getMarkedPoints(input_src, 400, 400);

	Point2f temp_standard_points[4];
	Point2f temp_marked_points[4];

	int counter = 0;

	for (int i = 0; i < 4; i++){
		if (marked_points[i].x < 0)
			continue;

		temp_marked_points[counter] = marked_points[i];
		temp_standard_points[counter] = standard_mark_points[i];


		counter++;

	}

	clock_t t1 = clock();

	output_dst.create(input_src.size(), input_src.type());



	clock_t t2 = clock();

	//cout << "time----------------------------------" << t2 - t1 << endl;



	if (counter == 0){
		output_dst = input_src.clone();

		return 0;
	}
	else if (counter == 1){


		return 1;
	}
	else if (counter == 2){


		return 2;
	}
	//Affine
	else if (counter == 3){
		Mat affine_homo;
		getHomoMatrix(temp_marked_points, temp_standard_points, 3, affine_homo);
		warpAffine(input_src, output_dst, affine_homo, output_dst.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar::all(255));

// 		for (int i = 0; i < 4; i++){
// 			circle(output_dst, standard_mark_points[i], 3, CV_RGB(0, 255, 0), -1);
// 		}

		return 3;
	}
	else if (counter >= 4){
		Mat warp_mat = getPerspectiveTransform(temp_marked_points, temp_standard_points);
		warpPerspective(input_src, output_dst, warp_mat, output_dst.size(), INTER_LINEAR, BORDER_CONSTANT, Scalar::all(255));


// 		for (int i = 0; i < 4; i++){
// 			circle(output_dst, standard_mark_points[i], 3, CV_RGB(255, 0, 0), -1);
// 		}

		return 4;
	}

	return 0;
}

string ScoreEngine::recognizeBarcode(const Mat & src){

	string result;

	string temp_name = "adadlsajdio.jpg";
	string barcode_folder = "./barcode/";

	if (src.empty()){
		cout << "src is emmty!" << endl;
		return result;
	}

	imwrite(barcode_folder + temp_name, src);


	char buffer[256];
	getcwd(buffer, 256);

	string cmd;
	string bs(buffer);


	string folder;
	for (int i = 0; i < bs.size(); i++){
		if (bs[i] == '\\'){
			folder.push_back('/');

		}
		else{
			folder.push_back(bs[i]);
		}
	}
	//cout << folder << endl;

	cmd = cmd + folder + "/barcode/barcode.exe " + folder + "/barcode/" + temp_name;

;

	system(cmd.c_str());

	//cout << "cmd = " << cmd << endl;
	ifstream in_file("./result.txt");
	if (!in_file.is_open()){
		cout << "failed to get result !" << endl;
		return result;
	}

	getline(in_file, result);

	in_file.close();
	

	if (!remove("result.txt")){
		//cout << "OK" << endl;
	}
	else{
		//cout << "failed to remove" << endl;
	}

	return result;


}


Point2f ScoreEngine::getStandardPoint(int i){
	if (init_is_ready == false || i < 0 || i >=4){
		return Point2f(0, 0);
	}


	return standard_mark_points[i];
}
