//
// Created by Rambo.Liu on 2023/5/16.
// 文件下载器 使用libcurl库进行

#ifndef M3U8_FILEDOWNLOAD_H
#define M3U8_FILEDOWNLOAD_H

#include <stdio.h>
#include <curl/curl.h>
#include <iostream>


typedef void(*progress_info_callback)(void *userdata,
                                      double downloadSpeed,
                                      double remainingTime,
                                      double progressPercentage);

typedef struct {
    CURL *handle;
    progress_info_callback cb;
} Progress_User_Data;


class FileDownload {

private:
    char *m_url;
    char *m_path;
    CURL *m_curl;


public:

    static int resumeByte;

    FileDownload();

    void initCurl();

    virtual ~FileDownload();

    /**
     * 获取下载文件大小
     *
     * @param url  待下载文件url
     * @return     文件大小 单位字节
     */
    double getDownloadFileLength(const char *url);

    /**
     * 获取本地文件大小
     *
     * @param path
     * @return
     */
    int getLocalFileLength(const char* path);

    /**
     * 下载
     *
     * @param url   下载url
     * @param path  本地路径
     * @param DownLoadListener 下载回调
     */
    CURLcode downLoad(const char *url, const char *path, void *sender, progress_info_callback cb);


};


#endif //M3U8_FILEDOWNLOAD_H
