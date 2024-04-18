import socket

# Definizione dell'indirizzo e della porta del server
server_address = ('localhost', 4444)
# password = input("Inserisci la password per connetterti al ServerIRC: ")
password = '30101998'

# Creazione del socket TCP/IP
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
    # Connessione al server
    sock.connect(server_address)

    # Invio della password al server
    sock.sendall(password.strip('\n').strip().encode())

    # Invio del messaggio al server
    message = "Ciao, sono un messaggio dal client!"
    sock.sendall(message.encode())

finally:
    # Chiusura del socket
    sock.close()
