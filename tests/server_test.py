def test_send_receive():
    message = b'echo!'
    sock = connect()
    sock.send(message)
    received = sock.recv(len(message))
    assert(received.decode('utf-8') == message.decode('utf-8'))
    sock.close()
