#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <memory>

class gate{
public:
	gate(){}
	virtual ~gate(){}
	virtual void host()=0;
	virtual std::vector<uint8_t> get()=0;
	virtual void send(const std::vector<uint8_t> &data)=0;
};

class socket_gate:public gate{
	boost::asio::io_service service;
	boost::asio::local::stream_protocol::endpoint ep;
	boost::asio::local::stream_protocol::socket sock;

	inline std::string sb_to_str(boost::asio::streambuf *sb)const{
		auto sb_data = sb->data();
		return std::string(boost::asio::buffers_begin(sb_data),
			boost::asio::buffers_end(sb_data));
	}

	inline void get_bytes(boost::asio::streambuf &sb, const size_t &num){
		sb.prepare(num);
		boost::asio::read(sock, sb, boost::asio::transfer_exactly(num));
	}
	int str_to_int(const std::string &str){
		int data=0;
		memcpy(&data, &str[0], 4);
		return std::move(data);
	}
	std::string int_to_str(int i){
		std::string data;
		data.resize(4);
		memcpy(&data[0], &i, 4);
		return std::move(data);
	}
public:
	socket_gate(const std::string &path)
		:gate(),
		sock(service),
		ep(path)
	{}

	~socket_gate(){
		sock.close();
	}
	void host()override{
		sock.connect(ep);
	}
	std::vector<uint8_t> get()override{
		boost::asio::streambuf sb;
		get_bytes(sb, 4);
		int length = str_to_int(
			std::string(boost::asio::buffers_begin(sb.data()),
				boost::asio::buffers_end(sb.data()))
		);
		std::cout<<"getting "<<length<<" bytes\n";
		sb.consume(4);
		get_bytes(sb, length);
		auto sb_data = sb.data();
		return std::vector<uint8_t>(boost::asio::buffers_begin(sb_data),
			boost::asio::buffers_end(sb_data));
	}
	void send(const std::vector<uint8_t> &data)override{
		auto len = int_to_str((int)data.size());
		std::cout<<"sending "<<data.size()<<" bytes\n";
		std::string data_str(data.begin(), data.end());
		boost::asio::write(sock, boost::asio::buffer(len));
		boost::asio::write(sock, boost::asio::buffer(data_str));
	}
};

int main(int argc, char*argv[]){
	size_t num = 10;
	if(argc > 1){
		num = std::atoi(argv[1]);
	}
	const std::string path = "/tmp/sock";
	while(num>0){
		try{
			std::unique_ptr<gate> gt = std::make_unique<socket_gate>(path);
			gt->host();
			auto data = gt->get();
			std::cout << "data: " <<data.size()<<" bytes\n";
			gt->send(data);
			num--;
		}catch(...){
			//std::cerr<<"WARNING, a error occured\n";
		}
	}
}