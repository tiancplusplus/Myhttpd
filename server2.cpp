#include<boost/asio.hpp>
#include<iostream>
#include<thread>
#include<vector>
#include<memory>
#include"message.h"


using boost::asio::ip::tcp;

class client_session : public std::enable_shared_from_this<client_session>{
private:
	tcp::socket socket_;  //client socket
	tcp::socket re_socket_ ;//remote cocket
	tcp::resolver resolver_ ; //resolve remote address
	message msg_;
	std::string re_addr_; //remote address
	std::string re_port_; //remote port
public:
	client_session(tcp::socket so) : resolver_(so.get_executor()),re_socket_(so.get_executor()),socket_(std::move(so)),msg_(1){}
	void start(){
		do_read();
	}
	void do_read(){ //de read client socket
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(msg_.get_read_buf(),msg_.get_read_length()),
			[this,self](auto error,auto length){
				if(!error){
					if(msg_.get_flag() == 1 ){ //socks5 protocol first message
						int next_read_length = msg_.parse_message();
						socket_.read_some(boost::asio::buffer(msg_.get_read_next_buf(),next_read_length));
						size_t write_length = msg_.parse_write_message();
						do_write(write_length);
					}
					else if(msg_.get_flag() == 2){ //socks5 seconed message
						int next_read_length = msg_.parse_message();
						socket_.read_some(boost::asio::buffer(msg_.get_read_next_buf(),next_read_length));
						//start built connect remote
						re_addr_ = msg_.get_re_addr();
						re_port_ = msg_.get_re_port();
						std::cout << "Re addr and port is :" << re_addr_ << " " << re_port_ << std::endl;
						boost::asio::connect(re_socket_, resolver_.resolve(re_addr_,re_port_));
						do_read_re_write_cl(); //read remote socket and send client socket

						size_t write_length = msg_.parse_write_message();
						do_write(write_length);
						//std::cout << "second handle is success \n";
					}
					else if(msg_.get_flag() == 3){
						do_write_re(length);
						//std::cout << "third handle is fail \n";
					}
				}else {
					socket_.close();//再关闭客户端socket
					std::cout << "client error socket close\n";
					if(re_socket_.is_open()){
						re_socket_.cancel();//取消远端socket注册的所有事件
						re_socket_.close();//关闭远端socket
						std::cout << "client error so close re_socket_\n";
					}
				}
			});
	}
	void do_read_re_write_cl(){
		auto self(shared_from_this());
		re_socket_.async_read_some(boost::asio::buffer(msg_.get_write_buf(),message::max_buf_length),
				[this,self](auto error,auto length){
					if(!error){
						socket_.write_some(boost::asio::buffer(msg_.get_write_buf(),length));
						do_read_re_write_cl();	
					}
					else{//读取远端socket失败后，关闭远端socket，取消客户端socket注册的所有事件
						//再关闭客户端socket
						re_socket_.close();				
						std::cout << "re error re_socket close\n";
						if(socket_.is_open()){
							socket_.cancel();
							socket_.close();
							std::cout << "re error so close client socket\n";
						}
					}
				});
	}
	void do_write_re(std::size_t length){ //do write remote
		re_socket_.write_some(boost::asio::buffer(msg_.get_read_buf(),length));
		//std::cout.write(msg_.get_read_buf(),length);
		do_read();
	}
	
	void do_write(std::size_t length){
		auto self(shared_from_this());
		socket_.async_write_some(boost::asio::buffer(msg_.get_write_buf(),length),
			[this,self](auto error,auto length){
				if(!error){
					do_read();
				}else {
					socket_.close();
					std::cout << " Write to client is error \n";
				}
			});	
	}
};

class server{
private:
	tcp::acceptor acceptor_ ;
	//std::vector<client_session> client_sessions ;
public:
	server(boost::asio::io_context& io ,short port)
		:acceptor_(io,tcp::endpoint(tcp::v4(),port))
	{	 
	}
	void start(){
		acceptor_.async_accept([this](auto error,tcp::socket so){
			if(!error){
				std::make_shared<client_session>(std::move(so)) -> start();
			}
			start();
		});
	}

};

int main(){
	try{
		boost::asio::io_context io;
		server s(io,10003);
		s.start();
		io.run();
	}catch(std::exception& e){
		std::cerr << "Exception is " << e.what() << std::endl;	
	}

	return 0;
}
