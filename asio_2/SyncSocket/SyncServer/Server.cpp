
#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <memory>
using boost::asio::ip::tcp;

const int max_length = 1024;

typedef std::shared_ptr<tcp::socket> socket_ptr;
std::set<std::shared_ptr<std::thread>> thread_set;


// ����������ͻ��˵Ķ�д
void session(socket_ptr sock)
{
    try {
        for (;;)
        {
            char data[max_length];
            memset(data, 0, max_length);
            boost::system::error_code error;

            // ���read�����1024���ֽڣ� ��������ͻ��˷��͵��������Ƚ�С�Ļ� ����������������еȴ� �˷���Դ
            //size_t length = boost::asio::read(sock, boost::asio::buffer(data, max_length), error);

            size_t length = sock->read_some(boost::asio::buffer(data, max_length), error);
            if (error == boost::asio::error::eof)
            {
                std::cout << "connection closed by peer" << std::endl;
                break;
            }
            else if (error)
            {
                throw boost::system::system_error(error);
            }

            std::cout << "receive from" << sock->remote_endpoint().address().to_string() << std::endl;
            std::cout << "receive message is " << data << std::endl;

            // �յ������ݻش����ͻ���
            boost::asio::write(*sock, boost::asio::buffer(data, length));

        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in thread: " << e.what() << "\n" << std::endl;
    }
}

/*
1 ͬ����д��ȱ�����ڶ�д�������ģ�����ͻ��˶Զ˲��������ݷ�������read�����������ģ��⽫���·��������������ȴ�״̬��
2 ����ͨ�������µ��߳�Ϊ�����ɵ����Ӵ����д������һ�����̿��ٵ��߳������޵ģ�ԼΪ2048���̣߳���Linux��������ͨ��unlimit����һ�����̿��ٵ��߳����������̹߳���Ҳ�ᵼ���л����ĵ�ʱ��Ƭ�϶ࡣ
3 �÷������Ϳͻ���ΪӦ��ʽ��ʵ�ʳ���Ϊȫ˫��ͨ��ģʽ�����ͺͽ���Ҫ�����ֿ���
4 �÷������Ϳͻ���δ����ճ������
��������������������������Ϳͻ��˴��ڵ����⣬Ϊ����������⣬�����ڽ���������������������ƺ͸Ľ�����Ҫ���첽��д�Ľ�����������
��Ȼͬ����д�ķ�ʽҲ�����ŵ㣬����ͻ������������࣬���ҷ����������Բ��ߵĳ���������ʹ��ͬ����д�ķ�ʽ��ʹ��ͬ����д�ܼ򻯱����Ѷȡ�
*/ 
void server(boost::asio::io_context& io_context, unsigned short port)
{
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        socket_ptr socket(new tcp::socket(io_context));
        a.accept(*socket); //������ͻ���ͨ�ŵ�socket
        // ����һ���߳�ȥ������ͻ��˵Ķ�д
        auto t = std::make_shared<std::thread>(session, socket);
        // �ŵ��������ֹ
        thread_set.insert(t);
    }
}

int main()
{

    // ���߳��˳�֮�����˳����߳�
    try {
        boost::asio::io_context ioc;
        server(ioc, 10086);
        for (auto& t : thread_set)
        {
            t->join();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in thread: " << e.what() << "\n" << std::endl;
    }
    return 0;
}