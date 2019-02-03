import zmq
import sys
import os#libreria que maneja el sistema operativo desde python
import math #libreria usada para poder utilizar la funcion math.ceil() funcion techo



#funcion cargar archivos que recibe como parametro 
#un directorio y retorna todos los nombres de archivos
#que se encuentran en ese directorio

def loadFiles(path):
    files = {}
    dataDir = os.fsencode(path)
    for file in os.listdir(dataDir):
        filename = os.fsdecode(file)
        print ("loadgin {}: ".format(filename))
        files[filename]=file
    return files

def main():
    if len (sys.argv) != 3:
        print ("Error calling the program")
        exit()


    port = sys.argv[1]#puerto
    filesDir = sys.argv[2]

    context = zmq.Context()
    s = context.socket(zmq.REP)#contexto para responder con REP

    s.bind("tcp://*:{}".format(port))

    files = loadFiles (filesDir)

    while True:
        print("waiting for connection dos")
        msg = s.recv_json()
        if msg ["op"] == "list":
            s.send_json({"files":list(files.keys())})
        elif msg ["op"] == "download":
            filename = msg["file"]
            with open (filesDir+"/"+filename,"rb") as input:
                data = input.read()
                s.send(data)

        elif msg ["op"] == "downloadP":
            filename = msg["file"]
            tamano = os.path.getsize(filesDir+"/"+filename)#se obtiene el tamaño del archivo solicitado
            cant_bloq= math.ceil(tamano/(1024*1024))#se aproxima con la funcion techo
            
            s.send_json(cant_bloq)

        elif msg ["op"] == "download_empieza":
            mega = 1024*1024
            filename = msg["file"]
            bloque_solicitado = msg["num_sol_bloq"]

            with open(filesDir+"/"+filename,"rb") as input:
                input.seek(bloque_solicitado*mega)#ubica en que parte dearchivo empezar a leer.
                data = input.read (1024*1024)#lee un mega
                s.send(data)# se envia la parte











        else:
            print ("Operation not Implemented")

if __name__ == '__main__':
    main()
