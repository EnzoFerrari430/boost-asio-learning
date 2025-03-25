#pragma once
extern int client_end_point();
extern int server_end_point();
extern int create_tcp_socket();

// 绑定acceptor socket新式
extern int create_acceptor_socket();

// 绑定acceptor socket老式
extern int bind_acceptor_socket();

// 客户端链接端点
extern int connect_to_end();
extern int dns_connect_to_end(); // 链接一个域名

// 服务器接受链接
extern int accept_new_connection();


/*
 asio_2
 buffer结构
*/
// 麻烦的写法
extern void use_const_buffer();
// 使用asio api
extern void use_buffer_str();
extern void use_buffer_array();

/*
 asio_2
 同步读写
*/
//extern void write_to_socket(asio::ip::tcp::socket& sock);

