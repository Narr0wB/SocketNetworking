from servermodules import mainServer

HOST = '127.0.0.1' # '192.168.43.82'
PORT = 8081 # 2222


if __name__ == "__main__":
    mainServer(HOST, PORT, True)

#IMPORTANTE: usare subprocess.run per i comandi che con popen aspettano la chiusura del processo generato dal comando (e.g. "start ($nomeprogramma)")