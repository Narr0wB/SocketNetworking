from http import client
import subprocess
import socket, sys
import struct
import servermodules as sm
from threading import Thread
from wexpect.wexpect_util import EOF

HOST = '127.0.0.1' # '192.168.43.82'
PORT = 8081 # 2222



sm.comunicazione(HOST, PORT, False)

#IMPORTANTE: usare subprocess.run per i comandi che con popen aspettano la chiusura del processo generato dal comando (e.g. "start ($nomeprogramma)")