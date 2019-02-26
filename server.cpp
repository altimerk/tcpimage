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
using namespace boost::asio;
using namespace std;
using namespace cv;
io_service service;
void handle_connections()
{
    ip::tcp::acceptor acceptor(service, ip::tcp::endpoint(ip::tcp::v4(),8002));
    vector<uchar> jpgbytes;
    char lengthPart[4];
    while ( true)
    {
        ip::tcp::socket sock(service);
        acceptor.accept(sock);
        sock.read_some(boost::asio::buffer(lengthPart, 4));
        int k;
        k = *(int*)lengthPart;
        char *data = new char[k];
            sock.read_some(boost::asio::buffer(data, k));
          //  counter+=bytes;
            jpgbytes.insert(jpgbytes.end(), &data[0], &data[k]);

            //read string
        sock.read_some(boost::asio::buffer(lengthPart, 4));
        k = *(int*)lengthPart;
        char *dataStr = new char[k];
        sock.read_some(boost::asio::buffer(dataStr,k));
        string text(dataStr,dataStr+k);
        Mat image = imdecode(jpgbytes,IMREAD_COLOR);
        putText(image, text, Point(30, 30), FONT_HERSHEY_SIMPLEX, 0.75,
                Scalar(0, 0, 255), 2);

        //write response
        vector<uchar> vbuf;
        imencode(".jpg",image,vbuf);
        uchar* buf = &vbuf[0];
        int length = vbuf.size();

        const size_t bytesLength = boost::asio::write(sock,
                                                      boost::asio::buffer(&length, sizeof(int) ));

        const size_t bytes = boost::asio::write(sock,
                                                boost::asio::buffer(buf,length )    );
        cout<<"writen "<<bytes<<"bytes."<<endl;
        sock.close();
    }
}

int main(int argc, char* argv[])
{
    handle_connections();
}