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
static vector<vector<Mat> > diffImages;
static int frameSum;
static vector<int> gsections;
static int picCount;

void* runCamera2(void* args)
{ 
    VideoCapture cap(0);
    if(!cap.isOpened())
	return (void*)-1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    cout<<"the picCount is"<<picCount<<endl;
    while(1)
    {
	Mat frame;
	cap >> frame;
	
	cout<<frameSum<<endl; 
	pthread_mutex_lock(&image_mutex);
	for(int i=0;i<gsections.size();i++)
	{
	    if(diffImages[i].size()<picCount)
	    {
		if(frameSum % gsections[i] == 0)
		{
		    cout<<"the frameSum is "<< frameSum<<endl;
		    diffImages[i].push_back(frame);		 		        
		}
	    }
	    else
		{
		    if(frameSum % gsections[i] == 0)
	            {
		        vector<Mat>::iterator k = diffImages[i].begin();
		        diffImages[i].erase(k);
		        diffImages[i].push_back(frame);
		    }
		}
	}
	frameSum++; 
	if(frameSum > maxImageNums)
	    pthread_cond_signal(&count_image);
	pthread_mutex_unlock(&image_mutex);
    } 
    return (void*)0;
}

int getFrames(vector<vector<Mat> >& frames)
{
    int ret = 0;
    pthread_mutex_lock(&image_mutex);	 
    while(frameSum<maxImageNums)
    {
	pthread_cond_wait(&count_image,&image_mutex);
    }
    for(int i=0;i<diffImages.size();i++)
    {
	frames.push_back(diffImages[i]);
    }
    pthread_mutex_unlock(&image_mutex);
    return ret;
}


int openCamera()
{
    int err = pthread_create(&tid,NULL,runCamera2,NULL);
    return err;
}

void init(vector<int> sections,int frameNums)
{
   tid = 0;
   frameSum = 0;
   picCount = frameNums;
   maxImageNums = frameNums*sections[sections.size()-1];
   for(int i=0;i<sections.size();i++)
   {
	gsections.push_back(sections[i]);
   }
   vector<vector<Mat> > test(sections.size());
   diffImages = test;
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
    int err;
    vector<int> sections;
    sections.push_back(1);
    sections.push_back(2);
    sections.push_back(5);
    vector<vector<Mat> >Imageframes; 
    int frameNums = 10;
    init(sections,frameNums);
    openCamera();
    for(int i=0;i<10;i++)
{
    err = getFrames(Imageframes);
    cout<<"size is"<< Imageframes.size()<<endl;
    for(int i=0;i<Imageframes.size();i++)
    {
	cout<< "size is" << (Imageframes[i].size()) << endl;
	for(int j=0;j<Imageframes[i].size();j++)
	{
	    waitKey(1);
	    imshow("display",Imageframes[i][j]);
	}
    }
}
    waitSubThread();   
    return 0;
}


