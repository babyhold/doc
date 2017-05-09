    #include <stdio.h>  
    #include <pthread.h>  
	#include <sys/prctl.h>
      
    void *mythread1(void)  
    {  
        int i;  
		    char tname[16];  
			int playId=1;
    memset(tname, 0, 16);  
    snprintf(tname, 16, "myboy%u", playId);  
    prctl(PR_SET_NAME, tname);  
        for(i = 0; i < 2; i++)  
        {  
            printf("This is the 1st pthread,created by xiaoqiang!\n");  
            sleep(1);  
        } 
		
while(1)
{            sleep(1); 
		i++;
	}
    }  
      
    void *mythread2(void)  
    {  
        int i;  
				    char tname[16];  
			int playId=2;
    memset(tname, 0, 16);  
    snprintf(tname, 16, "myboy%u", playId);  
    prctl(PR_SET_NAME, tname); 
        for(i = 0; i < 2; i++)  
        {  
            printf("This is the 2st pthread,created by xiaoqiang!\n");  
            sleep(1);  
        }  
		while(1)
			{       		i++;    // sleep(2);
		}	
    }  
      
    int main(int argc, const char *argv[])  
    {  
        int i = 0;  
        int ret = 0;  
        pthread_t id1,id2;  
      
        ret = pthread_create(&id1, NULL, (void *)mythread1,NULL);  
        if(ret)  
        {  
            printf("Create pthread error!\n");  
            return 1;  
        }  
      
        ret = pthread_create(&id2, NULL, (void *)mythread2,NULL);  
        if(ret)  
        {  
            printf("Create pthread error!\n");  
            return 1;  
        }  
          
        pthread_join(id1,NULL);  
        pthread_join(id2,NULL);  
        while(1);	
        return 0;  
    }  