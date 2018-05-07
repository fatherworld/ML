#include<stdio.h>
#include<opencv2/opencv.hpp>
//#include<cv.h>
#include<opencv2/highgui/highgui.hpp>
#include<vector>
#include<unistd.h>
using namespace std;
using namespace cv;
static pthread_t tid;
int maxImageNums = 0;
pthread_mutex_t image_mutex;
pthread_cond_t count_image;
vector<Mat> Images;

int GetFrames(vector<int> sections,vector<vector<Mat> > &frames,int frameNums)
{
    int ret = 0;

   // if(Images.size() < maxImageNums)
//	sleep(1);

    pthread_mutex_lock(&image_mutex);	
    while(Images.size()<maxImageNums)
    {
	pthread_cond_wait(&count_image,&image_mutex);
    }
    int start_num = 0;
    int end_num = sections.size();
    cout<<"end_num"<<end_num<<endl;
    cout<<"image.size is "<<Images.size()<<endl;
    for (vector<int>::iterator it = sections.begin();it != sections.end();it++)
    {
	cout<<"section's it is"<<*it<<endl;
	
	int temps = *it;
	int countSize = Images.size();
	vector<Mat> temp;
	for(int i=0;i<frameNums;i++)
	    {
		cout<<"i is"<<i<<endl;
		temp.push_back(Images[(temps)*i]);
		//frames[start_num][i] = Images[(temp)*i];
		cout<<"image is"<<endl;
	    }
	frames.push_back(temp);	
	temp.clear();
    }
    Images.clear();
    pthread_mutex_unlock(&image_mutex);
    return ret;
}

void* runCamera(void* args)
{ 
  //  int maxImages = *((int*)args);
    VideoCapture cap(0);
    if(!cap.isOpened())
	return (void*)-1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
//    cap.set(CV_CAP_PROP_FPS,15);
//    cvNamedWindow("video",CV_WINDOW_AUTOSIZE);
    while(1)
    {
	Mat frame;
	waitKey(10);
	cap >> frame;
        
	pthread_mutex_lock(&image_mutex);

        Images.push_back(frame);
	imshow("video",frame);

        pthread_mutex_unlock(&image_mutex);

    } 
    return (void*)0;
}

void* runCamera1(void* args)
{ 
  //  int maxImages = *((int*)args);
    VideoCapture cap(0);
    if(!cap.isOpened())
	return (void*)-1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
//    cap.set(CV_CAP_PROP_FPS,15);
//    cvNamedWindow("video",CV_WINDOW_AUTOSIZE);
    while(1)
    {
	Mat frame;
	waitKey(10);
	cap >> frame;
        
	pthread_mutex_lock(&image_mutex);
	if(Images.size() > maxImageNums)
	    pthread_cond_signal(&count_image);
        Images.push_back(frame);
	imshow("video",frame);
	pthread_mutex_unlock(&image_mutex);
    } 
    return (void*)0;
}


int openCamera(int maxImages)
{
    maxImageNums = maxImages;
    //int err = pthread_create(&tid,NULL,runCamera,(&maxImages));
    int err = pthread_create(&tid,NULL,runCamera1,NULL);
    return err;
}

void init()
{
   pthread_mutex_init(&image_mutex,NULL); 
   pthread_cond_init(&count_image,NULL);
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
    //while(1);
    pthread_join(tid, NULL);

    return 0;
}


int main1(int argc,char**argv)
{
    VideoCapture cap(0);
    if(!cap.isOpened())
	return -1;
    cap.set(CV_CAP_PROP_FRAME_WIDTH,640);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    cap.set(CV_CAP_PROP_FPS,15);
    Mat src_gray;
    cvNamedWindow("video",CV_WINDOW_AUTOSIZE);
    while(1)
    {
	Mat frame;
	waitKey(10);
        cap>>frame;
	Images.push_back(frame);
	cout<<"the size of Images is "<<Images.size()<<endl;
	
	imshow("video",Images[Images.size()-1]);
    }
/*
    Mat image;
    char* src = "/home/ys/Desktop/1.jpg";
    image = imread(src,0);
    namedWindow("DisplayImage",CV_WINDOW_AUTOSIZE);
    imshow("DisplayImage",image);
    waitKey(10000);
    return 0;
*/
}
