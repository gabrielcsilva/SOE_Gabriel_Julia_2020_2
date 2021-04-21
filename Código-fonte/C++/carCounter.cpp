#include <opencv2/opencv.hpp>
#include <stdio.h>
#include <iostream>
#include <list>
#include <iterator>
#include <unistd.h>
using namespace cv;
using namespace std;

//Variaveis globais
int carros = 0;
int caminhoes = 0;

int largura_min = 80;
int altura_min = 80;
int offset = 6;
int pos_linha = 550;
std::list<int> detecX;
std::list<int> detecY;

int pegaLargura(int x, int largura)
{
	int x1 = largura % 2;
	int cx = x + x1;
	return cx;
	}

int pegaAltura(int y, int altura)
{
	int y1 = altura % 2;
	int cy = y + y1;
	return cy;
	}

void setInfo(std::list<int> detecX, std::list<int> detecY, Mat frame)
{
	int sizeX = detecX.size();
	int sizeY = detecY.size();
	for (int x=0; x < sizeX; x++) //Tamanho do array
	{
		for (int y=0; y < sizeY; y++)
		{
			if(pos_linha + offset > y && pos_linha - offset < y)
			{
				carros++;
				line(frame, Point(25, pos_linha), Point(1200, pos_linha), (0, 127, 255), 3);
				//detecX.pop_back();
				//detecY.pop_back();
				printf("Carros detectados ate o momento: %d\n", carros);
				}
			}
		}
	}

int main(int argc, char** argv)
{
	
VideoCapture cap("../../Videos/highway_3.mp4"); // open the video file
if(!cap.isOpened())  // check if we succeeded
    return -1;

//create Background Subtractor objects
Ptr<BackgroundSubtractor> subtracao;
subtracao = createBackgroundSubtractorMOG2();

for(;;)
{
    Mat frame, grey, blur, img_sub, img_dilat, img_dilat_2, img_open, img_close;
    cap >> frame; // get a new frame from camera
    
    cvtColor(frame, grey, COLOR_RGB2GRAY);  //Pega o frame e transforma para preto e branco
    GaussianBlur(grey, blur, Size(3, 3), 0, 0);  //Faz um blur para tentar remover as imperfeições da imagem
    
    subtracao->apply(blur, img_sub);  //Faz a subtração da imagem aplicada no blur
    
    //Dilate
    Mat dilat = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    dilate(img_sub, img_dilat, dilat); 
    
    //Opening
    Mat open = getStructuringElement(MORPH_ELLIPSE, Size(2, 2)); 
    morphologyEx(img_dilat, img_open, MORPH_OPEN, open);
   
	//Dilate
	Mat dilat2 = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
    dilate(img_open, img_dilat_2, dilat2); 
   
	//Closing
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5)); 
    morphologyEx(img_dilat_2, img_close, MORPH_CLOSE, kernel);    
     
    std::vector<std::vector<cv::Point> > contorno;
     
    findContours(img_close, contorno, RETR_TREE, CHAIN_APPROX_SIMPLE);
    line(frame, Point(25, pos_linha), Point(1200, pos_linha), (255, 127, 0), 3);
    
    for (int i=0; i < contorno.size(); i++)
    {
		for (int j=0; j < contorno[i].size(); j++)
		{
			std::vector<cv::Rect> boundRect( contorno[i].size() );
			boundRect[j] = boundingRect(contorno[i]);
			
			int x = boundRect[j].x;
			int y = boundRect[j].y;
			int w = boundRect[j].width;
			int h = boundRect[j].height;
			
			bool validar_contorno = (w >= largura_min) && (h >= altura_min);
			if (!validar_contorno)
				continue;

			rectangle(frame, Point(x, y), Point(x + w, y + h), (0, 255, 0), 2);
			
			int largura = pegaLargura(x, w);
			int altura = pegaAltura(y, h);
			detecX.push_back(largura);
			detecY.push_back(altura);
			
			circle(frame, Point(largura, altura), 4, (0, 0, 255), -1);
			}
		}
	//printf("Um frame\n");
    setInfo(detecX, detecY, frame);
    imshow("Video", img_close);
    if(waitKey(30) >= 0) break;
}
return 0;
}
