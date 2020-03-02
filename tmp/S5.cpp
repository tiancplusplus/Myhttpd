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
	enum{max_buf_size = 1024};
	char read_buf_[max_buf_size];
	char write_buf_[max_buf_size];
	int flag ;
	int read_length ;
public:
	client_session(tcp::socket so) : socket_(std::move(so)),flag(1){}
	void start(){
		do_read();	
	}
	void do_read(){
		auto self(shared_from_this());
		if(flag == 1) {
			read_length = 2;	
		}else if(flag == 2){
			read_length = 4;	
		}else if(flag == 3){
			read_length = max_buf_size;	
		}
		socket_.async_read_some(boost::asio::buffer(read_buf_,read_length),
			[this,self](auto error,auto& result_length){
				if(!error){
					read_length = result_length;
					size_t write_length = prase_read();
					do_write(write_length);	
				}else {
					std::cout << "read is erro \n";	
					return ;
				}
			});
	}
	size_t prase_read(){
		if(flag == 1){
			if(read_buf_[0] == 0x05 && read_buf_[1] == 0x02){
				socket_.read_some(boost::asio::buffer(read_buf_+2,2));
				write_buf_[0] = 0x05;
				write_buf_[1] = 0x00;	
				flag = 2;
				return 2;
			}
		}else if(flag == 2){
			if(read_buf_[3] == 0x01	&& read_buf_[0] == 0x05){
				socket_.read_some(boost::asio::buffer(read_buf_+4,6));

				memset(write_buf_,0,10);
				write_buf_[0] = 0x05;
				write_buf_[3] = 0x01;	
				flag = 3;
				return 10;
			}
		
		}else if(flag==3){

			return 0;

		}
	}
	void do_write(size_t length){
		auto self(shared_from_this());
		socket_.async_write_some(boost::asio::buffer(write_buf_,length),
			[this,self](auto error,auto length){
				if(!error){
					do_read();	
				}else {
					std::cout << " write is error \n";	
					return ;
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
