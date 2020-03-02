#include<iostream>
#include<memory>
#include<boost/asio.hpp>
#include"message.h"

using boost::asio::ip::tcp;

class session : public std::enable_shared_from_this<session>{
private:
	tcp::socket socket_;
	message msg_;
	tcp::socket re_socket_;
public:
	session(tcp::socket client,tcp::socket re_socket) :
		socket_(std::move(client)) ,re_socket_(std::move(re_socket)),msg_(1) {}
	void start(){
		do_read_client_to_re();
		do_read_re_to_client();
	}
	void do_read_client_to_re(){
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(msg_.get_read_buf(),message::max_buf_length),
				[this,self](auto& error,auto length){
					if(!error){
						re_socket_.write_some(boost::asio::buffer(msg_.get_read_buf(),length));
						//std::cout.write(msg_.get_read_buf(),length) << std::endl;
						do_read_client_to_re();
					}
					else{
						std::cout << "client socket is close\n";
						socket_.close();	
						if(re_socket_.is_open()){
						re_socket_.cancel();
						re_socket_.close();
						}
						//如果客户端要断开连接，则应该同时断开服务端的连接，并把服务端socket上注册是时间全部丢弃。
					}
				});
	}
	void do_read_re_to_client(){
		auto self(shared_from_this());	
		re_socket_.async_read_some(boost::asio::buffer(msg_.get_write_buf(),message::max_buf_length),
				[this,self](auto& error,auto length){
					if(!error){
						socket_.write_some(boost::asio::buffer(msg_.get_write_buf(),length));
						do_read_re_to_client();	
					}
					else {
						std::cout << "re_socket is close\n";
						re_socket_.close();	
						if(socket_.is_open()){
						socket_.cancel();
						socket_.close();
						}
						//如果服务端已经断开连接，则将客户端的连接断开，并把客户端socket上注册的事件全部丢弃。
					}
				});
	}

};

class server{
private:
	tcp::acceptor acceptor_;
	tcp::resolver resolver_;
	tcp::socket re_socket_;
	std::string re_addr_;
	std::string re_port_;
	
public:
	server(boost::asio::io_context& io, short port, std::string re_addr, std::string re_port)
		:acceptor_(io,tcp::endpoint(tcp::v4(),port)), resolver_(io),
		re_socket_(io), re_addr_(re_addr), re_port_(re_port)
	{
	}
	void start(){
		acceptor_.async_accept(
			[this](auto error,tcp::socket client){
				if(!error){
					boost::asio::connect(re_socket_,resolver_.resolve(re_addr_,re_port_));
					std::make_shared<session>(std::move(client),std::move(re_socket_))->start();	
				}
				start();
			});	
	}
};

int main(){
	try{
		std::string re_addr_("45.220.67.49");
		std::string re_port_("10003");
		boost::asio::io_context io;
		server s(io,10004,re_addr_,re_port_);
		s.start();
		io.run();
			
	}catch(std::exception& e){
		std::cerr << "Exception is " << e.what() << std::endl;	
	}
	return 0;
}
