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
public:
	socket_gate(const std::string &path)
		:gate(),
		sock(service),
		ep(path)
	{}

	~socket_gate(){}
	void host()override{
		sock.connect(ep);
	}
	std::vector<uint8_t> get()override{
		boost::asio::streambuf sb;
		get_bytes(sb, 4);
		auto data = sb_to_str(&sb);
		size_t length = std::stoul(std::move(data));
		sb.consume(4);
		get_bytes(sb, length);
		auto sb_data = sb.data();
		return std::vector<uint8_t>(boost::asio::buffers_begin(sb_data),
			boost::asio::buffers_end(sb_data));
	}
	void send(const std::vector<uint8_t> &data)override{

	}
};

int main(){
	const std::string path = "/tmp/sock";
	std::unique_ptr<gate> gt = std::make_unique<socket_gate>(path);
	gt->host();
	auto data = gt->get();
	std::copy(data.begin(), data.end(), std::ostream_iterator<int>(std::cout, " "));
}