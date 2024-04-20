import socket

# Definizione dell'indirizzo e della porta del server
server_address = ('localhost', 4444)

# Creazione del socket TCP/IP
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connessione al server
    sock.connect(server_address)
    # password = '30101998'
    # sock.sendall(password.encode())
    while True:
        message = input("Enter your message (type 'quit' to exit): ")
        if message == 'quit':
            break
        sock.sendall((message + '\n').encode())
        print('MESSAGE SENT')
    del message

finally:
    # Chiusura del socket
    sock.close()
