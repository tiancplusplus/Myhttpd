#include<boost/asio.hpp>
#include<iostream>
#include<thread>
#include<vector>
#include<memory>
#include<cstring>
#include<string>



using boost::asio::ip::tcp;

class client_session : public std::enable_shared_from_this<client_session>{
private:
	tcp::socket socket_;
	enum{max_buf_size = 1024,first_read_length = 2};
	char data_[max_buf_size];
	char reply[max_buf_size];
	char something[100] = "hello world\n This is a test \n";
public:
	client_session(tcp::socket so) : socket_(std::move(so)){}
	void start(){
		do_read_1();	
	}
	void do_read_1(){
		auto self(shared_from_this());
		socket_.async_read_some(boost::asio::buffer(data_,2),
			[this,self](auto error,auto length){
				if(!error){
					if((int)data_[0] == 0x05)
					{
						if((int)data_[1] == 0x02)
						{
							socket_.read_some(boost::asio::buffer(data_+2,2));
							//std::cout.write(data_,4) << std::endl;
							std::cout << "success\n";
							reply[0] = data_[0];
							reply[1] = (char)0x00;
							length = 2;
						}else{
							std::cout << "length is not 2 length \n";	
							return ;
						}
					}else{
						std::cout << "0x05 protocol is error\n";	
						return ;
					}
					do_write_1(length);	
				}else {
					std::cout << "read is erro \n";	
				}
			});
	}
	void do_write_1(std::size_t length){
		auto self = shared_from_this();
		socket_.async_write_some(boost::asio::buffer(reply,length),
			[this,self](auto error,auto length){
				if(!error){
					do_read_2();	
				}else {
					std::cout << " write is error \n";	
				}	
			});	
	}
	void do_read_2(){
		auto self = shared_from_this();
		socket_.async_read_some(boost::asio::buffer(data_,4),
			[this,self](auto& error,auto length){
				if(!error){
					if((int)data_[3] == 0x01){
						socket_.read_some(boost::asio::buffer(data_+4,6));
						std::cout << "read ip success \n";	
						memset(&reply,0,max_buf_size);
						reply[0] = data_[0];
						reply[3] = (char)0x01;
						length = 10;
					}
				}else
				{
					return ;	
				}
				do_write_2(length);
			});
	}
	void do_write_2(std::size_t length){
		auto self = shared_from_this();
		socket_.async_write_some(boost::asio::buffer(reply,length),
			[this,self](auto error,auto length){
				if(!error){
					do_read_3();	
				}
			});	
	}
	void do_read_3(){
		size_t len = socket_.read_some(boost::asio::buffer(data_,max_buf_size));
		std::cout.write(data_,len) << std::endl;
		size_t something_length = strlen(something);
		socket_.write_some(boost::asio::buffer(something,something_length));
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
