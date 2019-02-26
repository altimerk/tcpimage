#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include "utils.h"

using namespace std;
using namespace boost::system;
using namespace boost::asio;
using namespace cv;
io_service service;
ip::tcp::endpoint ep( ip::address::from_string("127.0.0.1"), 8002);
int invokeServer(char *buffer,int length,string text){
    boost::asio::io_service ios;
    boost::asio::ip::tcp::socket socket( ios );
    socket.connect(ep);
    const size_t bytesLength = boost::asio::write(socket,
                                                  boost::asio::buffer(&length, sizeof(int) ));

    const size_t bytes = boost::asio::write(socket,
                                            boost::asio::buffer(buffer,length )    );
    //write string
    length = text.size();
    const size_t strRes = boost::asio::write(socket,boost::asio::buffer(&length, sizeof(int) ));



    const size_t strBytes = boost::asio::write(socket,
                                               boost::asio::buffer(text.c_str(),length)    );

    std::cout << "sent image" << bytes << " bytes" << std::endl;
    std::cout << "sent string" << strBytes << " bytes" << std::endl;
    char lengthPart[sizeof(int)];
    //read size of image
    socket.read_some(boost::asio::buffer(lengthPart, 4));
    int k;

    k = *(int*)lengthPart;
    char *data = new char[k];
    vector<uchar> jpgbytes;
    //read image from socket
    socket.read_some(boost::asio::buffer(data, k));
    jpgbytes.insert(jpgbytes.end(), &data[0], &data[k]);
    //decode image from JPEG
    Mat image = imdecode(jpgbytes,IMREAD_COLOR);
    namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
    imshow( "Display window", image );                   // Show our image inside it.
    auto key = waitKey(0);
    destroyAllWindows();
    socket.close();
}