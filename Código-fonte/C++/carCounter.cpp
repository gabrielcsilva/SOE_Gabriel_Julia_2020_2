#include <opencv2/opencv.hpp>
#include <opencv2/bgsegm.hpp>
#include <opencv2/tracking.hpp>
#include <stdio.h>
#include <iostream>
#include <list>
#include <iterator>
#include <unistd.h>
using namespace cv;
using namespace std;

int carros = 0;

std::vector<int> pegaCentro(int x, int y, int largura, int altura)
{
    int x1 = largura / 2;
    int cx = x + x1;
    int y1 = altura / 2;
    int cy = y + y1;
    
    std::vector<int> centro = {cx, cy};
    
    return centro;
    }

	
void setInfo(vector< vector<int> > detec, Mat frame, int pos_linha, int offset)
{
    for(int i=0; i < detec.size(); i++)
    {
	 if(pos_linha + offset > detec[i][1])
	{
	    if(detec[i][1] > pos_linha - offset)
	    {
		cout << "X" << i << ", ";
		cout << "Eixo X" << detec[i][0] << ", ";
		cout << "Eixo Y" << detec[i][1] << endl;
		carros++;
		line(frame, Point(25, pos_linha), Point(1200, pos_linha), (0, 127, 255), 3);
		cout << "Carros detectados ate o momento:" << carros << endl;
		}
	    }
	}
    }	

Mat createBG(Mat frame, Ptr<BackgroundSubtractor> subtracao)
{
    Mat grey, blur, img_sub, img_dilat, img_dilat_2, img_open, img_close;
    
    cvtColor(frame, grey, COLOR_BGR2GRAY);  //Pega o frame e transforma para preto e branco
    GaussianBlur(grey, blur, Size(3, 3), 5);  //Faz um blur para tentar remover as imperfeições da imagem
    
    subtracao->apply(blur, img_sub);  //Faz a subtração da imagem aplicada no blur
    
    //Dilate
    Mat dilat = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    dilate(img_sub, img_dilat, dilat); 
    
    //Opening
    Mat open = getStructuringElement(MORPH_ELLIPSE, Size(5, 5)); 
    morphologyEx(img_dilat, img_open, MORPH_OPEN, open);
   
    //Dilate
    Mat dilat2 = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
    dilate(img_open, img_dilat_2, dilat2); 
   
    //Closing
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5)); 
    morphologyEx(img_dilat_2, img_close, MORPH_CLOSE, kernel);   
     
    return img_close;
    }

int main(int argc, char** argv){
    
    int largura_min = 80;
    int altura_min = 80;
    int largura_max = 280;
    int altura_max = 280;
    int offset = 6;
    int pos_linha = 550;

    //std::vector< vector<int> > detec;
    
    Ptr<Tracker> tracker = TrackerKCF::create();
    vector<Ptr<Tracker>> trackers;
    
    vector<Rect> tempPos;
    Rect roi;

    VideoCapture cap("../../Videos/highway_3.mp4"); // open the video file
    if(!cap.isOpened())  // check if we succeeded
    return -1;

    //create Background Subtractor objects
    Ptr<BackgroundSubtractor> subtracao;
    subtracao = bgsegm::createBackgroundSubtractorMOG();

    for(;;){
	Mat frame, img_close;
	cap >> frame;

	img_close = createBG(frame, subtracao);

	//Obj detection
	std::vector<std::vector<cv::Point> > contorno;
	findContours(img_close, contorno, RETR_TREE, CHAIN_APPROX_SIMPLE);

	for (int i=0; i < contorno.size(); i++){
	    //Determine valid contour
	    cv::Rect boundRect = boundingRect(contorno[i]);

	    int x = boundRect.x;
	    int y = boundRect.y;
	    int w = boundRect.width;
	    int h = boundRect.height;

	    bool validar_contorno = (w >= largura_min) && (h >= altura_min) && (w < largura_max) && (h < altura_max);
	    if (!validar_contorno)
		continue;
		
	    tempPos.push_back(boundRect);

	    //std::vector<int> centro = pegaCentro(x, y, w, h);
	    //detec.push_back(centro);

	    //circle(frame, Point(centro[0], centro[1]), 4, (0, 0, 255), -1);
	    }

	bool isTrack = false;
	for (int i = 0; i < tempPos.size(); i++){
	    for(int j = 0; j < trackers.size(); j++){
		trackers[j]->update(frame, roi);
		if(roi == tempPos[i])
		    isTrack = true;
		}
	    if(!isTrack)
		tracker->init(frame, tempPos[i]);
		trackers.push_back(tracker);
	    rectangle(frame, tempPos[i], (0, 255, 0), 2);
	    }

	line(frame, Point(25, pos_linha), Point(1200, pos_linha), (255, 127, 0), 3);
	//setInfo(detec, frame, pos_linha, offset);
	imshow("Video", frame);
	if(waitKey(30) >= 0) break;
	tempPos.clear();
	}
    return 0;
    }
