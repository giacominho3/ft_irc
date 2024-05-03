import socket
import threading

server_address = ('localhost', 4444)
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def receive_messages(sock):
    try:
        while True:
            data = sock.recv(1024)
            if not data:
                print("Server closed the connection.")
                break
            print("\n" + data.decode())
    except socket.error as e:
        print("")
    finally:
        print("Closing socket from receiver.")
        sock.close()

try:
    sock.connect(server_address)
    receiver_thread = threading.Thread(target=receive_messages, args=(sock,))
    receiver_thread.start()

    while True:
        message = input()
        sock.sendall((message + '\n').encode())
        if (message == 'QUIT'):
            sock.close()
            exit(0)

finally:
    if sock.fileno() != -1:
        sock.close()
    receiver_thread.join()
