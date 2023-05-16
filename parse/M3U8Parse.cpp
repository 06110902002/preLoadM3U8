//
// Created by Rambo.Liu on 2023/5/11.
//

#include "M3U8Parse.h"
#include <regex>

//正则匹配数值包括负数 小数 整数
std::string str{ "-[0-9]+(.[0-9]+)?|[0-9]+(.[0-9]+)?" };
std::regex re(str);
bool retMatchStr = false;

M3U8Parse::M3U8Parse() {

    std::string str = "#EXTINF:9.97667,";
    std::regex pattern("(\\d+\\.\\d+)");
    std::sregex_iterator it(str.begin(), str.end(), pattern);
    std::sregex_iterator end;
    while (it != end)
    {
        std::cout << it->str() << std::endl;
        ++it;
    }
}

M3U8Parse::~M3U8Parse() {

}

void M3U8Parse::parse(const char &m3u8Url, M3U8 **m3u8) {

    ifstream file;
    file.open(&m3u8Url,ios::in);
    if(!file.is_open()) {
        printf("190-----文件打开失败\n");
        return;
    }

    M3U8* tmpM3U8 = new M3U8();

    float targetDuration = 0;
    int32_t version = 0;
    float segDuration = 0;
    int segIndex = 0;
    int32_t sequence = 0;

    bool hasDiscontinuity = false;
    bool hasEndList = false;
    bool hasMasterList = false;
    bool hasKey = false;
    bool hasInitSegment = false;
    char* initSegmentUri;
    char* parentUrl = "http://devimages.apple.com.edgekey.net/streaming/examples/bipbop_4x3/gear1/";

    string strLine;
    while(getline(file,strLine)){
        if(strLine.empty()) {
            continue;
        }
        if (strLine.rfind(TAG_PREFIX, 0) == 0) {
            if (strLine.rfind(TAG_MEDIA_DURATION, 0) == 0) {
                string duration = regNumFromStr(strLine,NUM_PATTERN);
                segDuration = strtod(duration.c_str(), nullptr);
                printf("60---- duration = %s   segduration = %f\n",duration.c_str(),segDuration);
            } else if (strLine.rfind(TAG_TARGET_DURATION, 0) == 0) {
                string targetDurationStr = regNumFromStr(strLine,NUM_PATTERN);
                targetDuration = strtod(targetDurationStr.c_str(), nullptr);
            } else if (strLine.rfind(TAG_VERSION, 0) == 0) {
                string verisonStr = regNumFromStr(strLine,NUM_PATTERN);
                version = strtod(verisonStr.c_str(), nullptr);
            } else if (strLine.rfind(TAG_MEDIA_SEQUENCE, 0) == 0) {
                string tmp = regNumFromStr(strLine,NUM_PATTERN);
                sequence = strtod(tmp.c_str(), nullptr);
            } else if (strLine.rfind(TAG_STREAM_INF, 0) == 0) {
                hasMasterList = true;
            } else if (strLine.rfind(TAG_DISCONTINUITY, 0) == 0) {
                hasDiscontinuity = true;
            } else if (strLine.rfind(TAG_ENDLIST, 0) == 0) {
                hasEndList = true;
            } else if (strLine.rfind(TAG_KEY, 0) == 0) {
               hasKey = true;
               //加密处理
            } else if (strLine.rfind(TAG_INIT_SEGMENT, 0) == 0) {
                hasInitSegment = true;
                string tempInitSegmentUri = regNumFromStr(strLine,REGEX_URI);
            }
            continue;
        }

        if(abs(segDuration) < 0.001f) {
            continue;
        }

        M3U8Seg* seg = new M3U8Seg();
        seg->mParentUrl = parentUrl;
        char* full_real_url = (char*)malloc(strlen(parentUrl) + strLine.length() + 1);
        memset(full_real_url,0,strlen(parentUrl) + strLine.length() + 1);
        strcpy(full_real_url,parentUrl);
        strcat(full_real_url,strLine.c_str());
        seg->mUrl = full_real_url;
        seg->mSegIndex = segIndex;

        seg->mDuration = segDuration;
        seg->mHasKey = hasKey;

        tmpM3U8->mSegList.push_back(seg);
        tmpM3U8->mTargetDuration = targetDuration;
        tmpM3U8->mIsLive = !hasEndList;
        tmpM3U8->mSequence = sequence;
        tmpM3U8->mVersion = version;

        *m3u8 = tmpM3U8;

        segIndex++;
        segDuration = 0;
        hasDiscontinuity = false;
        hasKey = false;
        hasInitSegment = false;
        initSegmentUri = nullptr;

        printf("29----- str = %s\n",strLine.c_str());
    }
}

string M3U8Parse::regNumFromStr(string str,const char* p) {

    //std::string str = "#EXTINF:9.97667,";
    std::regex pattern(p);
    std::sregex_iterator it(str.begin(), str.end(), pattern);
    std::sregex_iterator end;
    string result;
    while (it != end) {
        //std::cout << it->str() << std::endl;
        result.append(it->str());
        ++it;
    }
    return result;
}

void M3U8Parse::createLocalM3U8File(const char* m3u8file, M3U8 &m3u8) {
    if (!m3u8file) {
        printf("140-----写入文件失败，请检查文件路径是否存在\n");
        return;
    }
    fstream f;
    //追加写入,在原来基础上加了ios::app
    f.open(m3u8file,ios::out|ios::app);
    //输入你想写入的内容
//    f<<"今天天气不错"<<endl;
//    f<<"今天天气不错"<<endl;

    //写入标签头
    f.write(PLAYLIST_HEADER,sizeof(PLAYLIST_HEADER) -1);
    f.write("\n",1);
    //写入版本信息
    f.write(TAG_VERSION,sizeof(TAG_VERSION) - 1);
    f.write(":",1);
    char version_buf[1];
    sprintf(version_buf, "%d", m3u8.mVersion);// m3u8.mTargetDuration = 9.976670
    f.write(version_buf,1);
    f.write("\n",1);

    //写入媒体序列号 即ts 索引序列号
    f.write(TAG_MEDIA_SEQUENCE,sizeof(TAG_MEDIA_SEQUENCE) - 1);
    f.write("\n",1);
    //写入标签片段播放耗时
    f.write(TAG_TARGET_DURATION,sizeof (TAG_TARGET_DURATION) - 1);
    f.write(":",1);
    char buf[9];
    sprintf(buf, "%f", m3u8.mTargetDuration);// m3u8.mTargetDuration = 9.976670
    f.write(buf,8);
    f.write("\n",1);

    //写入单个片段 seg 信息
    list<M3U8Seg*>::iterator it;
    for (it = m3u8.mSegList.begin(); it != m3u8.mSegList.end(); it++) {
        f.write(TAG_MEDIA_DURATION,sizeof(TAG_MEDIA_DURATION) - 1);
        f.write(":",1);

        char buf[9];
        sprintf(buf, "%f", (*it)->mDuration);
        f.write(buf,8);
        f.write(",",1);
        f.write("\n",1);
        uint32_t length = strlen((*it)->mUrl);
        f.write((*it)->mUrl, length);
        f.write("\n",1);
        delete((*it)->mUrl);
    }

    f.close();
    printf("140-----%s 写入完毕\n",m3u8file);
}
