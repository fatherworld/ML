#include<stdio.h>
#include<opencv2/opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<vector>
#include<unistd.h>
using namespace std;
using namespace cv;
static pthread_t tid;
static int maxImageNums;
static pthread_mutex_t image_mutex;
static pthread_cond_t count_image;
static vector<Mat> Images;

int GetFrames(vector<int> sections,vector<vector<Mat> > &frames,int frameNums)
{
    int ret = 0;
    pthread_mutex_lock(&image_mutex);	
    while(Images.size()<maxImageNums)
    {
	pthread_cond_wait(&count_image,&image_mutex);
    }
    int start_num = 0;
    int end_num = sections.size();
    for (vector<int>::iterator it = sections.begin();it != sections.end();it++)
    {
	int temps = *it;
	int countSize = Images.size();
	vector<Mat> temp;
	for(int i=0;i<frameNums;i++)
	    {
		temp.push_back(Images[(temps)*i]);
	    }
	frames.push_back(temp);	
	temp.clear();
    }
    Images.clear();
    pthread_mutex_unlock(&image_mutex);
    return ret;
}
void* runCamera1(void* args)
{ 
    VideoCapture cap(0);
    if(!cap.isOpened())
	return (void*)-1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    while(1)
    {
	Mat frame;
	cap >> frame;  
	pthread_mutex_lock(&image_mutex);
	if(Images.size() > maxImageNums)
	    pthread_cond_signal(&count_image);
        Images.push_back(frame);
	pthread_mutex_unlock(&image_mutex);
    } 
    return (void*)0;
}

int openCamera(int maxImages)
{
    maxImageNums = maxImages;
    int err = pthread_create(&tid,NULL,runCamera1,NULL);
    return err;
}

void init()
{
   tid = 0;
   maxImageNums = 0;
   pthread_mutex_init(&image_mutex,NULL); 
   pthread_cond_init(&count_image,NULL);
}

int waitSubThread()
{
    void* nRes;
    while(1)
    {
	pthread_join(tid, &nRes);
	if(*(int*)nRes >= 0)
	{
	     break;
	}   
    }
    return 0;
}

int main(int argc,char** argv)
{
    init();
    int err;
    openCamera(54);
    vector<int> sections;
    sections.push_back(1);
    sections.push_back(2);
    sections.push_back(5);
    vector<vector<Mat> >frames; 
    int frameNums = 10;
    err = GetFrames(sections,frames,frameNums);
    waitSubThread();   
    return 0;
}


