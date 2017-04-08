/* THIS PROGRAM OPENS THE WEBCAM USING OPENCV, ALLOWS USER TO DRAG A LINE USING MOUSE */

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <string.h>
 
using namespace std;
using namespace cv;
char key;
bool clicked=false;
Mat img,imgGray; //ADDED
Point P1(0,0);
Point P2(0,0);
//const char* winName="Original";


/*This function places the x and y coordinates of a mouse click into point variables */
void onMouse( int event, int x, int y, int f, void* ){
    switch(event){
        case  CV_EVENT_LBUTTONDOWN  :
	  //one mouse click shows up as a dot
                                        clicked=true;
                                        P1.x=x;
                                        P1.y=y;
                                        P2.x=x;
                                        P2.y=y;
                                        break;
        case  CV_EVENT_LBUTTONUP    :
	  //when the user releases the mouse, the second point is recorded
                                        P2.x=x;
                                        P2.y=y;
                                        clicked=false;
                                        break;
        case  CV_EVENT_MOUSEMOVE    :
	  //as the mouse moves, the second point is recorded; making a "drag and drop" system
                                      if(clicked){
                                        P2.x=x;
                                        P2.y=y;
                                        }
                                        break;
        default                     :   break;
   }
}


int main(int argc,char** argv)
{

	//NETWORKING STUFF: socket, bind, listen
	int localSocket;
	int remoteSocket;
	int port = 4097;

	struct sockaddr_in localAddr,remoteAddr;
	int addrLen = sizeof(struct sockaddr_in);

	if ( (argc > 1) && (strcmp(argv[1],"-h") == 0) ) {
          std::cerr << "usage: ./cv_video_srv [port] [capture device]\n" <<
                       "port           : socket port (4097 default)\n" <<
                       "capture device : (0 default)\n" << std::endl;
 
          exit(1);
	    }
	 
	    if (argc == 2) port = atoi(argv[1]);
	 
	    localSocket = socket(AF_INET , SOCK_STREAM , 0);
	    if (localSocket == -1){
		 perror("socket() call failed!!");
	    }    
	 
	    localAddr.sin_family = AF_INET;
	    localAddr.sin_addr.s_addr = INADDR_ANY;
	    localAddr.sin_port = htons( port );
	 
	    if( bind(localSocket,(struct sockaddr *)&localAddr , sizeof(localAddr)) < 0) {
		 perror("Can't bind() socket");
		 exit(1);
	    }
	    
	    //Listening
	    listen(localSocket , 1);
	    
	    std::cout <<  "Waiting for connections...\n"
		      <<  "Server Port:" << port << std::endl;
	 
	    //accept connection from an incoming client
	    remoteSocket = accept(localSocket, (struct sockaddr *)&remoteAddr, (socklen_t*)&addrLen);
	    if (remoteSocket < 0) {
		perror("accept failed!");
		exit(1);
	    }
	    std::cout << "Connection accepted" << std::endl;

	//OPENCV CODE
	int capDev = 0;
    	if (argc == 3) capDev = atoi(argv[2]);
	img = Mat::zeros(480, 640, CV_8UC1);    
 
    //make it continuous
    if (!img.isContinuous()) {
        img = img.clone();
    }
 
    int imgSize = img.total() * img.elemSize();
    int bytes = 0;
    int key;
    
 
    //make img continuos
    if ( ! img.isContinuous() ) { 
          img = img.clone();
          imgGray = img.clone();
    }
        
    std::cout << "Image Size:" << imgSize << std::endl;
	


  //creates a window for the camera
    cvNamedWindow("Camera_Output", 1);    

  //apparently this command captures any camera linked to system *MUST EXPERIMENT*
    CvCapture* capture = cvCaptureFromCAM(CV_CAP_ANY); 

 //Create infinte loop for live streaming
    while(1){ 
 
        IplImage* frame = cvQueryFrame(capture); //Create image frames from capture     
	img =frame;	

	//namedWindow(winName, WINDOW_NORMAL); //creates window to figure out where to place the line
	//imshow(winName, img);
	//int imgSize = img.total() * img.elemSize(); 
	setMouseCallback("Camera_Output",onMouse,0); //Runs the mouse callback function, records the points for the line
	line(img, P1, P2, Scalar(0,255,255), 5, 8, 0); //Physically draws the line over the image
        cvShowImage("Camera_Output", frame);   //Show image frames on created window
        key = cvWaitKey(10);     //captures Keyboard stroke
        if (char(key) == 27){
            break;      //If you hit ESC key loop will break while in the original frame window
        }
	
	cvtColor(img, imgGray,CV_BGR2GRAY); //CV_BRG2GRAY
	if ((bytes = send(remoteSocket,imgGray.data,imgSize,0)) <0) {
		std::cerr << "bytes = " <<bytes <<std::endl;
		break;	
	}

    }

    close(remoteSocket);
    cvReleaseCapture(&capture);          //Stops the webcam
    cvDestroyWindow("Camera_Output");   //closes streaming window
    return 0;
}
