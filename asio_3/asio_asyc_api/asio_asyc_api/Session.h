#pragma once


/*
asio �첽��д��api

*/

#include <memory>
#include <boost/asio.hpp>
#include <iostream>
#include <queue>

#include "MsgNode.h"

using namespace boost;

// �Ự��
class Session
{
public:
    Session(std::shared_ptr<asio::ip::tcp::socket> socket);
    void Connect(const asio::ip::tcp::endpoint& ep);

    //�첽д���� ����д���е����� ���Ժ��������������Err
    void WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
        std::shared_ptr<MsgNode> msg_node);
    void WriteToSocketErr(const std::string& buf);

    // ͨ������ʵ��˳���д
    // use async_write_some API
    void WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void WriteToSocket(const std::string& buf);

    // use async_send API
    void WriteAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void WriteAllToSocket(const std::string& buf);


    // �첽������
    // use async_read_some API
    void ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void ReadFromSocket();

    void ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void ReadAllFromSocket();
private:
    // ��Ϣ���б�֤���͵�˳����
    std::queue<std::shared_ptr<MsgNode>> _send_queue;
    std::shared_ptr<asio::ip::tcp::socket> _socket;
    std::shared_ptr<MsgNode> _send_node; // д���
    bool _send_pending;

    std::shared_ptr<MsgNode> _recv_node;
    bool _recv_pending;
};

