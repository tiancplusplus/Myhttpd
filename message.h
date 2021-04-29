#include<stdio.h>
#include<string.h>
#include<string>
#include<sstream>
#ifndef _MESSAGE_
#define _MESSAGE_

class message{
public:
	enum { max_buf_length = 1024, fir_length = 2, sec_length = 4 };
private:
	char read_buf_[max_buf_length];
	char write_buf_[max_buf_length];
	int flag_ ;
public:
	message(int flag):flag_(flag){}

	void add_flag(){
		flag_ ++ ;	
	}

	int get_read_length(){
		if(flag_ == 1){
			return fir_length;	
		}
		else if(flag_ == 2){
			return sec_length ;	
		}
		else if(flag_ == 3){
			return max_buf_length;	
		}
	}
	int get_flag(){
		return flag_;	
	}
	char* get_read_buf(){
		return read_buf_;	
	}
	char* get_read_next_buf(){
		if(flag_ == 1){
			return read_buf_ + 2;	
		}
		else if(flag_ == 2){
			return read_buf_ + 4;	
		}
	}
	char* get_write_buf(){
		return write_buf_;	
	}
	int  parse_message(){
		if(flag_ == 1){
			if(read_buf_[0] == 0x05){
				return ((int)read_buf_[1]);	
			}	
		}
		else if(flag_ == 2){
			if(read_buf_[0] == 0x05 && read_buf_[3] == 0x01){
				return 6; //continue read 6 byte
			}	
		}else if(flag_ == 3){
				
		}
	}

	size_t parse_write_message(){
		if(flag_ == 1){
			write_buf_[0] = read_buf_[0];
			write_buf_[1] = 0x00;	
			add_flag();
			return 2;
		}
		else if(flag_ == 2){
			memset(write_buf_,0,max_buf_length);
			write_buf_[0] = 0x05;
			write_buf_[3] = 0x01;	
			add_flag();
			return 10;
		}
	}
	std::string get_re_addr(){
		std::stringstream ss ;
		std::string str ;
		ss << (unsigned short)(unsigned char)read_buf_[4] << "." << (unsigned short)(unsigned char)read_buf_[5] 
			<< "." << (unsigned int)(unsigned char)read_buf_[6] << "." << (unsigned int)(unsigned char)read_buf_[7] ;
		ss >> str ;
		return str ;
	}
	std::string get_re_port(){
		unsigned short port = ((unsigned char)read_buf_[8] << 8 | (unsigned char)read_buf_[9]) ;
		std::stringstream ss ;
		std::string str ;
		ss << port;
		ss >> str ;
		return str ;	
	}
};


#endif
//end
