#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

using namespace boost::asio::ip;

constexpr int kMaxLength = 1024;

class Session {
public:
    explicit Session(boost::asio::io_service& io_service) : socket_(io_service) {};
    tcp::socket& Socket() {
        return socket_;
    }

    void Start() {
        socket_.async_read_some(
                boost::asio::buffer(data_, kMaxLength),
                boost::bind(&Session::HandleRead, this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred));
    }

    void HandleRead(const boost::system::error_code& error, size_t bytes_transferred) {
        if (!error) {
            boost::asio::async_write(socket_, boost::asio::buffer(data_, bytes_transferred),
                                     boost::bind(&Session::HandleWrite, this,
                                                 boost::asio::placeholders::error));
        } else {
            delete this;
        }
    }

    void HandleWrite(const boost::system::error_code& error) {
        if (!error) {
            Start();
        } else {
            delete this;
        }
    }


private:
    tcp::socket socket_;
    char data_[kMaxLength];
};

class Server {
public:
    void NewAsyncAccept() {
        auto new_session = new Session(io_service_);
        acceptor_.async_accept(new_session->Socket(), boost::bind(
                &Server::HandleAccept, this, new_session, boost::asio::placeholders::error));
    }

    Server(boost::asio::io_service& io_service, short port)
            : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
        NewAsyncAccept();
    }

    void HandleAccept(Session* new_session, const boost::system::error_code& error) {
        if (!error) {
            new_session->Start();
            NewAsyncAccept();
        } else {
            delete new_session;
        }
    }

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
};

int main() {
    boost::asio::io_service io_service;
    Server server(io_service, 12345);
    io_service.run();
    return 0;
}
