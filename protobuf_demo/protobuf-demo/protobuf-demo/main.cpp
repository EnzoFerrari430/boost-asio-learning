#include <iostream>

#include "msg.pb.h"

/*

使用dll库需要在工程配置里添加宏定义 PROTOBUF_USE_DLLS

*/
int main()
{
    Book book;
    book.set_name("CPP Programing");
    book.set_pages(100);
    book.set_price(200);
    std::string bookstr;
    book.SerializeToString(&bookstr);

    std::cout << "serialize str is: " << bookstr << std::endl;
    Book book2;
    book2.ParseFromString(bookstr);
    std::cout << "book name: " << book2.name() << "\nbook page: " << book2.pages() << "\nbook price: " << book2.price() << std::endl;

    return 0;
}