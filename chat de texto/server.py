#!/usr/bin/env python
import socket
import sys
import collections
import time
import Queue
import threading


from threading import Thread
from SocketServer import ThreadingMixIn

class ClientThread(Thread):

    def __init__(self,socket,ip,port):
        Thread.__init__(self)
        self.socket = socket
        self.ip = ip
        self.port = port
        #print "New thread started"
    def run(self):



        status = 0
        userpresent = 0
        while True:
            self.socket.send(str(TIME_OUT))
            data2 = "successful"
            while userpresent == 0:
                num = 0
                userdata = self.socket.recv(2048)


                if not userdata: break

                line = open('user_pass.txt').readlines()
                for userpass in line:
                    user = userpass.split(" ")
                    if userdata == user[0]:
                        userpresent = 1
                if userpresent == 0:

                    data2 = "login invalido"
                    status = 0
                    print data2
                    self.socket.send(data2)

                    continue
                else:

                    for p in blockusers:
                        print "usuariosbloqueados: " , p
                        val = p.partition(" ")
                        valin = val[2].partition(" ")
                        curtime =time.time()
                        if val[0] == userdata and float(valin[0]) >= curtime - BLOCK_TIME and valin[2] == str(ip):     #Blocktime and ip
                            data2 = " blocked "
                            status = 2

                    for p in curusers:
                        print "curusers:", p
                        if userdata == p:
                            data2 = "same user"
                            status = 1
                            print data2
                    if data2 == " blocked ":

                        self.socket.send(data2)
                        status = 2
                    elif data2 == "same user":
                        self.socket.send(data2)
                        status = 1


            if data2 == "successful":
                self.socket.send(data2)
                passpresent = 0
                while status == 0:
                    passdata = self.socket.recv(2048)
                    validity = userdata + " " + passdata

                    if (validity not in open('user_pass.txt').read()):
                        data2 = "contrasena invalida"
                        print data2
                        num = num + 1
                        if num == 3:
                            status = 2
                            self.socket.send(data2)
                            print "se detuvo"
                            break


                        else:
                            status = 0


                            self.socket.send(data2)

                    else:
                        data2 = "successful"
                        self.socket.send(data2)
                        for p in offlineusers:
                            t = p.partition(" ")
                            if t[0] == userdata:
                                lock.acquire()
                                offlineusers.remove(p)
                                lock.release()

                        lock.acquire()
                        curusers.append(userdata)
                        lock.release()
                        print userdata + "se ha logeado"
                        status = 1  # 0 for offline , 1 for online , 2 for blocked
                        logtime=time.time()
                        fd = self.socket.fileno()
                        userfd = userdata + " " + str(fd)
                        lock.acquire()
                        userfdmap.append(userfd)
                        lock.release()






           # print "[+] thread ready for "+ip+":"+str(port)
            if (status == 2 and num == 3):
                blockuserdata = userdata + " " + str(time.time()) + " " + str(ip)
                blockusers.append(blockuserdata)
                fd = self.socket.fileno()
                lock.acquire()
                del sendqueues[fd]
                lock.release()
                print blockuserdata, "Bloqueado por 60 segundos "
                sys.exit()


            else:

                while True:
                    self.socket.settimeout(TIME_OUT)
                    command = self.socket.recv(2048)
                    if "/" in command:
                        content = command.partition(" ")
                        contentinner = content[2].partition(" ")
                        sendmsg = userdata + "(privado): " + contentinner[2]

                        receiver = contentinner[0]
                        errorflag = 1


                        for z in userfdmap:
                            zi = z.partition(" ")
                            if zi[0] == receiver:
                                receiverfd = int(zi[2])

                                errorflag = 0
                                lock.acquire()
                                sendqueues[receiverfd].put(sendmsg)
                                lock.release()



                        if errorflag == 1:
                            replymsg = "User is offline.  Don't worry , we will get it delivered."     #offline messaging
                            file = open('{0}.txt'.format(receiver),"a+")
                            localtime = time.asctime( time.localtime(time.time()) )
                            sendmsg = sendmsg + " " + "on" + " " + localtime
                            file.write(sendmsg)
                            file.write("\n")
                            file.close()

                        else:

                            replymsg = "enviado"

                        self.socket.send(replymsg)

                    elif command == "salir":
                        curusers.remove(userdata)
                        offlinedata = userdata + " " + str(logtime)
                        lock.acquire()
                        offlineusers.append(offlinedata)
                        lock.release()
                        logoutack = "fin de la sesion"
                        self.socket.send(logoutack)
                        print "[+] usuario se ha desconectado con la  "+ip+":"+str(port)
                        fd = self.socket.fileno()
                        lock.acquire()
                        del sendqueues[fd]
                        userfdmap.remove(userfd)
                        lock.release()
                        sys.exit()

                    elif "#" in command:
                          message = command.partition(" ")
                          messagef = message[2].partition(" ")

                          msg = userdata + "(public): " + messagef[2]
                          lock.acquire()
                          for  q in sendqueues.values():
                              q.put(msg)
                          lock.release()
                          ack = "broadcasted"
                          #self.socket.send(ack)
                    else:
                          error = "comando invalido"
                          self.socket.send(error)#Ciclo para que un usuaeio una vez validad pueda mandar mensajes
        lock.acquire()
        curusers.remove(userdata)
        lock.release()
        offlinedata = userdata + " " + str(logtime)
        lock.acquire()
        offlineusers.append(offlinedata)
        lock.release()
        print "fin de la sesion"
        sys.exit()

class ClientThreadread(Thread):
    def __init__(self,sock):
        Thread.__init__(self)

        self.sock = sock

        #print "New thread for chat relying started"
    def run(self):


         tcpsock2.listen(1)
         (conn2, addr) = tcpsock2.accept()
         welcomemsg = "Bienvenido"
         conn2.send(welcomemsg)
         chat = "initial"
        # print "ind here is"
         #print self.sock.fileno()
         while True:
             for p in userfdmap:           #userfdmap contains mapping between usernames and their socket's file despcriptor which we use as index to access their respective queue
                 if str(self.sock.fileno()) in p:
                     connectionpresent = 1
                 else:
                     connectionpresent = 0         #We will use this to implement other features - no use as of now



             try:
                 chat = sendqueues[self.sock.fileno()].get(False)

                 print chat
                 conn2.send(chat)
             except Queue.Empty:

                 chat = "none"
                 time.sleep(2)
             except KeyError, e:
                 pass

lock = threading.Lock()
global command
command = ""

sendqueues = {}
TCP_IP = '0.0.0.0'
TCP_PORT = int(sys.argv[1])
TCP_PORT2 = 125
BUFFER_SIZE = 20
TIME_OUT = 1800.0
BLOCK_TIME = 60.0



curusers = []
offlineusers = []
blockusers = []
userlog = {}
userfdmap = []


tcpsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcpsock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
#host = socket.gethostname()
tcpsock.bind(('', TCP_PORT))

tcpsock2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tcpsock2.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
tcpsock2.bind(('', TCP_PORT2))


threads = []

while True:
    tcpsock.listen(6)
    print "Esperando clientes que se conecten..."
    (conn, (ip,port)) = tcpsock.accept()
    q = Queue.Queue()
    lock.acquire()


    sendqueues[conn.fileno()] = q
    lock.release()



    newthread = ClientThread(conn,ip,port)
    newthread.daemon = True
    newthread.start()
    newthread2 = ClientThreadread(conn)
    newthread2.daemon = True

    newthread2.start()
    threads.append(newthread)
    threads.append(newthread2)



for t in threads:
    t.join()

    print "Termino"
