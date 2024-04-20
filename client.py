import socket
import threading

# Definizione dell'indirizzo e della porta del server
server_address = ('localhost', 4444)

# Creazione del socket TCP/IP
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def receive_messages(sock):
    try:
        while True:
            data = sock.recv(1024)
            if not data:
                print("Server closed the connection.")
                break
            print("\nReceived:", data.decode())
    finally:
        sock.close()

try:
    # Connessione al server
    sock.connect(server_address)
    
    # Creazione e avvio del thread per la ricezione dei messaggi
    receiver_thread = threading.Thread(target=receive_messages, args=(sock,))
    receiver_thread.start()

    while True:
        message = input("Enter your message (type 'quit' to exit): ")
        if message == 'quit':
            break
        sock.sendall((message + '\n').encode())
        print('MESSAGE SENT')

finally:
    # Chiusura del socket
    sock.close()
    receiver_thread.join()  # Assicurati che il thread della ricezione termini
