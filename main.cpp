#include <iostream>
#include <vector>
#include <fstream>
#include <io.h>
#include <stdlib.h>
#include <ctime>

#include <set>  


#include <opencv2/opencv.hpp>

//local
#include "ScoreEngine.h"

#include <direct.h>

using namespace std;


void testID();

void getSamples();


void testScore();

void testOption();

void testLocation();

void testBarcode();


bool left_button_down = false;


char * win_name = "src";
Rect sample_rect;

static int start_index = 1;

Mat dst;

void mouseHandle(int event, int x, int y, int flags, void* userdata){
	//cout << x << " " << y << endl;

	Mat src;

	Mat * p_src = (Mat *)userdata;

	resize(*p_src, src, p_src->size() / 2);

	imshow(win_name, src);

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN:
		left_button_down = true;
		sample_rect.x = x;
		sample_rect.y = y;
		sample_rect.width = 0;
		sample_rect.height = 0;
		break;
	case CV_EVENT_MOUSEMOVE:
		if (left_button_down){
			sample_rect.width = abs(x - sample_rect.x);
			sample_rect.height = abs(y - sample_rect.y);

			if (x < sample_rect.x){
				sample_rect.x = x;
			}
			if (y < sample_rect.y){
				sample_rect.y = y;
			}
			rectangle(src, sample_rect, CV_RGB(255, 0, 0));
			imshow(win_name, src);
			break;
		}
	case CV_EVENT_LBUTTONUP:

		if (left_button_down){
			left_button_down = false;

			string filename = "./test_score3/";
			char buffer[10];

			sprintf_s(buffer, "%d", start_index);
			start_index++;
			filename = filename + buffer + ".jpg";

			Rect temp_src(sample_rect.x * 2, sample_rect.y * 2, sample_rect.width * 2, sample_rect.height * 2);


			Mat roi = (*p_src)(temp_src);
			imwrite(filename, roi);

			line(*p_src, temp_src.tl(), Point(temp_src.x + temp_src.width, temp_src.y), CV_RGB(255, 0, 0));
			//rectangle(*((Mat *)userdata), sample_rect, CV_RGB(255, 0, 0));
		}

	default:
		break;
	}
}



int main(){

	//getSamples();
	testScore();
	//testID();
	//testOption();

	//testLocation();
	//ScoreEngine scoreEngine;

	//testBarcode();

	return 0;

}


void getSamples(){

	Mat src = imread("./sample/a3-150-003.jpg");

	dst = src.clone();
	
	namedWindow(win_name, CV_WINDOW_AUTOSIZE);

	//imshow(win_name, src);

	setMouseCallback(win_name, mouseHandle, &src);

	while (1){
		char c = waitKey(0);
		if (c == 32){
			break;
		}
	}


}

void testID(){

	ScoreEngine score_engine;

	vector<string> filenames;
	string folder = "./testID1/";

	//filenames.push_back("20150630english801-0003.jpg");

	score_engine.getAllFiles(folder, filenames);

	clock_t t1, t2;

	for (int i = 0; i < filenames.size(); i++){
		t1 = clock();

		cout << i << " th test==========================================" << endl;

		Mat src = imread(folder + filenames[i], CV_LOAD_IMAGE_GRAYSCALE);
		//Mat src = imread(folder + "00.jpg", CV_LOAD_IMAGE_GRAYSCALE);

		if (src.empty()){
			cout << "src empty" << endl;
			continue;
		}

		Rect search_rect(0, 0, src.cols, src.rows);

//		search_rect = Rect(Point(638, 238), Point(1166, 702));
// 		if (i == 0){
// 			search_rect = Rect(Point(434, 600), Point(948, 882));
// 		}

		char result_id[32];
		int output_len;
		int return_value = score_engine.scanSpottedID(src, search_rect, 6, result_id, &output_len, 1);
		cout << "output len = " << output_len << endl;

		if (return_value == -1){
			cout << "illegal please check!" << endl;
		}
		else if (return_value == -2){
			cout << "undefined" << endl;
		}
		else if (return_value == -3){
			cout << "multi-defined" << endl;
		}
		else if (return_value == 0){
			cout << "result = " << result_id << endl;
		}

		waitKey(0);

	}
}

void testScore(){
	ScoreEngine score_engine;

	vector<string> filenames;
	string folder = "./test_score1/";

	score_engine.getAllFiles(folder, filenames);
	cout << "filenames.size = " << filenames.size() << endl;

	clock_t t1, t2;

	for (int i = 0; i < filenames.size(); i++){
		t1 = clock();
// 
		if (i < 18)
			continue;

		cout << i << " th test==========================================" << endl;

		Mat src = imread(folder + filenames[i], CV_LOAD_IMAGE_GRAYSCALE);

		if (src.empty()){
			cout << "src empty" << endl;
			continue;
		}


		Rect search_rect(0, 0, src.cols, src.rows);



		short bar_type = 0;		//数字框的类型，暂且不用
		short head_mark =0;	//输出识别的第一个空格有没有划过, 0,没有划过；1划过了
		short output_prime =0; //输出识别结果整数位, 测试期间可能会越界
		short output_decimal = 0;//输出识别结果小数位（5），0，没有；5，表示0.5被划过
		short prime_num = 0;
		
		//int return_value = score_engine.scanScoreSquare(src, search_rect, result_score, 1);


		int return_value = score_engine.scanScoreBar(src, search_rect, bar_type, &head_mark, &output_prime, &output_decimal, &prime_num);
		if (return_value == 0){
			cout << "head_mark = " << head_mark  << endl;
			cout << "output_prime = " << output_prime << endl;
			cout << "output_decimal = " << output_decimal << endl;
		}
		


		waitKey(0);

	}


}


void testOption(){

	ScoreEngine score_engine;

	vector<string> filenames;
	string folder = "./testOP/";

	//filenames.push_back("20150630english801-0003.jpg");

	score_engine.getAllFiles(folder, filenames);

	clock_t t1, t2;

	for (int i = 0; i < filenames.size(); i++){

		// 		if (i != 47 && i != 71 && i != 86 && i != 88 && i != 103 && i != 127)
		// 			continue;

		// 		if (i != 4 && i != 14 && i != 28 && i != 68 && i != 81 && i != 94 && i!= 103 )
		// 			continue;

		t1 = clock();

		cout << i << " th test==========================================" << endl;

		//Mat src = imread(folder + filenames[i], CV_LOAD_IMAGE_GRAYSCALE);
		Mat src = imread("./testOP/20157518215D103D00055.jpg", CV_LOAD_IMAGE_GRAYSCALE);

		if (src.empty()){
			cout << "src empty" << endl;
			continue;
		}

		Rect search_rect(0, 0, src.cols, src.rows);
		//Rect search_rect(Point(225*2, 387*2), Point(673*2, 469*2));
		//Rect search_rect(Point(442,1340), Point(1320,1438));

		search_rect = Rect(Point(246, 946), Point(1154, 1112));
		// 		if (i == 0){
		// 			search_rect = Rect(Point(434, 600), Point(948, 882));
		// 		}

		//vector<int> results;
		int result[256];
		int ques_num = 0;
		int item_num = 10;
		int exclusive_region[4];


		int return_value = score_engine.scanScoreOptionpanel_H(src, search_rect, exclusive_region, item_num, result, &ques_num, 0, 1);


		if (return_value == -1){
			cout << "illegal please check!" << endl;
		}
		else if (return_value == 0){


			cout << "normal" << endl;

			for (int i = 0; i < ques_num; i++){
				cout << result[i] << " ";
			}
			cout << endl;
		}


		waitKey(0);

	}

}


void testLocation(){


	ScoreEngine score_engine;

	vector<string> filenames;
	string folder = "./test_location2/";

	//filenames.push_back("20150630english801-0003.jpg");

	score_engine.getAllFiles(folder, filenames);

	int match_method = 0;
	clock_t t1, t2;

	double total_time = 0;


	Point2f srcTri[3];
	Point2f dstTri[3];

	Mat rot_mat(2, 3, CV_32FC1);
	Mat warp_mat(2, 3, CV_32FC1);


	Mat src, warp_dst, warp_rotate_dst;

	for (int i = 0; i < filenames.size(); i++){

		cout << i << " th test==" << filenames[i] <<endl;

// 		if (i < 4)
// 			continue;
		
		Mat img = imread(folder + filenames[i], 0);

		warp_mat = Mat(2, 3, CV_32FC1);

		if (img.cols < img.rows){
			/// 设置源图像和目标图像上的三组点以计算仿射变换
			srcTri[0] = Point2f(0, 0);
			srcTri[1] = Point2f(img.cols - 1, 0);
			srcTri[2] = Point2f(img.cols - 1, img.rows -1);
			//srcTri[3] = Point2f(0, img.rows - 1);

			dstTri[0] = Point2f(0, img.cols-1);
			dstTri[1] = Point2f(0,0);
			dstTri[2] = Point2f(img.rows - 1, 0);
			//dstTri[3] = Point2f(img.rows - 1, img.cols - 1);

			/// 求得仿射变换
			//warp_mat = getAffineTransform(srcTri, dstTri);
			score_engine.getHomoMatrix(srcTri, dstTri, 3, warp_mat);
			

			/// 设置目标图像的大小和类型与源图像一致
			warp_dst = Mat::zeros(img.cols, img.rows, img.type());

			/// 对源图像应用上面求得的仿射变换

			warpAffine(img, warp_dst, warp_mat, warp_dst.size());

			img = warp_dst.clone();

		}
	
		Mat dst;
		

		int flag = score_engine.alignImage(img, dst);


		if (flag < 0){
			cout << "align image failed, flag = " << flag << endl;
			continue;
		}
			

		
		//Rect rect(Point2f(1250, 120), Point2f(2150, 180));
		//Rect rect(Point2f(1250, 360), Point2f(2150, 420));
		//Rect rect(Point2f(1250, 900), Point2f(2150, 960));

		Rect rect(Point2f(276, 418), Point2f(946, 774));

		//rectangle(dst, rect, CV_RGB(255, 0, 0));

		Mat image_disp;

		resize(dst, image_disp, dst.size() / 2);
		cvtColor(image_disp, image_disp, CV_GRAY2BGR);

		for (int i = 0; i < 4; i++){
			circle(image_disp, score_engine.getStandardPoint(i) / 2, 2, CV_RGB(255, 0, 0), -1);
		}

		Rect rect0(Point2f(133, 200), Point2f(486, 396));
		rectangle(image_disp, rect0, CV_RGB(255, 0, 0));

		Rect rect1(Point2f(63, 420), Point2f(632, 524));
		rectangle(image_disp, rect1, CV_RGB(255, 0, 0));

		Rect rect2(Point2f(63, 587), Point2f(632, 670));
		rectangle(image_disp, rect2, CV_RGB(255, 0, 0));

		Rect rect3(Point2f(63, 705), Point2f(632, 760));
		rectangle(image_disp, rect3, CV_RGB(255, 0, 0));

		Rect rect4(Point2f(63, 796), Point2f(632, 846));
		rectangle(image_disp, rect4, CV_RGB(255, 0, 0));

		Rect rect5(Point2f(640, 60), Point2f(1084, 97));
		rectangle(image_disp, rect5, CV_RGB(255, 0, 0));

		Rect rect6(Point2f(640, 184), Point2f(1084, 217));
		rectangle(image_disp, rect6, CV_RGB(255, 0, 0));

		Rect rect7(Point2f(640, 307), Point2f(1084, 351));
		rectangle(image_disp, rect7, CV_RGB(255, 0, 0));

		Rect rect8(Point2f(640, 440), Point2f(1084, 498));
		rectangle(image_disp, rect8, CV_RGB(255, 0, 0));

		Rect rect9(Point2f(640, 624), Point2f(1084, 657));
		rectangle(image_disp, rect9, CV_RGB(255, 0, 0));


		imwrite("./temp1/" + filenames[i], image_disp);
		imshow("img", image_disp);



		
		char result_id[32];
		int output_len;
		int return_value = score_engine.scanSpottedID(dst, rect, 6, result_id, &output_len, 1);

		cout << "output len = " << output_len << endl;

		if (return_value == -1){
			cout << "illegal please check!" << endl;
		}
		else if (return_value == -2){
			cout << "undefined" << endl;
		}
		else if (return_value == -3){
			cout << "multi-defined" << endl;
		}
		else if (return_value == 0){
			cout << "result = " << result_id << endl;
		}


// 		short bar_type = 0;		//数字框的类型，暂且不用
// 		short head_mark = 0;	//输出识别的第一个空格有没有划过, 0,没有划过；1划过了
// 		short output_prime = 0; //输出识别结果整数位, 测试期间可能会越界
// 		short output_decimal = 0;//输出识别结果小数位（5），0，没有；5，表示0.5被划过
// 		short prime_num = 0;
// 
// 		//int return_value = score_engine.scanScoreSquare(src, search_rect, result_score, 1);
// 
// 
// 		int return_value = score_engine.scanScoreBar(dst, rect, bar_type, &head_mark, &output_prime, &output_decimal, &prime_num);
// 		if (return_value == 0){
// 			cout << "head_mark = " << head_mark << endl;
// 			cout << "output_prime = " << output_prime << endl;
// 			cout << "output_decimal = " << output_decimal << endl;
// 		}
		
		//total_time = total_time + (t2 - t1);
		//cout << "time = " << t2 - t1 << " ms " << endl;
		waitKey(0);
	}
	cout << "average time = " << total_time / filenames.size() << endl;

	

}

void testBarcode(){

// 	char * cmd = "E:/VS2013/HandwritenDigit/ScoringSystem1_1/barcode/barcode.exe E:/VS2013/HandwritenDigit/ScoringSystem1_1/barcode/3.jpg";
// 
// 	system(cmd);

// 	char buffer[256];
// 	getcwd(buffer, 256);
// 
// 	string cmd;
// 	string bs(buffer);
// 
// 
// 	
// 
// 	string folder;
// 	for (int i = 0; i < bs.size(); i++){
// 		if (bs[i] == '\\'){
// 			folder.push_back('/');
// 			
// 		}
// 		else{ 
// 			folder.push_back(bs[i]);
// 		}
// 	}
// 	cout << folder << endl;
// 
// 	cmd = cmd + folder + "/barcode/barcode.exe " + folder + "/barcode/3.jpg";
// 
// 	cout << "cmd = " << cmd << endl;
// 
// 	
// 	system(cmd.c_str());



	string barcode_folder = "./barcode/";

	ScoreEngine score_engine;
	vector<string> filenames;

	string file_folder = "./test_barcode/";
	score_engine.getAllFiles(file_folder, filenames);

	for (int i = 0; i < filenames.size(); i++){

		cout << i << " th test " << endl;

		Mat src = imread(file_folder + filenames[i], CV_LOAD_IMAGE_GRAYSCALE);

		if (src.empty()){
			cout << "empty " << endl;
			continue;
		}

		string result = score_engine.recognizeBarcode(src);

		cout << " result ========================= " << result <<endl;
	}


}
