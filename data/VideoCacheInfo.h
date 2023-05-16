//
// Created by Rambo.Liu on 2023/5/15.
//

#ifndef M3U8_VIDEOCACHEINFO_H
#define M3U8_VIDEOCACHEINFO_H

#include "../serialize/serialize.h"
#include <map>


/**
 * 记录缓存的信息实体
 *
 */
class VideoCacheInfo : public Serializable {

public:
    std::string mVideoUrl;                 //视频的url
    int mVideoType;                        //视频类型
    long mCachedSize;                      //已经缓存的大小，M3U8文件忽略这个变量
    long mTotalSize;                       //总大小
    int mCachedTs;                         //已经缓存的ts个数
    int mTotalTs;                          //总的ts个数
    std::string mMd5;                      //videourl对应的md5
    std::string mSavePath;                 //videocacheinfo存储的目录
    int mLocalPort;                        //本地代理的端口号，每次可能不一样
    int mIsCompleted;                      //文件是否缓存完
    float mPercent;                        //缓存视频的百分比
    float mSpeed;                          //缓存速度
    std::map<int, long> mTsLengthMap;      //key表示ts的索引，value表示索引分片的content-length
    std::map<long, long> mVideoSegMap;     //视频分片的保存结构

    VideoCacheInfo();

    virtual ~VideoCacheInfo();

    std::string serialize() override;

    int deserialize(std::string &str) override;

};


#endif //M3U8_VIDEOCACHEINFO_H
