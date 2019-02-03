import zmq	#libreria socket
import sys  #libreria de la terminal

def main():
    if len(sys.argv) < 4:#verifica si el programase llamo de manera correcta en la terminal
        print ("error csalling the program!!")
        exit()

    serverIp= sys.argv[1] #ip del servidor	
    serverPort =sys.argv[2] #puerto del servidor
    operation= sys.argv[3] #operacion que el cliente quiere ejecutar
    


    context = zmq.Context()#contexto

    s= context.socket(zmq.REQ)#se crea un socket 
    s.connect("tcp://{}:{}".format(serverIp,serverPort))#aqui se conecta al servidor con el protocolo TCP

    if operation == "list":#operacion listar
        s.send_json({"op":"list"})
        files = s.recv_json()
        print (files)

    elif operation == "download": #operacion descargar 
    	namefile = sys.argv[4]  #tomo de la terminal el nombre del archivo que quiero descargar
    	s.send_json({"op":"download","file":namefile})#con json puedo enviar datos con sus etiquetas 
    	data = s.recv()#cuando recibe una respuesta del servidor se guarda en la variable data
    	with open (namefile,"wb") as output:#funcion que permite crear un archivo en el directorio
    		output.write(data)#en output se escribe lo que nos envio el servidor

    elif operation == "downloadP": #operacion descargar por partes
        namefile = sys.argv[4]
        mega = 1024*1024# tamano de una mega
        s.send_json({"op":"downloadP","file":namefile})
        data = s.recv_json() #lo que nos responde el server lo guardamos en data en este caso viene 
        #la cantidad de bloques que el servidor calculo 
        print(data)
        cant_bloq = data
        #un ciclo que va desde 0 hasta la cantidad de bloques del archivo +1
        for i in range(0,cant_bloq+1):
                    if i == cant_bloq+1:#si i es igual a cantidad bloques +1
                        print("final Descarga")
                        
                        #break
                    elif i != cant_bloq+1:#si i es diferente de cantidad bloques +1
                        print("entro")
                        #ahora se envia un mensaje que reenvia la parte que requiere el cliente en este caso
                        #el valor de i este valor el servidor la usara para ubicar que parte del archivo leer y enviar
                        s.send_json({"op":"download_empieza","file":namefile,"num_sol_bloq":i})
                        data = s.recv()#aqui recibe la parte que solicito el cliente


                        #con la funcion with open abrimos un archivo y escribimos el parametro "ab"
                        #que sirve para abrir el archivo en una parte especifica y no solo al inicio

                        with open(namefile,"ab") as output:
                        	output.seek(i*mega)
                        	output.write(data)
        print("archivo descargado por partes Completado.")



        #with open(namefile, "ab") as output:

        #partes =s.recv()
        #print ("se empezaran a descargar {} partes".format(partes))



        


        print("archivo guardado")


        

        
            

        
        

    else:
        print ("unsupported operation")

if __name__ == '__main__':
    main()
