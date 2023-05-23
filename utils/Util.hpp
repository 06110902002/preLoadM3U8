#pragma once

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>

// 声明类的成员为静态时，这意味着无论创建多少个类的对象，静态成员都只有一个副本。
// 静态成员函数即使在类对象不存在的情况下也能被调用，静态函数只要使用类名加范围解析运算符 :: 就可以访问。
// 综上，这个Util类的两个成员都设计成静态成员函数，这样不需要实例化类就能调用
class Util {
public:
    // 读取一行:不同的操作系统的行分隔符可能不同，\r、\n或者\r\n，下面统一处理成\n
    static int ReadLine(int sock, std::string &out) {
        char ch = 'x';//ch只要不初始化为\n即可（保证能够进入while循环）
        while (ch != '\n') {
            ssize_t size = recv(sock, &ch, 1, 0);
            if (size > 0) {
                if (ch == '\r') {
                    recv(sock, &ch, 1, MSG_PEEK);// 窥探下一个字符是否为\n
                    if (ch == '\n')// 下一个字符是\n
                    {
                        recv(sock, &ch, 1, 0); // //\r\n->\n
                    } else // 下一个字符不是\n
                    {
                        ch = '\n'; // \r->\n
                    }
                }

                // 普通字符或\n
                out.push_back(ch);
            } else if (size == 0)// 对方关闭连接
            {
                return 0;
            } else  // 读取失败
            {
                return -1;
            }
        }
        return out.size(); //返回读取到的字符个数
    }

    // 分割字符串
    static bool CutString(std::string &target, std::string &sub1_out, std::string &sub2_out, std::string sep) {
        size_t pos = target.find(sep, 0);
        if (pos != std::string::npos) {
            sub1_out = target.substr(0, pos);
            sub2_out = target.substr(pos + sep.size());
            return true;
        }
        return false;
    }

private:
};