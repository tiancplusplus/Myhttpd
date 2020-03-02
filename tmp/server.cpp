#include<boost/asio.hpp>
#include<iostream>
#include<thread>
#include<vector>
#include<memory>



using boost::asio::ip::tcp;

class client_session : public std::enable_shared_from_this<client_session>{
private:
	tcp::socket socket_;
	enum{max_buf_size = 10};
	char data_[max_buf_size];
public:
	client_session(tcp::socket so) : socket_(std::move(so)){}
	void start(){
		do_read();	
	}
	void do_read(){
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_,max_buf_size),
			[this,self](auto error,auto length){
				if(!error){
					std::cout << "length : " << length << std::endl;
					do_write(length);	
				}else {
					std::cout << "read is erro \n";	
				}
			});
	}
	void do_write(std::size_t length){
		auto self(shared_from_this());
		socket_.async_write_some(boost::asio::buffer(data_,length),
			[this,self](auto error,auto length){
				if(!error){
					do_read();	
				}else {
					std::cout << " write is error \n";	
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
