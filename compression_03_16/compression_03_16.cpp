#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <winsock2.h>
#include <cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "struct.h"
#include <vector>
//#include <pthread.h>


using namespace cv;
using namespace std;

#define PORTNUM 8888
#define SEND_PORT 8887
#define IP_ADDRESS "10.42.0.1"

bool conn_flag = false;

#pragma comment(lib,"ws2_32.lib")
#define  oops(msg)  {perror(msg); exit(1);}

//SOCKET soc;
//sockaddr_in serAddr;

FloatRect initBB;
bool if_rectangle = false;

FloatRect box;
bool drawing_box = false;
bool gotBB = false; // got tracking box or not
string video;
bool fromfile=false;
void mouseHandler(int event, int x, int y, int flags, void *param);

void rectangle(Mat& rMat, const FloatRect& rRect, const Scalar& rColour)
{
        IntRect r(rRect);
        rectangle(rMat, Point(r.XMin(), r.YMin()), Point(r.XMax(), r.YMax()), rColour,2,7,0);
}

int main(int argc, char* argv[])
{
	//char IP_ADDRESS[16];
	//cout <<"Please input IP_address:";

	//fgets(IP_ADDRESS,16,stdin);

	WORD sockVersion = MAKEWORD(2,2);
	WSADATA data; 
	if(WSAStartup(sockVersion, &data) != 0)
	{
		exit(0);
	}

	SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(sclient == INVALID_SOCKET)
	{
		printf("invalid socket !\n");
		exit(0);
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(8888);
	serAddr.sin_addr.S_un.S_addr = inet_addr(IP_ADDRESS); 
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("connect error !\n");
		closesocket(sclient);
		exit(0);
	}


//      Mat frame;
        cvNamedWindow("server", CV_WINDOW_AUTOSIZE);
		setMouseCallback("server", mouseHandler, NULL);

		int framelen;
		char s[20] = {0};


        while(true)
        {
			// 读取数据的大小，文本转化成int
			recv(sclient, s, 15, 0);
			framelen = atoi(s);
			cout<<"framelen = "<<framelen<<endl;
			// 接收压缩后的图像数据
			vector<uchar> buff(framelen);
			int bytes = 0;
			for (int i = 0; i < framelen ; i += bytes)
			{
				if ((bytes = recv(sclient, (char*)&buff[0] + i, framelen - i, 0)) == -1)
					oops("Receive");
				//	cout<<"bytes = "<<bytes<<endl;
			}


				if(if_rectangle)
				{
				//	send(soc, (const char *)&initBB, sizeof(initBB), 0);
					send(sclient, (const char *)&initBB, sizeof(initBB), 0);
					printf("Initial Tracking initBB = x:%f y:%f h:%f w:%f\n", initBB.m_xMin, initBB.m_yMin, initBB.m_width, initBB.m_height);
					if_rectangle = false;
				}

			// 解压图像
			Mat frame = imdecode(Mat(buff), CV_LOAD_IMAGE_COLOR);
            imshow("server", frame);
            cvWaitKey(1);

                //usleep(100000);
        }

	cvDestroyWindow("server");	
	closesocket(sclient);
//	closesocket(soc);
	WSACleanup();
	return 0;
}

void mouseHandler(int event, int x, int y, int flags, void *param)
{

       switch (event)
        {
                case CV_EVENT_MOUSEMOVE:
                if (drawing_box)
                {
                        //box.width = x - box.x;
                        //box.height = y - box.y;

                        box.m_width = x - box.m_xMin;
                        box.m_height = y - box.m_yMin;
                }
                break;
                case CV_EVENT_LBUTTONDOWN:
                        drawing_box = true;
                        //box = IntRect(x, y, 0, 0);
                        box = FloatRect(x, y, 0, 0);
                break;
                case CV_EVENT_LBUTTONUP:
                        drawing_box = false;
                        if (box.m_width < 0)
                        {
                                box.m_xMin += box.m_width;
                                box.m_width *= -1;
                        }
                        if( box.m_height < 0 )
                        {
                                box.m_yMin += box.m_height;
                                box.m_height *= -1;
                        }

                        printf("Initial Tracking Box = x:%f y:%f h:%f w:%f\n", box.m_xMin, box.m_yMin, box.m_width, box.m_height);
                        initBB = FloatRect(box.m_xMin, box.m_yMin, box.m_width, box.m_height);
						if_rectangle = true;
//                        position_ready = true;
//						cout<<"huakuang----------"<<endl;
		
                break;
                default:
                break;
        }
}
