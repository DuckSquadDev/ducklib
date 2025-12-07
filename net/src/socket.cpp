#include "net/Socket.h"
#include "net/Net.h"

#include <cassert>
#include <exception>
#include <WS2tcpip.h>

namespace ducklib::net {
Socket::Socket(uint16_t bindPort)
    : socket_handle(INVALID_SOCKET) {
    // Create socket and set options
    socket_handle = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (socket_handle == INVALID_SOCKET)
        net_log_error("Failed to create socket");

    // Bind socket
    sockaddr_in socketAddress{};

    socketAddress.sin_addr.s_addr = INADDR_ANY;
    socketAddress.sin_port = htons(bindPort);
    socketAddress.sin_family = AF_INET;

    if (bind(socket_handle, reinterpret_cast<sockaddr*>(&socketAddress), sizeof(socketAddress)) < 0)
        net_log_error("Failed to bind socket");

    // Get which port was bound
    sockaddr_in boundSocketAddress{};
    int boundSocketAddressSize = sizeof boundSocketAddress;
    int boundNameResult = getsockname(
        socket_handle,
        reinterpret_cast<sockaddr*>(&boundSocketAddress),
        &boundSocketAddressSize);

    if (boundNameResult != 0)
        net_log_error("Failed to get bound address of socket (%d)", WSAGetLastError());

    this->address = Address(boundSocketAddress);

    // Set non-blocking mode
    DWORD nonBlockFlag = 1;
    if (ioctlsocket(socket_handle, FIONBIO, &nonBlockFlag) != 0)
        net_log_error("Failed to set non-blocking mode on socket");
}

Socket::~Socket() {
    assert(socket_handle != INVALID_SOCKET);

    closesocket(socket_handle);
}

auto Socket::get_port() const -> uint16_t {
    assert(socket_handle != INVALID_SOCKET);

    return address.get_port();
}

auto Socket::send(Address to, std::span<const std::byte> data) const -> size_t {
    assert(socket_handle != INVALID_SOCKET);

    auto socketAddress = to.as_sockaddr_in();
    auto sent_bytes = sendto(
        socket_handle,
        reinterpret_cast<const char*>(data.data()),
        static_cast<int>(data.size()),
        0,
        reinterpret_cast<sockaddr*>(&socketAddress),
        sizeof(socketAddress));

    if (sent_bytes == SOCKET_ERROR) {
        net_log_error("Failed to send data over socket");
    }

    return sent_bytes;
}

auto Socket::receive(Address& from, std::span<std::byte> receive_buffer) const -> size_t {
    assert(socket_handle != INVALID_SOCKET);
    assert(&from);

    sockaddr_in socketAddress{};
    int socketAddressSize = sizeof(socketAddress);
    HRESULT received_bytes = recvfrom(
        socket_handle,
        reinterpret_cast<char*>(receive_buffer.data()),
        static_cast<int>(receive_buffer.size()),
        0,
        reinterpret_cast<sockaddr*>(&socketAddress),
        &socketAddressSize);

    // TODO: Check socket address size value?

    // TODO: Propagate this out to the caller
    if (received_bytes == SOCKET_ERROR) {
        int errorCode = WSAGetLastError();

        if (errorCode == WSAEWOULDBLOCK)
            return 0;

        net_log_error("Failed to receive data over socket");
    }

    new(&from) Address(socketAddress);

    return received_bytes;
}
}
