//
// Created by Rambo.Liu on 2023/5/16.
//

#include "FileDownload.h"
#include <cmath>
#include <stdlib.h>
#include <sys/stat.h>


/**
 * 下载接收函数
 *
 * @param buffer 待接收的数据缓存指针
 * @param size   待接收数据缓存大小 以字节为单位
 * @param nmemb  这是元素的个数，每个元素的大小为 size 字节。通常为1
 * @param user_p 文件描述符，指向 FILE 对象的指针，该 FILE 对象指定了一个输出流
 * @return
 */
static size_t dl_req_reply(void *buffer, size_t size, size_t nmemb, void *user_p) {
    if (user_p) {
        FILE *fp = (FILE *) user_p;
        size_t return_size = fwrite(buffer, size, nmemb, fp);
        return return_size;
    }
    return -1;

}


FileDownload::FileDownload() {
    m_url = nullptr;
    m_path = nullptr;
    initCurl();
}

FileDownload::~FileDownload() {

}

void FileDownload::initCurl() {
    m_curl = curl_easy_init();
}

int FileDownload::resumeByte = -1;

/**
 * 需要下载进度回调
 *
 * @param progress_data  clientp为用户自定义参数，通过设置CURLOPT_XFERINFODATA属性来传递。此函数返回非0值将会中断传输，
 *                       错误代码是 CURLE_ABORTED_BY_CALLBACK
 * @param dltotal  需要下载的总字节数
 * @param d        dlnow是已经下载的字节数
 * @param ultotal  ultotal是将要上传的字节数  仅仅下载数据的话，那么ultotal，ulnow将会是0  仅仅上传的话，那么dltotal和dlnow也会是0
 * @param ulnow    ulnow是已经上传的字节数
 * @return
 */
int progress_func(Progress_User_Data *progress_data,
                  double dltotal,
                  double dlnow,
                  double ultotal,
                  double ulnow) {

//    printf("已经下载的数据 %f /  总大小 %f ( 已经下载： %f %%)\n",
//           dlnow, dltotal,
//           dlnow * 100.0 / dltotal);

    Progress_User_Data* userData = static_cast<Progress_User_Data*>(progress_data);
    if (!userData) {
        printf("%s error app stop ",__FUNCTION__ );
        return 0;
    }
    CURL *easy_handle = userData->handle;
    char timeFormat[9] = "Unknow";

    // Defaults to bytes/second
    double speed;
    std::string unit = "B";
    curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD, &speed); // curl_get_info必须在curl_easy_perform之后调用
    double leftTime = 0;
    if (speed != 0) {
        // Time remaining
        leftTime = (dltotal - dlnow - FileDownload::resumeByte) / speed;
        int hours = leftTime / 3600;
        int minutes = (leftTime - hours * 3600) / 60;
        int seconds = leftTime - hours * 3600 - minutes * 60;

        sprintf(timeFormat, "%02d:%02d:%02d", hours, minutes, seconds);
        //printf("84-----timeFormat = %s  left time = %f\n", timeFormat, leftTime);
    }

    if (speed > 1024 * 1024 * 1024) {
        unit = "G";
        speed /= 1024 * 1024 * 1024;
    } else if (speed > 1024 * 1024) {
        unit = "M";
        speed /= 1024 * 1024;
    } else if (speed > 1024) {
        unit = "kB";
        speed /= 1024;
    }

    //printf("speed:%.2f%s/s", speed, unit.c_str());

    if (dltotal != 0) {
        double progress = (dlnow + FileDownload::resumeByte) / dltotal * 100;
        //printf("\t%.2f%%\tRemaing time:%s\n", progress, timeFormat);
        if (userData->cb) {
            userData->cb(nullptr, speed, leftTime, progress);
        }
    }

    return 0;
}


CURLcode FileDownload::downLoad(const char *url, const char *path, void *sender, progress_info_callback cb) {
    double size = -1;
    FILE *fp = fopen(path, "wb");

    if (!m_curl) {
        initCurl();
    }
    CURLcode res;
    if (m_curl) {
        //设置curl的请求头
        struct curl_slist *header_list = NULL;
        header_list = curl_slist_append(header_list,
                                        "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64; Trident/7.0; rv:11.0) like Gecko");
        curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, header_list);

        //不接收响应头数据0代表不接收 1代表接收
        curl_easy_setopt(m_curl, CURLOPT_HEADER, 0);

        //设置请求的URL地址
        curl_easy_setopt(m_curl, CURLOPT_URL, url);

        //设置ssl验证
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST, false);

        //CURLOPT_VERBOSE的值为1时，会显示详细的调试信息
        curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 0);

        curl_easy_setopt(m_curl, CURLOPT_READFUNCTION, NULL);

        //设置数据接收函数
        curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, &dl_req_reply);
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, fp);

        curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1);

        //设置超时时间
        curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, 6); // set transport and time out time
        curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, 6);

        //获取下载进度
        Progress_User_Data userData = {m_curl, cb};
        curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, progress_func);
        curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, &userData);

        //断点续传 参数 设置相关
        //用libcurl实现断点续传很简单,只用两步即可实现,一是要得到本地文件已下载的大小,通过函数getLocalFileLenth()方法来得到,
        //二是设置CURLOPT_RESUME_FROM_LARGE参数的值为已下载本地文件大小.
        resumeByte = getLocalFileLength(path);
        if (resumeByte > 0) {
            // Set a point to resume transfer
            curl_easy_setopt(m_curl, CURLOPT_RESUME_FROM_LARGE, resumeByte);
        }

        // 开启请求
        res = curl_easy_perform(m_curl);

//        if (res == CURLE_OK) {
//            curl_easy_getinfo(m_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
//            printf("87-----fize = %f\n",size);
//        }
    }
    curl_easy_cleanup(m_curl);
    fclose(fp);
    return res;
}

double FileDownload::getDownloadFileLength(const char *url) {
    double size = -1;
    if (!m_curl) {
        m_curl = curl_easy_init();
    }
    if (m_curl) {
        curl_easy_setopt(m_curl, CURLOPT_URL, url);
        if (curl_easy_perform(m_curl) == CURLE_OK) {
            curl_easy_getinfo(m_curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &size);
        } else {
            size = -1;
        }
    }
    return size;
}

int FileDownload::getLocalFileLength(const char *path) {
    struct stat fileStat;

    int ret = stat(path, &fileStat);
    if (ret == 0) {
        return fileStat.st_size;
    }
    return ret;
}



