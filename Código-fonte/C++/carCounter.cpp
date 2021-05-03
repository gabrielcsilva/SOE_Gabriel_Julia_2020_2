#include <opencv2/opencv.hpp>
#include <opencv2/bgsegm.hpp>
#include <stdio.h>
#include <iostream>
#include <list>
#include <iterator>
#include <unistd.h>
#include <dlib/image_processing.h>
#include <dlib/geometry.h>
#include <dlib/opencv.h>

using namespace cv;
using namespace std;

std::vector<int> pegaCentro(int x, int y, int largura, int altura)
{
    int x1 = largura / 2;
    int cx = x + x1;
    int y1 = altura / 2;
    int cy = y + y1;
    
    std::vector<int> centro = {cx, cy};
    
    return centro;
    }

bool pointInRect(int x, int y, int w, int h, int cx, int cy)
{
    int x1, y1;
    x1 = cx;
    y1 = cy;
    if ((x < x1) && (x1 < x+w))
        if ((y < y1) && (y1 < y+h))
            return true;
    else
        return false;
    }	

int main(int argc, char** argv)
{
	
VideoCapture cap("../../Videos/highway_3.mp4"); // open the video file
if(!cap.isOpened())  // check if we succeeded
    return -1;

int carros = 0; //Contador

//Ajustes de detecção
int largura_min = 80;
int altura_min = 80;
int offset = 6;
int pos_linha = 550;

std::vector< dlib::correlation_tracker > trackers;
vector< bool > trackerId;

//Background Subtractor =
Ptr<BackgroundSubtractor> subtracao;
subtracao = bgsegm::createBackgroundSubtractorMOG();

for(;;)
{
    Mat frame, grey, blur, img_sub, img_dilat, img_dilat_2, img_open, img_close;
    cap >> frame; // Pega frame do video
    
    cvtColor(frame, grey, COLOR_BGR2GRAY);  //Transforma frame para preto e branco
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
    
    //Atualizando trackers
    //dlib::array2d< dlib::bgr_pixel > dlibFrame
    dlib::cv_image<dlib::bgr_pixel> dlibFrame(frame); 
    for (int i = 0; i < trackers.size(); i++)
    {
	trackers[i].update(dlibFrame);
    }
    
    //Acha o contorno dos veículos
    std::vector<std::vector<cv::Point> > contorno;
    findContours(img_close, contorno, RETR_TREE, CHAIN_APPROX_SIMPLE);
    
    for (int i=0; i < contorno.size(); i++)
    {
	//Identifica contorno a validar
	cv::Rect boundRect = boundingRect(contorno[i]);
	
	//Pega as dimensões do contorno
	int x = boundRect.x;
	int y = boundRect.y;
	int w = boundRect.width;
	int h = boundRect.height;
	
	//Verifica se as dimensões respeitam as condições, caso não, pula fora
	bool validar_contorno = (w >= largura_min) && (h >= altura_min);
	if (!validar_contorno)
		continue;

	//Printa o retângulo do contorno na tela
	rectangle(frame, Point(x, y), Point(x + w, y + h), (0, 255, 0), 2);
	
	//Pega o centro do contorno e printa na tela
	std::vector<int> centro = pegaCentro(x, y, w, h);
	circle(frame, Point(centro[0], centro[1]), 4, (0, 0, 255), -1);
	
	//Insere novos contornos ao trackers
	bool tracking = false;
	
	for (int i = 0; i < trackers.size(); i++)
	{
	    //Identifica posição dos contornos já identificados
	    dlib::rectangle dRect;
	    dRect = trackers[i].get_position();
	    float startX = dRect.left();
	    float startY = dRect.top();
	    float endX = dRect.right();
	    float endY = dRect.bottom();
	    
	    std::vector<int> trackerCenter = pegaCentro((int)startX, (int)startY, (int)endX, (int)endY);
	    bool t_location_chk = pointInRect(x, y, w, h, centro[0], centro[1]);
	    
	    if(t_location_chk)
		tracking= true;
		
	    }
	    
	if (tracking == false)
	{
	    dlib::correlation_tracker tracker;
	    tracker.start_track(dlibFrame, dlib::centered_rect(dlib::point(centro[0], centro[1]), w, h));
	    trackers.push_back(tracker);
	    trackerId.push_back(false);
	    }

	    }
    
    //Desenha linha no frame
    line(frame, Point(25, pos_linha), Point(1200, pos_linha), (255, 127, 0), 3);

    //Atualiza contagem
    for(int i=0; i < trackers.size(); i++)
    {
	 //Identifica posição dos blobs
	 dlib::rectangle dRect;
	 dRect = trackers[i].get_position();
	 float startX = dRect.left();
	 float startY = dRect.top();
	 float endX = dRect.right();
	 float endY = dRect.bottom();
	 std::vector<int> trackerCenter = pegaCentro((int)startX, (int)startY, (int)endX, (int)endY);

	 if(pos_linha + offset > trackerCenter[1] && trackerCenter[1] > pos_linha - offset)
	{
		if(trackerId[i] == false)
		    carros++;
		line(frame, Point(25, pos_linha), Point(1200, pos_linha), (0, 127, 255),3);
		cout << "Carros detectados ate o momento:" << carros << endl;
		trackerId[i] = true;
	    }
	else
	{
	    trackers.pop_back();
	    trackerId.pop_back();
	    }
	}
    imshow("Video", frame);
    if(waitKey(30) >= 0) break;
    }
return 0;
}
