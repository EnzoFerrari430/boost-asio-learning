#pragma once


const int RECVSIZE = 1024;

/*
MsgNode�� ���ڹ���Ҫ���ͺͽ��յ�����

��������Ա��������Ϣ���׵�ַ �����ܳ��� ��ǰ�Դ�����

Ҫ���͵����ݾ͹��������Ľṹȥ����
*/
class MsgNode
{
    friend class Session;
public:
    // д���
    MsgNode(const char* msg, int total_len);
    // �����
    MsgNode(int total_len);
    ~MsgNode();
private:
    char* _msg{ nullptr };
    int _total_len;
    int _cur_len;
};

