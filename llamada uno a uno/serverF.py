import zmq
import sys
import os
import json

def main():
    if len(sys.argv) != 2:
        print("error al llamar el programa")
        exit()

    #chanel={}

    #chanel[cliens[nickname]]=cliens[destina]

    serverport= sys.argv[1]
    context=zmq.Context()
    s= context.socket(zmq.REP)
    s.bind("tcp://*:{}".format(serverport))

    clients= {} #diccionario que contiene los sockets de los clientes

    while True:
        print("esperando conexion")
        msg = s.recv_multipart()

        #msg= s.recv_json()


        if msg[0].decode('ascii') == "login":
            nickname=msg[1].decode('ascii')
            ipclient=msg[2].decode('ascii')
            portclient=msg[3].decode('ascii')

            clientSocket = context.socket(zmq.REQ)
            clientSocket.connect("tcp://{}:{}".format(ipclient,portclient))

            clients[nickname] = clientSocket
            s.send_json({"result":"has iniciado"})
            print (clients)


        elif msg[0].decode('ascii') == "listar":
            s.send_json({"userlist":list(clients.keys())})
            print("lista de usuarios online enviada")



        elif msg[0].decode('ascii') == "audio":
            destino=msg[1].decode('ascii')
            origen=msg[2].decode('ascii')
            audio=msg[3]
            s.send_json({"enviado":"el audio se envio"})

            clients[destino].send_multipart([bytes(origen,'ascii'),bytes(audio)])
            #answer=
            resp_destino= clients[destino].recv_json()
            #mensaje=answer[0].decode('ascii')
            #name_receptor=answer[1].decode('ascii')
            print(resp_destino["visto"])

            #print(answer["recibido"])
            #s.send_json({"enviado":"audio enviado"})


        elif msg[0].decode('ascii') == "hablar":
            destino= msg[1].decode('ascii')
            print("actualizacion de estado destino")
            if destino in clients:
                s.send_json({"respuesta":"on"})

            else:
                s.send_json({"respuesta":"off"})

        elif msg[0].decode('ascii') == "hablando":
            destino= msg[1].decode('ascii')
            origen= msg[2].decode('ascii')
            audio= msg[3]
            s.send_json({"respuesta":"ok"})
            clients[destino].send_multipart([bytes(origen,'ascii'),bytes(audio)])
            resp_destino = clients[destino].recv_json()
            print(resp_destino["visto"])











        '''
        elif msg["op"] == "escribir":
            emisor = msg["emisor"]
            dest=msg["dest"]
            text=msg["mesage"]
            s.send_json({"result":"ok"})

            data={"emisor":emisor,
            "text":text
            }
            clients[dest].send_json(data)
            answer=clients[dest].recv_json()
            print(answer)

        '''







if __name__ == '__main__':
    main()
