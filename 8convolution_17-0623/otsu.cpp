#include <iostream>
#include <vector>
#include<fstream>
#include<iostream>
#include<string>
#include<sstream> //文字ストリーム
#include<math.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <omp.h>	//openMP


//メモリ確保を行うためのヘッダ
#define ANSI				
#include "nrutil.h"	


using namespace cv;
using namespace std;

////////////////////////ヒストグラムの作成及び閾値の設定////////////////////////////////////////////////////////////////////////

int discriminantAnalysis(char date_directory4[], int &image_x, int &image_y, std::vector<std::vector<double>> &edge_st){

	
	int DATA_NUM = image_x *image_y;
	//double tmp = 0;
	double tmp_h = 0;
	double tmp_l = 0;
	double k;	//幅

	//Nrutilを用いたメモリの確保
	double **data = matrix(0, DATA_NUM -1, 0, 1);
//	double **tmp = matrix(0, DATA_NUM - 1, 0, 1);
	double **tmp = matrix(0, image_y - 1, 0, 1);

	//確保したメモリを初期化する
	for (int i = 0; i < 1; i++) {
		for (int j = 0; j < DATA_NUM; j++) {
			data[j][i] = 0;
//			tmp[j][i] = 0;
		}
	}

	for (int i = 0; i < 1; i++) {
		for (int j = 0; j < image_y - 1; j++) {
			tmp[j][i] = 0;
		}
	}

	//一次元配列に代入
	for (int i = 0; i < image_y; i++) {
		for (int j = 0; j < image_x; j++) {
			data[j*(i+1)][0] = edge_st[j][i];
		}
	}

	//行ごとにデータを大きい順に並び替える
#pragma omp parallel for
	for (int i = 0; i < image_y; ++i) {
		for (int k = 1; k < image_x; k++)
		{
			for (int j = 0; j < image_x - k; j++)
			{
				if (edge_st[j][i] > edge_st[j + 1][i])
				{
					tmp[i][0] = edge_st[j][i];
					edge_st[j][i] = edge_st[j + 1][i];
					edge_st[j + 1][i] = tmp[i][0];
				}
			}
		}
	//	printf("edge_st[image_x-1][i]=%f\n", edge_st[image_x - 1][i]);
	}

#pragma omp parallel for
	for (int i = 1; i<image_y; i++)
	{
		for (int j = 0; j < image_y - i; j++)
		{
			if (edge_st[image_x-1][j] > edge_st[image_x - 1][j+1])
			{
				tmp_h = edge_st[image_x - 1][j];
				edge_st[image_x - 1][j] = edge_st[image_x - 1][j + 1];
				edge_st[image_x - 1][j + 1] = tmp_h;
			}
		}
	}
	for (int i = 1; i<image_y; i++)
	{
		for (int j = 0; j < image_y - i; j++)
		{
			if (edge_st[0][j] < edge_st[0][j + 1])
			{
				tmp_l = edge_st[0][j];
				edge_st[0][j] = edge_st[0][j + 1];
				edge_st[0][j + 1] = tmp_l;
			}
		}
	}
/*
	
	//データを大きい順に並び替える
//#pragma omp parallel for
	for (int i = 1; i<DATA_NUM; i++)
	{
		for (int j = 0; j < DATA_NUM - i; j++)
		{
			if (data[j][0] > data[j + 1][0])
			{
				tmp[i][0] = data[j][0];
				data[j][0] = data[j + 1][0];
				data[j + 1][0] = tmp[i][0];
			}
		}
	}
*/
	
//	double max_data = data[DATA_NUM - 1][0];
//	double min_data =  data[0][0];
	double max_data = edge_st[image_x - 1][image_y-1];
	double min_data = edge_st[0][image_y - 1];
	printf("max_data=%f,min_data=%f", max_data, min_data);
	k = (max_data - min_data) / 256;
	double k2;
	int j_start=0;


  /* ヒストグラム作成 */
  std::vector<int> hist(256, 0);  // 0-255の256段階のヒストグラム（要素数256、全て0で初期化）
  int kaidan = 0;
  for (int i = 0; i < DATA_NUM; ++i) {
	 // printf("j_start=%d\n", j_start);
	  for (int j = j_start; j < 255; ++j) {
		  
		  if (data[i][0] > k*j && data[i][0] < k*(j + 1)) {
			  hist[j]++;
			 // j_start = j;
			 // break;
		  }
	  }

  }
  
  //ヒストグラム描画
   int hist_max=0;
   for(int i=0;i<256;++i){
	   if(hist[i]>hist_max)hist_max=hist[i];
   }
   printf("hist_max=%d\n",hist_max);

   float histf[256];
   for (int i = 0; i < 256; ++i){
	   histf[i]=hist[i]/(float)hist_max;
	   //printf("histf=%f",histf[i]);
   }

   Mat hist_image= Mat(Size(276,320),CV_8UC3,Scalar(255,255,255));
	rectangle(hist_image,Point(10,20),Point(265,100),Scalar(220,220,220),-1);
	 for(int i=0;i<256;i++){
	 line(hist_image,Point(10+i,100),Point(10+i,100-(float)(histf[i])*80),Scalar(0,0,255),1,8,0);
	}
	//namedWindow("ヒストグラム");
	//imshow("ヒストグラム",hist_image);

	//ヒストグラム画像の保存
	char Output_hist_name[128];
	sprintf(Output_hist_name, "%s\\hist.bmp", date_directory4);	//出力画像.bmp
	FILE *fp_Output_hist;
	fp_Output_hist = fopen(Output_hist_name, "w");
	if (fp_Output_hist == NULL) {
		printf("%sファイルが開けません\n", Output_hist_name);
		//exit(1);
	}
	imwrite(Output_hist_name, hist_image);
	fclose(fp_Output_hist);

   
  /* 判別分析法 */
  int t = 0;  // 閾値
  double t2 = 0;	//実際の閾値
  double max = 0.0;  // w1 * w2 * (m1 - m2)^2 の最大値
  
  for (int i = 0; i < 256; ++i){
    int w1 = 0;  // クラス１の画素数
    int w2 = 0;  // クラス２の画素数
    long sum1 = 0;  // クラス１の平均を出すための合計値
    long sum2 = 0;  // クラス２の平均を出すための合計値
    double m1 = 0.0;  // クラス１の平均
    double m2 = 0.0;  // クラス２の平均
    
    for (int j = 0; j <= i; ++j){
      w1 += hist[j];
      sum1 += j * hist[j];
    }
    
    for (int j = i+1; j < 256; ++j){
      w2 += hist[j];
      sum2 += j * hist[j];
    }
    
    if (w1)
      m1 = (double)sum1 / w1;
    
    if (w2)
      m2 = (double)sum2 / w2;
    
    double tmp = ((double)w1 * w2 * (m1 - m2) * (m1 - m2));
    
    if (tmp > max){
      max = tmp;
      t = i;
    }
  }
  
  
  t2 = (k - 1)*t;

  free_matrix(data, 0, DATA_NUM - 1, 0, 1);
//  free_matrix(tmp, 0, DATA_NUM - 1, 0, 1);
  free_matrix(tmp, 0, image_y - 1, 0, 1);

  return t2;
}







///////////////対象画像での判別分析法//////////////////////////////////////////////////////
int otsu(char date_directory4[],int &image_x, int &image_y ,std::vector<std::vector<double>> &edge_st){
	printf("****************************************\n");
	printf("start： 判別分析法\n");
	printf("****************************************\n");

  double b = discriminantAnalysis(date_directory4,image_x,image_y, edge_st);

  printf("threshold_b=%f", b);
 
  return b;
}