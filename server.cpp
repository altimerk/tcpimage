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
#include <thread>

using namespace boost::asio;
using namespace std;
using namespace cv;
using namespace boost::posix_time;
io_service service;
struct talk_to_client;
std::vector<shared_ptr<talk_to_client>> clients;

struct talk_to_client : enable_shared_from_this<talk_to_client> {
    talk_to_client(int numberInQueue) : sock(service) {

        this->numberInQueue = numberInQueue;
    }

    void answer_to_client(int clientsLimit) {
        try {

            if (numberInQueue >= clientsLimit) {
                stop();
            } else
                process_request();
        }
        catch (boost::system::system_error &) {
            stop();
        }
        if (timed_out())
            stop();
    }

    ip::tcp::socket &getSocket() { return sock; }

    bool toBeDeleted() const {
        return deleteFlag;
    }

    bool timed_out() const {
        ptime now = microsec_clock::local_time();
        long long ms = (now - start_time).total_milliseconds();
        //cout << "waiting ms:" << ms << endl;
        return ms > 3000;
    }

    void stop() {
        boost::system::error_code err;
        sock.close();
        deleteFlag = true;
    }

    void process_request() {
        vector<uchar> jpgbytes;
        char lengthPart[sizeof(int)];
//        while ( true)
        if (sock.available()) {
            start_time = microsec_clock::local_time();
            sock.read_some(boost::asio::buffer(lengthPart, sizeof(int)));
            int k;
            k = *(int *) lengthPart;
            char *data = new char[k];
            sock.read_some(boost::asio::buffer(data, k));
            jpgbytes.insert(jpgbytes.end(), &data[0], &data[k]);

            //read string
            sock.read_some(boost::asio::buffer(lengthPart, sizeof(int)));
            k = *(int *) lengthPart;
            char *dataStr = new char[k];
            sock.read_some(boost::asio::buffer(dataStr, k));
            string text(dataStr, dataStr + k);
            Mat image = imdecode(jpgbytes, IMREAD_COLOR);
            putText(image, text, Point(30, 30), FONT_HERSHEY_SIMPLEX, 0.75,
                    Scalar(0, 0, 255), 2);
            //write response
            vector<uchar> vbuf;
            imencode(".jpg", image, vbuf);
            uchar *buf = &vbuf[0];
            int length = vbuf.size();

            const size_t bytesLength = boost::asio::write(sock,
                                                          boost::asio::buffer(&length, sizeof(int)));
            const size_t bytes = boost::asio::write(sock,
                                                    boost::asio::buffer(buf, length));
            //  sock.close();
        }
    }

private:
    bool deleteFlag = false;
    ip::tcp::socket sock;
    int numberInQueue;
    boost::posix_time::ptime start_time;
};

recursive_mutex cs; // thread-safe access to clients vector

void accept_thread(int clientLimit) {
    cout << "acceptor starting" << endl;
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(), 8002));
    while (true) {
        //cout<<"acceptor: count of client:"<<clients.size()<<endl;
        if (clients.size() >= clientLimit) {
            this_thread::sleep_for(1ms);
            continue;
        }
        shared_ptr<talk_to_client> new_ = make_shared<talk_to_client>(clients.size());
        acceptor.accept(new_->getSocket());
        cs.lock();
        clients.push_back(new_);
        cs.unlock();
    }
}

void handle_connections(int clientsLimit) {
    while (true) {
        this_thread::sleep_for(1ms);
        cs.lock();
        //cout << "handler: client of clients:" << clients.size() << endl;
        for (auto b = clients.begin(); b != clients.end(); ++b)
            (*b)->answer_to_client(clientsLimit);
        // erase clients that timed out
        clients.erase(std::remove_if(clients.begin(), clients.end(),
                                     boost::bind(&talk_to_client::timed_out, _1)), clients.end());
        cs.unlock();
    }
}

int main(int argc, char *argv[]) {
    int limitOfClient=1000000;
    if (argc > 1)
        limitOfClient = stoi(argv[1]);

    thread t1(accept_thread, limitOfClient);
    thread t2(handle_connections, limitOfClient);
    t1.join();
    t2.join();
}