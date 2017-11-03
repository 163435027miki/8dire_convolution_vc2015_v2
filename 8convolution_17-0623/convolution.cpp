
//メモリ確保を行うためのヘッダ
#define ANSI				
#include "nrutil.h"	

#include<stdio.h>
#include<math.h>
#include <omp.h>	//openMP

#include<fstream>
#include<iostream>
#include<string>
#include<sstream> //文字ストリーム

#include<time.h>//時間を用いる

#include <direct.h>//フォルダを作成する

#include <opencv2/opencv.hpp>	//画像読み込み

#include<thread>	//複数スレッド

#include <tuple>
std::tuple<int, int, std::vector<std::vector<double>>> read_txt(const char *filename);

using namespace std;

int conv_eco_mode_flag = 1;

int x, y, fx, fy;
int hfs;
FILE *fp;

//property関係_変数
//int &image_x, &image_y;		//画像サイズ
int Rvector_create=0;		//1なら基準ベクトルを作る
int Rvector_pointX;		//基準ベクトルを作成する位置.0をスタートで考える
int Rvector_pointY;		//特殊な指定をする場合は下へ
int fs;					//フィルタの大きさ(奇数)
int All_property[10];	//propertyから上記の情報を読み込む用

//property関係_ディレクトリ
char propety[255];
char Allpropety[255];
char outputrvector_directory[128];				//Rvectorの場所
char inputimage_directory[128];
char inputfilter_directory[255];
char inputfilter_directory1[255];
char inputfilter_directory2[255];
char inputfilter_directory3[255];
char inputfilter_directory4[255];
char inputfilter_directory5[255];
char inputfilter_directory6[255];
char inputfilter_directory7[255];
char inputfilter_directory8[255];			//繰返し数を増やしたい1/4
int count_property = 0;
int count_Allproperty = 0;

char InputImage[128];						//入力画像

//出力ファイル名・出力先の設定
char Filename[64];
char Filename1[64];
char Filename2[64];
char Filename3[64];
char Filename4[64];
char Filename5[64];
char Filename6[64];
char Filename7[64];
char Filename8[64];

//出力ファイル名・出力先の設定_グラデーション用
char Filename1G[64];
char Filename2G[64];
char Filename3G[64];
char Filename4G[64];
char Filename5G[64];
char Filename6G[64];
char Filename7G[64];
char Filename8G[64];

//Rvector関連
char Rvector_directory[128];
double Rvector[9];
char Rvector_Filename[255];
char *Rvector_Filename_s;

//出力ファイルディレクトリ
//char date_directory[128];
char date_directory2[128];
char outputbmp_directory[128];

void read_property(ifstream &propety_dire,int &image_x,int &image_y,int &fs);
void Rvector_createF();
void set_outputfile(char date[],char date_directory[],int paramerter[],int paramerter_count,int sd);
std::tuple<int,std::vector<std::vector<double>>> read_filter(char inputfilter_directory[]);
int convolution_gaus_sobel(int &image_y,int &image_x,int fs, int hfs,double *output1[],double *spfil1[],double *input_bmp[],double magnification);
void read_filter_gaus(int fs,double *spfil1_g[]);

int convolution(int argc, char** argv,char image_nameP2[],int &image_x,int &image_y,int paramerter[],int paramerter_count,int sd,char date[],char date_directory[]) {

	//個別で画像を指定したいとき
	//std::ifstream propety_dire("..\\property_usa\\simulation17-0613\\property_3k_conv_sd0.txt");
	//基準ベクトルを取得するときはこちら
	//int filter_number = 0;
	//sprintf(Rvector_Filename, "%s\\Rvector%d.csv", Rvector_directory, filter_number);
	//フィルタ演算，基準ベクトル作成するときのみ指定．ここを変更する
	/*	char propety_dire_char[128];
	sprintf(propety_dire_char,"..\\property_usa\\simulation17-0203\\Rvector\\8dire_100k\\property_8dire_100k_I~%d.txt",filter_number);
	std::ifstream propety_dire(propety_dire_char);*/
	
	//閾値の設定．今後property.txtに加えるかも
	int Offset = 128;
	double magnification = 1;
	float Upper_threshold = 35;
	float Under_threshold = -35;
	int use_upperthreshold_flag = 1;
	int use_underthreshold_flag = 1;
	int Save_image_flag[9][9];

/////////////画像作成の有無////////////////////////////////////////////////////////////////

	//画像作成判断の初期化(デフォルトでは作成しない)
	for (int i = 0; i<9; ++i) {
		Save_image_flag[1][i] = 0;
		Save_image_flag[2][i] = 0;
	}

	//ecoモード
	if (conv_eco_mode_flag == 1) {
		for (int j = 0; j < 2; ++j) {
			for (int i = 0; i < 9; ++i) {
				Save_image_flag[j][i] = 0;
			}
		}
	}

	//多値画像を作成する
	Save_image_flag[2][0] = 1;		//多値画像を作成するとき1


	//8方向に画像を作成する
	for (int i = 1; i <= 8; ++i) {

		Save_image_flag[2][i] = 1;	//作成したい方向の画像の選択．iを指定
	}

	


////////////////////////////////////////////入力画像の読み込み////////////////////////////////////////////////////////////////////////////
	std::ifstream propety_dire;
	propety_dire.open(image_nameP2,ios::in);
	
	read_property(propety_dire,image_x,image_y,fs);	//propertyから読み取り
	
	sprintf(InputImage, inputimage_directory);	//propertyから読み取った入力画像情報を代入
	cv::Mat ImputImageM = cv::imread(InputImage);	//入力画像の読み込み．opencv
	image_x=ImputImageM.cols;
	image_y=ImputImageM.rows;
	
//////////////////////////初期設定の確認用//////////////////////////////////////////////////////////////////////////////

	if (Rvector_create == 1)printf("基準ベクトル取得座標：X=%d，Y=%d\n", Rvector_pointX, Rvector_pointY);
	printf("InputImage=%s\n", InputImage);
	printf("画像サイズ：X=%d，Y=%d\n", image_x, image_y);
	//printf("fs=%d\n", fs);
	printf("Offset=%d\n", Offset);
	printf("Upper_threshold : %f, Under_threshold : %f\n", Upper_threshold, Under_threshold);
	printf("use_upperthreshold_flag : %d, use_underthreshold_flag : %d\n", use_upperthreshold_flag, use_underthreshold_flag);

	//Nrutilを用いたメモリの確保
	double **input_bmp = matrix(0, image_x-1, 0, image_y-1);
	double **output1 = matrix(0, image_x-1, 0, image_y-1);
	double **spfil1 = matrix(1, image_x, 1, image_y);
	double **output_bmp_image = matrix(0, image_x-1, 0, image_y-1);	//追加1
	double **output_bmp_flag = matrix(0, image_x-1, 0, image_y-1);	//追加3
	
	//確保したメモリを初期化する
	for (y = 0; y < image_y; y++) {
		for (x = 0; x < image_x; x++) {
			input_bmp[x][y] = 0;
			output1[x][y] = 0;
			output_bmp_image[x][y] = 0;	//追加2
			output_bmp_flag[x][y] = 0;	//追加3
		}
	}
	
	for (fy = 1; fy <= image_y; fy++) {
		for (fx = 1; fx <= image_x; fx++) {
			spfil1[fx][fy] = 0;
		}
	}
	

	//int hfs = (fs + 1) / 2;			//フィルタの大きさの半分

////////////////////////画像の読み取り//////////////////////////////////////////////////////////////////////////////////////////////
	for (y = 0; y < image_y; ++y) {
		for (x = 0; x < image_x; ++x) {
			//opencvでは画素がR,G,Bの順で並んでいる
			int R = 3 * x;
			int G = 3 * x + 1;
			int B = 3 * x + 2;
			//グレースケールでの読み込み
			input_bmp[x][y] = (((ImputImageM.at<unsigned char>(y, R)+ImputImageM.at<unsigned char>(y, G)+ImputImageM.at<unsigned char>(y, B))/3)-Offset);
			//input_bmp[x][y] = ImputImageM.at<unsigned char>(y, R)-Offset;
		}
	}
	//出力ファイルについての設定１
	set_outputfile(date,date_directory,paramerter,paramerter_count,sd);

	if(Rvector_create==1)Rvector_createF();	//基準ベクトルの作成

	printf("****************************************\n");
	printf("start：convolution\n");
	printf("****************************************\n");

	int Rvector_number = 0;				//Rvectorの番号のカウント

	std::vector<std::vector<double>>spfil1_kari;
	spfil1_kari.resize(image_x);
	for (int i = 0; i<image_x; ++i) {
		spfil1_kari[i].resize(image_y);
	}

///////////////////////フィルタの読み込み/////////////////////////////////////////
	for(int kernel_number=0;kernel_number<8;++kernel_number){

		

		//フィルタの選択．書き込みファイルの選択
		if(kernel_number==0){sprintf(inputfilter_directory,inputfilter_directory1);sprintf(Filename,Filename1);}
		if(kernel_number==1){sprintf(inputfilter_directory,inputfilter_directory2);sprintf(Filename,Filename2);}
		if(kernel_number==2){sprintf(inputfilter_directory,inputfilter_directory3);sprintf(Filename,Filename3);}
		if(kernel_number==3){sprintf(inputfilter_directory,inputfilter_directory4);sprintf(Filename,Filename4);}
		if(kernel_number==4){sprintf(inputfilter_directory,inputfilter_directory5);sprintf(Filename,Filename5);}
		if(kernel_number==5){sprintf(inputfilter_directory,inputfilter_directory6);sprintf(Filename,Filename6);}
		if(kernel_number==6){sprintf(inputfilter_directory,inputfilter_directory7);sprintf(Filename,Filename7);}
		if(kernel_number==7){sprintf(inputfilter_directory,inputfilter_directory8);sprintf(Filename,Filename8);}

		//read_filter(inputfilter_directory,fs,spfil1);	//フィルタの読み込み
		std::tie(fs, spfil1_kari) = read_filter(inputfilter_directory);
		
		for (fy = 1; fy <= fs; fy++) {
			for (fx = 1; fx <= fs; fx++) {
				//fscanf(fp, "%lf	", &spfil1[fx][fy]);
				spfil1[fx][fy] = spfil1_kari[fx-1][fy-1];
			}
		}
		
		if (kernel_number == 0) {
			printf("fs=%d\n", fs);
			hfs = (fs + 1) / 2;			//フィルタの大きさの半分
		}
		

		//畳み込み
		if(paramerter[0]==2){
		convolution_gaus_sobel(image_y,image_x,fs,hfs,output1,spfil1,input_bmp,magnification);
		}else{
		convolution(image_y,image_x,fs,hfs,output1,spfil1,input_bmp,magnification);
		}
		

		//ファイルへの書き込み
		write_file(Filename,image_x,image_y,output1,Rvector_create,Rvector_pointX,Rvector_pointY,Rvector,Rvector_number);	

		printf("convolution：filter %s\n",inputfilter_directory);	//用いたフィルタの表示

		Rvector_number++;					//Rvectorの番号のカウント

	}
	Rvector_number=0;					//Rvectorの番号のカウント

///////////////////////////////////基準ベクトルの作成結果ファイルの作成/////////////////////////////////////////////////////

	if (Rvector_create == 1) {
		if ((fp = fopen(Rvector_Filename, "w")) == NULL) { exit(1); }
		Rvector_number = 0;
		for (Rvector_number = 0 ; Rvector_number < 8 ; ++Rvector_number) {
			fprintf(fp, "%lf,", Rvector[Rvector_number]);
		}
		fclose(fp);
	}

//////////////////////////////opencvを用いた畳み込み画像の作成///////////////////////////////////////////////////////////////////////////////////

	if (Save_image_flag[2][0] == 1) {
		for (int make_image_repeat = 1; make_image_repeat <= 8; ++make_image_repeat) {
			//画像の作成
			make_bmp(date_directory2, Filename1, image_x, image_y, make_image_repeat, Save_image_flag);
		}
	}

//////////////////////////////logの作成///////////////////////////////////////////////////////////////////////////////////
	FILE *fp_date;
	char filename_log[128];
	//sprintf(filename_log, "..\\log\\log-%2d-%02d%02d-%02d%02d%02d.txt",pnow->tm_year+1900,pnow->tm_mon + 1,pnow->tm_mday,pnow->tm_hour,pnow->tm_min,pnow->tm_sec);	//logファイル作成のディレクトリ指定
	sprintf(filename_log, "%s\\log.txt",date_directory2);	//logファイル作成のディレクトリ指定
	if ((fp_date = fopen(filename_log, "w")) == NULL) { printf("logファイルが開けません"); exit(1); }
	fprintf(fp_date, "Time : %s\n", date);								//時間
	fprintf(fp_date, "Rvector_create=%d\n", Rvector_create);			//Rvectorの作成したかどうか
	if (Rvector_create == 1)fprintf(fp_date, "Rvector_Coordinate：X=%d，Y=%d\n", Rvector_pointX, Rvector_pointY);
	fprintf(fp_date, "ImputImage=%s\n", InputImage);					//入力画像
	fprintf(fp_date, "ImputImage_size：X=%d，Y=%d\n", image_x, image_y);
	fprintf(fp_date, "Upper_threshold : %f, Under_threshold : %f\n", Upper_threshold, Under_threshold);
	fprintf(fp_date, "use_upperthreshold_flag : %d, use_underthreshold_flag : %d\n", use_upperthreshold_flag, use_underthreshold_flag);
	fprintf(fp_date, "Offset : %d\n", Offset);						//畳み込みの際のoffset
	fprintf(fp_date, "Kernel\n");									//使用したカーネルの記録
	fprintf(fp_date, "Kernel_size=%d\n", fs);
	fprintf(fp_date, "V0   = %s\n", inputfilter_directory1);
	fprintf(fp_date, "V45  = %s\n", inputfilter_directory2);
	fprintf(fp_date, "V90  = %s\n", inputfilter_directory3);
	fprintf(fp_date, "V135 = %s\n", inputfilter_directory4);
	fprintf(fp_date, "V180 = %s\n", inputfilter_directory5);
	fprintf(fp_date, "V225 = %s\n", inputfilter_directory6);
	fprintf(fp_date, "V270 = %s\n", inputfilter_directory7);
	fprintf(fp_date, "V315 = %s\n", inputfilter_directory8);
	fclose(fp_date);
	printf("logファイル %s を保存しました\n", filename_log);
	

	//メモリの開放
	free_matrix(input_bmp, 0, image_x-1, 0, image_y-1);
	free_matrix(output1,0, image_x-1, 0, image_y-1);
	free_matrix(spfil1,  1, image_x,  1, image_y);
	free_matrix(output_bmp_image, 0, image_x-1, 0, image_y-1);
	free_matrix(output_bmp_flag, 0, image_x-1, 0, image_y-1);

	printf("finish：convolution\n");

	//return ;
	
}

void set_outputfile(char date[],char date_directory[],int paramerter[],int paramerter_count,int sd){
	
//////////////////////////////Outputディレクトリの設定//////////////////////////////////////////////////////////////////////////////////
	//出力結果のファイル名の指定
	char *Filename1_s = "V(0).csv";
	char *Filename2_s = "V(45).csv";
	char *Filename3_s = "V(90).csv";
	char *Filename4_s = "V(135).csv";
	char *Filename5_s = "V(180).csv";
	char *Filename6_s = "V(225).csv";
	char *Filename7_s = "V(270).csv";
	char *Filename8_s = "V(315).csv";

	// 出力結果のファイル名の指定_量子化用のデータ
	char *Filename1G_s = "V(0)G.csv";
	char *Filename2G_s = "V(45)G.csv";
	char *Filename3G_s = "V(90)G.csv";
	char *Filename4G_s = "V(135)G.csv";
	char *Filename5G_s = "V(180)G.csv";
	char *Filename6G_s = "V(225)G.csv";
	char *Filename7G_s = "V(270)G.csv";
	char *Filename8G_s = "V(315)G.csv";


	//結果を保存するフォルダの作成
	//フォルダ名は実行日時になる
	sprintf(date_directory, "..\\result_usa\\%s\\", date);
	if (_mkdir(date_directory) == 0) {
		printf("フォルダ %s を作成しました\n", date_directory);
	}
	else {
		printf("フォルダ作成に失敗しました。もしくは作成済みです\n");
	}

	if(paramerter[0]==1 || paramerter[0]==2){
		sprintf(date_directory2, "%s%d×%dsobel_conv_sd%d\\", date_directory,paramerter[paramerter_count],paramerter[paramerter_count],sd);		
	}else{
		sprintf(date_directory2, "%s%dk_conv_sd%d\\", date_directory,paramerter[paramerter_count],sd);
	}
	//Outputディレクトリの作成
	if (_mkdir(date_directory2) == 0) {
		printf("フォルダ %s を作成しました\n", date_directory2);
	}
	else {
		printf("フォルダ作成に失敗しました。\n");
	}
	

	//Outputファイルのディレクトリ設定
	sprintf(Filename1,"%s%s", date_directory2, Filename1_s);
	sprintf(Filename2, "%s%s", date_directory2, Filename2_s);
	sprintf(Filename3, "%s%s", date_directory2, Filename3_s);
	sprintf(Filename4, "%s%s", date_directory2, Filename4_s);
	sprintf(Filename5, "%s%s", date_directory2, Filename5_s);
	sprintf(Filename6, "%s%s", date_directory2, Filename6_s);
	sprintf(Filename7, "%s%s", date_directory2, Filename7_s);
	sprintf(Filename8, "%s%s", date_directory2, Filename8_s);

	//Outputファイルのディレクトリ設定_グラデーション用
	sprintf(Filename1G, "%s%s", date_directory2, Filename1G_s);
	sprintf(Filename2G, "%s%s", date_directory2, Filename2G_s);
	sprintf(Filename3G, "%s%s", date_directory2, Filename3G_s);
	sprintf(Filename4G, "%s%s", date_directory2, Filename4G_s);
	sprintf(Filename5G, "%s%s", date_directory2, Filename5G_s);
	sprintf(Filename6G, "%s%s", date_directory2, Filename6G_s);
	sprintf(Filename7G, "%s%s", date_directory2, Filename7G_s);
	sprintf(Filename8G, "%s%s", date_directory2, Filename8G_s);

	printf("使用したカーネルは\n");
	printf("V0   = %s\n", inputfilter_directory1);
	printf("V45  = %s\n", inputfilter_directory2);
	printf("V90  = %s\n", inputfilter_directory3);
	printf("V135 = %s\n", inputfilter_directory4);
	printf("V180 = %s\n", inputfilter_directory5);
	printf("V225 = %s\n", inputfilter_directory6);
	printf("V270 = %s\n", inputfilter_directory7);
	printf("V315 = %s\n", inputfilter_directory8);
}

	///////////////////////////////初期設定2 : 基準ベクトル作成か否か(修正をしなければならない）/////////////////////////////////////////////////////////////
void Rvector_createF(){

//if(Rvector_create==1)sprintf(InputImage, "..\\bmp\\255-0\\%d.bmp", filter_number);	//Rvectorを作成する際の入力画像指定.
											
	sprintf(Rvector_directory, outputrvector_directory, propety);		//Rvectorを保存するフォルダ名を指定.property.txtの2行目																//Rvectorを保存するフォルダの作成
	printf("Rvector_create=%d\n", Rvector_create);
	if (Rvector_create == 1) {

		if (_mkdir(Rvector_directory) == 0) {
			printf("Rvecor保存フォルダ %s を作成しました\n", Rvector_directory);
		}
		else {
			printf("Rvecor保存フォルダ %s は作成済みです\n", Rvector_directory);
		}
	}
}


void read_property(ifstream &propety_dire,int &image_x,int &image_y,int &fs){

	count_property=0;

	//プロパティtxtファイルの読み込み
	if (propety_dire.fail())
	{
		printf("propertyテキストを読み取ることができません\n");
		printf("propertyテキスト : %s\n",propety_dire);
	}
	while (propety_dire.getline(propety, 256 - 1)){

		if (count_property == 0)++count_property;

		if (count_property == 0)sprintf(Allpropety, propety);				//一行目の画像サイズなど
		if (count_property == 1)sprintf(outputrvector_directory, propety);	//使用する基準ベクトル
		if (count_property == 2)sprintf(inputimage_directory, propety);		//入力画像
		if (count_property == 3)sprintf(inputfilter_directory1, propety);	//使用するカーネル1
		if (count_property == 4)sprintf(inputfilter_directory2, propety);	//使用するカーネル2
		if (count_property == 5)sprintf(inputfilter_directory3, propety);	//使用するカーネル3
		if (count_property == 6)sprintf(inputfilter_directory4, propety);	//使用するカーネル4
		if (count_property == 7)sprintf(inputfilter_directory5, propety);	//使用するカーネル5
		if (count_property == 8)sprintf(inputfilter_directory6, propety);	//使用するカーネル6
		if (count_property == 9)sprintf(inputfilter_directory7, propety);	//使用するカーネル7
		if (count_property == 10)sprintf(inputfilter_directory8, propety);	//使用するカーネル8

		++count_property;
	}

	//プロパティtxtファイルの一行目を文字列から数値に変換して関数に代入
/*	std::string tmp_Allpropety;
	std::istringstream stream_Allpropety(Allpropety);
	count_Allproperty=0;
	while (getline(stream_Allpropety, tmp_Allpropety, ',')) {

		int All_tmp_property = stof(tmp_Allpropety); //stof(string str) : stringをintに変換
		All_property[count_Allproperty] = All_tmp_property;

////////////////property.txtの一行目///////////////////////////////////////////////////////////
		if (count_Allproperty == 0)image_x = All_property[count_Allproperty];				//&image_x:画像の横サイズ(256pixelの画像の場合は256と指定)
		if (count_Allproperty == 1)image_y = All_property[count_Allproperty];				//&image_y:画像の縦サイズ(256pixelの画像の場合は256と指定)
		if (count_Allproperty == 2)Rvector_create = All_property[count_Allproperty];		//Rvector_create:1なら基準ベクトルを作成．
		if (count_Allproperty == 3)Rvector_pointX = All_property[count_Allproperty];		//Rvector_pointX:基準ベクトル取得X座標．0からスタート
		if (count_Allproperty == 4)Rvector_pointY = All_property[count_Allproperty];		//Rvector_pointY:基準ベクトル取得Y座標．0からスタート
		if (count_Allproperty == 5)fs = All_property[count_Allproperty];					//fs:フィルタサイズ（奇数）

		++count_Allproperty;
	}
	*/
	propety_dire.close();
}


//////////////////////////フィルタ読み込み////////////////////////////
std::tuple<int,std::vector<std::vector<double>>> read_filter(char inputfilter_directory[]){

	std::vector<std::vector<double>>filter_txt;
	filter_txt.resize(100);
	for (int i = 0; i<100; ++i) {
		filter_txt[i].resize(100);
	}

	int filter_x, filter_y;

	//サイズ不定のtxtファイルの読み取り
	std::tie(filter_x, filter_y, filter_txt) = read_txt(inputfilter_directory);
	
	if (filter_x != filter_y) {
		printf("フィルターサイズがおかしい");
		exit(1);
	}
	fs = filter_x;

	std::vector<std::vector<double>>spfil1_kari;
	spfil1_kari.resize(fs);
	for (int i = 0; i<fs; ++i) {
		spfil1_kari[i].resize(fs);
	}
	
	for (fy = 1; fy <= fs; fy++) {
		for (fx = 1; fx <= fs; fx++) {
			spfil1_kari[fx-1][fy-1] = filter_txt[fx-1][fy-1];
		}
	}
	return std::forward_as_tuple(fs, spfil1_kari);

}

///////////////////////畳み込み結果の書き込み////////////////////////////////////////
void write_file(char *Filename,int &image_x,int &image_y,double *output1[],int Rvector_create,int Rvector_pointX,int Rvector_pointY,double Rvector[],int Rvector_number){

	//------------------------ファイルへの書き込み--------------------------//
	if ((fp = fopen(Filename, "w")) == NULL){
		printf("ファイル：%sが開けません\n",Filename);
		exit(1);
	}
	for (y = 0; y < image_y; y = y + 1) {
		for (x = 0; x < image_x; x = x + 1) {
			fprintf(fp, "%lf,", output1[x][y]);


			if (Rvector_create == 1) {
				if (y == Rvector_pointY) {
					if (x == Rvector_pointX) {
						Rvector[Rvector_number] = output1[x][y];
					}
				}
			}

		}
		fprintf(fp, "\n");
	}
	fclose(fp);

	//確保したメモリを初期化する
	for (y = 0; y < image_y; y++) {
		for (x = 0; x < image_x; x++) {
			output1[x][y] = 0;
		}
	}

}

///////////////////////畳み込み演算//////////////////////////////////////////////////
void convolution(int &image_y,int &image_x,int fs, int hfs,double *output1[],double *spfil1[],double *input_bmp[],double magnification){

//#pragma omp parallel num_threads(4)
	for (y = 0; y < image_y; y++) {

//#pragma omp parallel num_threads(2)
#pragma omp parallel for
		for (x = 0; x < image_x; x++) {
//#pragma omp parallel for
			for (fy = 1; fy <= fs; fy++) {
//#pragma omp parallel for
//#pragma omp parallel num_threads(2)
				for (fx = 1; fx <= fs; fx++) {
					
					//フィルタの真ん中が描画点になるようにhsfを用いて調整する
					if(		0<=x+(fx-hfs) && x+(fx-hfs)<image_x
						&&	0<=y+(fy-hfs) && y+(fy-hfs)<image_y )
					{
						output1[x][y] += spfil1[fx][fy] * input_bmp[x + (fx - hfs)][y + (fy - hfs)];
					}
				}
			}
			output1[x][y] = output1[x][y] * magnification;		//倍率に応じてゲインを変化
		}
	
	}

	//フィルタの初期化
//#pragma omp parallel for
	for (fy = 1; fy <= fs; fy++) {
		for (fx = 1; fx <= fs; fx++) {
			spfil1[fx][fy] = 0;
		}
	}

}

///////////////////sobelフィルタに対応したガウスフィルタを用いた畳み込み演算//////////////////////////////////////////////////////////
int convolution_gaus_sobel(int &image_y,int &image_x,int fs, int hfs,double *output1[],double *spfil1[],double *input_bmp[],double magnification){

	//Nrutilを用いたメモリの確保
	double **spfil1_g = matrix(1, fs, 1, fs);
	double **output1_g = matrix(0, image_x-1, 0, image_y-1);

	read_filter_gaus(fs,spfil1_g);

	//初期化
	for (y = 0; y < image_y; y++) {
		for (x = 0; x < image_x; x++) {
			output1_g[x][y] = 0;
		}
	}

	//gausフィルタによるなめし
	for (y = 0; y < image_y; y++) {
#pragma omp parallel for
		for (x = 0; x < image_x; x++) {
			for (fy = 1; fy <= fs; fy++) {
				for (fx = 1; fx <= fs; fx++) {
					
					//フィルタの真ん中が描画点になるようにhsfを用いて調整する
					if(		0<=x+(fx-hfs) && x+(fx-hfs)<image_x
						&&	0<=y+(fy-hfs) && y+(fy-hfs)<image_y )
					{
						output1_g[x][y] += spfil1_g[fx][fy] * input_bmp[x + (fx - hfs)][y + (fy - hfs)];
					}
				}
			}
			//output1_g[x][y] = output1_g[x][y] * magnification;		//倍率に応じてゲインを変化
		}
	
	}
	
	
	//なめした画像に対してsobelフィルタを掛ける
	for (y = 0; y < image_y; y++) {
#pragma omp parallel for
		for (x = 0; x < image_x; x++) {
			
			for (fy = 1; fy <= fs; fy++) {
				for (fx = 1; fx <= fs; fx++) {
					
					//フィルタの真ん中が描画点になるようにhsfを用いて調整する
					if(		0<=x+(fx-hfs) && x+(fx-hfs)<image_x
						&&	0<=y+(fy-hfs) && y+(fy-hfs)<image_y )
					{
						output1[x][y] += spfil1[fx][fy] * output1_g[x + (fx - hfs)][y + (fy - hfs)];
					}
				}
			}
			output1[x][y] = output1[x][y] * magnification;		//倍率に応じてゲインを変化
		}
	
	}
	
	//フィルタの初期化
	for (fy = 1; fy <= fs; fy++) {
		for (fx = 1; fx <= fs; fx++) {
			spfil1[fx][fy] = 0;
		}
	}

	free_matrix(output1_g,0, image_x-1, 0, image_y-1);
	free_matrix(spfil1_g,  1,  fs,  1,  fs);

	//return 0;
	return **input_bmp;

}

void read_filter_gaus(int fs,double *spfil1_g[]){

	//初期化
	for (y = 1; y <= fs; y++) {
		for (x = 1; x <= fs; x++) {
			spfil1_g[x][y] = 0;
		}
	}

	char inputfilter_gaus[128];
	sprintf(inputfilter_gaus,"..\\filter\\gaus\\%d×%dgaus.txt",fs,fs);

	if ((fp = fopen(inputfilter_gaus, "r")) == NULL){
		printf("ガウスフィルタ：%sが読み込めません\n",inputfilter_gaus);
		exit(1);
	}
	
	for (fy = 1; fy <= fs; fy++) {
		for (fx = 1; fx <= fs; fx++) {
			fscanf(fp, "%lf	", &spfil1_g[fx][fy]);
		}
	}
	fclose(fp); 
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////量子化_Vの諧調を落とす///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

int V315_divide_gradation(int &image_x, int &image_y, double low_gradation, double max_gradation, int gradation_number, double gradient) {

	//Nrutilを用いたメモリの確保
	double **V315 = matrix(0, image_x - 1, 0, image_y - 1);
	//確保したメモリを初期化する
	for (int i8 = 0; i8 < image_y; i8++) {
		for (int j8 = 0; j8 < image_x; j8++) {
			V315[j8][i8] = 0;
		}
	}

	//Inputファイルを開く
	ifstream V_315(Filename8);
	//Outputファイルを開く
	FILE *fp_V315G;
	if ((fp_V315G = fopen(Filename8G, "w")) == NULL) { printf("入力エラー V0G.csvが開けません\nFile_name :%s ", Filename8G); exit(1); }

	////////////////////////エラー出力/////////////////////////////////////////////////////////////////////////////////////////////
	if (!V_315) { printf("入力エラー V(0).csvがありません_cos-sim\nInput_Filename=%s", Filename8G); return 1; }

	///////////////////////応答電圧のcsvの読み込み/////////////////////////////////////////////////////////////////////////////////
	int i8 = 1;
	string str_315;
	int count_large8 = 0;
	while (getline(V_315, str_315)) {					//このループ内ですべてを行う
		int	count_small8 = 0;			//初期化

										///////////////いろいろ定義．ここでやらないといけない///////////////////////////////////////////////////////////////////////////
		string token_V_0;
		istringstream stream_V_0(str_315);


		//////////////////////////配列に代入//////////////////////////////////////////////////////////////////////////////////////////////

		while (getline(stream_V_0, token_V_0, ',')) {	//一行読み取る．V315のみ，繰り返しの範囲指定に用いる
			double tmp_V_0 = stof(token_V_0);			//文字を数字に変換
			V315[count_small8][count_large8] = tmp_V_0;				//配列に代入
																	//V315[count_small][count_large] = Rvectormagni[1] * V315[count_small][count_large]

			++count_small8;
		}
		++count_large8;
	}

	//量子化
	for (int i8 = 0; i8 < image_y; ++i8) {
		for (int j8 = 0; j8 < image_x; ++j8) {
			for (int k8 = 0; k8 < gradation_number; ++k8) {
				if (V315[j8][i8] > low_gradation + gradient*k8 && V315[j8][i8]<low_gradation + gradient*(k8 + 1)) {

					fprintf(fp_V315G, "%d,", k8);
					if (j8 == image_x - 1)fprintf(fp_V315G, "\n");

				}
			}
		}
	}

	fclose(fp_V315G);
	//領域の解放
	free_matrix(V315, 0, image_x - 1, 0, image_y - 1);
}

int V270_divide_gradation(int &image_x, int &image_y, double low_gradation, double max_gradation, int gradation_number, double gradient) {

	//Nrutilを用いたメモリの確保
	double **V270 = matrix(0, image_x - 1, 0, image_y - 1);
	//確保したメモリを初期化する
	for (int i7 = 0; i7 < image_y; i7++) {
		for (int j7 = 0; j7 < image_x; j7++) {
			V270[j7][i7] = 0;
		}
	}

	//Inputファイルを開く
	ifstream V_270(Filename7);
	//Outputファイルを開く
	FILE *fp_V270G;
	if ((fp_V270G = fopen(Filename7G, "w")) == NULL) { printf("入力エラー V0G.csvが開けません\nFile_name :%s ", Filename7G); exit(1); }

	////////////////////////エラー出力/////////////////////////////////////////////////////////////////////////////////////////////
	if (!V_270) { printf("入力エラー V(0).csvがありません_cos-sim\nInput_Filename=%s", Filename7G); return 1; }

	///////////////////////応答電圧のcsvの読み込み/////////////////////////////////////////////////////////////////////////////////
	int i7 = 1;
	string str_270;
	int count_large7 = 0;
	while (getline(V_270, str_270)) {					//このループ内ですべてを行う
		int	count_small7 = 0;			//初期化

										///////////////いろいろ定義．ここでやらないといけない///////////////////////////////////////////////////////////////////////////
		string token_V_0;
		istringstream stream_V_0(str_270);


		//////////////////////////配列に代入//////////////////////////////////////////////////////////////////////////////////////////////

		while (getline(stream_V_0, token_V_0, ',')) {	//一行読み取る．V270のみ，繰り返しの範囲指定に用いる
			double tmp_V_0 = stof(token_V_0);			//文字を数字に変換
			V270[count_small7][count_large7] = tmp_V_0;				//配列に代入
																	//V270[count_small][count_large] = Rvectormagni[1] * V270[count_small][count_large]

			++count_small7;
		}
		++count_large7;
	}

	//量子化
	for (int i7 = 0; i7 < image_y; ++i7) {
		for (int j7 = 0; j7 < image_x; ++j7) {
			for (int k7 = 0; k7 < gradation_number; ++k7) {
				if (V270[j7][i7] > low_gradation + gradient*k7 && V270[j7][i7]<low_gradation + gradient*(k7 + 1)) {

					fprintf(fp_V270G, "%d,", k7);
					if (j7 == image_x - 1)fprintf(fp_V270G, "\n");

				}
			}
		}
	}

	fclose(fp_V270G);
	//領域の解放
	free_matrix(V270, 0, image_x - 1, 0, image_y - 1);
}

int V225_divide_gradation(int &image_x, int &image_y, double low_gradation, double max_gradation, int gradation_number, double gradient) {

	//Nrutilを用いたメモリの確保
	double **V225 = matrix(0, image_x - 1, 0, image_y - 1);
	//確保したメモリを初期化する
	for (int i6 = 0; i6 < image_y; i6++) {
		for (int j6 = 0; j6 < image_x; j6++) {
			V225[j6][i6] = 0;
		}
	}

	//Inputファイルを開く
	ifstream V_225(Filename6);
	//Outputファイルを開く
	FILE *fp_V225G;
	if ((fp_V225G = fopen(Filename6G, "w")) == NULL) { printf("入力エラー V0G.csvが開けません\nFile_name :%s ", Filename6G); exit(1); }

	////////////////////////エラー出力/////////////////////////////////////////////////////////////////////////////////////////////
	if (!V_225) { printf("入力エラー V(0).csvがありません_cos-sim\nInput_Filename=%s", Filename6G); return 1; }

	///////////////////////応答電圧のcsvの読み込み/////////////////////////////////////////////////////////////////////////////////
	int i6 = 1;
	string str_225;
	int count_large6 = 0;
	while (getline(V_225, str_225)) {					//このループ内ですべてを行う
		int	count_small6 = 0;			//初期化

										///////////////いろいろ定義．ここでやらないといけない///////////////////////////////////////////////////////////////////////////
		string token_V_0;
		istringstream stream_V_0(str_225);


		//////////////////////////配列に代入//////////////////////////////////////////////////////////////////////////////////////////////

		while (getline(stream_V_0, token_V_0, ',')) {	//一行読み取る．V225のみ，繰り返しの範囲指定に用いる
			double tmp_V_0 = stof(token_V_0);			//文字を数字に変換
			V225[count_small6][count_large6] = tmp_V_0;				//配列に代入
																	//V225[count_small][count_large] = Rvectormagni[1] * V225[count_small][count_large]

			++count_small6;
		}
		++count_large6;
	}

	//量子化
	for (int i6 = 0; i6 < image_y; ++i6) {
		for (int j6 = 0; j6 < image_x; ++j6) {
			for (int k6 = 0; k6 < gradation_number; ++k6) {
				if (V225[j6][i6] > low_gradation + gradient*k6 && V225[j6][i6]<low_gradation + gradient*(k6 + 1)) {

					fprintf(fp_V225G, "%d,", k6);
					if (j6 == image_x - 1)fprintf(fp_V225G, "\n");

				}
			}
		}
	}

	fclose(fp_V225G);
	//領域の解放
	free_matrix(V225, 0, image_x - 1, 0, image_y - 1);
}

int V180_divide_gradation(int &image_x, int &image_y, double low_gradation, double max_gradation, int gradation_number, double gradient) {

	//Nrutilを用いたメモリの確保
	double **V180 = matrix(0, image_x - 1, 0, image_y - 1);
	//確保したメモリを初期化する
	for (int i5 = 0; i5 < image_y; i5++) {
		for (int j5 = 0; j5 < image_x; j5++) {
			V180[j5][i5] = 0;
		}
	}

	//Inputファイルを開く
	ifstream V_180(Filename5);
	//Outputファイルを開く
	FILE *fp_V180G;
	if ((fp_V180G = fopen(Filename5G, "w")) == NULL) { printf("入力エラー V0G.csvが開けません\nFile_name :%s ", Filename5G); exit(1); }

	////////////////////////エラー出力/////////////////////////////////////////////////////////////////////////////////////////////
	if (!V_180) { printf("入力エラー V(0).csvがありません_cos-sim\nInput_Filename=%s", Filename5G); return 1; }

	///////////////////////応答電圧のcsvの読み込み/////////////////////////////////////////////////////////////////////////////////
	int i5 = 1;
	string str_180;
	int count_large5 = 0;
	while (getline(V_180, str_180)) {					//このループ内ですべてを行う
		int	count_small5 = 0;			//初期化

										///////////////いろいろ定義．ここでやらないといけない///////////////////////////////////////////////////////////////////////////
		string token_V_0;
		istringstream stream_V_0(str_180);


		//////////////////////////配列に代入//////////////////////////////////////////////////////////////////////////////////////////////

		while (getline(stream_V_0, token_V_0, ',')) {	//一行読み取る．V180のみ，繰り返しの範囲指定に用いる
			double tmp_V_0 = stof(token_V_0);			//文字を数字に変換
			V180[count_small5][count_large5] = tmp_V_0;				//配列に代入
																	//V180[count_small][count_large] = Rvectormagni[1] * V180[count_small][count_large]

			++count_small5;
		}
		++count_large5;
	}

	//量子化
	for (int i5 = 0; i5 < image_y; ++i5) {
		for (int j5 = 0; j5 < image_x; ++j5) {
			for (int k5 = 0; k5 < gradation_number; ++k5) {
				if (V180[j5][i5] > low_gradation + gradient*k5 && V180[j5][i5]<low_gradation + gradient*(k5 + 1)) {

					fprintf(fp_V180G, "%d,", k5);
					if (j5 == image_x - 1)fprintf(fp_V180G, "\n");

				}
			}
		}
	}

	fclose(fp_V180G);
	//領域の解放
	free_matrix(V180, 0, image_x - 1, 0, image_y - 1);
}

int V135_divide_gradation(int &image_x, int &image_y, double low_gradation, double max_gradation, int gradation_number, double gradient) {

	//Nrutilを用いたメモリの確保
	double **V135 = matrix(0, image_x - 1, 0, image_y - 1);
	//確保したメモリを初期化する
	for (int i4 = 0; i4 < image_y; i4++) {
		for (int j4 = 0; j4 < image_x; j4++) {
			V135[j4][i4] = 0;
		}
	}

	//Inputファイルを開く
	ifstream V_135(Filename4);
	//Outputファイルを開く
	FILE *fp_V135G;
	if ((fp_V135G = fopen(Filename4G, "w")) == NULL) { printf("入力エラー V0G.csvが開けません\nFile_name :%s ", Filename4G); exit(1); }

	////////////////////////エラー出力/////////////////////////////////////////////////////////////////////////////////////////////
	if (!V_135) { printf("入力エラー V(0).csvがありません_cos-sim\nInput_Filename=%s", Filename4G); return 1; }

	///////////////////////応答電圧のcsvの読み込み/////////////////////////////////////////////////////////////////////////////////
	int i4 = 1;
	string str_135;
	int count_large4 = 0;
	while (getline(V_135, str_135)) {					//このループ内ですべてを行う
		int	count_small4 = 0;			//初期化

										///////////////いろいろ定義．ここでやらないといけない///////////////////////////////////////////////////////////////////////////
		string token_V_0;
		istringstream stream_V_0(str_135);


		//////////////////////////配列に代入//////////////////////////////////////////////////////////////////////////////////////////////

		while (getline(stream_V_0, token_V_0, ',')) {	//一行読み取る．V135のみ，繰り返しの範囲指定に用いる
			double tmp_V_0 = stof(token_V_0);			//文字を数字に変換
			V135[count_small4][count_large4] = tmp_V_0;				//配列に代入
																	//V135[count_small][count_large] = Rvectormagni[1] * V135[count_small][count_large]

			++count_small4;
		}
		++count_large4;
	}

	//量子化
	for (int i4 = 0; i4 < image_y; ++i4) {
		for (int j4 = 0; j4 < image_x; ++j4) {
			for (int k4 = 0; k4 < gradation_number; ++k4) {
				if (V135[j4][i4] > low_gradation + gradient*k4 && V135[j4][i4]<low_gradation + gradient*(k4 + 1)) {

					fprintf(fp_V135G, "%d,", k4);
					if (j4 == image_x - 1)fprintf(fp_V135G, "\n");

				}
			}
		}
	}

	fclose(fp_V135G);
	//領域の解放
	free_matrix(V135, 0, image_x - 1, 0, image_y - 1);
}

int V90_divide_gradation(int &image_x, int &image_y, double low_gradation, double max_gradation, int gradation_number, double gradient) {

	//Nrutilを用いたメモリの確保
	double **V90 = matrix(0, image_x - 1, 0, image_y - 1);
	//確保したメモリを初期化する
	for (int i3 = 0; i3 < image_y; i3++) {
		for (int j3 = 0; j3 < image_x; j3++) {
			V90[j3][i3] = 0;
		}
	}

	//Inputファイルを開く
	ifstream V_90(Filename3);
	//Outputファイルを開く
	FILE *fp_V90G;
	if ((fp_V90G = fopen(Filename3G, "w")) == NULL) { printf("入力エラー V0G.csvが開けません\nFile_name :%s ", Filename3G); exit(1); }

	////////////////////////エラー出力/////////////////////////////////////////////////////////////////////////////////////////////
	if (!V_90) { printf("入力エラー V(0).csvがありません_cos-sim\nInput_Filename=%s", Filename3G); return 1; }

	///////////////////////応答電圧のcsvの読み込み/////////////////////////////////////////////////////////////////////////////////
	int i3 = 1;
	string str_90;
	int count_large3 = 0;
	while (getline(V_90, str_90)) {					//このループ内ですべてを行う
		int	count_small3 = 0;			//初期化

										///////////////いろいろ定義．ここでやらないといけない///////////////////////////////////////////////////////////////////////////
		string token_V_0;
		istringstream stream_V_0(str_90);


		//////////////////////////配列に代入//////////////////////////////////////////////////////////////////////////////////////////////

		while (getline(stream_V_0, token_V_0, ',')) {	//一行読み取る．V90のみ，繰り返しの範囲指定に用いる
			double tmp_V_0 = stof(token_V_0);			//文字を数字に変換
			V90[count_small3][count_large3] = tmp_V_0;				//配列に代入
																	//V90[count_small][count_large] = Rvectormagni[1] * V90[count_small][count_large]

			++count_small3;
		}
		++count_large3;
	}

	//量子化
	for (int i3 = 0; i3 < image_y; ++i3) {
		for (int j3 = 0; j3 < image_x; ++j3) {
			for (int k3 = 0; k3 < gradation_number; ++k3) {
				if (V90[j3][i3] > low_gradation + gradient*k3 && V90[j3][i3]<low_gradation + gradient*(k3 + 1)) {

					fprintf(fp_V90G, "%d,", k3);
					if (j3 == image_x - 1)fprintf(fp_V90G, "\n");

				}
			}
		}
	}

	fclose(fp_V90G);
	//領域の解放
	free_matrix(V90, 0, image_x - 1, 0, image_y - 1);
}

int V45_divide_gradation(int &image_x, int &image_y, double low_gradation, double max_gradation, int gradation_number, double gradient) {

	//Nrutilを用いたメモリの確保
	double **V45 = matrix(0, image_x - 1, 0, image_y - 1);
	//確保したメモリを初期化する
	for (int i2 = 0; i2 < image_y; i2++) {
		for (int j2 = 0; j2 < image_x; j2++) {
			V45[j2][i2] = 0;
		}
	}

	//Inputファイルを開く
	ifstream V_45(Filename2);
	//Outputファイルを開く
	FILE *fp_V45G;
	if ((fp_V45G = fopen(Filename2G, "w")) == NULL) { printf("入力エラー V0G.csvが開けません\nFile_name :%s ", Filename2G); exit(1); }

	////////////////////////エラー出力/////////////////////////////////////////////////////////////////////////////////////////////
	if (!V_45) { printf("入力エラー V(0).csvがありません_cos-sim\nInput_Filename=%s", Filename2G); return 1; }

	///////////////////////応答電圧のcsvの読み込み/////////////////////////////////////////////////////////////////////////////////
	int i2 = 1;
	string str_45;
	int count_large2 = 0;
	while (getline(V_45, str_45)) {					//このループ内ですべてを行う
		int	count_small2 = 0;			//初期化

										///////////////いろいろ定義．ここでやらないといけない///////////////////////////////////////////////////////////////////////////
		string token_V_0;
		istringstream stream_V_0(str_45);


		//////////////////////////配列に代入//////////////////////////////////////////////////////////////////////////////////////////////

		while (getline(stream_V_0, token_V_0, ',')) {	//一行読み取る．V45のみ，繰り返しの範囲指定に用いる
			double tmp_V_0 = stof(token_V_0);			//文字を数字に変換
			V45[count_small2][count_large2] = tmp_V_0;				//配列に代入
																	//V45[count_small][count_large] = Rvectormagni[1] * V45[count_small][count_large]

			++count_small2;
		}
		++count_large2;
	}

	//量子化
	for (int i2 = 0; i2 < image_y; ++i2) {
		for (int j2 = 0; j2 < image_x; ++j2) {
			for (int k2 = 0; k2 < gradation_number; ++k2) {
				if (V45[j2][i2] > low_gradation + gradient*k2 && V45[j2][i2]<low_gradation + gradient*(k2 + 1)) {

					fprintf(fp_V45G, "%d,", k2);
					if (j2 == image_x - 1)fprintf(fp_V45G, "\n");

				}
			}
		}
	}

	fclose(fp_V45G);
	//領域の解放
	free_matrix(V45, 0, image_x - 1, 0, image_y - 1);
}

int V0_divide_gradation(int &image_x, int &image_y, double low_gradation, double max_gradation, int gradation_number,double gradient) {

	//Nrutilを用いたメモリの確保
	double **V0 = matrix(0, image_x - 1, 0, image_y - 1);
	//確保したメモリを初期化する
	for (int i1 = 0; i1 < image_y; i1++) {
		for (int j1 = 0; j1 < image_x; j1++) {
			V0[j1][i1] = 0;
		}
	}

	//Inputファイルを開く
	ifstream V_0(Filename1);
	//Outputファイルを開く
	FILE *fp_V0G;
	if ((fp_V0G = fopen(Filename1G, "w")) == NULL) { printf("入力エラー V0G.csvが開けません\nFile_name :%s ", Filename1G); exit(1); }

////////////////////////エラー出力/////////////////////////////////////////////////////////////////////////////////////////////
	if (!V_0) { printf("入力エラー V(0).csvがありません_cos-sim\nInput_Filename=%s", Filename1G); return 1; }

///////////////////////応答電圧のcsvの読み込み/////////////////////////////////////////////////////////////////////////////////
	int i1 = 1;
	string str_0;
	int count_large1 = 0;
	while (getline(V_0, str_0)) {					//このループ内ですべてを行う
		int	count_small1 = 0;			//初期化

///////////////いろいろ定義．ここでやらないといけない///////////////////////////////////////////////////////////////////////////
		string token_V_0;
		istringstream stream_V_0(str_0);


//////////////////////////配列に代入//////////////////////////////////////////////////////////////////////////////////////////////

		while (getline(stream_V_0, token_V_0, ',')) {	//一行読み取る．V0のみ，繰り返しの範囲指定に用いる
			double tmp_V_0 = stof(token_V_0);			//文字を数字に変換
			V0[count_small1][count_large1] = tmp_V_0;				//配列に代入
			//V0[count_small][count_large] = Rvectormagni[1] * V0[count_small][count_large]

			++count_small1;
		}
		++count_large1;
	}

	//量子化
	for (int i1 = 0; i1 < image_y; ++i1) {
		for (int j1 = 0; j1 < image_x; ++j1) {
			for (int k1 = 0; k1 < gradation_number; ++k1) {
				if (V0[j1][i1] > low_gradation + gradient*k1 && V0[j1][i1]<low_gradation + gradient*(k1 + 1)) {

					fprintf(fp_V0G, "%d,", k1);
					if (j1 == image_x - 1)fprintf(fp_V0G, "\n");

				}
			}
		}
	}

	fclose(fp_V0G);
	//領域の解放
	free_matrix(V0, 0, image_x-1, 0, image_y-1);
}

//量子化処理のスレッド制御
int divide_dradation(int &image_x, int &image_y, double low_gradation, double max_gradation, int gradation_number) {


	//量子化する
	double gradient=0;

	gradient = (max_gradation - low_gradation) / gradation_number;

	/*
	//単スレッド処理
	V0_divide_gradation(image_x,image_y,low_gradation,max_gradation,gradation_number,gradient);
	V45_divide_gradation(image_x, image_y, low_gradation, max_gradation, gradation_number, gradient);
	V90_divide_gradation(image_x, image_y, low_gradation, max_gradation, gradation_number, gradient);
	V135_divide_gradation(image_x, image_y, low_gradation, max_gradation, gradation_number, gradient);
	V180_divide_gradation(image_x, image_y, low_gradation, max_gradation, gradation_number, gradient);
	V225_divide_gradation(image_x, image_y, low_gradation, max_gradation, gradation_number, gradient);
	V270_divide_gradation(image_x, image_y, low_gradation, max_gradation, gradation_number, gradient);
	V315_divide_gradation(image_x, image_y, low_gradation, max_gradation, gradation_number, gradient);
	//単スレッド処理終わり
	*/
	
	//マルチスレッド処理
	std::thread t1(V0_divide_gradation, std::ref(image_x), std::ref(image_y), std::ref(low_gradation), std::ref(max_gradation), std::ref(gradation_number), std::ref(gradient));
	std::thread t2(V45_divide_gradation, std::ref(image_x), std::ref(image_y), std::ref(low_gradation), std::ref(max_gradation), std::ref(gradation_number), std::ref(gradient));
	std::thread t3(V90_divide_gradation, std::ref(image_x), std::ref(image_y), std::ref(low_gradation), std::ref(max_gradation), std::ref(gradation_number), std::ref(gradient));
	std::thread t4(V135_divide_gradation, std::ref(image_x), std::ref(image_y), std::ref(low_gradation), std::ref(max_gradation), std::ref(gradation_number), std::ref(gradient));
	std::thread t5(V180_divide_gradation, std::ref(image_x), std::ref(image_y), std::ref(low_gradation), std::ref(max_gradation), std::ref(gradation_number), std::ref(gradient));
	std::thread t6(V225_divide_gradation, std::ref(image_x), std::ref(image_y), std::ref(low_gradation), std::ref(max_gradation), std::ref(gradation_number), std::ref(gradient));
	std::thread t7(V270_divide_gradation, std::ref(image_x), std::ref(image_y), std::ref(low_gradation), std::ref(max_gradation), std::ref(gradation_number), std::ref(gradient));
	std::thread t8(V315_divide_gradation, std::ref(image_x), std::ref(image_y), std::ref(low_gradation), std::ref(max_gradation), std::ref(gradation_number), std::ref(gradient));
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	t7.join();
	t8.join();
	//マルチスレッド処理_終わり
	

	return 0;
}

//量子化を行う場合の畳み込み
int convolution_low_gradation(int argc, char** argv, char image_nameP2[], int &image_x, int &image_y, int paramerter[], int paramerter_count, int sd, char date[], char date_directory[], double low_gradation, double max_gradation, int gradation_number) {

	printf("****************************************\n");
	printf("start：convolution_gradation\n");
	printf("****************************************\n");

	printf("max_gradation=%f\n", max_gradation);
	printf("low_gradation=%f\n", low_gradation);
	printf("gradation_number=%d\n", gradation_number);


	convolution(argc, argv, image_nameP2, image_x, image_y, paramerter, paramerter_count, sd, date, date_directory);

	//量子化
	divide_dradation(image_x, image_y,low_gradation,max_gradation,gradation_number);

	return 0;
}