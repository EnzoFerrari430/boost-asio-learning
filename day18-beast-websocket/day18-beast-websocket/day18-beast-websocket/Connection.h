#pragma once

#include <memory>
#include <queue>
#include <mutex>

#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

namespace net = boost::asio;
namespace beast = boost::beast;

using namespace boost::beast;
using namespace beast::websocket;

class ConnectionMgr;

class Connection : public std::enable_shared_from_this<Connection>
{
public:
    Connection(net::io_context& ioc);

    std::string GetUid();
    net::ip::tcp::socket& GetSocket();
    void AsyncAccept(); // 从tcp建立好连接之后 进行升级
    void Start();
    void AsyncSend(std::string msg);
    void SendCallBack(std::string msg);
private:
    std::unique_ptr<stream<tcp_stream>> _ws_ptr;
    std::string _uuid;
    net::io_context& _ioc;
    flat_buffer _recv_buffer;

    std::queue<std::string> _send_que;
    std::mutex _send_mtx;
};

