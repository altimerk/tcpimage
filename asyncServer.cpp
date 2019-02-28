#include <iostream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "utils.h"
using namespace boost::asio;
using namespace std;
using namespace cv;
using namespace boost::posix_time;
#define MEM_FN(x) boost::bind(&talk_to_client::x, shared_from_this())
#define MEM_FN1(x,y) boost::bind(&talk_to_client::x, shared_from_this(),y)
#define MEM_FN2(x,y,z) boost::bind(&talk_to_client::x, shared_from_this(),y,z)

io_service service;
int clientCount=0;
class talk_to_client :public std::enable_shared_from_this<talk_to_client>,boost::noncopyable
{
    //static int clientCount;
public:
    talk_to_client():sock_(service),timer_(service){
        cout<<"change this code"<<endl;
    }

    //typedef std::shared_ptr<talk_to_client> ptr;
    static std::shared_ptr<talk_to_client> init() {
        //std::shared_ptr<talk_to_client> new_ = std::make_shared<talk_to_client>();
        std::shared_ptr<talk_to_client> new_ = std::shared_ptr<talk_to_client>(new talk_to_client);
        //clientCount++;
        return new_;
    }
    typedef boost::system::error_code error_code;


    std::string username() const { return username_; }
    void set_clients_changed() { clients_changed_ = true; }
    ip::tcp::socket & sock() { return sock_; }
    bool timed_out() const
    {
        ptime now = microsec_clock::local_time();
        long long ms = (now - last_ping).total_milliseconds();
        return ms > 5000 ;
    }
	void stop()
	{
		if ( !started_) return;
		started_ = false;
		sock_.close();
		shared_ptr<talk_to_client> self = shared_from_this();
		auto it = std::find(clients.begin(), clients.end(), self);
		clients.erase(it);
		//update_clients_changed();
	}
    void start()
    {
        started_ = true;
        auto s = shared_from_this();
        clients.push_back(s);
        last_ping = boost::posix_time::microsec_clock::local_time();
        do_read(); // first, we wait for client to login
    }

    void do_read()
    {
        char bufLen[sizeof(int)];
        //async_read(sock_, buffer(read_buffer_), MEM_FN2(read_complete,_1,_2), MEM_FN2(on_read,_1,_2));
        //async_read_some(sock_, buffer(chLengthOfImage), MEM_FN2(read_complete,_1,_2), MEM_FN2(on_read,_1,_2));
        //boost::asio::read(sock_, boost::asio::buffer(buf,sizeof(int)), boost::asio::transfer_exactly(sizeof(int)));
        sock_.read_some(boost::asio::buffer(bufLen, sizeof(int)));

        auto k = *(int*)bufLen;
        cout<<"get Length="<<k<<endl;
        char *data = new char[k];

        sock_.read_some(boost::asio::buffer(data, k));
        vector<uchar> jpgbytes;
        jpgbytes.insert(jpgbytes.end(), &data[0], &data[k]);
        //read string
        sock_.read_some(boost::asio::buffer(bufLen, sizeof(int)));
        k = *(int*)bufLen;
        char *dataStr = new char[k];
        sock_.read_some(boost::asio::buffer(dataStr,k));
        string text(dataStr,dataStr+k);
        Mat image = imdecode(jpgbytes,IMREAD_COLOR);
        putText(image, text, Point(30, 30), FONT_HERSHEY_SIMPLEX, 0.75,
                Scalar(0, 0, 255), 2);
        //write response
        vector<uchar> vbuf;
        imencode(".jpg",image,vbuf);
        uchar* buf = &vbuf[0];
        int length = vbuf.size();

        const size_t bytesLength = boost::asio::write(sock_,
                                                      boost::asio::buffer(&length, sizeof(int) ));
        const size_t bytes = boost::asio::write(sock_,
                                                boost::asio::buffer(buf,length )    );
        //sock_.close();
        clientCount--;
        cout<<"client closed. count="<<clientCount<<endl;
//        post_check_ping();
    }

    void on_check_ping()
    {
        cout<<"on_check_ping"<<endl;
        ptime now = microsec_clock::local_time();
        if ( (now - last_ping).total_milliseconds() > 5000) stop();
        last_ping = boost::posix_time::microsec_clock::local_time();
    }
    void post_check_ping()
    {
        timer_.expires_from_now(boost::posix_time::millisec(5000));
        timer_.async_wait( MEM_FN(on_check_ping));
        //timer_.async_wait(boost::bind(&self_type::on_check_ping, shared_from_this()));
    }
    size_t read_complete(const boost::system::error_code & err, size_t bytes)
    {
        // ... as before
    }
    void on_read(const error_code & err, size_t bytes)
    {
        cout<<"on_read"<<endl;
        if ( err) stop();
        if ( !started_ ) return;
        std::string msg(read_buffer_, bytes);
//        if ( msg.find("login ") == 0) on_login(msg);
//        else if ( msg.find("ping") == 0) on_ping();
//        else if ( msg.find("ask_clients") == 0) on_clients();
    }

//    void on_login(const std::string & msg)
//    {
//        std::istringstream in(msg);
//        in >> username_ >> username_;
//        do_write("login ok\n");
//        update_clients_changed();
//    }
//    void on_ping()
//    {
//        do_write(clients_changed_ ? "ping client_list_changed\n" : "ping ok\n");
//        clients_changed_ = false;
//    }
//    void on_clients()
//    {
//        std::string msg;
//        for(std::vector<client_ptr>::const_iterator b =clients.begin(),e =clients.end(); b != e; ++b)
//            msg += (*b)->username() + " ";
//        do_write("clients " + msg + "\n");
//    }
//    void do_write(const std::string & msg)
//    {
//        if ( !started_ )
//            return;
//        std::copy(msg.begin(), msg.end(), write_buffer_);
//        sock_.async_write_some( buffer(write_buffer_, msg.size()), MEM_FN2(on_write,_1,_2));
//    }
//    void on_write(const error_code & err, size_t bytes) {
//        do_read();
//    }
//    void update_clients_changed(){
//        for(auto &client :clients){
//            client->set_clients_changed();
//        }
//        clients_changed_ = true;
//    }
    int getClientCount(){
        return clients.size();
    }
private:

    ip::tcp::socket sock_;
    typedef std::shared_ptr<talk_to_client> client_ptr;
    std::vector<client_ptr> clients;
    //enum { max_msg = 1024 };
    char *read_buffer_;//[max_msg];
    char *write_buffer_;//[max_msg];
    bool started_;
    std::string username_;
    deadline_timer timer_;
    boost::posix_time::ptime last_ping;
    bool clients_changed_;

};

ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8002));
void handle_accept(shared_ptr<talk_to_client> client, const error_code & err)
{
    cout<<"clientCount start="<<clientCount<<endl;
    clientCount++;
    client->start();
    shared_ptr<talk_to_client> new_client = talk_to_client::init();
    cout<<"use_count:"<<new_client.use_count()<<endl;
    cout<<"clientCount end="<<clientCount<<endl;
    cout<<"count of client:"<<new_client->getClientCount()<<endl;
    acceptor.async_accept(new_client->sock(),
                          boost::bind(handle_accept,new_client,_1));
}
int main(int argc, char* argv[])
{
    shared_ptr<talk_to_client> client = talk_to_client::init();
    acceptor.async_accept(client->sock(), boost::bind(handle_accept,client,_1));
    service.run();
}