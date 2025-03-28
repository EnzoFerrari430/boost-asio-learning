#include <iostream>
#include <boost/asio.hpp>
#include <thread>

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

using namespace std;
using namespace boost::asio::ip;
const int MAX_LENGTH = 1024 * 2;
const int HEAD_LENGTH = 2;
int main()
{
    try {
        //���������ķ���
        boost::asio::io_context   ioc;
        //����endpoint
        tcp::endpoint  remote_ep(make_address("127.0.0.1"), 10086);
        tcp::socket  sock(ioc);
        boost::system::error_code   error = boost::asio::error::host_not_found; ;
        sock.connect(remote_ep, error);
        if (error) {
            cout << "connect failed, code is " << error.value() << " error msg is " << error.message();
            return 0;
        }

        Json::Value root;
        root["id"] = 1001;
        root["data"] = "hello world";
        std::string request = root.toStyledString();
        short request_length = request.length();
        char send_data[MAX_LENGTH] = { 0 };
        // תΪ�����ֽ���
        short request_host_length = boost::asio::detail::socket_ops::host_to_network_short(request_length);
        memcpy(send_data, &request_host_length, 2);
        memcpy(send_data + 2, request.c_str(), request_length);
        boost::asio::write(sock, boost::asio::buffer(send_data, request_length + 2));



        cout << "begin to receive..." << endl;
        char reply_head[HEAD_LENGTH];
        size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_LENGTH));
        short msglen = 0;
        memcpy(&msglen, reply_head, HEAD_LENGTH);
        // �������ֽ���ת���ɱ����ֽ���
        msglen = boost::asio::detail::socket_ops::network_to_host_short(msglen);
        char msg[MAX_LENGTH] = { 0 };
        size_t  msg_length = boost::asio::read(sock, boost::asio::buffer(msg, msglen));

        // �����л�
        Json::Value root2;
        Json::Reader reader;
        reader.parse(msg, msg + msg_length, root2);
        std::cout << "msg id is " << root2["id"] << " msg is " << root2["data"] << std::endl;
        getchar();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}

// ���г���: Ctrl + F5 ����� >����ʼִ��(������)���˵�
// ���Գ���: F5 ����� >����ʼ���ԡ��˵�

// ����ʹ�ü���: 
//   1. ʹ�ý��������Դ�������������/�����ļ�
//   2. ʹ���Ŷ���Դ�������������ӵ�Դ�������
//   3. ʹ��������ڲ鿴���������������Ϣ
//   4. ʹ�ô����б��ڲ鿴����
//   5. ת������Ŀ��>���������Դ����µĴ����ļ�����ת������Ŀ��>�����������Խ����д����ļ���ӵ���Ŀ
//   6. ��������Ҫ�ٴδ򿪴���Ŀ����ת�����ļ���>���򿪡�>����Ŀ����ѡ�� .sln �ļ�