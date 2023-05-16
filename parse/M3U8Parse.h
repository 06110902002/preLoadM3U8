//
// Created by Rambo.Liu on 2023/5/11.
// m3u8 文件解析器

#ifndef M3U8_M3U8PARSE_H
#define M3U8_M3U8PARSE_H

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <list>


#define  PLAYLIST_HEADER    "#EXTM3U"    // must
#define  TAG_PREFIX         "#EXT"           // must
#define  TAG_VERSION        "#EXT-X-VERSION" // must
#define  TAG_MEDIA_SEQUENCE   "#EXT-X-MEDIA-SEQUENCE" // must
#define  TAG_TARGET_DURATION  "#EXT-X-TARGETDURATION"                               // must
#define  TAG_MEDIA_DURATION   "#EXTINF" // must
#define  TAG_DISCONTINUITY    "#EXT-X-DISCONTINUITY" // Optional
#define  TAG_ENDLIST          "#EXT-X-ENDLIST" // It is not live if hls has '#EXT-X-ENDLIST' tag; Or it
// is.
#define  TAG_KEY              "#EXT-X-KEY" // Optional
#define  TAG_INIT_SEGMENT     "#EXT-X-MAP"

// extra hls tag:

// #EXT-X-PLAYLIST-TYPE:VOD       is not live
// #EXT-X-PLAYLIST-TYPE:EVENT   is live, we also can try '#EXT-X-ENDLIST'
#define  TAG_PLAYLIST_TYPE      "#EXT-X-PLAYLIST-TYPE"
#define  TAG_STREAM_INF         "#EXT-X-STREAM-INF" // Multiple m3u8 stream, we usually fetch the first.
#define  TAG_ALLOW_CACHE        "EXT-X-ALLOW-CACHE" // YES : not live; NO: live


#define NUM_PATTERN         "([1-9]\d*\.?\d*)|(0\.\d*[1-9])"      //从字符串取数字的正则表达式
#define REGEX_URI           "URI=\"(.+?)\""      //攫取URL 正则表达式



using namespace std;

typedef struct M3U8Seg {
    char *mParentUrl;             //分片的上级M3U8的url
    char *mUrl;                   //分片的网络url
    char *mName;                  //分片的文件名
    float mDuration;               //分片的时长
    int mSegIndex;                 //分片索引位置，起始索引为0
    long mFileSize;                //分片文件大小
    long mContentLength;           //分片文件的网络请求的content-length
    bool mHasDiscontinuity;     //当前分片文件前是否有Discontinuity
    bool mHasKey;               //分片文件是否加密
    char *mMethod;                //分片文件的加密方法
    char *mKeyUrl;                //分片文件的密钥地址
    char *mKeyIv;                 //密钥IV
    int mRetryCount;               //重试请求次数
    bool mHasInitSegment;       //分片前是否有#EXT-X-MAP
    char *mInitSegmentUri;        //MAP的url
    char *mSegmentByteRange;      //MAP的range

} M3U8Seg;


/**
 * 根据m3u8 解析出来的 结构对象
 */
typedef struct M3U8 {

    char *mUrl;                  //M3U8的url
    float mTargetDuration;       //指定的duration
    int mSequence = 0;           //序列起始值
    int mVersion = 3;            //版本号
    bool mIsLive;                //是否是直播
    list<M3U8Seg*> mSegList;      //分片seg 列表

} M3U8;


class M3U8Parse {

public:

    M3U8Parse();

    virtual ~M3U8Parse();

    /**
     * 解析 m3u8
     *
     * @param m3u8Url m3u8 文件地址
     * @param m3u8    m3u8 结构实体
     */
    void parse(const char &m3u8Url, M3U8 **m3u8);

    string regNumFromStr(string str,const char* p);

    /**
     * 创建本地m3u8 索引文件
     *
     * @param m3u8file 本地m3u8文件路径
     * @param m3u8     待写入本地m3u8 信息
     */
    void createLocalM3U8File(const char* m3u8file,M3U8& m3u8);

};


#endif //M3U8_M3U8PARSE_H
