cmake_minimum_required(VERSION 3.23)
project(m3u8)

set(CMAKE_CXX_STANDARD 14)

#链接本地openssl 库
set(INK_DIR /opt/homebrew/opt/openssl@3/include)
set(LINK_DIR /opt/homebrew/opt/openssl@3/lib)


include_directories(${INK_DIR})
link_directories(${LINK_DIR})
link_libraries(ssl crypto)
#链接本地openssl 库 end

#-------连接本地libcurl 库
set(CUR_INCLUDE_DIR opt/homebrew/opt/curl/include)
set(CUR_LINK_DIR opt/homebrew/opt/curl/lib)


include_directories(${CUR_INCLUDE_DIR})
link_directories(${CUR_LINK_DIR})
link_libraries(curl)
#--------连接本地libcurl 库 end


add_executable(m3u8 main.cpp
        parse/M3U8Parse.cpp
        parse/M3U8Parse.h
        serialize/serialize.h
        data/VideoCacheInfo.cpp
        data/VideoCacheInfo.h
        md5/md5.h
        downloader/FileDownload.cpp
        downloader/FileDownload.h
        queue/SafeQueue.h
        threadpool/TaskQueue.cpp
        threadpool/TaskQueue.h
        threadpool/ThreadPool.cpp
        threadpool/ThreadPool.h
        utils/Log.hpp
        utils/Util.hpp
        proxy/HttpServer.hpp
        proxy/TcpServer.hpp
        proxy/Protocol.hpp)
