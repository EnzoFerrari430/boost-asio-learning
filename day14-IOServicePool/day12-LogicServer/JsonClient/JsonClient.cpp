#include <iostream>
#include <thread>
#include <chrono>

#include <boost/asio.hpp>
#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

using namespace std;
using namespace boost::asio::ip;
const int MAX_LENGTH = 1024 * 2;
const int HEAD_LENGTH = 2;
const int HEAD_TOTAL_LEN = 4;

std::vector<std::thread> vec_threads;

int main()
{
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100; ++i)
    {
        vec_threads.emplace_back([]() {
            try {
                //创建上下文服务
                boost::asio::io_context   ioc;
                //构造endpoint
                tcp::endpoint  remote_ep(make_address("127.0.0.1"), 10086);
                tcp::socket  sock(ioc);
                boost::system::error_code   error = boost::asio::error::host_not_found; ;
                sock.connect(remote_ep, error);
                if (error) {
                    cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
                    return 0;
                }

                int i = 0;
                while (i < 500)
                {
                    Json::Value root;
                    root["id"] = 1001;
                    root["data"] = "hello world";
                    std::string request = root.toStyledString();
                    short request_length = request.length();
                    char send_data[MAX_LENGTH] = { 0 };

                    // 消息id
                    short msgid = 1001;
                    msgid = boost::asio::detail::socket_ops::host_to_network_short(msgid);
                    memcpy(send_data, &msgid, 2);

                    // 转为网络字节序
                    short request_host_length = boost::asio::detail::socket_ops::host_to_network_short(request_length);
                    memcpy(send_data + 2, &request_host_length, 2);
                    memcpy(send_data + HEAD_TOTAL_LEN, request.c_str(), request_length);
                    boost::asio::write(sock, boost::asio::buffer(send_data, request_length + HEAD_TOTAL_LEN));



                    cout << "begin to receive..." << endl;
                    char reply_head[HEAD_TOTAL_LEN];
                    size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_TOTAL_LEN));
                    msgid = 0;
                    memcpy(&msgid, reply_head, HEAD_LENGTH);
                    // 将网络字节序转换成本地字节序
                    msgid = boost::asio::detail::socket_ops::network_to_host_short(msgid);

                    short msglen = 0;
                    memcpy(&msglen, reply_head + HEAD_LENGTH, HEAD_LENGTH);
                    // 将网络字节序转换成本地字节序
                    msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
                    char msg[MAX_LENGTH] = { 0 };
                    size_t  msg_length = boost::asio::read(sock, boost::asio::buffer(msg, msglen));

                    // 反序列化
                    Json::Value root2;
                    Json::Reader reader;
                    reader.parse(msg, msg + msg_length, root2);
                    std::cout << "msg id is " << root2["id"] << " msg is " << root2["data"] << std::endl;
                    i++;
                }


            }
            catch (std::exception& e) {
                std::cerr << "Exception: " << e.what() << endl;
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    for (auto& t : vec_threads)
    {
        t.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
    std::cout << "Time spent: " << duration.count() << " seconds." << std::endl;
    getchar();

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件