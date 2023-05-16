//
// Created by Rambo.Liu on 2023/5/15.
//

#include "VideoCacheInfo.h"

VideoCacheInfo::VideoCacheInfo() {

    mVideoType = -1;
    mCachedSize = 0;
    mTotalSize = 0;
    mCachedTs = 0;
    mTotalTs = 0;

    mLocalPort = 0;
    mIsCompleted = 0;
    mPercent = 0;
    mSpeed = 0;
}

VideoCacheInfo::~VideoCacheInfo() {
    mVideoUrl.clear();
    mMd5.clear();
    mSavePath.clear();
    mIsCompleted = false;
    mPercent = 0;
    mSpeed = 0;
    mTsLengthMap.clear();
    mVideoSegMap.clear();
    printf("30-------  ~VideoCacheInfo\n");
}

std::string VideoCacheInfo::serialize() {
    OutStream os;
    os << mVideoUrl << mVideoType << mCachedSize << mCachedTs << mTotalTs << mTotalSize << mMd5 << mSavePath << mLocalPort
       << mIsCompleted << mPercent << mSpeed << mTsLengthMap << mVideoSegMap;

    return os.str();
}

int VideoCacheInfo::deserialize(std::string &str) {
    InStream is(str);
    is >> mVideoUrl >> mVideoType >> mCachedSize >> mCachedTs >>mTotalTs>> mTotalSize >> mMd5 >> mSavePath >> mLocalPort
       >> mIsCompleted >> mPercent >> mSpeed >> mTsLengthMap >> mVideoSegMap;

    return is.size();
}
