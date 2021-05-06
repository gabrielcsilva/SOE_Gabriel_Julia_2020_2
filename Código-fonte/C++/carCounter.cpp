#include <opencv2/opencv.hpp>
#include <opencv2/bgsegm.hpp>
#include <opencv2/tracking.hpp>
#include <stdio.h>
#include <iostream>
#include <list>
#include <iterator>
#include <unistd.h>
#include <vector>
#include <list>
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

bool pointInRect(int x, int y, int w, int h, int cx, int cy){
    int x1 = cx;
    int y1 = cy;
    if (x < x1 && x1 < x+w)
        if (y < y1 && y1 < y+h)
            return true;
    else
        return false;  
    }


int main(int argc, char** argv){
    
    int largura_min = 80;
    int altura_min = 80;
    int offset = 12;
    int pos_linha = 550;
    
    int frameCounter = 0;
    Rect boundRect;
    
    
    vector<Ptr<Tracker>> trackers;
    vector<Rect> roi;
    vector< bool > flag;

    VideoCapture cap("../../Videos/highway_3.mp4"); // open the video file
    if(!cap.isOpened())  // check if we succeeded
    return -1;

    //create Background Subtractor objects
    Ptr<BackgroundSubtractor> subtracao;
    subtracao = bgsegm::createBackgroundSubtractorMOG();

    for(;;){
	Mat frame, img_close;
	cap >> frame;

	if(frameCounter % 2 == 0){
	    //cout << "Detect";
	    //Obj detection
	    img_close = createBG(frame, subtracao);
	    
	    std::vector<std::vector<cv::Point> > contorno;
	    findContours(img_close, contorno, RETR_TREE, CHAIN_APPROX_SIMPLE);

	    for (int i=0; i < contorno.size(); i++){
		//cout << "-" ;
		//Determine valid contour
		boundRect = boundingRect(contorno[i]);

		int x = boundRect.x;
		int y = boundRect.y;
		int w = boundRect.width;
		int h = boundRect.height;

		bool validar_contorno = (w >= largura_min) && (h >= altura_min);
		if (!validar_contorno)
		    continue;
		
		bool isTracked = false;
		std::vector<int> centro = pegaCentro(x, y, w, h);
		
		//cout << ".";
		
		for(int i = 0; i < trackers.size(); i++){
		    //cout << "!";
		    Rect pos; 
		    trackers[i]->update(frame, pos);
		    int x = pos.x;
		    int y = pos.y;
		    int w = pos.width;
		    int h = pos.height;
		    std::vector<int> centroTracker = pegaCentro(x, y, w, h);
		    isTracked = pointInRect(x, y, w, h, centroTracker[0], centroTracker[1]);
		    }
		
		if (isTracked == false){
		    //cout << "?";
		    Ptr<Tracker> tracker = TrackerKCF::create();
		    tracker->init(frame, boundRect);
		    trackers.push_back(tracker);
		    roi.push_back(boundRect);
		    flag.push_back(isTracked);
		    }
		}
	    }
	else{
	    //cout << "Count - size: ";
	    //cout << trackers.size() << " ";
	    vector<int> it;
	    for(int i = 0; i < trackers.size(); i++)
	    {
		//cout << "|";
		trackers[i]->update(frame, roi[i]);
		if(roi[i].y+roi[i].height > pos_linha && flag[i] == false){
		    carros++;
		    flag[i] = true;
		    line(frame, Point(25, pos_linha), Point(1200, pos_linha), (0, 127, 255), 6);
		    cout << "Carros detectados ate o momento:" << carros << endl;
		    }
		else if (roi[i].y < pos_linha && flag[i] == false){
		    carros++;
		    cout << "Carros detectados ate o momento:" << carros << endl;
		    line(frame, Point(25, pos_linha), Point(1200, pos_linha), (0, 127, 255), 6);
		    }
		if(roi[i].y+roi[i].height > 600 || roi[i].y < 200){
		    it.push_back(i);
		    }
		}
	    for(int i = 0; i < it.size(); i++){
		trackers.erase(trackers.begin()+it[i]);
		trackers.shrink_to_fit();
		roi.erase(roi.begin()+it[i]);
		roi.shrink_to_fit();
		flag.erase(flag.begin()+it[i]);
		flag.shrink_to_fit();
		}
	    }

	for(int i = 0; i < trackers.size(); i++){
	    rectangle(frame, roi[i], Scalar( 0, 255, 0 ), 2);
	    vector<int> center = pegaCentro(roi[i].x, roi[i].y, roi[i].width, roi[i].height);
	    circle(frame, Point(center[0], center[1]), 4, (0, 0, 255), -1);
	    }

	line(frame, Point(25, pos_linha), Point(1200, pos_linha), (255, 127, 0), 6);
	imshow("Video", frame);
	frameCounter++;
	if(waitKey(30) >= 0) break;
	}
    return 0;
    }
