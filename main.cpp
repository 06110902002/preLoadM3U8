#include <iostream>
#include <fstream>
#include <stdio.h>
#include "parse/M3U8Parse.h"
#include "data/VideoCacheInfo.h"
#include "md5/md5.h"
#include "openssl/md5.h"
#include <curl/curl.h>
#include <unistd.h>
#include "downloader/FileDownload.h"
#include "queue/SafeQueue.h"
#include "threadpool/ThreadPool.h"
#include "proxy/HttpServer.hpp"

//using namespace std;


//下载文件数据接收函数
size_t dl_req_reply(void *buffer, size_t size, size_t nmemb, void *user_p)
{
    FILE *fp = (FILE *)user_p;
    size_t return_size = fwrite(buffer, size, nmemb, fp);
    //cout << (char *)buffer << endl;
    return return_size;
}
//http GET请求文件下载
CURLcode dl_curl_get_req(const std::string &url, std::string filename)
{
    //int len = filename.length();
    //char* file_name = new char(len + 1);//char*最后有一个结束字符\0
    //strcpy_s(file_name, len + 1, filename.c_str());

    const char* file_name = filename.c_str();
    char* pc = new char[1024];//足够长
    strcpy(pc, file_name);

    FILE *fp = fopen(pc, "wb");

    //curl初始化
    CURL *curl = curl_easy_init();
    // curl返回值
    CURLcode res;
    if (curl)
    {
        //设置curl的请求头
        struct curl_slist* header_list = NULL;
        header_list = curl_slist_append(header_list, "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

        //不接收响应头数据0代表不接收 1代表接收
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);

        //设置请求的URL地址
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        //设置ssl验证
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);

        //CURLOPT_VERBOSE的值为1时，会显示详细的调试信息
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);

        curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);

        //设置数据接收函数
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &dl_req_reply);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

        //设置超时时间
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 6); // set transport and time out time
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 6);

        // 开启请求
        res = curl_easy_perform(curl);
    }
    // 释放curl
    curl_easy_cleanup(curl);
    //释放文件资源
    fclose(fp);

    return res;
}


size_t header_callback(void *buffer, size_t size, size_t nitems, void *userdata)
{
    std::string sHeaderData = (char*)buffer;
    std::string sKey = "Content-Disposition";
    std::size_t nFound = sHeaderData.find(sKey);
    if (nFound != std::string::npos)
    {
        printf("Header data: %s \n", (char*)buffer);
        std::stringstream ss(sHeaderData);
        char cSplit = ';'; // 设定好分隔符号
        sKey = "filename=";
        std::vector<std::string> results; // 用来存储结果
        std::string strResult;
        while (getline(ss, strResult, cSplit))
        {
            nFound = strResult.find(sKey);
            if (nFound != std::string::npos)
            {
                std::string sFileName = strResult.substr(nFound + sKey.length());
                sKey = '"';
                nFound = sFileName.find(sKey);
                if (nFound != std::string::npos)
                {
                    sFileName = sFileName.substr(1, sFileName.length()-2);
                }
                //m_sFileName = sFileName;
                printf("sFileName: %s \n", sFileName.c_str());
                break;
            }
        }
    }

    return nitems * size;
}

double getDownloadFileInfo(const std::string url)
{
    CURL *easy_handle = NULL;
    int ret = CURLE_OK;
    double size = -1;

    do
    {
        easy_handle = curl_easy_init();
        if (!easy_handle)
        {
            printf("curl_easy_init error\n");
            break;
        }

        // Only get the header data
        curl_easy_setopt(easy_handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(easy_handle, CURLOPT_CUSTOMREQUEST, "GET");    //使用CURLOPT_CUSTOMREQUEST
        //ret |= curl_easy_setopt(easy_handle, CURLOPT_HEADER, 1L);
        curl_easy_setopt(easy_handle, CURLOPT_NOBODY, 1L);
        //ret |= curl_easy_setopt(easy_handle, CURLOPT_WRITEFUNCTION, nousecb);

        curl_easy_setopt(easy_handle, CURLOPT_HEADERFUNCTION, header_callback);

        ret = curl_easy_perform(easy_handle);
        if (ret != CURLE_OK)
        {
            char s[100] = {0};
            //sprintf(s, sizeof(s), "error:%d:%s", ret, curl_easy_strerror(static_cast<CURLcode>(ret)));

            //ZLOG(s);
            break;
        }

        // size = -1 if no Content-Length return or Content-Length=0
        ret = curl_easy_getinfo(easy_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);

        if (ret != CURLE_OK)
        {
            printf("curl_easy_getinfo error\n");
            break;
        }

    } while (0);

    curl_easy_cleanup(easy_handle);

    if (size > 0.0)
    {
        //downloadFileLength = size;
    }

    return size;
}

void test_serialize() {
    //序列化 测试
    VideoCacheInfo videoCacheInfo;
    videoCacheInfo.mVideoUrl = "http://";
    videoCacheInfo.mVideoType = 23;
    videoCacheInfo.mSpeed = 23;
    videoCacheInfo.mPercent = 23;
    videoCacheInfo.mIsCompleted  = 0;
    videoCacheInfo.mLocalPort = 12580;
    videoCacheInfo.mSavePath = "../video.info";
    videoCacheInfo.mTotalSize = 52480;
    videoCacheInfo.mMd5 = "ufvsufvhsuvhsv232323";
    videoCacheInfo.mCachedSize = 234;
    videoCacheInfo.mTsLengthMap[0] = 245;
    videoCacheInfo.mTsLengthMap[1] = 345;
    videoCacheInfo.mTsLengthMap[2] = 235;
    videoCacheInfo.mTsLengthMap[3] = 145;

    videoCacheInfo.mVideoSegMap[0] = 45;
    videoCacheInfo.mVideoSegMap[1] = 75;
    videoCacheInfo.mVideoSegMap[2] = 25;
    videoCacheInfo.mVideoSegMap[3] = 15;

    //序列化
    OutStream os;
    os<<videoCacheInfo;

    std::string serializestr = os.str();
    VideoCacheInfo videoCacheInfo2;
    InStream is(serializestr);
    is >> videoCacheInfo2;

    //printf("61-----mvideo = %s:\n",videoCacheInfo2.mVideoUrl.c_str());
}



void progress_listener (void *userdata, double downloadSpeed,double remainingTime,double progressPercentage) {

    printf("177-----downloadSpeed = %f,  remainingTime = %f s  progressPercentage = %f \n", downloadSpeed, remainingTime,progressPercentage);
}

void taskCallBack(void* arg) {
    char* parmas = (char*) arg;
    std::cout << "thread "<< pthread_self() << " 正在执行，执行的参数 = " << parmas << std::endl;
    sleep(1);
}
void taskCallBack2(void* arg) {
    char* parmas = (char*) arg;
    std::cout << "thread "<< pthread_self() << " 正在执行，执行的参数 = " << parmas << std::endl;
    sleep(10);
}

void taskCallBack3(void* arg) {
    char* parmas = (char*) arg;
    std::cout << "thread "<< pthread_self() << " 正在执行，执行的参数 = " << parmas << std::endl;
    sleep(2);
}

void taskCallBack4(void* arg) {
    char* parmas = (char*) arg;
    std::cout << "thread "<< pthread_self() << " 正在执行，执行的参数 = " << parmas << std::endl;
    sleep(15);
}

void testThread() {
    ThreadPool* threadPool = new ThreadPool(1,3);
    Task task;
    task.arg = (void*)"test task---0";
    task.function = taskCallBack;
    threadPool->addTask(task);


    Task task1;
    task1.arg = (void *) "test task---1";
    task1.function = taskCallBack2;
    threadPool->addTask(task1);


    Task task2;
    task2.arg = (void *) "test task---2";
    task2.function = taskCallBack3;
    threadPool->addTask(task2);

    Task task3;
    task3.arg = (void *) "test task---3";
    task3.function = taskCallBack4;
    threadPool->addTask(task3);

    sleep(20);


    delete threadPool;
    threadPool = nullptr;

}

void* runnable(void* arg) {
    printf("248--------\n");
}

/**
 * 测试 openssl 的md5 使用
 *
 * @param src
 * @return
 */
std::string test_MD5(const std::string &src) {
    MD5_CTX ctx;

    std::string md5_string;
    unsigned char md[16] = {0};
    char tmp[33] = {0};

    MD5_Init(&ctx);
    MD5_Update(&ctx, src.c_str(), src.size());
    MD5_Final(md, &ctx);

    for (int i = 0; i < 16; ++i) {
        memset(tmp, 0x00, sizeof(tmp));
        sprintf(tmp, "%02X", md[i]);
        md5_string += tmp;
    }
    return md5_string;
}

/**
 * 测试 本地服务器
 * 客户端的请求 类似为：http://127.0.0.1:12580/test=123&userId=2226
 */
void testHttpServer() {
    std::shared_ptr<HttpServer> svr(new HttpServer(12580));
    svr->InitServer();
    svr->Loop();

}



int main() {
    //testThread();
    testHttpServer();

//    pthread_t thread;
//    int result = pthread_create(&thread, nullptr,runnable, nullptr);
//    sleep(1);
//    printf("255---------thread_create result = %d\n",result);

    const char *filePath = "../prog_index.m3u8";
//    ifstream file;
//    file.open(filePath,ios::in);
//    if(!file.is_open()) {
//        printf("190-----文件打开失败\n");
//        return 0;
//    }
//    string strLine;
//    while(getline(file,strLine)){
//        if(strLine.empty()) {
//            break;
//        }
//
//        printf("197----- str = %s\n",strLine.c_str());
//    }

//    M3U8* m3U8 = new M3U8();
//
//    M3U8Parse* m3U8Parse = new M3U8Parse();
//    m3U8Parse->parse(*filePath,&m3U8);
//
//    const char *local_m3u8_file = "../local.m3u8";
//    m3U8Parse->createLocalM3U8File(local_m3u8_file,*m3U8);
//    delete m3U8;
//    char buf[9];
//    sprintf(buf, "%f", 9.976670);//这句是将浮点数转换为字符串
//    printf("parse done = %s  sizeof (char*) = %lu\n",buf,sizeof (local_m3u8_file));



   

    //测试md5
//    md5::MD5 md5Utils;
//    char* http_url = "http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear1/prog_index.m3u8";
//    char* md5_str = md5Utils.digestString(http_url);
//    printf("80------http md5 = %s\n",md5_str);




    char* dl_get_url = "http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear1/prog_index.m3u8";
//    string filename ="../test.m3u8";
//    dl_curl_get_req(dl_get_url, filename);
//
//    printf("155-------- 下载完成\n");

    //getDownloadFileInfo(dl_get_url);

//    char* mp4 = "https://static-dev.nio.com/dc-operation/2022/11/16/g5sX1zY2BcfjORwH.mp4";
//    FileDownload* fileDownload = new FileDownload();
//    fileDownload->downLoad(mp4,"../test.mp4", nullptr, progress_listener);


    return 0;
}
