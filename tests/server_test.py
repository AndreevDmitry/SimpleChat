import socket

HOST = "localhost"
PORT = 2115

def connect():
    sock = socket.socket(socket.AF_INET,socket.SOCK_DGRAM)
    sock.connect((HOST, PORT))
    return sock

def test_send_receive():
    message = b'echo!'
    sock = connect()
    sock.send(message)
    received = sock.recv(len(message))
    assert(received.decode('utf-8') == message.decode('utf-8'))
    sock.close()
