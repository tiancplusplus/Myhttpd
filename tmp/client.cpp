#include<iostream>
#include<memory>
#include<boost/asio.hpp>
#include<string>
#include<cstring>

using boost::asio::ip::tcp;


class client{
private:
	tcp::socket socket_;
	tcp::resolver resolver_;
	enum{max_buf_size = 1024};
	char buf[max_buf_size];
	char buf_r[max_buf_size];
	std::string& addr_;
	std::string& port_;
public:
	client(boost::asio::io_context& io, std::string& addr,std::string& port)
		: socket_(io),resolver_(io),addr_(addr),port_(port){}
	
	void start(){
		resolver_.async_resolve(addr_,port_,
			[this](auto& error,tcp::resolver::results_type rt){
				if(!error){
					async_connect(rt);	
				}	
			});
	}
	void async_connect(tcp::resolver::results_type rt){
		boost::asio::async_connect(socket_,rt,
			[this](auto& error,auto& ){
				if(!error){
					do_read();	
				}
			});
	}

	void do_read(){	
		std::cout << "input \n" ;
		std::cin.getline(buf,max_buf_size);
		size_t len = std::strlen(buf);
		boost::asio::write(socket_,boost::asio::buffer(buf,len));
		
		size_t len_r = boost::asio::read(socket_,
				boost::asio::buffer(buf_r,len));
		std::cout << "reply \n" ;
		std::cout.write(buf_r,len_r);
		std::cout << std::endl;
	}
		//boost::asio::connect(socket_,result_type);
};



int main(){
	try{
		boost::asio::io_context io;
		std::string s("127.0.0.1");
		std::string s1("10003");
		client c(io,s,s1);
		c.start();
		io.run();
	
	}catch(std::exception& e){
		std::cerr << "Exception is " << e.what() << std::endl;	
	}
}
