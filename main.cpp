/*　１．大恒相机硬触发采集图像
*　 ２．通过OpenCV处理图像
*  ３．动态计算升运器的速度
*  ４.串口发送流量传感器数据
*/
#include "GxIAPI.h"

//串口发送模块
#include "serialSend/serialSend.h"

//init can module
#include "canInit/canInit.h"

//图像处理模块
#include "ImageProcessing/ImageProcessing.h"


//实际测得小麦的容重是841.23g/L = 0.84123g/cm3 
float volume_weight = 0.84;

//刮板的截面积是140cm2
float CSA = 140.0;

//这一帧刮板上谷物的质量
float this_weight;

//谷物总的产量
float sum_weight=0.0;

//当前谷物的流量
float this_yield;
//当前帧
Mat thisFrame(1024,1280,CV_8UC1);   //OpenCV Mat

//掩模的图像
Mat mask;

//保存的照片张数
int g_count;

//用来记录时间戳
struct timeval tstart,tend;


//用来计算使用的时间，单位是微秒，usec，1s = 1,000,000 us；
long int use_microconds = 50000;


//串口发送的消息
unsigned char send_buf[8];


//Image processing callback function.
static void GX_STDC OnFrameCallbackFun(GX_FRAME_CALLBACK_PARAM* pFrame)
{
      gettimeofday(&tend,NULL);

      use_microconds=1000000*(tend.tv_sec - tstart.tv_sec) + (tend.tv_usec - tstart.tv_usec);

      cout << "used microseconds is  "<< use_microconds << " us" << endl;
      cout << "used milliseconds is  "<< use_microconds/1000.0 << " ms" << endl;


      tstart.tv_sec=tend.tv_sec ;
      tstart.tv_usec=tend.tv_usec ;

      if (pFrame->status == 0)
      {
        //Performs some image processing operations
        g_count++ ;
#if 0

        //将相机拍摄的照片从内存中拷贝到Mat变量的data指针，这样才可以调用OpenCV来处理,关键代码
      	memcpy(thisFrame.data,pFrame->pImgBuf,pFrame->nImgSize);

      	/*put OpenCV image processing code here,consider using multi-thread next time*/
      	//存储原始的照片,调试时使用，实际开发的时候应该注释掉此语句
      	 //if(g_count<1000)
         //{
         //  imwrite("before_test_"+to_string(g_count)+".jpg",thisFrame);
         //  g_count++;
         //}

	
	//输入:当前帧的图像，掩膜图像，容重，截面积，当前帧刮板上的谷物重量，谷物总的重量,标定的比例系数
	float coeff = 6;
        ImageProcessing(thisFrame,mask,volume_weight,CSA,this_weight,sum_weight,coeff);

        //计算流量，单位是g/s
        this_yield = this_weight*1000000/use_microconds;

	//单位转为kg,然后强制转成int型的
	int m_yield = (int)(this_yield);

	send_buf[0]=m_yield%256;
	send_buf[1]=m_yield/256;

        cout<<"yield is "<< m_yield <<"  g/s" << endl;

        cout << "weight is " << sum_weight<<" g-------"<<int(sum_weight/1000) << " kg" << endl;
#endif

	send_buf[0]=g_count%256;
	send_buf[1]=g_count/256;

	send_buf[2]=g_count%256;
	send_buf[3]=g_count/256;

        cout <<"--------------------captured---------------"<<endl<<endl;

    }

      return;

}


int main(int argc, char* argv[])
{
   /*串口相关的设置*/
   int fd;                            //文件描述符
   int err;                           //返回调用函数的状态
   int len;

   struct termios options;

   //初始化开始时间戳
   tstart.tv_sec = 0;
   tstart.tv_usec = 0;

   for(int i=0;i<8;i++)
	send_buf[i] = 0;

   /* 打开串口，返回文件描述符，在此之前必须先给串口写权限，或者直接将用户加入diaout用户组，否则会报错  */
   fd = open( "/dev/ttyUSB0", O_RDWR|O_NOCTTY|O_NDELAY);

   serial_setup(fd,&options);

   InitCanBus(fd);

   g_count=0;

   /**读取背景图片，下面用来去除背景
   *
   */
   mask = imread("MASK.jpg",0);

   threshold(mask, mask, 80, 255, CV_THRESH_BINARY);

   GX_STATUS status = GX_STATUS_SUCCESS;

   GX_DEV_HANDLE hDevice = NULL;

   GX_OPEN_PARAM stOpenParam;

   uint32_t nDeviceNum = 0;

   // 初始化库
   status = GXInitLib();
   if (status!= GX_STATUS_SUCCESS)
   {
       cout<<"Unable to Init Camera"<<endl;
       return 0;
   }

   // 更新设备列表
   status = GXUpdateDeviceList(&nDeviceNum, 1000);
   if ((status != GX_STATUS_SUCCESS) || (nDeviceNum<= 0))
   {
       cout<<"Unable to Update Device List"<<endl;
       return 0;
   }

   //控制方式打开设备
   stOpenParam.accessMode = GX_ACCESS_CONTROL;

   //通过设备序号打开设备
   stOpenParam.openMode = GX_OPEN_INDEX;

   //从１开始
   stOpenParam.pszContent = "1";

    //打开设备
   status = GXOpenDevice(&stOpenParam, &hDevice);

    if (status == GX_STATUS_SUCCESS)
    {
	//触发模式置为On 
        status =GXSetEnum(hDevice,GX_ENUM_TRIGGER_MODE, GX_TRIGGER_MODE_ON);


        //选择触发源为Line0 
        status =GXSetEnum(hDevice,GX_ENUM_TRIGGER_SOURCE, GX_TRIGGER_SOURCE_LINE0);


	//设置引脚方向为输入
        status = GXSetEnum(hDevice, GX_ENUM_LINE_MODE, GX_ENUM_LINE_MODE_INPUT);

        //设置曝光模式
        status = GXSetEnum(hDevice, GX_ENUM_EXPOSURE_MODE,GX_EXPOSURE_MODE_TIMED);


        //设置曝光时间 1000us
	status = GXSetFloat(hDevice, GX_FLOAT_EXPOSURE_TIME, 2000);

        //设置触发极性为下降沿触发
        status=GXSetEnum(hDevice,GX_ENUM_TRIGGER_ACTIVATION,GX_TRIGGER_ACTIVATION_FALLINGEDGE);
	//status=GXSetEnum(hDevice,GX_ENUM_TRIGGER_ACTIVATION,GX_TRIGGER_ACTIVATION_RISINGEDGE);

        //注册图像处理回调函数
        status = GXRegisterCaptureCallback(hDevice, NULL, OnFrameCallbackFun);

        //发送图像获取命令
        status = GXSendCommand(hDevice, GX_COMMAND_ACQUISITION_START);

       //---------------------
       //If a valid trigger is generated before sending the stop call order,
       //the image will be returned to the user via the OnFrameCallbackFun interface.
       //---------------------
    	while(true)
    	{
    	 //使用串口将数据发送到下位机
    	  serial_send(fd,send_buf,8);

	 //每隔1000ms发送一次
          sleep(1);

        //  cout<<"------------------------------------"<<endl;
        //  cout<<"send message every 1 second "<<endl;
          cout<<"------------------In while----------------------- "<<endl;       
    	}

       //发送图像停止采集命令
       status = GXSendCommand(hDevice, GX_COMMAND_ACQUISITION_STOP);

       //Unregisters image processing callback function.
       status = GXUnregisterCaptureCallback(hDevice);
    }

    status = GXCloseDevice(hDevice);
    status = GXCloseLib();
    return 0;
}
