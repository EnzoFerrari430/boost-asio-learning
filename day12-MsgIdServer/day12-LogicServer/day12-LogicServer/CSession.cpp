#include "CSession.h"
#include "CServer.h"
#include "MsgNode.h"
#include <iomanip>

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

CSession::CSession(boost::asio::io_context& ioc, CServer* server) : _socket(ioc), _server(server), _b_close(false), _b_head_parse(false) {
    // 每个session有一个唯一id
    boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(a_uuid);
    _recv_head_node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN); // 头部现在是id + 消息体长度
}

CSession::~CSession()
{
    std::cout << "~CSession destruct" << this << std::endl;
}

void CSession::Start()
{
    memset(_data, 0, MAX_LENGTH);
    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, SharedSelf()));

}

tcp::socket& CSession::GetSocket()
{
    return _socket;
}

std::string& CSession::GetUuid()
{
    return _uuid;
}

void CSession::Send(char* msg, int max_length, short msgid)
{
    std::lock_guard<std::mutex> lock(_send_lock);

    // 发送队列数据控制
    int send_que_size = _send_que.size();
    if (send_que_size > MAX_SENDQUE)
    {
        std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
        return;
    }

    bool pending = false;
    if (_send_que.size() > 0)
    {
        pending = true;
    }

    _send_que.push(std::make_shared<SendNode>(msg, max_length, msgid));
    if (pending)
        return;

    auto& msgnode = _send_que.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
        std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Send(std::string msg, short msgid)
{
    std::lock_guard<std::mutex> lock(_send_lock);

    // 发送队列数据控制
    int send_que_size = _send_que.size();
    if (send_que_size > MAX_SENDQUE)
    {
        std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SENDQUE << std::endl;
        return;
    }

    bool pending = false;
    if (_send_que.size() > 0)
    {
        pending = true;
    }

    _send_que.push(std::make_shared<SendNode>(msg.c_str(), msg.length(), msgid));
    if (pending)
        return;

    auto& msgnode = _send_que.front();
    boost::asio::async_write(_socket, boost::asio::buffer(msgnode->_data, msgnode->_total_len),
        std::bind(&CSession::HandleWrite, this, std::placeholders::_1, SharedSelf()));
}

void CSession::Close()
{
    _socket.close();
    _b_close = true;
}

std::shared_ptr<CSession>CSession::SharedSelf() {
    return shared_from_this();
}

void CSession::PrintRecvData(char* data, int length) {
    std::stringstream ss;
    std::string result = "0x";
    for (int i = 0; i < length; i++) {
        std::string hexstr;
        ss << std::hex << std::setw(2) << std::setfill('0') << int(data[i]) << std::endl;
        ss >> hexstr;
        result += hexstr;
    }
    std::cout << "receive raw data is : " << result << std::endl;
}

//#define Test_Packet_Sticking
// 读数据回调函数
void CSession::HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred, std::shared_ptr<CSession> _self_shared)
{
    if (!error)
    {
#ifdef Test_Packet_Sticking
        PrintRecvData(_data, bytes_transferred);
        std::chrono::milliseconds dura(2000);
        std::this_thread::sleep_for(dura);
#endif
        // 已经移动的字节数
        int copy_len = 0;
        while (bytes_transferred > 0)
        {
            if (!_b_head_parse)
            {
                // 收到数据不足头部大小
                if (bytes_transferred + _recv_head_node->_cur_len < HEAD_TOTAL_LEN)
                {
                    memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_head_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                    return;
                }

                // 收到的数据比头部多
                int head_remain = HEAD_TOTAL_LEN - _recv_head_node->_cur_len;
                memcpy(_recv_head_node->_data + _recv_head_node->_cur_len, _data + copy_len, head_remain);
                // 更新已处理的data长度和剩余未处理的长度
                copy_len += head_remain;
                bytes_transferred -= head_remain;

                // 先获取消息id
                short msg_id = 0;
                memcpy(&msg_id, _recv_head_node->_data, HEAD_ID_LEN);
                // 将网络字节序转换成本地字节序
                msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
                std::cout << "msg id is " << msg_id << std::endl;
                // 判断消息id是否合规
                if (msg_id > MAX_LENGTH)
                {
                    std::cout << "invalid msg_id is " << msg_id << std::endl;
                    Close();
                    _server->ClearSession(_uuid);
                    return;
                }

                //再获取消息体长度
                short msg_len = 0;
                memcpy(&msg_len, _recv_head_node->_data + HEAD_ID_LEN, HEAD_DATA_LEN);
                // 将网络字节序转换成本地字节序
                msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
                std::cout << "msg len is " << msg_len << std::endl;
                if (msg_len > MAX_LENGTH)
                {
                    std::cout << "invalid data length is " << msg_len << std::endl;
                    Close();
                    _server->ClearSession(_uuid);
                    return;
                }

                // 接收消息
                _recv_msg_node = std::make_shared<RecvNode>(msg_len, msg_id);
                if(bytes_transferred < msg_len)
                {
                    // 消息的长度小于头部规定的长度，说明数据没有收全，先将部分消息放到接收结点里
                    memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                    _recv_msg_node->_cur_len += bytes_transferred;
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));

                    // 头部处理完成
                    _b_head_parse = true;
                    return;
                }



                // 消息体接收完全的情况
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, msg_len);
                _recv_msg_node->_cur_len += msg_len;
                copy_len += msg_len;
                bytes_transferred -= msg_len;
                _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
                std::cout << "receive data is " << _recv_msg_node->_data << std::endl;

                {
                    Json::Value root;
                    Json::Reader reader;
                    std::string receive_data;
                    reader.parse(std::string(_recv_msg_node->_data, _recv_msg_node->_total_len), root);
                    std::cout << "receive msg id is " << root["id"].asInt() << " msg data is " << root["data"].asString() << std::endl;
                    std::string return_str = "server has received msg, msg data is " + root["data"].asString();

                    Json::Value root2;
                    root2["id"] = root["id"];
                    root2["data"] = return_str;
                    std::string send_str = root2.toStyledString();
                    Send(send_str, msg_id);
                }


                //一个数据包处理完成 继续轮询处理剩余数据
                _b_head_parse = false;
                _recv_head_node->Clear();
                if (bytes_transferred <= 0)
                {
                    ::memset(_data, 0, MAX_LENGTH);
                    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                    return;
                }
                continue;
            }


            // 已经处理完头部，处理上次未接收完的消息数据
            int remain_msg = _recv_msg_node->_total_len - _recv_msg_node->_cur_len;
            if (bytes_transferred < remain_msg)
            {
                // 接收的数据仍然不足剩余未处理的
                memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, bytes_transferred);
                _recv_msg_node->_cur_len += bytes_transferred;
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                return;
            }

            memcpy(_recv_msg_node->_data + _recv_msg_node->_cur_len, _data + copy_len, remain_msg);
            _recv_msg_node->_cur_len += remain_msg;
            bytes_transferred -= remain_msg;
            copy_len += remain_msg;
            _recv_msg_node->_data[_recv_msg_node->_total_len] = '\0';
            std::cout << "receive data is " << _recv_msg_node->_data << std::endl;

            {
                Json::Value root;
                Json::Reader reader;
                std::string receive_data;
                reader.parse(std::string(_recv_msg_node->_data, _recv_msg_node->_total_len), root);
                std::cout << "receive msg id is " << root["id"].asInt() << " msg data is " << root["data"].asString() << std::endl;
                std::string return_str = "server has received msg, msg data is " + root["data"].toStyledString();

                Json::Value root2;
                root2["id"] = root["id"];
                root2["data"] = return_str;
                std::string send_str = root2.toStyledString();
                Send(send_str, root["id"].asInt()); // TODO: 这里应该使用_recv_head_node里存储的id值
            }

            //一个数据包处理完成 继续轮询处理剩余数据
            _b_head_parse = false;
            _recv_head_node->Clear();
            if (bytes_transferred <= 0)
            {
                ::memset(_data, 0, MAX_LENGTH);
                _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                    std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
                return;
            }
            continue;
        }
    }
    else
    {
        std::cout << "read error" << std::endl;
        Close();
        _server->ClearSession(_uuid);
    }
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared)
{
    if (!error)
    {
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_que.pop();
        if (!_send_que.empty())
        {
            auto& msgNode = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgNode->_data, msgNode->_total_len),
                std::bind(&CSession::HandleWrite, this, std::placeholders::_1, _self_shared));
        }
    }
    else
    {
        std::cout << "write error " << error.value() << std::endl;
        Close();
        _server->ClearSession(_uuid);
    }
}

