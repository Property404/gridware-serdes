#include "Socket.h"
#include <stdexcept>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

// Hints struct used in Socket::bind()
// and Socket::connect()
static const addrinfo hints = {
	.ai_flags = 0,
	.ai_family=AF_UNSPEC,
	.ai_socktype=SOCK_STREAM,
	.ai_protocol = 0,
	.ai_addrlen = 0,
	.ai_addr = nullptr,
	.ai_canonname = nullptr,
	.ai_next = nullptr
};

bool Socket::isValid() const noexcept
{
	return (this->socket_fd > 0);
}

void Socket::assertValidity() const
{
	if(!isValid())
		throw std::runtime_error("Socket is uninitialized");
}
void Socket::assertInvalidity() const
{
	if(isValid())
	{
		throw std::runtime_error("Socket is already initialized");
	}
}

size_t Socket::receive(uint8_t* buffer, size_t buffer_size) const
{
	assertValidity();
	if(buffer==nullptr || buffer_size==0)
		throw std::invalid_argument("Inalid arguments to Socket::receive");

	// Total message size
	ssize_t _message_size;
	if(sizeof(_message_size) != recv(this->socket_fd, &_message_size, sizeof(_message_size), 0))
		throw std::runtime_error("Failed to receive size");
	const auto& message_size = _message_size;
	if(message_size>static_cast<ssize_t>(buffer_size))
		throw std::runtime_error("Way too big");

	// Where we're currently receiving in buffer
	// (we receive in small segments)
	uint8_t* subsection = buffer;

	ssize_t bytes_received=0;
	do
	{
		// Receive some amount of bytes into buffer
		const auto length =
			recv(socket_fd, subsection, std::min(message_size-bytes_received,
						static_cast<ssize_t>(0x100)), 0);
		if(length <= 0)
			throw std::runtime_error("Recv returned "+std::to_string(length));
		
		// Shift position we're looking at
		subsection += length;
		bytes_received += length;
	}while(bytes_received<message_size);

	if(bytes_received>message_size)
	{
		throw std::runtime_error("Too many bytes!: "
				+std::to_string(bytes_received)+"(actual) vs "
				+std::to_string(message_size)+"(expected)"
		);
	}

	return message_size;
}

void Socket::send(const uint8_t* buffer, size_t size) const
{
	assertValidity();

	if(buffer == nullptr || size == 0)
		throw std::invalid_argument("Socket::send: null arguments");

	ssize_t bytes_sent;

	// Send size as first few bytes
	bytes_sent = ::send(this->socket_fd, &size, sizeof(size), 0);
	if(bytes_sent != static_cast<ssize_t>(sizeof(size)))
		throw std::runtime_error("Size send failed");

	// Then send actual message
	bytes_sent = ::send(this->socket_fd, buffer, size, 0);
	if(bytes_sent != static_cast<ssize_t>(size))
		throw std::runtime_error("Send failed");
}

int Socket::accept()
{
	assertValidity();

	sockaddr_storage client_address;
	socklen_t address_size = sizeof(client_address);
	const auto new_socket_fd =
		::accept(this->socket_fd, (sockaddr*)&client_address, &address_size);

	return new_socket_fd;
}

void Socket::listen(int backlog) const
{
	assertValidity();
	if(::listen(this->socket_fd, backlog) < 0)
	{
		throw std::runtime_error("Socket failed to listen on TCP port");
	}
}

void Socket::bind(int port)
{
	assertInvalidity();

	int cstatus;// return val of C API functions
	addrinfo* server_info_list = nullptr;

	if((cstatus = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &server_info_list)))
	{
		throw std::runtime_error(std::string("getaddrinfo failed: ")+std::to_string(cstatus));
	}

	// bind to whoever will let us
	auto server_info = server_info_list;
	for ( ; server_info != nullptr; server_info = server_info->ai_next)
	{
		this->socket_fd = 
			socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

		if(! this->isValid())
		{
			std::cerr<<(std::string("Failed to create socket: ")+std::to_string(this->socket_fd));
			continue;
		}

		// Prevent annoying "address in use" error

		// for setting SO_REUSEADDR to true
		int optval = 1;

		if((cstatus = setsockopt(this->socket_fd,
				SOL_SOCKET,
				SO_REUSEADDR,
				&optval,
				sizeof(optval))))
		{
			freeaddrinfo(server_info_list);
			throw std::runtime_error(std::string("setsockopt failed: ")+std::to_string(cstatus));
		}

		
		if(::bind(this->socket_fd, server_info->ai_addr, server_info->ai_addrlen))
		{
			this->close();
			std::cerr<<(std::string("bind() failed. errno=")+std::to_string(errno));
			continue;
		}
		break;
	}
	freeaddrinfo(server_info_list);
	if(server_info == nullptr)
		throw std::runtime_error("Failed to bind() socket");
}

void Socket::connect(std::string host, int port)
{
	assertInvalidity();
	int cstatus;
	addrinfo* server_info_list = nullptr;

	if((cstatus = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &server_info_list)))
	{
		throw std::runtime_error(std::string("getaddrinfo() failed: ")+std::to_string(cstatus));
	}

	auto server_info = server_info_list;
	for ( ; server_info != nullptr; server_info = server_info->ai_next)
	{
		this->socket_fd = 
			socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

		if(!this->isValid())
		{
			std::cerr<<(std::string("Failed to create socket: ")+std::to_string(this->socket_fd));
			continue;
		}

		if(::connect(this->socket_fd, server_info->ai_addr, server_info->ai_addrlen))
		{
			this->close();
			std::cerr<<(std::string("connect() failed. errno=")+std::to_string(errno));
			continue;
		}
		break;
	}
	freeaddrinfo(server_info_list);
	if(server_info == nullptr)
		throw std::runtime_error("Failed to connect() socket");
}

void Socket::close()
{
	assertValidity();
	::close(this->socket_fd);
	this->socket_fd = 0;
}

Socket::Socket(Socket&& rhs)
{
	this->socket_fd = rhs.socket_fd;
	rhs.socket_fd = 0;
}
Socket::~Socket()
{
	if(this->isValid())
		::close(this->socket_fd);
}
