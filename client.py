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
            print("\nReceived:", data.decode())
    except socket.error as e:
        print(f"Socket error: {e}")
    finally:
        print("Closing socket from receiver.")
        sock.close()

try:
    sock.connect(server_address)
    receiver_thread = threading.Thread(target=receive_messages, args=(sock,))
    receiver_thread.start()

    while True:
        message = input("Enter your message (type 'QUIT' to exit): ")
        sock.sendall((message + '\n').encode())
        print('MESSAGE SENT')
        if (message == 'QUIT'):
            sock.close()
            exit(0)

finally:
    # Aggiungi un controllo per evitare di chiudere un socket già chiuso
    if sock.fileno() != -1:
        sock.close()
    receiver_thread.join()
