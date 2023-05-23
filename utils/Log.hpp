//
// Created by Rambo.Liu on 2023/5/22.
//

#ifndef M3U8_LOG_HPP
#define M3U8_LOG_HPP

#include <iostream>
#include <string>
#include <ctime>
#include <sys/time.h>

#define INFO    1
#define WARNING 2
#define ERROR   3
#define FATAL   4

// 输入level肯定不希望带双引号，可以理解成level加上#将宏参数level变成对应的字符串，比如：LOG(INFO, "This is a demo");
#define LOG(message) Log(message,__FILE__,__FUNCTION__,__LINE__)


long long get_timestamp()//获取时间戳函数
{
    long long tmp;
    struct timeval tv{};

    gettimeofday(&tv, nullptr);
    tmp = tv.tv_sec;
    tmp = tmp * 1000;
    tmp = tmp + (tv.tv_usec / 1000);

    return tmp;
}

void get_format_time_string(char *str_time) //获取格式化时间
{
    time_t now;
    struct tm *tm_now;
    char datetime[128];

    time(&now);
    tm_now = localtime(&now);
    strftime(datetime, 128, "%Y-%m-%d %H:%M:%S", tm_now);

    printf("now datetime : %s\n", datetime);
    strcpy(str_time, datetime);
}

void get_format_time_ms(char **str_time) {
    struct tm *tm_t;
    struct timeval time{};

//    if (sizeof(str_time) < 32) {
//        printf("input buff len less than 32");
//        return;
//    }
    gettimeofday(&time,nullptr);
    tm_t = localtime(&time.tv_sec);
    char datetime[128];
    if(nullptr != tm_t) {
        sprintf(datetime,"%04d-%02d-%02d %02d:%02d:%02d:%3d",
                tm_t->tm_year+1900,
                tm_t->tm_mon+1,
                tm_t->tm_mday,
                tm_t->tm_hour,
                tm_t->tm_min,
                tm_t->tm_sec,
                time.tv_usec/1000);
        //printf("69-----time = %s\n",datetime);
        //strcpy(str_time, datetime);
        *str_time = datetime;
    }

    return;
}



//void Log(std::string level, std::string message, std::string file_name, int line) {
//    std::cout << "[" << level << "][" << time(nullptr) << "][" << line << "-------" << message << "][" << file_name
//              << std::endl;
//}

void Log(std::string message,const char* file,const char* fun, int line) {
    struct tm *tm_t;
    struct timeval time{};
    gettimeofday(&time,nullptr);
    tm_t = localtime(&time.tv_sec);
    char datetime[128];
    if(nullptr != tm_t) {
        sprintf(datetime,"%04d-%02d-%02d %02d:%02d:%02d:%3d",
                tm_t->tm_year+1900,
                tm_t->tm_mon+1,
                tm_t->tm_mday,
                tm_t->tm_hour,
                tm_t->tm_min,
                tm_t->tm_sec,
                time.tv_usec/1000);
    }
    std::cout << "[" << datetime << "][" << file << "][" << fun << "][" << line << "-----][" << message << "]"
              << std::endl;
}



#endif //M3U8_LOG_HPP
