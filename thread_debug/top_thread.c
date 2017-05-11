    /* top_thread.c -- mimic "top -H -p pid" 
     * author: ludi 2013.11 
     
    doc:  
    $ cat /proc/stat 
    cpu  3723999 28230 813821 136182538 6215783 187445         373893  0 
          user,  nice,   sys,     idle,     irq, softirq, stealstolen, guest 
     
    $ cat /proc/31628/stat 
    31628 (pdflush) S 19 1 1 0 -1 8400960 0 0 0 0  0 161 0 0  15 0 1  0 36070699 0 0 4294967295 0 0 0 0   0 0 2147483647 65536 0 0 0 0 17 2 0 0 2648 
    $ cat /proc/31628/task/31628/stat 
    31628 (pdflush) S 19 1 1 0 -1 8400960 0 0 0 0  0 161 0 0  15 0 1  0 36070699 0 0 4294967295 0 0 0 0   0 0 2147483647 65536 0 0 0 0 17 2 0 0 2648 
     1      2       3 4  5 6 7 8   9      10--13  14------17  18-20   21-----------------------------29   30---------------33 
     
     
    pid comm state ppid pgid sid tty_nr tty_pgrp 1-8 
    flags 9 
    flt_{min, cmin, maj, cmaj}10-13 
    time_{u,s,cu,cs} 14-17 
    priority nice num_threads 18-20 
    start_time vsize mm rsslim start_code end_code start_stack esp eip 21-29 
    sig_{pending, blocked, ign, catch} 30-33 
     
     
    ULL totalCpuTime; //from /proc/stat 
    ULL processCpuTime; //from /proc/<pid>/stat 
    ULL threadCpuTime; //from /proc/<pid>/task/<tid>/stat 
     
    totalCpuTime = user + nice + system + idle + iowait + irq + softirq + stealstolen +  guest; 
    processCpuTime = utime + stime + cutime + cstime; 
    threadCpuTime = utime + stime; 
     
     pthread_create(&task_id,...) 碌脛ask_id潞脥id虏禄同拢卢 
    脨要脳一赂枚娄: 
     #include <sys/syscall.h>  
     printf("%u --> tid %u\n", pthread_self(), syscall(SYS_gettid)); 
    */  
      
    #include <stdio.h>  
    #include <string.h>  
    #include <stdlib.h>  
      
    #include <unistd.h>  
    #include<sys/stat.h>  
    #include <unistd.h> /*fstat*/  
    #include <dirent.h> /*opendir*/  
      
    #define THREAD_MAX 512  
    typedef unsigned long long ULL;  
      
    typedef struct{  
        ULL total, idle;  
        ULL process;  
        int tcnt;  
        unsigned tid[THREAD_MAX];  
        ULL ttime[THREAD_MAX];    
    }cpu_time_t;  
      
      
    typedef struct{  
        unsigned tid; 
	 char tname[15];
        float usage;  
    }sorted_time_t;  

static cpu_time_t last_ct = {0};  
static sorted_time_t st[THREAD_MAX];  
      
    int sorted_time_cmp(const void *a, const void *b)  
    {  
        return ((sorted_time_t*)b)->usage - ((sorted_time_t*)a)->usage;  
    }  
      
      
    ULL get_ttime(cpu_time_t *ct, unsigned tid)  
    {  
        int i;  
        for(i = 0; i < ct->tcnt; ++i){  
            if(ct->tid[i] == tid)return ct->ttime[i];  
        }  
      
        return 0;  
    }  
      
      
    int get_tids(unsigned pid, unsigned tids[], int *cnt)  
    {  
        DIR *dp;  
        struct dirent *entry;  
        struct stat statbuf;  
        char dir[64];  
        int num;  
      
        if(!tids || !cnt){printf("bad arg"); return -1;}  
        sprintf(dir, "/proc/%u/task", pid);  
      
        if(NULL == (dp = opendir(dir))){  
            printf("can not open directory :%s\n",dir);  
            exit( -1);  
        }  
      
        num = 0;  
        while(NULL != (entry = readdir(dp))){  
            stat(entry->d_name,&statbuf);  
            if(S_ISDIR(statbuf.st_mode)  
             &&(!strcmp(".",entry->d_name)||!strcmp("..",entry->d_name))  
             ){continue;}  
      
            tids[num] = atoi(entry->d_name);  
            num++;  
            if(num >= *cnt){break;}  
        }  
        closedir(dp);  
      
        *cnt = num;  
        return 0;  
    }  
      
    int get_processor_cnt(void)  
    {  
        FILE *fp;  
        char name[64];  
        int cnt;  
      
        sprintf(name, "%s", "/proc/cpuinfo");  
        fp = fopen(name, "r");  
        if(!fp){printf("can't open %s\n", name); exit(-1);}  
      
        cnt = 0;  
        while(fgets(name, sizeof(name), fp)){  
            if(!strncmp(name, "processor", strlen("processor"))  
            ||!strncmp(name, "Processor", strlen("Processor")) ){++cnt;}  
        }  
          
        fclose(fp);  
          
        if(!cnt){  
            printf("set processor cnt to default 1");  
            cnt = 1;  
        }  
      
        return cnt;  
    }  
      
    ULL cpu_time_snap_cpu(ULL *idle_)  
    {  
        ULL user, nice, sys, idle, irq, softirq, stealstolen, guest;  
        ULL totalCpuTime;  
        FILE *fp;  
        char name[64];  
      
        sprintf(name, "%s", "/proc/stat");  
        fp = fopen(name, "r");  
        if(!fp)exit(-1);  
        fscanf(fp, "%*s%llu%llu%llu%llu%llu%llu%llu%llu",   
                &user, &nice, &sys, &idle, &irq, &softirq, &stealstolen, &guest);  
        fclose(fp);  
        totalCpuTime = user + nice + sys + idle + irq + softirq + stealstolen +  guest;  
        *idle_ = idle;  
        return totalCpuTime;  
    }  
      
    ULL cpu_time_snap_process(ULL pid)  
    {  
        ULL utime, stime, cutime, cstime;  
        ULL processCpuTime;  
        FILE *fp;  
        char name[64];  
          
        sprintf(name, "/proc/%llu/stat", pid);  
        fp = fopen(name, "r");  
        if(!fp)exit(-1);  
        fscanf(fp, "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s");  
        fscanf(fp, "%llu%llu%llu%llu", &utime, &stime, &cutime, &cstime);  
        fclose(fp);  
      
        processCpuTime = utime + stime + cutime + cstime;  
        return processCpuTime;  
    }  
      
    ULL cpu_time_snap_thread(ULL pid, ULL tid)  
    {  
        ULL utime, stime, cutime, cstime;  
        ULL threadCpuTime;  
        FILE *fp;  
        char name[64];  
          
        sprintf(name, "/proc/%llu/task/%llu/stat", pid, tid);  
        fp = fopen(name, "r");  
        if(!fp)return 0;//exit(-1);  
      
        fscanf(fp, "%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s%*s");  
        fscanf(fp, "%llu%llu%llu%llu", &utime, &stime, &cutime, &cstime);  
        fclose(fp);  
      
        threadCpuTime = utime + stime;  
        return threadCpuTime;  
    }  

    int   get_tname(int index,ULL tid)  
    {  

        FILE *fp;  
        char name[15];  
          
        sprintf(name, "/proc/%llu/comm", tid);  
        fp = fopen(name, "r");  
 
    if (fp  == NULL)
        printf ("Error opening file");
    else {
		
       // fgets(st[index].tname, 20 , fp) ;
        fscanf(fp, "%s", st[index].tname);  

        fclose (fp);
    }
        return 1;  
    }  
	  
    ULL cpu_time_snap(unsigned pid)  
    {  
        static int is_first = 1, processor_cnt = 0;  

        ULL delta_totalCpuTime, delta_idle, delta_processCpuTime, delta_threadCpuTime;  
        cpu_time_t ct = {0};  
              
        float pcpu[4];  
        int i;  
      
        if(is_first){processor_cnt = get_processor_cnt();}  


        ct.tcnt = THREAD_MAX;  
        get_tids(pid, ct.tid, &ct.tcnt);  

		//  return 0;
      
        ct.total = cpu_time_snap_cpu(&ct.idle);  
        ct.process = cpu_time_snap_process(pid);  
        for(i = 0; i < ct.tcnt; ++i){  
            ct.ttime[i] = cpu_time_snap_thread(pid, ct.tid[i]);  
        }  
        if(is_first)goto _end;  
      
        delta_totalCpuTime = ct.total - last_ct.total;  
        delta_idle = ct.idle - last_ct.idle;  
        delta_processCpuTime = ct.process - last_ct.process;  
        if(!delta_totalCpuTime)return 0;  
      
        //cpu usage  
        pcpu[0] = 100*(delta_totalCpuTime - delta_idle)/(float)delta_totalCpuTime ;//*processor_cnt;  
        //process usage  
        pcpu[1] = 100*delta_processCpuTime/(float)delta_totalCpuTime;// *processor_cnt;  
      
        printf("\033[H\033[J");  //清屏
	 printf("\n\rpid=%d, thread num=%d,processor_cnt=%d\n\r",pid,ct.tcnt,processor_cnt);
        printf("*****************************************************************\n");  	 
        printf("cpu usage %.2f %%   process usage %.2f %%\n\n", pcpu[0], pcpu[1]);   
      
        //thread usage  
        for(i = 0; i < ct.tcnt; ++i){  
            delta_threadCpuTime = ct.ttime[i] - get_ttime(&last_ct, ct.tid[i]);  
            pcpu[2] = 100*delta_threadCpuTime/(float)delta_totalCpuTime;// *processor_cnt;  
            st[i].tid = ct.tid[i];  
            st[i].usage = pcpu[2];  
	     get_tname(i,st[i].tid);;
        }  
          
        qsort(st, ct.tcnt, sizeof(st[0]), sorted_time_cmp);  
        for(i = 0; i < ct.tcnt; ++i){  
            printf("tid=%u	tname=%s                   usage=%.2f%% \n\n", st[i].tid,st[i].tname,st[i].usage);  

        }  
        printf("*****************************************************************\n");  
        char mbuff[100];
	 printf("open files:\n");
		sprintf(mbuff, ("ls -l /proc/%llu/fd |busybox awk -F ' ' '{print $9  $10 $11}'"), pid); //产生："I love CSDN. "  这字符串写到s中
		//printf(mbuff);
         system(mbuff);

	//	ls -al fd|busybox awk -F ' ' '{print $6 $7 $8}'
	 printf("\ntcnt %d", ct.tcnt);  
	 


	// ls -al fd|busybox awk -F ' ' '{print $6 $7 $8}' 
        fflush(stdout);  
      
    _end:  
        last_ct = ct;  
        is_first = 0;  
        return delta_totalCpuTime;  
    }  
      
    int main(int argc, char **argv)  
    {  
        int i;  
        //Hertz = sysconf(_SC_CLK_TCK);  
        //printf("HZ %u\n", Hertz);  
          
        if(argc < 2){return printf("usage: topH pid\n");}  
      
        cpu_time_snap(atoi(argv[1]));  
          
        for(i = 0; ; ++i){  
            cpu_time_snap(atoi(argv[1]));  
            usleep(1000000*1);  
        }  
      
        return 0;  
    }  
