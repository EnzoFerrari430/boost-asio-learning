#pragma once
extern int client_end_point();
extern int server_end_point();
extern int create_tcp_socket();

// ��acceptor socket��ʽ
extern int create_acceptor_socket();

// ��acceptor socket��ʽ
extern int bind_acceptor_socket();

// �ͻ������Ӷ˵�
extern int connect_to_end();
extern int dns_connect_to_end(); // ����һ������

// ��������������
extern int accept_new_connection();


/*
 asio_2
 buffer�ṹ
*/
// �鷳��д��
extern void use_const_buffer();
// ʹ��asio api
extern void use_buffer_str();
extern void use_buffer_array();

/*
 asio_2
 ͬ����д
*/
//extern void write_to_socket(asio::ip::tcp::socket& sock);

