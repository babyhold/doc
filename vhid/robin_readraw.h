#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>



#include <unistd.h>  
#include <stdlib.h>  
#include <stdio.h>	
#include <string.h>  



#define FIFO_SERVER "/tmp/myfifo"  

/*
 * Ugly hack to work around failing compilation on systems that don't
 * yet populate new version of hidraw.h to userspace.
 *
 * If you need this, please have your distro update the kernel headers.
 */
#ifndef HIDIOCSFEATURE
#define HIDIOCSFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x06, len)
#define HIDIOCGFEATURE(len)    _IOC(_IOC_WRITE|_IOC_READ, 'H', 0x07, len)
#endif

/* Unix */
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* C */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>



//#include "EventHub.h"

#define  DOUBLEMODE 1

#define TOUCH_MODE_TEST_ENABLE    1
#define KEY_MODE_TEST_ENABLE     0

#define TOUCH_MODE 1

static pthread_t  cmd_thread;

static  struct timeval t_last;  
static int gesnum,gestype;

#define QINTERTIME 400
#define GESMINCOUNT 9

enum touch_act_t{
   TOUCH_ACTION_NOFINGER  = 0,
   TOUCH_ACTION_DOWN      = 1,
   TOUCH_ACTION_PRESSED   = 2,
   TOUCH_ACTION_UP        = 3
};

typedef struct touch_info_t{
   enum touch_act_t action;
   unsigned int x;
   unsigned int y;
   unsigned int time;
} touch_info_type;



typedef enum gesture_e{
    ROBIN_NO_GESTURE  = 0,
    ROBIN_GESTURE_MOVE_RIGHT,
    ROBIN_GESTURE_MOVE_LEFT,
    ROBIN_GESTURE_MOVE_UP,
    ROBIN_GESTURE_MOVE_DOWN,
    ROBIN_GESTURE_MOVE_ENTER,
    ROBIN_GESTURE_MOVE_HOME,
    ROBIN_GESTURE_MOVE_BACK,
    
    /*rub a point*/
    ROBIN_GESTURE_RUB_RIGHT_1,
    ROBIN_GESTURE_RUB_RIGHT_2,
    ROBIN_GESTURE_RUB_RIGHT_3,
    ROBIN_GESTURE_RUB_RIGHT_4,
    ROBIN_GESTURE_RUB_RIGHT_5,
    ROBIN_GESTURE_RUB_RIGHT_6,
    ROBIN_GESTURE_RUB_RIGHT_7,
    ROBIN_GESTURE_RUB_RIGHT_8,

    
    ROBIN_GESTURE_RUB_LEFT_1,
    ROBIN_GESTURE_RUB_LEFT_2,
    ROBIN_GESTURE_RUB_LEFT_3,
    ROBIN_GESTURE_RUB_LEFT_4,
    ROBIN_GESTURE_RUB_LEFT_5,
    ROBIN_GESTURE_RUB_LEFT_6,
    ROBIN_GESTURE_RUB_LEFT_7,
    ROBIN_GESTURE_RUB_LEFT_8,


    ROBIN_GESTURE_RUB_UP_1,
    ROBIN_GESTURE_RUB_UP_2,
    ROBIN_GESTURE_RUB_UP_3,
    ROBIN_GESTURE_RUB_UP_4,
    ROBIN_GESTURE_RUB_UP_5,
    ROBIN_GESTURE_RUB_UP_6,
    ROBIN_GESTURE_RUB_UP_7,
    ROBIN_GESTURE_RUB_UP_8,
    

    ROBIN_GESTURE_RUB_DOWN_1,
    ROBIN_GESTURE_RUB_DOWN_2,
    ROBIN_GESTURE_RUB_DOWN_3,
    ROBIN_GESTURE_RUB_DOWN_4,
    ROBIN_GESTURE_RUB_DOWN_5,
    ROBIN_GESTURE_RUB_DOWN_6,
    ROBIN_GESTURE_RUB_DOWN_7,
    ROBIN_GESTURE_RUB_DOWN_8,

}gesture_t;




///////////////////////////////////////////////////////////////////////////////////////////////////////
//For Touch Input event
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include <linux/input.h>   
#include <linux/uinput.h>   
#include <sys/types.h>   
#include <sys/stat.h>   
#include <fcntl.h>   
#include <unistd.h>   
#include <string.h>   
#include <stdint.h>   
#include <stdio.h>   
//#include <input.h>

#include "cutils/log.h"


#define  MAX_POINT_SIZE  3000
#define MAX_GESTURE_BUF_SIZE 100
static  struct touch_info_t last_touch_info;



#define GES_BUFFER_SIZE 200

struct ges_filter_t{
    unsigned int pressed;
	unsigned int press_down_time;
	unsigned int ges_num;
	unsigned int ges_v[GES_BUFFER_SIZE];
	gesture_t ges_buffer[GES_BUFFER_SIZE];
} ges_filter;

#define FILTER_TIME 300 //300

//去抖动
//300ms以内永远只有一个方向动作
#define SAVE_AND_FILTER_GES(g1,t,v,gout)  do{\
	unsigned long long dt; \
	if(ges_filter.ges_num < GES_BUFFER_SIZE)\
	{\
		ges_filter.ges_buffer[ges_filter.ges_num] = g1;\
		ges_filter.ges_v[ges_filter.ges_num]      = v;\
		ges_filter.ges_num ++;\
	}\
    dt = (t) > ges_filter.press_down_time ? ((t) - ges_filter.press_down_time): ((t) + 0x10000 - ges_filter.press_down_time);\  
    gout = g1;\
    if(dt < FILTER_TIME && ges_filter.ges_num >= 2 && v > 800){\
		if(ges_filter.ges_buffer[0] == ges_filter.ges_buffer[1])\
			if(((gout == ROBIN_GESTURE_MOVE_RIGHT || gout == ROBIN_GESTURE_MOVE_LEFT) && \
					(ges_filter.ges_buffer[0] == ROBIN_GESTURE_MOVE_UP || ges_filter.ges_buffer[0] == ROBIN_GESTURE_MOVE_DOWN)) ||  \
				((gout == ROBIN_GESTURE_MOVE_UP || gout == ROBIN_GESTURE_MOVE_DOWN) && \
					(ges_filter.ges_buffer[0] == ROBIN_GESTURE_MOVE_RIGHT || ges_filter.ges_buffer[0] == ROBIN_GESTURE_MOVE_LEFT))) \
						(gout) = ges_filter.ges_buffer[0];\
    }\
	}while(0)
	//防溢出
//#define LOG_TAG "KEY_TEST"

static  int input_fd=-1; 

static int reportkey(int fd, uint16_t type, uint16_t keycode, int32_t value)  
{  
    int ret;  
  
    struct input_event ev;  
  
    memset(&ev, 0, sizeof(struct input_event));  
  
    ev.type = type;  
    ev.code = keycode;  
    ev.value = value;  
  
    ret = write(fd, &ev, sizeof(struct input_event));  
    if (ret < 0) {  
        ALOGE("report key error!\n");  
        return ret;  
    }  
  
/*  printf("key is %d %d\n", keycode, value);*/  
  
    return 0;  
}  
  
static int add(int a,int b)
{
  return (a+b);
}

static int write_event(int fd, int type, int code, int value)
{
    struct input_event event;
 
    memset(&event, 0, sizeof(event));
    event.type = type;
    event.code = code;
    event.value = value;
   // printf("sunming:%s,%s",__FUNCTION__,__LINE__);
    if(write(fd, &event, sizeof(event)) < sizeof(event)) {
        ALOGE("write event failed[%d]: %s", errno, strerror(errno));
        return -1;
    }
    return 0;
}


static  int mt_tracking_id = -1;
#define ABS_MT_PRESSURE_VALUE  1 //0x86
#define ABS_MT_TOUCH_MAJOR_VALUE 0x0f

static int EVT_touch(enum touch_act_t action, int  x, int y)
{
int ret=-1;
char buff[2];
	buff[0]=x;
	buff[1]=y;

	if(input_fd == -1){
		ALOGD("NO INPUT EVENT FD !!! ");
		return -1;
	}

    switch(action)
    {
    case TOUCH_ACTION_DOWN:
     	//write_event(input_fd, EV_MSC, MSC_SCAN, 0x900001);
	// write_event(input_fd, EV_KEY, BTN_TOUCH, 1);
		//write_event(input_fd, EV_ABS, ABS_MT_TRACKING_ID, ++mt_tracking_id);
				 write_event(input_fd, EV_KEY, BTN_TOOL_PEN, 1);	
        write_event(input_fd, EV_ABS, ABS_X, x);
        write_event(input_fd, EV_ABS, ABS_Y, y);

	// write_event(input_fd, EV_ABS, ABS_PRESSURE,ABS_MT_PRESSURE_VALUE );
        write_event(input_fd, EV_SYN, 0, 0);
		//ALOGW("NO INPUT EVENT x:%d,y:%d !!! ",x,y);
        break;
    case TOUCH_ACTION_UP:

		
   //     write_event(input_fd, EV_KEY, BTN_TOUCH, 0);
   	 write_event(input_fd, EV_KEY, BTN_TOOL_PEN, 0);
        write_event(input_fd, EV_ABS, ABS_X, 0);
        write_event(input_fd, EV_ABS, ABS_Y, 0);	
		



        write_event(input_fd, EV_SYN, 0, 0);

        break;
    case TOUCH_ACTION_PRESSED:
		//write_event(input_fd, EV_ABS, ABS_MT_TRACKING_ID, ++mt_tracking_id);
        write_event(input_fd, EV_ABS, ABS_X, x);
        write_event(input_fd, EV_ABS, ABS_Y, y);
		//write_event(input_fd, EV_ABS, ABS_MT_TOUCH_MAJOR,ABS_MT_TOUCH_MAJOR_VALUE );
		
	//	write_event(input_fd, EV_ABS, ABS_PRESSURE,ABS_MT_PRESSURE_VALUE );
	//	write_event(input_fd, EV_ABS, ABS_MT_WIDTH_MAJOR, 1);
	 write_event(input_fd, EV_KEY, BTN_TOOL_PEN, 1);
		
        write_event(input_fd, EV_SYN, 0, 0);

        break;
    }

  //  write_point(buff);

    return 0;
}

static int EVT_touch3(enum touch_act_t action, int  startx, int starty,int endx,int endy,int dt)
{
	int ret=-1;
	char buff[5];
	buff[0]=endx;
	buff[1]=endy;
	//buff[2]=endx;
	//buff[3]=endy;
	buff[2]=dt;
		ALOGW(" INPUT EVENT FD start:%d,%d end:%d,%d !!! ",startx,starty,endx,endy);
    write_point(buff);

    return 0;
}


static int EVT_touch2(enum touch_act_t action, int  x, int y)
{

	if(input_fd == -1){
		ALOGD("NO INPUT EVENT FD !!! ");
		return -1;
	}

    switch(action)
    {
    case TOUCH_ACTION_DOWN:
       
	 write_event(input_fd, EV_KEY, BTN_MOUSE, 1);
		//write_event(input_fd, EV_ABS, ABS_MT_TRACKING_ID, ++mt_tracking_id);
      //  write_event(input_fd, EV_REL, REL_X, x);
       // write_event(input_fd, EV_REL, REL_Y, y);
		//write_event(input_fd, EV_ABS, ABS_MT_PRESSURE,ABS_MT_PRESSURE_VALUE );
        write_event(input_fd, EV_SYN, 0, 0);


        break;
    case TOUCH_ACTION_UP:
        write_event(input_fd, EV_KEY, BTN_MOUSE, 0);
    	//write_event(input_fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
        write_event(input_fd, EV_SYN, 0, 0);

        break;
    case TOUCH_ACTION_PRESSED:
		//write_event(input_fd, EV_ABS, ABS_MT_TRACKING_ID, ++mt_tracking_id);
        write_event(input_fd, EV_REL, REL_X, x);
        write_event(input_fd, EV_REL, REL_Y, y);
		//write_event(input_fd, EV_ABS, ABS_MT_TOUCH_MAJOR,ABS_MT_TOUCH_MAJOR_VALUE );
		
		//write_event(input_fd, EV_ABS, ABS_MT_PRESSURE,ABS_MT_PRESSURE_VALUE );
	//	write_event(input_fd, EV_ABS, ABS_MT_WIDTH_MAJOR, 1);
        write_event(input_fd, EV_SYN, 0, 0);

    ALOGW("[SM]:TOUCH_ACTION_PRESSED x:%d,y:%d",x,y);

        break;
    }
    return 0;
}



static int sendKey(int keycode,int keyValue)
{

   reportkey(input_fd, EV_KEY, keycode, 1);  
   reportkey(input_fd, EV_SYN, SYN_REPORT, 0);
   reportkey(input_fd, EV_KEY, keycode, 0);  
   reportkey(input_fd, EV_SYN, SYN_REPORT, 0);
   return 1;
}

/*
static int exit(void)
{
    ioctl(input_fd, UI_DEV_DESTROY);  
  
    close(input_fd); 
	input_fd = -1;
    return 1; 
} 
*/ 

#define SCREEN_X      1919// 400// 1920
#define SCREEN_Y       1079//400// 1080

#define TOUCH_MAX_X      400
#define TOUCH_MAX_Y      400


static void calc_xy(unsigned int x,unsigned int y,unsigned int *abs_x,unsigned int *abs_y)
{
	x = x > TOUCH_MAX_X ? TOUCH_MAX_X:x;
	y = y > TOUCH_MAX_Y ? TOUCH_MAX_Y:y;


	*abs_x = x;// * SCREEN_X / TOUCH_MAX_X;
	*abs_y = y;// * SCREEN_Y / TOUCH_MAX_Y;
}

static void calc_xy2(unsigned int x,unsigned int y, int *abs_x, int *abs_y)
{


	*abs_x = x-last_touch_info.x;
	*abs_y = y-last_touch_info.y;

		*abs_x = *abs_x * SCREEN_X / TOUCH_MAX_X;
	*abs_y = *abs_y * SCREEN_Y / TOUCH_MAX_Y;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//For Touch Input event END
///////////////////////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////////
//FIFO Queue
//////////////////////////////////////////////////////////////////////////////////////
#define MAX_FIFO_SIZE_NUM      100 //fifo队列的大小
#define QueueFull      0  //fifo满置0
#define QueueEmpty     1  //FIFO空置1
#define QueueOperateOk 2  //队列操作完成 赋值为2

typedef enum queue_state_e{
    QUEUE_FULL,
	QUEUE_EMPTY,
	QUEUE_SUCCESS
}queue_state_t;

typedef struct gesture_buf_s{
	gesture_t    ges;
	unsigned int velocity;
	unsigned int delay_ms;
} gesture_buf_t;

struct FifoQueue
{
    unsigned int front;     //队列头
    unsigned int rear;        //队列尾
    unsigned int count;       //队列计数
    gesture_buf_t dat[MAX_FIFO_SIZE_NUM];
};

#ifdef GESFILTER
#define MAX_gesFIFO_SIZE_NUM      20 //fifo队列的大小
struct gesfilterQ
{
    unsigned int front;     //队列头
    unsigned int rear;        //队列尾
    unsigned int count;       //队列计数
    gesture_buf_t dat[MAX_gesFIFO_SIZE_NUM];
	
};
#endif



static struct FifoQueue ges_queue_fifo;
static int queue_init_flg = 0;

//Queue Init
static void QueueInit(struct FifoQueue *Queue)
{
    Queue->rear=0;
    Queue->front = Queue->rear;//初始化时队列头队列首相连
    Queue->count = 0;   //队列计数为0
}
// Queue In
static queue_state_t  QueueIn(struct FifoQueue *Queue,gesture_buf_t *sdat) //数据进入队列
{  
    struct timeval tv;  
    int difftime;
    if((Queue->front == Queue->rear) && (Queue->count >= MAX_FIFO_SIZE_NUM))
    {                    // full //判断如果队列满了
        QueueInit(Queue);
        return QUEUE_FULL;    //返回队列满的标志
    }                    // in


 //   system("echo hello >>/tmp/fifo.txt ");
  //  system("cat /proc/uptime >>/tmp/fifo.txt ");
 


 #if 1
    gettimeofday(&tv,0);
    if(tv.tv_sec>t_last.tv_sec)
		difftime=(tv.tv_sec-t_last.tv_sec)*1000;
    else difftime=0;
    if(tv.tv_usec>=t_last.tv_usec)
    	{
		difftime=difftime+(tv.tv_usec-t_last.tv_usec)/1000;
	}
    else
    	{
		difftime=difftime-(t_last.tv_usec-tv.tv_usec)/1000;
	}

   ALOGE("time %u:%u,difftime:%u\n",tv.tv_sec,tv.tv_usec,difftime);
    t_last =tv;

	if(difftime<QINTERTIME)
	{
		if(sdat->ges==gestype)
		{
			gesnum++;
			gestype=sdat->ges;
		}
		else
		{
			if(gesnum>GESMINCOUNT)
			{
			   	ALOGE("filter one valid gesture :%d\n",sdat->ges);
				//Queue->dat[Queue->front].rear=ROBIN_NO_GESTURE;
				 return QUEUE_SUCCESS;

			}

		}

	}
	else
	{
		gesnum=0;
		gestype=sdat->ges;
	}
	#endif

    memcpy((char *)&Queue->dat[Queue->rear],(char *)sdat,sizeof(touch_info_type));
    Queue->rear = (Queue->rear + 1) % MAX_FIFO_SIZE_NUM;
    Queue->count = Queue->count + 1;
    ALOGE("[SM]:QueueIn count %d,GES:%d",Queue->count,sdat->ges);	
	
    return QUEUE_SUCCESS;
}
// Queue Out
static queue_state_t QueueOut(struct FifoQueue *Queue,gesture_buf_t *sdat)
{
    if((Queue->front == Queue->rear) ||(Queue->count == 0))
    {                    // empty
        QueueInit(Queue);
        return QUEUE_EMPTY;
    }                   // out
        ALOGW("[SM]:QueueOut count %d",Queue->count);

    *sdat = Queue->dat[Queue->front];
    Queue->front = (Queue->front + 1) % MAX_FIFO_SIZE_NUM;
    Queue->count = Queue->count - 1;
    return QUEUE_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////////
//FIFO Queue END
//////////////////////////////////////////////////////////////////////////////////////
















const char *bus_str(int bus);
const char *bus_str(int bus)
{
 switch (bus) {
 case BUS_USB:
  return "USB";
  break;
 case BUS_HIL:
  return "HIL";
  break;
 case BUS_BLUETOOTH:
  return "Bluetooth";
  break;
 case BUS_VIRTUAL:
  return "Virtual";
  break;
 default:
  return "Other";
  break;
 }
}

#define MAX_TOUCH_BUF_SIZE   64
#define MIN_VALID_X          1023
#define MIN_VALID_Y          1023

static struct touch_info_t s_touch_info_buf[MAX_TOUCH_BUF_SIZE];
static int touch_info_buf_size      = 0;
#define MIN_VALID_DOTS_NUM              3


static void mov_clear_touch_buf(struct touch_info_t *buf,int size ,int step)
{
}

static int process_bt_touch_action(struct touch_info_t *touch_action)
{
   int x_mov[MAX_TOUCH_BUF_SIZE ],y_mov[MAX_TOUCH_BUF_SIZE ],t_dif[MAX_TOUCH_BUF_SIZE];
   int i,t;

	if(touch_action->x > MIN_VALID_X || touch_action->y >  MIN_VALID_Y){
		return -1;
	}

	//if()
		
	if(touch_info_buf_size >= MAX_TOUCH_BUF_SIZE){
		mov_clear_touch_buf(s_touch_info_buf,touch_info_buf_size,1);
		touch_info_buf_size -= 1;
	}

    
	if(touch_info_buf_size > 0 ){
		t = touch_action->time - s_touch_info_buf[touch_info_buf_size - 1].time;
		if(t < 0)
			t += 0xFFFF;
		
		//if()
	}
		
	memcpy(&s_touch_info_buf[touch_info_buf_size],(char *)touch_action,sizeof(struct touch_info_t));
	touch_info_buf_size++;

    if(touch_info_buf_size < MIN_VALID_DOTS_NUM){
		return -1;
    }

	for(i=0;i < MIN_VALID_DOTS_NUM - 1;i++){
		x_mov[i] = s_touch_info_buf[i+1].x - s_touch_info_buf[i].x;
		y_mov[i] = s_touch_info_buf[i+1].y - s_touch_info_buf[i].y;
		t_dif[i] = s_touch_info_buf[i+1].time- s_touch_info_buf[i].time;
		if(t_dif[i] < 0)
			t_dif[i] += 0xFFFF;
	}


	
}






static struct touch_info_t point_arry[MAX_POINT_SIZE];
static int point_idx_read = 0;
static int point_num = 0;
static gesture_t gesture_buf[MAX_GESTURE_BUF_SIZE];
static int gesture_num = 0;
static int repeat_num=0;
static gesture_t repeat_gesture=ROBIN_NO_GESTURE;
static int repeat_gesture_num=0;


#define REPEAT_MIN  4
#define REPEAT_MAX 18

#define MAX_X_SIZE 400
#define MAX_Y_SIZE 400

static int add_point(struct touch_info_t *tp)
{
	int i;
    ALOGD("[SM]:current point %d",point_num);
    if(point_num>1)
    	{
		if ((point_arry[point_num-1].x==tp->x)&&(point_arry[point_num-1].y==tp->y)&&(point_arry[point_num-1].time<tp->time)&&(tp->action==TOUCH_ACTION_PRESSED))
			{
				repeat_num++;
    				ALOGW("[SM]:repeat point %d",repeat_num);
			//return -1;
			}
		else 
				repeat_num=0;  
		if((tp->action==TOUCH_ACTION_UP)||(tp->action==TOUCH_ACTION_DOWN))
			{
				repeat_num=0;
				//point_arry[point_num-1].action=TOUCH_ACTION_UP;
    				//ALOGW("[SM]:repeat point up %d",point_num);
		
			//return 0;
			}

    	}
    if(point_num >= MAX_POINT_SIZE){		     
		// system("cat /proc/uptime");
		/*discard the earliest point,and move left all the points in the arry. */
		for(i=0;i<MAX_POINT_SIZE - 1;i++){
		   point_arry[i].action =  point_arry[i+1].action;
		   point_arry[i].time   =  point_arry[i+1].time;
		   point_arry[i].x      =  point_arry[i+1].x;
		   point_arry[i].y      =  point_arry[i+1].y;
		}
		point_num = MAX_POINT_SIZE -1;	

    }

    //filt the wrong data
    if(tp->x > MAX_X_SIZE || tp->y > MAX_Y_SIZE){
		if(tp->action != TOUCH_ACTION_UP)
			return -1;

		tp->x = point_arry[point_num -1].x;
		tp->y = point_arry[point_num -1].y;
    }


   //memcpy((char *)&point_arry[ppint_num],(char *)tp,sizeof(struct touch_info_t));	
   point_arry[point_num].action = tp->action;
   point_arry[point_num].time   = tp->time;
   point_arry[point_num].x      = tp->x;
   point_arry[point_num].y      = tp->y;

 
   #if 0
      if(point_num>=4)
      	{
		int k1=0;
		int k2=0;
		if((point_arry[point_num-2].y>(point_arry[point_num-4].y+0))&&(point_arry[point_num].y<(point_arry[point_num-2].y)))
			{
				k1=1;
			}
		else if((point_arry[point_num-2].y<(point_arry[point_num-4].y-0))&&(point_arry[point_num].y>(point_arry[point_num-2].y)))
			{
				k1=2;
			}
		if((point_arry[point_num-2].x>(point_arry[point_num-4].x+0))&&(point_arry[point_num].x<(point_arry[point_num-2].x)))
			{
				k2=1;
			}
		else if((point_arry[point_num-2].x<(point_arry[point_num-4].x-0))&&(point_arry[point_num].x>(point_arry[point_num-2].x+0)))
			{
				k2=2;
			}	
		if(k1>0)
    			ALOGW("sm:y guaidian =%d !! ",k1);
		if(k2>0)
    			ALOGW("sm:x guaidian =%d !! ",k2);
      	}
   #endif
  point_num ++;
   return 0;
}

 int clear_points(void /*int start_idx*/)
{
#if 0
    int i,num;

	num = point_num - start_idx;
		
    if(start_idx >= point_num-1)
        point_num = 0;
	else{

		for(i=0;i<num;i++){
		   point_arry[i].action =  point_arry[i+start_idx].action;
		   point_arry[i].time   =  point_arry[i+start_idx].time;
		   point_arry[i].x      =  point_arry[i+start_idx].x;
		   point_arry[i].y      =  point_arry[i+start_idx].y;
		}

		point_num = num;
	}
#else
#if 0
    ALOGD("haha clear point !! ");
    int i,num;
    char str[200];
   for(i=0;i<point_num;i++){
		   memset(str, 0, sizeof(str));
		   sprintf(str,"echo %d %d %d %d >>/tmp/poit_data.txt",(int)point_arry[i].action,point_arry[i].time,point_arry[i].x, point_arry[i].y);
		   ALOGD(str);
		   system(str);
		}
   #endif
      point_idx_read=0;
     point_num = 0;
#endif 

    return 0;
}



typedef struct 
{
double x;
double y;
} Points;

static int CalRadius(Points pt[3],double *radius_out)
{
	double slope_r = 0.0, slope_t = 0.0;
	double centre_x = 0.0, centre_y = 0.0;
	double radius = 0.0;
	slope_r = (pt[0].y - pt[1].y) / (pt[0].x - pt[1].x);
	slope_t = (pt[1].y - pt[2].y) / (pt[1].x - pt[2].x);

	centre_x = (slope_r * slope_t * (pt[2].y - pt[0].y) + slope_r * (pt[1].x + pt[2].x) - slope_t * (pt[0].x + pt[1].x)) / (2.0 * (slope_r - slope_t));
	centre_y =  -(1.0 / slope_r)*(centre_x - (pt[0].x + pt[1].x)/2.0) + (pt[0].y + pt[1].y)/2.0;

	radius = sqrt((pt[0].x - centre_x) * (pt[0].x - centre_x) + (pt[0].y - centre_y) * (pt[0].y - centre_y));
	ALOGD(" (x, y) = (%f %f), r=%f\n", centre_x, centre_y, radius);

    *radius_out = radius;
	return 0;

} 


#define TOUCH_MOV_LIMIT 95//90// 130 //70
#define TOUCH_MOV_UP_DOWN_LIMIT 95//90//100//140 //100 //100
#define TOUCH_SINGLE_SLIP_LIMIT 50// 70
#define TOUCH_SINGLE_SLIP_SECENDADD 25// 70

/*define the angle, tan (angle) = x/y  */

/*angle = 45 degrees*/
#define QUADRANT_FIRST_X_LIMIT    100
#define QUADRANT_FIRST_Y_LIMIT    120

/*angle = 30 degrees, tan(angle) = x/y = 1/1.73*/
#define QUADRANT_SECOND_X_LIMIT   100
#define QUADRANT_SECOND_Y_LIMIT   120  //173

/*angle = 45 degrees*/
#define QUADRANT_THIRD_X_LIMIT    100
#define QUADRANT_THIRD_Y_LIMIT    120

/*angle = 30 degrees*/
#define QUADRANT_FOURTH_X_LIMIT   100
#define QUADRANT_FOURTH_Y_LIMIT   120 //173





static int find_press_down_point_idx(struct touch_info_t *points,int current_idx)
{
	int i;
    int press_down_idx = -1;
	
	for(i = current_idx;i >= 0;i --)
	{
		if(points[i].action == TOUCH_ACTION_DOWN){
			press_down_idx = i;
			break;
		}
	}

	return press_down_idx;
}


#define POINTS_TIMEOUT_MS  150

static int find_timeout_point_idx(struct touch_info_t *points,int current_idx)
{
	int i;
	unsigned long long t;
	unsigned int current_time;
    int timeout_idx = -1;

	current_time = points[current_idx].time;
	
	for(i = current_idx;i > 0;i --)
	{
	    if (current_time > points[i-1].time)
			t = current_time - points[i-1].time;
		else
			t = (current_time + 0x10000)- points[i-1].time;

		if(t > POINTS_TIMEOUT_MS){
			timeout_idx = i;
			break;
		}
	}

	return timeout_idx;
}


#define MIX_SMOOTH_POINTS_NUM 5
#define REPEATS_SMOOTH 2

static int smooth_points_trail(struct touch_info_t *points,int num)
{
    int i,j;
	int Xn,Yn;

	if(num < MIX_SMOOTH_POINTS_NUM)
		return -1;

    for(j = 0;j < REPEATS_SMOOTH;j++)
    {
		for(i = 0;i < num -1;i++)
		{
			Xn = (points[i].x + points[i+1].x) >> 1;
			Yn = (points[i].y + points[i+1].y) >> 1;

			points[i].x = Xn;
			points[i].y = Yn;
		}

		points[num -1].x = points[num -2].x * 2 - points[num -3].x;
		points[num -1].y = points[num -2].y * 2 - points[num -3].y;
    }
	
    return 0;
}


static int pre_precess_points(struct touch_info_t *points,int current_idx)
{
	int press_down_idx,time_out_idx,unit_start_idx;

	 
   press_down_idx = find_press_down_point_idx(points,current_idx);
   if(press_down_idx == -1)
   	press_down_idx = 0;

   time_out_idx = find_timeout_point_idx(points,current_idx);
   if(time_out_idx == -1)
   	time_out_idx = 0;

   unit_start_idx = press_down_idx > time_out_idx ? press_down_idx:time_out_idx;

//   smooth_points_trail(&points[unit_start_idx],current_idx - unit_start_idx);

   return 0;
}


#define Ges_min_time 50//50
#define DIS_TIME 240*150
static int is_it_enough(int start_idx,int end_idx)   //判断滑动的直线距离够不够90dots
{
   int start_x,start_y,end_x,end_y,dis_x,dis_y,idx;
   unsigned long long dis_squal;
   unsigned int dis_min_limit;
   unsigned int dis_real;
   unsigned int dis_time;
   unsigned long long dt;
   
   struct touch_info_t points[MAX_POINT_SIZE];

   memcpy((char *)points,(char *)point_arry,sizeof(struct touch_info_t) * MAX_POINT_SIZE);
  
   pre_precess_points(points,end_idx); //对轨迹做平滑处理，去抖动

   

   //计算直线距离

   start_x = points[start_idx].x;
   start_y = points[start_idx].y;
   end_x = points[end_idx].x;
   end_y = points[end_idx].y;

   dis_x = end_x > start_x ? (end_x - start_x):(start_x - end_x);
   dis_y = end_y > start_y ? (end_y - start_y):(start_y - end_y);
   
   dis_squal = dis_x * dis_x + dis_y * dis_y;

   //if y > 1.5 x,this gestrue must be the mov up or mov down.
  #if 1
   if(dis_x * 12 /14 < dis_y)  //think to be updown
   	{
   		if(repeat_gesture_num==0)
   			dis_min_limit = TOUCH_MOV_UP_DOWN_LIMIT * TOUCH_MOV_UP_DOWN_LIMIT;
		else
   			dis_min_limit = (TOUCH_MOV_UP_DOWN_LIMIT+TOUCH_SINGLE_SLIP_SECENDADD) * (TOUCH_MOV_UP_DOWN_LIMIT+TOUCH_SINGLE_SLIP_SECENDADD);
   			//dis_min_limit = (TOUCH_MOV_UP_DOWN_LIMIT) * (TOUCH_MOV_UP_DOWN_LIMIT);
   }
   else
   	{
   		if(repeat_gesture_num==0)
			dis_min_limit = TOUCH_MOV_LIMIT * TOUCH_MOV_LIMIT;
		else
			dis_min_limit = (TOUCH_MOV_LIMIT+TOUCH_SINGLE_SLIP_SECENDADD) * (TOUCH_MOV_LIMIT+TOUCH_SINGLE_SLIP_SECENDADD);
   	}
#else
   if(dis_x * 10 /14 < dis_y)  //think to be updown
   	{
   			dis_min_limit = TOUCH_MOV_UP_DOWN_LIMIT * TOUCH_MOV_UP_DOWN_LIMIT;
   	}
   else
   	{

			dis_min_limit = TOUCH_MOV_LIMIT * TOUCH_MOV_LIMIT;
   	}
   #endif

	//判断是否达到limit
   if(dis_squal < dis_min_limit) 
        return 0;

 /*
   	//判断是否达到30ms
   if(points[end_idx].time > points[start_idx].time) 
   		dis_time = points[end_idx].time - points[start_idx].time;
   else
   		dis_time = (unsigned int)((unsigned long long)(points[end_idx].time + 0x10000 - points[start_idx].time));
   if(dis_time < Ges_min_time) 
        return 0;
        */
/*
	//判断时间与距离的乘积
   dis_real = (unsigned int)(sqrt((double) dis_squal) ); 	
   dt=dis_time*dis_real;
   if(dt < DIS_TIME) 
   	{
   	   ALOGW("not enough contitions: dis_time:%d,dis_real :%d,dt : %lld",dis_time,dis_real,dt);
          return 0;
   	}

   ALOGD("get one long gesture !!! %lld ",dis_squal);
   	*/
   return 1;
}

typedef enum quadrant_e{
    QUADRANT_POS_X_AXIS,
    QUADRANT_FIRST,
    QUADRANT_POS_Y_AXIS,
    QUADRANT_SECOND,
    QUADRANT_NEG_X_AXIS,
    QUADRANT_THIRD,
    QUADRANT_NEG_Y_AXIS,
    QUADRANT_FOURTH,
    QUADRANT_ZERO
}quadran_t;

#include <math.h>
static int parse_a_gesture(int start_idx,int end_idx,gesture_t *g,unsigned *v)
{
   int start_x,start_y,end_x,end_y,dis_x,dis_y;
   unsigned long long dis_squal;
   unsigned int dis_real;
   int abs_x,abs_y;
   unsigned int time;
   quadran_t quadrant = 0;
   gesture_t ges = ROBIN_NO_GESTURE;

   struct touch_info_t points[MAX_POINT_SIZE];

   memcpy((char *)points,(char *)point_arry,sizeof(struct touch_info_t) * MAX_POINT_SIZE);

  //预处理
   pre_precess_points(points,end_idx);

//获取时间差
//溢出处理
   if(points[end_idx].time > points[start_idx].time) 
   		time = points[end_idx].time - points[start_idx].time;
   else
   		time = (unsigned int)((unsigned long long)(points[end_idx].time + 0x10000 - points[start_idx].time));
   
   start_x = points[start_idx].x;
   start_y = points[start_idx].y;
   end_x = points[end_idx].x;
   end_y = points[end_idx].y;

   dis_x = end_x - start_x;
   dis_y = end_y - start_y;

   if(dis_x > 0 && dis_y > 0)
		quadrant = QUADRANT_FIRST;
   else if(dis_x < 0 && dis_y > 0)
        quadrant = QUADRANT_SECOND;
   else if(dis_x < 0 && dis_y < 0)
        quadrant = QUADRANT_THIRD;
   else if(dis_x > 0 && dis_y < 0)
        quadrant = QUADRANT_FOURTH;
   else if(dis_x == 0 && dis_y > 0)
        quadrant = QUADRANT_POS_Y_AXIS;
   else if(dis_x > 0 && dis_y == 0)
        quadrant = QUADRANT_POS_X_AXIS;
   else if(dis_x < 0 && dis_y == 0)
        quadrant = QUADRANT_NEG_X_AXIS;
   else if(dis_x == 0 && dis_y < 0)
        quadrant = QUADRANT_NEG_Y_AXIS;
   else
        quadrant = QUADRANT_ZERO;

   
   abs_x = dis_x > 0 ? dis_x : dis_x * -1;
   abs_y = dis_y > 0 ? dis_y : dis_y * -1;


   switch (quadrant)
   {
      case QUADRANT_FIRST:
           ges = (abs_y * QUADRANT_FIRST_X_LIMIT  > abs_x * QUADRANT_FIRST_Y_LIMIT)? ROBIN_GESTURE_MOVE_UP:ROBIN_GESTURE_MOVE_RIGHT;           
           break;
      case QUADRANT_SECOND:
           ges = (abs_y * QUADRANT_SECOND_X_LIMIT  > abs_x * QUADRANT_SECOND_Y_LIMIT)? ROBIN_GESTURE_MOVE_UP:ROBIN_GESTURE_MOVE_LEFT;           
           break;
      case QUADRANT_THIRD:
           ges = (abs_y * QUADRANT_THIRD_X_LIMIT  > abs_x * QUADRANT_THIRD_Y_LIMIT)? ROBIN_GESTURE_MOVE_DOWN:ROBIN_GESTURE_MOVE_LEFT;           
           break;
      case QUADRANT_FOURTH:
           ges = (abs_y * QUADRANT_FOURTH_X_LIMIT  > abs_x * QUADRANT_FOURTH_Y_LIMIT)? ROBIN_GESTURE_MOVE_DOWN:ROBIN_GESTURE_MOVE_RIGHT;           
           break;
      case QUADRANT_POS_Y_AXIS:
           ges = ROBIN_GESTURE_MOVE_UP;
           break;
      case QUADRANT_POS_X_AXIS:
           ges = ROBIN_GESTURE_MOVE_RIGHT;
           break;
      case QUADRANT_NEG_X_AXIS:
           ges = ROBIN_GESTURE_MOVE_LEFT;
           break;
      case QUADRANT_NEG_Y_AXIS:
           ges = ROBIN_GESTURE_MOVE_DOWN;
           break;
      default:
           break;
   }

   dis_squal = abs_x * abs_x + abs_y * abs_y ;
   /*
   if(ges == ROBIN_GESTURE_MOVE_UP || ges == ROBIN_GESTURE_MOVE_DOWN)
   {
		if(dis_squal < TOUCH_MOV_UP_DOWN_LIMIT * TOUCH_MOV_UP_DOWN_LIMIT)
			ges = ROBIN_NO_GESTURE;
   }
   */

   

   dis_real = (unsigned int)(sqrt((double) dis_squal) * 1000);   //dots per SEC

//手指短滑，直线距离在30到90之间，则认为滑动有效
   if(points[start_idx].action == TOUCH_ACTION_DOWN && points[end_idx].action == TOUCH_ACTION_UP){
   	   if(dis_squal < TOUCH_SINGLE_SLIP_LIMIT * TOUCH_SINGLE_SLIP_LIMIT){
	   	   ges =  ROBIN_NO_GESTURE;
   	   }
   }
   
   *g = ges;
   *v = time != 0 ? (unsigned)(dis_real / time) : 0;
   
   ALOGD("[Robin]:ACTION ges:%d,start_x:%d,start_y:%d,end_x:%d,end_y:%d,dis_real :%d,time :%d v %d",ges,start_x,start_y,end_x,end_y,dis_real,time,*v);
   return 0;
}

static int motion_speed(int start_idx,int end_idx,unsigned *v)
{
   int start_x,start_y,end_x,end_y,dis_x,dis_y;
   unsigned long long dis_squal;
   unsigned int dis_real;
   int abs_x,abs_y;
   unsigned int time;
   quadran_t quadrant = 0;

   struct touch_info_t points[MAX_POINT_SIZE];

   memcpy((char *)points,(char *)point_arry,sizeof(struct touch_info_t) * MAX_POINT_SIZE);

  //预处理
   pre_precess_points(points,end_idx);

//获取时间差
//溢出处理
   if(points[end_idx].time > points[start_idx].time) 
   		time = points[end_idx].time - points[start_idx].time;
   else
   		time = (unsigned int)((unsigned long long)(points[end_idx].time + 0x10000 - points[start_idx].time));
   
   start_x = points[start_idx].x;
   start_y = points[start_idx].y;
   end_x = points[end_idx].x;
   end_y = points[end_idx].y;

   dis_x = end_x - start_x;
   dis_y = end_y - start_y;

   
   abs_x = dis_x > 0 ? dis_x : dis_x * -1;
   abs_y = dis_y > 0 ? dis_y : dis_y * -1;


   dis_squal = abs_x * abs_x + abs_y * abs_y ;

   

   dis_real = (unsigned int)(sqrt((double) dis_squal) * 1000);   //dots per SEC

   *v = time != 0 ? (unsigned)(dis_real / time) : 0;
   
   ALOGW("[SM]:motion_speed v %d",*v);
   return 0;
}

static int save_gesture(gesture_t ges,unsigned int v,unsigned int delay_ms)
{
#if 0
	if(gesture_num >=  MAX_GESTURE_BUF_SIZE - 1)
		return -1; 

	gesture_buf[gesture_num] = ges;
	gesture_num ++;
#endif
	gesture_buf_t g;
    	queue_state_t s;

       g.ges 		= ges;
	g.velocity = v;
	g.delay_ms = delay_ms;
	
    ALOGW("[SM]:ACTION g.ges %d, g.velocity %d",ges,v);
	
	s = QueueIn(&ges_queue_fifo,&g);
    if(s == QUEUE_FULL)
		return -1;

	return 0;
}



#define MOTION_VELOCITY 1900
#define MOTION_NUMBERS 6

#define  DECEL_SPEED (MOTION_VELOCITY * MOTION_VELOCITY /TOUCH_MOV_LIMIT/(2 * MOTION_NUMBERS))   // 1900 dots per sec can run 5 icons.
#define  MOTION_TRIGGER_VELOCITY   2700//4000 //4000
#define  MOTION_TRIGGER_VELOCITY2   4500//5000 //4000
#define  MOTION_TRIGGER_VELOCITY3   6300 //4000

#define  MAX_VELOCITY       10000

#define  TIME_CONSTANT   80//  80//325

static int decel_motion_and_save_points(unsigned *v)
{
   unsigned int tmp,V0,squal_V0,Sl,Sn,Vn,n,max_v,Tn,Tn_1;
   int i;
   double f;
   gesture_t g;
   gesture_buf_t gbuf;
   float newV;
/*
   if(ges_filter.ges_num < 2)
   		return -1;
*/
   if(ges_filter.ges_num==0) 
   	return -1;
   
   g  = ges_filter.ges_buffer[ges_filter.ges_num -1];
   V0 = ges_filter.ges_v[ges_filter.ges_num - 1];
      if(*v < MOTION_TRIGGER_VELOCITY)
   	return -1;


      ALOGW("**decel_motion_and_save_pointss:V0 %d ",*v);
	     V0=*v*7/8;
	  /*
   if(V0<3000)
   	V0=2580;
    if(V0<4500)
   	V0=4800;
   else if(V0<6000)
   	V0=5600;
   else
   	V0=5600;
   	*/
   squal_V0 = V0 * V0;

  // newV=pow(1.1,(V0-4000))+4000;
   //ALOGW("**decel_motion_and_save_pointss:newV %d ",newV);

   for(i=0,max_v = 0;i < ges_filter.ges_num;i++)
   		max_v = ges_filter.ges_v[i] > max_v ? ges_filter.ges_v[i]:max_v;
   


   if(g == ROBIN_GESTURE_MOVE_RIGHT || g == ROBIN_GESTURE_MOVE_LEFT)
	   	Sl = TOUCH_MOV_LIMIT;
   else if(g == ROBIN_GESTURE_MOVE_UP || g == ROBIN_GESTURE_MOVE_DOWN)
   		Sl = TOUCH_MOV_UP_DOWN_LIMIT-30 ;
   else
   		return -1;

#if 0	   
   for(n = 1;;n++)
   {
	   Sn = Sl * n;
	   tmp = 2 * DECEL_SPEED * Sn;
		
	   
	   if(tmp >= squal_V0)
	   		break;
	   
	   //Vn = sqrt(squal(V0) - 2 * a * Sn)
	   Vn = sqrt(squal_V0 - tmp);

	   if(Vn > MAX_VELOCITY)
	   		break;
   	
	   gbuf.velocity = Vn;
	   gbuf.ges      = g;

	   QueueIn(&queue_fifo,&gbuf);
	}
#else
    //Tn = TIME_CONSTANT * ln (V0 * TIME_CONSTANT / (V0 * TIME_CONSTANT - Sn))


    for(n = 1,Tn_1 = 0;;n++)
   {
	   Sn = Sl * n;
	   tmp = V0 * TIME_CONSTANT / 1000;//V0 is Dot/S,t is ms,so it need div 1000
		
	   
	   if(tmp <= Sn)
	   		break;

	  f = (double)(TIME_CONSTANT * log10((double)tmp / (double)(tmp - Sn)))/M_LOG10E;

	  Tn = (unsigned int)f;
	  
	  //Vn = V0 * e^-(t/TIME_CONSTANT)
	   Vn = (double)V0 * exp( -1 * f /TIME_CONSTANT );

	   if(Vn > MAX_VELOCITY)
	   		break;
   	   
   	   
	   gbuf.velocity = Vn;
	   gbuf.ges      = g;
	   gbuf.delay_ms = Tn - Tn_1;

       //the last event,it is no need to delay
	   if(tmp <= (Sl * (n + 1)))
	   	 gbuf.delay_ms = 0;
       
	   QueueIn(&ges_queue_fifo,&gbuf);
	   
	   Tn_1 = Tn;
	}
#endif
   
	return 0;
}

static    struct timeval tg_1,tg_2;  
static int dis1,dis2;

    //    if(uhid_write(rcu_vhid_fd, &ev) < 0)
static int process_points(void)
{
   unsigned int v,v2;
   int i;
   gesture_t ges,g;
   int start_idx = point_idx_read;
   int clear_point_buf_flg = 0;
   int abs_x,abs_y;
  if(point_num>0)
   for(i=point_num-1;i < point_num;i++){
     switch(point_arry[i].action)
     {
     	case TOUCH_ACTION_DOWN :
             start_idx   = i;
			ALOGW("\n\r****************************************************************************\n\r",i);
			 ges_filter.press_down_time = point_arry[i].time;
			 ges_filter.pressed         = 1;
			 ges_filter.ges_num = 0;
			 repeat_gesture_num=0;
			 repeat_gesture=ROBIN_NO_GESTURE;
			 		   system("rm -rf /tmp/poit_data.txt");
	/*calc_xy(point_arry[i].x,point_arry[i].y,&abs_x,&abs_y);

	 
	memcpy(&last_touch_info,&point_arry[i],sizeof( struct touch_info_t));

	if(EVT_touch(TOUCH_ACTION_DOWN,abs_x,abs_y) == -1){
			 		}
*/
	     //     		ALOGW("SET TOUCH_ACTION_DOWN ok !!!");       

             break;
        case TOUCH_ACTION_PRESSED :
		if(0)//point_num==2)
			{
			start_idx=1;
			point_arry[start_idx].action = TOUCH_ACTION_DOWN;
			}
		if(repeat_num>=REPEAT_MIN)
		{
			       start_idx = i;
		}
		if(0)//repeat_num>=REPEAT_MAX)
		{            
			        start_idx = i;
				 point_arry[start_idx].action = TOUCH_ACTION_DOWN;	
			        repeat_num=0;
				  if(repeat_gesture)
				  {
				  	v=200;
	                       	save_gesture(repeat_gesture,v,30);
				  	ALOGW("**get one repeat gesture !!!ges:%d ",repeat_gesture);
				  }
		}
			#if 1
             else if(is_it_enough(start_idx,i)){
                parse_a_gesture(start_idx,i,&ges,&v);
				                                           //速度太大认为是异常
                if(ges != ROBIN_NO_GESTURE && v < MAX_VELOCITY){
			ALOGD("**get one long gesture !!!ges:%d ",ges);

			 SAVE_AND_FILTER_GES(ges,point_arry[i].time,v,g);
			   ALOGD("**SAVE_AND_FILTER_GES:V %d ",v); 
				 if(g!=repeat_gesture)
				 {
				       if(repeat_gesture!=ROBIN_NO_GESTURE)
				       {     
					 //	unsigned long long  dt;
					  //     dt = (point_arry[i].time) > point_arry[start_idx].time ? (point_arry[i].time-point_arry[start_idx].time): (point_arry[i].time+0x10000-point_arry[start_idx].time); 
					//	if(dt<1000)
					//	    break;
				       }
					repeat_gesture_num=0;

				 }

				 if(repeat_gesture_num<2) 
				 {
				 	  if(repeat_gesture_num==0)
				 	  	{
		                         		 save_gesture(g,v,70);
				 	  	}
					  else
		                         		 save_gesture(g,v,1);
					  repeat_gesture=g;
					  repeat_gesture_num++;
					//  point_arry[start_idx].action = TOUCH_ACTION_DOWN;	
		                       start_idx = i;
				 }
				  //clear_point_buf_flg  = 1;
                }
             }

	      	
			 #endif
             break;

        case TOUCH_ACTION_UP :
	// single slip			 	
			 if(point_arry[start_idx].action == TOUCH_ACTION_DOWN){
			 	parse_a_gesture(start_idx,i,&ges,&v);
	    			// if((repeat_num>3)&&(v<40))
				//	 	break;
			ALOGE("get one short gesture !!!%d ",i);

			    if(ges != ROBIN_NO_GESTURE){
				  	SAVE_AND_FILTER_GES(ges,point_arry[i].time,v,g);
					save_gesture(g,v,100);
			    }
			 }		
			// need com a v 
			 motion_speed(0,i,&v);
			// motion_speed(start_idx,i-1,&v2);
			// v=(v+v2)/2;
               	 decel_motion_and_save_points(&v);//减速处理，后续交给候工处理时需要注掉
	               clear_point_buf_flg  = 1;
			 ges_filter.pressed         = 0;
			 repeat_gesture=ROBIN_NO_GESTURE;
			 //start_idx = point_num-1;
			 start_idx = 0;

#ifdef soft_OK
int gtime=0;
int difftime=0;
gtime=point_arry[i].time-point_arry[0].time;
ALOGE("this gesture  total time :%d! ",gtime);

 gettimeofday(&tg_2,0);		
    if(tg_2.tv_sec>tg_1.tv_sec)
		difftime=(tg_2.tv_sec-tg_1.tv_sec)*1000;
    else difftime=0;
    if(tg_2.tv_usec>=tg_1.tv_usec)
    	{
		difftime=difftime+(tg_2.tv_usec-tg_1.tv_usec)/1000;
	}
    else
    	{
		difftime=difftime-(tg_1.tv_usec-tg_2.tv_usec)/1000;
	}
 tg_1=tg_2;
ALOGE("the two ges intervel :%d! ",difftime);


   int start_x,start_y,end_x,end_y,dis_x,dis_y,dis_squal;


   //计算直线距离

   start_x = point_arry[0].x;
   start_y = point_arry[0].y;
   end_x = point_arry[i].x;
   end_y = point_arry[i].y;

   dis_x = end_x > start_x ? (end_x - start_x):(start_x - end_x);
   dis_y = end_y > start_y ? (end_y - start_y):(start_y - end_y);
   
   dis_squal = dis_x * dis_x + dis_y * dis_y;
ALOGE("the two ges distance dis_squal :%d! ",dis_squal);

if((gtime<=140)&&(difftime<=2650)&&(dis_squal<=300)&&(point_arry[i].x>60)&&(point_arry[i].x<320))
{

sendKey(0x1c,0);
    //uhid_write(rcu_vhid_fd, &evOK);
ALOGE("press OK ");
}

#endif
             break;      

        default:
             break;
     }
   }

   point_idx_read = start_idx;

  
   if(clear_point_buf_flg){
   	
	clear_points(/*start_idx*/);
   }

   return 0;
}

static int  get_gesture(gesture_buf_t *ges)
{
     if (QueueOut(&ges_queue_fifo,ges) == QUEUE_EMPTY)
	 	   return -1;

	 return 0;
}

#define MIN_UP_DOWN_TIME 1000 * 20
#define DELAY_TIME 1000 * 150
#define ADJ_DELAY_TIME 1

static unsigned int calc_next_motion_time(unsigned int V0,unsigned int a,unsigned int S)
{
	// Tn = (V0 - sqrt(V0 * V0 - 2 * a * Sn))/a

	double tmp,y;
    unsigned int time_ms,x;

    
	x = 2 * a * S;  
	y = V0 * V0;

	if(y < x)
		return 0;
		
	tmp = V0 - sqrt(y - x);

	time_ms = 1000 * tmp/a; //MS

	return time_ms;
}

#include <time.h>

typedef unsigned long long nsecs_t;  // nano-seconds

enum {
    SYSTEM_TIME_REALTIME = 0,  // system-wide realtime clock
    SYSTEM_TIME_MONOTONIC = 1, // monotonic time since unspecified starting point
    SYSTEM_TIME_PROCESS = 2,   // high-resolution per-process clock
    SYSTEM_TIME_THREAD = 3,    // high-resolution per-thread clock
};

static inline nsecs_t systemTime(int clock)
{
#if defined(HAVE_POSIX_CLOCKS)
    static const clockid_t clocks[] = {
            CLOCK_REALTIME,
            CLOCK_MONOTONIC,
            CLOCK_PROCESS_CPUTIME_ID,
            CLOCK_THREAD_CPUTIME_ID
    };
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(clocks[clock], &t);
    return (nsecs_t)((nsecs_t)(t.tv_sec) *1000000000LL + t.tv_nsec);
#else
    // we don't support the clocks here.
    struct timeval t;
    t.tv_sec = t.tv_usec = 0;
    gettimeofday(&t, NULL);
    return (nsecs_t)(t.tv_sec)*1000000000LL + (nsecs_t)(t.tv_usec)*1000LL;
#endif
}

static int send_movement(gesture_t g,unsigned int v,unsigned int delay_ms)
{
        unsigned int delay_time;

		if(input_fd == -1){
			ALOGD("NO INPUT EVENT FD !!! ");
			return -1;
		}

        if(ROBIN_GESTURE_MOVE_RIGHT == g || ROBIN_GESTURE_MOVE_LEFT == g){
			delay_time =delay_ms* 1000;//(1000 / ADJ_DELAY_TIME) * calc_next_motion_time(v,DECEL_SPEED,TOUCH_MOV_LIMIT);   //time unit is MS
			//if(ges_filter.pressed)
			//	delay_time = 0;// delay_time /2;
        }else if(ROBIN_GESTURE_MOVE_DOWN == g || ROBIN_GESTURE_MOVE_UP == g){
			delay_time = delay_ms * 1000;//(1000 / ADJ_DELAY_TIME) * calc_next_motion_time(v,DECEL_SPEED,TOUCH_MOV_UP_DOWN_LIMIT) ;
			//if(ges_filter.pressed || delay_time < MIN_UP_DOWN_TIME)
			//	delay_time = MIN_UP_DOWN_TIME;
		}else
			delay_time = 0;

		nsecs_t now;
				now = systemTime(SYSTEM_TIME_MONOTONIC);

		//ALOGD("[Robin]ACTION: send_movement >> ges:%d systemTime:%llu next delay_time:%d v:%d",g,now,delay_time,v);
		ALOGD("[sunming]ACTION:movement");

		switch(g)
		{
     		case ROBIN_GESTURE_MOVE_RIGHT:
     			write_event(input_fd, EV_MSC, MSC_SCAN, 0xf0a8);  //getevent 可得到键值
     			write_event(input_fd, EV_KEY, KEY_RIGHT, 1);
     			write_event(input_fd, EV_SYN, 0, 0);
			write_event(input_fd, EV_KEY, KEY_RIGHT, 0);
			write_event(input_fd, EV_SYN, 0, 0);
			usleep(delay_time);
     			break;
     		case ROBIN_GESTURE_MOVE_LEFT:
     			write_event(input_fd, EV_MSC, MSC_SCAN, 0xf0a9);
     			write_event(input_fd, EV_KEY, KEY_LEFT, 1);
     			write_event(input_fd, EV_SYN, 0, 0);
			write_event(input_fd, EV_KEY, KEY_LEFT, 0);
			write_event(input_fd, EV_SYN, 0, 0);
			usleep(delay_time);
     			break;
     		case ROBIN_GESTURE_MOVE_DOWN:
     			write_event(input_fd, EV_MSC, MSC_SCAN, 0xf0a6);
     			write_event(input_fd, EV_KEY, KEY_UP, 1);
     			write_event(input_fd, EV_SYN, 0, 0);
				write_event(input_fd, EV_KEY, KEY_UP, 0);
				write_event(input_fd, EV_SYN, 0, 0);
                usleep(delay_time);
     			break;
     		case ROBIN_GESTURE_MOVE_UP:
				write_event(input_fd, EV_MSC, MSC_SCAN, 0xf0a7);
     			write_event(input_fd, EV_KEY, KEY_DOWN, 1);
     			write_event(input_fd, EV_SYN, 0, 0);
				write_event(input_fd, EV_KEY, KEY_DOWN, 0);
				write_event(input_fd, EV_SYN, 0, 0);
				usleep(delay_time);
     			break;
		    case ROBIN_GESTURE_MOVE_ENTER:
				write_event(input_fd, EV_MSC, MSC_SCAN, 0xf00b);
     			write_event(input_fd, EV_KEY, KEY_ENTER, 1);
     			write_event(input_fd, EV_SYN, 0, 0);
				write_event(input_fd, EV_KEY, KEY_ENTER, 0);
				write_event(input_fd, EV_SYN, 0, 0);
				usleep(delay_time);
     			break;
			case ROBIN_GESTURE_MOVE_HOME:
				write_event(input_fd, EV_MSC, MSC_SCAN, 0xf0f7);
     			write_event(input_fd, EV_KEY, KEY_HOME, 1);
     			write_event(input_fd, EV_SYN, 0, 0);
				write_event(input_fd, EV_KEY, KEY_HOME, 0);
				write_event(input_fd, EV_SYN, 0, 0);
				usleep(delay_time);
     			break;
			case ROBIN_GESTURE_MOVE_BACK:
				write_event(input_fd, EV_MSC, MSC_SCAN, 0xf0f9);
     			write_event(input_fd, EV_KEY, KEY_BACK, 1);
     			write_event(input_fd, EV_SYN, 0, 0);
				write_event(input_fd, EV_KEY, KEY_BACK, 0);
				write_event(input_fd, EV_SYN, 0, 0);
				usleep(delay_time);
     			break;
			default:
				//usleep(1000 * 50);
				break;
		}

	
       
	   return 0;
		
}


void  write_point(char *buff)  
{  
    int fd;  
    char w_buf[10];  
    int nwrite;  
      
    if(fd==-1)  
        if(errno==ENXIO)  
            printf("open error; no reading process\n");  
    /*打开有名管道，并设置为非阻塞*/  
    fd=open(FIFO_SERVER,O_WRONLY|O_NONBLOCK,0);  
     if(fd==-1)  
    {  
        ALOGW("open error");  
        return;     
    } 
    strcpy(w_buf,buff);  
    ALOGW("\n\rw_buff:%s",w_buf);
    /*向管道写入字符串*/  
    if((nwrite=write(fd,w_buf,10))==-1)  
    {  
        if(errno==EAGAIN)  
            ALOGW("The FIFO has not been read yet.Please try later\n");  
	 else
           ALOGW("write to the FIFO error error:%d\n",errno);  
    }  
    else   
        ALOGW("write %s to the FIFO\n",w_buf);  

  // unlink(FIFO);  
}  
  


static int robin_read_hidraw_test( char buf[10])
{
	int fd,bt_flag = 0;
	int i, res, desc_size = 0;
	int ret ;
	struct hidraw_report_descriptor rpt_desc;
	struct hidraw_devinfo info;
	struct touch_info_t touch_action;
	unsigned int abs_x,abs_y,dt, v;
	float f;

    static struct touch_info_t last_bt_point;

	static struct touch_info_t first_bt_point;
	unsigned int dis;
	ALOGD(">>>>>>>>>*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> READ_HIDRAW_TEST >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");

	touch_action.action = (enum touch_act_t)(buf[3] >> 4);
	touch_action.time   =  (buf[6] << 8) | buf[7];
	touch_action.x      = ((buf[3] & 0x0F) << 6 ) | (buf[4]>> 2);
	touch_action.y      = ((buf[4] & 0x03) << 8) | buf[5];

      //给耐薄分析工具使用
    	v = 0;
	dt=0;
	if(touch_action.action != TOUCH_ACTION_DOWN && touch_action.x <= TOUCH_MAX_X && touch_action.y <= TOUCH_MAX_Y &&
		last_bt_point.x  <= TOUCH_MAX_X && last_bt_point.y <= TOUCH_MAX_Y){
		abs_x = touch_action.x > last_bt_point.x ? touch_action.x - last_bt_point.x:last_bt_point.x - touch_action.x;
		abs_y = touch_action.y > last_bt_point.y ? touch_action.y - last_bt_point.y:last_bt_point.y - touch_action.y;
		f = sqrt(abs_x * abs_x + abs_y * abs_y);

		 if(touch_action.time >= last_bt_point.time) 
   			dt = touch_action.time - last_bt_point.time;
   		else
   			dt = (unsigned int)((unsigned long long)(touch_action.time + 0x10000 - last_bt_point.time));
		if(dt!=0)
			v =(unsigned int) ((f * 1000) / dt);
	}
    	memcpy(&last_bt_point,&touch_action,sizeof(struct touch_info_t));
	if(touch_action.action == TOUCH_ACTION_DOWN)
		memcpy(&first_bt_point,&touch_action,sizeof(struct touch_info_t));
	
	abs_x = touch_action.x > first_bt_point.x ? touch_action.x - first_bt_point.x:touch_action.x - first_bt_point.x;
	abs_y = touch_action.y > first_bt_point.y ? touch_action.y - first_bt_point.y:touch_action.y - first_bt_point.y;
	dis = sqrt(abs_x * abs_x + abs_y * abs_y);

	
	ALOGW("[P_info] No:%d,ACTION:%d  X_: %d  Y_: %d  TIME:%d, dt:%d,f:%f, V :%d Dis:%d\n",point_num+1,(unsigned int)touch_action.action,
			touch_action.x,
			touch_action.y,
			touch_action.time,
			dt,
			f,
			v,dis);
	//ALOGD("[POINTS_INFO_TAG]              abs_x:%d,abs_y:%d,f:%3f, dt:%d v:%d\n",abs_x,abs_y,f,dt,v);

#if TOUCH_MODE_TEST_ENABLE	  
	//if(last_touch_info.action == TOUCH_ACTION_UP || last_touch_info.action == TOUCH_ACTION_NOFINGER)
	//	touch_action.action = TOUCH_ACTION_DOWN;
	
	//touch_action.action = TOUCH_ACTION_PRESSED;
    #if TOUCH_MODE
	calc_xy(touch_action.x,touch_action.y,&abs_x,&abs_y);

	 
	memcpy((char *)&last_touch_info,(char *)&touch_action,sizeof( struct touch_info_t));
///*
	if(EVT_touch(touch_action.action,abs_x,abs_y) == -1){
		ALOGD("SET TOUCH EVENT FAILED !!!");
		return -2;
		}
	/*
		if(touch_action.action!=TOUCH_ACTION_DOWN)
			if(EVT_touch3(touch_action.action,last_touch_info.x,last_touch_info.y,touch_action.x,touch_action.y,dt) == -1)
			{
					ALOGD("SET TOUCH EVENT FAILED !!!");
				return -2;
			}
		*/

	return 0;
	#else
	calc_xy2(touch_action.x,touch_action.y,&abs_x,&abs_y);

	 
	memcpy((char *)&last_touch_info,(char *)&touch_action,sizeof( struct touch_info_t));
	if(touch_action.action == TOUCH_ACTION_DOWN)
		{
		 abs_x =0; 
		  abs_y=0;
		}
	if(EVT_touch2(touch_action.action,abs_x,abs_y) == -1){
		ALOGD("SET TOUCH EVENT FAILED !!!");
		return -2;
	}
	#endif

	ALOGW("SEND EVENT > ACT:%d ABS_X:%d ABS_Y:%d,last_touch_info:%d,%d",touch_action.action,abs_x,abs_y,last_touch_info.x,last_touch_info.y);
#endif


#if KEY_MODE_TEST_ENABLE

        gesture_buf_t gesture;

        if(!Touch_Pressed)
	{	 if(add_point(&touch_action)==0){
	        
	        process_points();
			/*
			while((ret = get_gesture(&gesture)) != -1){
				ALOGD("[Robin]:ACTION >> GES %d,V %d",(int)gesture.ges,gesture.velocity);
	       		send_movement(gesture.ges,gesture.velocity,gesture.delay_ms);
			};
			*/
		 }
        	}
	else
		{

	calc_xy2(touch_action.x,touch_action.y,&abs_x,&abs_y);

	 
	memcpy((char *)&last_touch_info,(char *)&touch_action,sizeof( struct touch_info_t));
	if(touch_action.action == TOUCH_ACTION_DOWN)
		{
		 abs_x =0; 
		  abs_y=0;
		}
	if(EVT_touch2(touch_action.action,abs_x,abs_y) == -1){
		ALOGD("SET TOUCH EVENT FAILED !!!");
		return -2;
	}

	}

#endif

	return 0;
	APPL_TRACE_DEBUG("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< READ_HIDRAW_TEST <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}




static void* bt_cmd_thread(void* arg)
{
	int toggle = 0;
       gesture_buf_t gesture;
	int ret ;
	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	//pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);


	while(1) 
	{
		while((ret = get_gesture(&gesture)) != -1){
			ALOGW("[SM]:DO ACTION >> GES %d,V %d,delay time ms:%d",(int)gesture.ges,gesture.velocity,gesture.delay_ms);
       		send_movement(gesture.ges,gesture.velocity,gesture.delay_ms);
		};

		usleep(100);
	}

	return NULL;
}	
 int touch_event_init(void)  
{  
    struct uinput_user_dev uidev;  

    int  ret,i;  

   ALOGE("%s  luckluo2016 ------\n", __FUNCTION__);
  
    input_fd= open("/dev/uinput", O_WRONLY | O_NONBLOCK);  
  
    if (input_fd < 0) { 
         
   		ALOGE("#########open input failed\n");
        return input_fd;  
    }  
  
   ALOGW("ffffinput_fd open input successed:%d\n",input_fd);
 #if TOUCH_MODE_TEST_ENABLE	  

  #if TOUCH_MODE

  ret = ioctl(input_fd, UI_SET_EVBIT, EV_KEY);  
    ret = ioctl(input_fd, UI_SET_EVBIT, EV_SYN); 
	
	ret = ioctl(input_fd, UI_SET_EVBIT, EV_ABS); 
       ret = ioctl(input_fd, UI_SET_KEYBIT, BTN_TOUCH); 
       ret = ioctl(input_fd, UI_SET_KEYBIT, BTN_TOOL_PEN); 
	   
	//	ret = ioctl(input_fd, UI_SET_KEYBIT, BTN_MOUSE); 
	ret = ioctl(input_fd, UI_SET_ABSBIT, ABS_X);
       ret = ioctl(input_fd, UI_SET_ABSBIT, ABS_Y);

	//ret = ioctl(input_fd, UI_SET_RELBIT, REL_X);
    //ret = ioctl(input_fd, UI_SET_RELBIT, REL_Y);	
	//    ret = ioctl(input_fd, UI_SET_ABSBIT, ABS_PRESSURE);
   if(ret<0)
           ALOGW("sm create input error111 n");
   else
           ALOGW("sm create input success 22\n");
	#else
  ret = ioctl(input_fd, UI_SET_EVBIT, EV_KEY);  
	
    ret = ioctl(input_fd, UI_SET_EVBIT, EV_SYN); 
	
	ret = ioctl(input_fd, UI_SET_EVBIT, EV_REL); 

	ret = ioctl(input_fd, UI_SET_KEYBIT, BTN_MOUSE); 
	ret = ioctl(input_fd, UI_SET_RELBIT, REL_X);
    ret = ioctl(input_fd, UI_SET_RELBIT, REL_Y);	
	#endif
   
 #endif
   /*ioctl(input_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(input_fd, UI_SET_EVBIT, EV_SYN);
	ioctl(input_fd, UI_SET_EVBIT, EV_MSC);

	ioctl(input_fd, UI_SET_KEYBIT, KEY_UP);
	ioctl(input_fd, UI_SET_KEYBIT, KEY_DOWN);
	ioctl(input_fd, UI_SET_KEYBIT, KEY_LEFT);
	ioctl(input_fd, UI_SET_KEYBIT, KEY_RIGHT);
    ioctl(input_fd, UI_SET_KEYBIT, KEY_ENTER);
    ioctl(input_fd, UI_SET_KEYBIT, KEY_HOME);
    ioctl(input_fd, UI_SET_KEYBIT, KEY_BACK);
	ioctl(input_fd, UI_SET_MSCBIT, MSC_SCAN);

*/

#if KEY_MODE_TEST_ENABLE
 ioctl(input_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(input_fd, UI_SET_EVBIT, EV_SYN);
	ioctl(input_fd, UI_SET_EVBIT, EV_MSC);



	
	ret = ioctl(input_fd, UI_SET_EVBIT, EV_REL); 

	ret = ioctl(input_fd, UI_SET_KEYBIT, BTN_MOUSE); 
	ret = ioctl(input_fd, UI_SET_RELBIT, REL_X);
    ret = ioctl(input_fd, UI_SET_RELBIT, REL_Y);	

	

	ioctl(input_fd, UI_SET_KEYBIT, KEY_UP);
	ioctl(input_fd, UI_SET_KEYBIT, KEY_DOWN);
	ioctl(input_fd, UI_SET_KEYBIT, KEY_LEFT);
	ioctl(input_fd, UI_SET_KEYBIT, KEY_RIGHT);
    ioctl(input_fd, UI_SET_KEYBIT, KEY_ENTER);
    ioctl(input_fd, UI_SET_KEYBIT, KEY_HOME);
    ioctl(input_fd, UI_SET_KEYBIT, KEY_BACK);
	ioctl(input_fd, UI_SET_MSCBIT, MSC_SCAN);

#endif

	//		write_event(input_fd, EV_KEY, BTN_TOUCH, 1);
		//write_event(input_fd, EV_ABS, ABS_MT_TRACKING_ID, ++mt_tracking_id);
      //  write_event(input_fd, EV_ABS, ABS_X, x);
     //   write_event(input_fd, EV_ABS, ABS_Y, y);
		//write_event(input_fd, EV_ABS, ABS_MT_PRESSURE,ABS_MT_PRESSURE_VALUE );
     //   write_event(input_fd, EV_SYN, 0, 0);
 //   ret = ioctl(input_fd, UI_SET_ABSBIT, ABS_PRESSURE);

	/*
   for(i=0;i<0xff;i++)  
    ret = ioctl(input_fd, UI_SET_KEYBIT,i);  
  */

	
    memset(&uidev, 0, sizeof(struct uinput_user_dev));  
  
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "tcl_bluetooth_input");  
    uidev.id.bustype = BUS_USB;  
    uidev.id.vendor = 0x1234;  
    uidev.id.product = 0xfedc;  
    uidev.id.version = 1;  
    uidev.absmin[ABS_X]=0;
    uidev.absmax[ABS_X]=399;//1919;//400;
    uidev.absmin[ABS_Y]=0;
    uidev.absmax[ABS_Y]=399;//1079;//;//400;

    ret = write(input_fd, &uidev, sizeof(struct uinput_user_dev));  
  
    ret = ioctl(input_fd, UI_DEV_CREATE);  
    if (ret < 0) {  
   		ALOGE("create input failed\n");
        close(input_fd);  
        return ret;  
    }  
   ALOGE("create input successed\n");

    QueueInit(&ges_queue_fifo);

    if(pthread_create(&cmd_thread, NULL, bt_cmd_thread, NULL))
    	{
	   
	}
    return 0;  
}


