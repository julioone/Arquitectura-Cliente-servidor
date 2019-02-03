import zmq
import pyaudio
import sys
CHUNK = 1024
FORMAT = pyaudio.paInt16
CHANNELS = 2
RATE = 44100
RECORD_SECONDS = 0.2
import threading

serveraddress=sys.argv[1]
ipclient= sys.argv[2]
portclient=sys.argv[3]
nickname= sys.argv[4]


context=zmq.Context()
serverSocket= context.socket(zmq.REQ)
serverSocket.connect("tcp://{}".format(serveraddress))
clientSocket=context.socket(zmq.REP)
clientSocket.bind("tcp://*:{}".format(portclient))
poller=zmq.Poller()
poller.register(sys.stdin,zmq.POLLIN)
poller.register(clientSocket,zmq.POLLIN)
#funcion que graba un audio y lo pone en una variable data y lo devuelve
def grabar():
	p = pyaudio.PyAudio()

	stream = p.open(format=FORMAT,
	                channels=CHANNELS,
	                rate=RATE,
	                input=True,
	                frames_per_buffer=CHUNK)
	frames = []
	for i in range(0, int(RATE / CHUNK * RECORD_SECONDS)):
		data = stream.read(CHUNK)
		frames.append(data)
	data = b"".join(frames)

	return data

def reproducir(audio):
	p = pyaudio.PyAudio()
	#stream es es el formato de wav y se obtiene del archivo
	stream = p.open(format=FORMAT,
	                channels=CHANNELS,
	                rate=RATE,
	                output=True,
	                frames_per_buffer=CHUNK)
	#data es igual a la lectura de 1024 bits
	#data =audio.readframe(CHUNK)
	#mientras data no sea vacio

	stream.write(audio)

	    #data = audio.readframes(CHUNK)


def procesoenhilo(destino):
	print("el proceso de grabar y enviar se ejecutara en un hilo")
	while True:
		data= grabar()
		serverSocket.send_multipart([bytes("hablando",'ascii'),bytes(destino,'ascii'),bytes(nickname,'ascii'),bytes(data)])
		answer= serverSocket.recv_json()
		if answer["respuesta"]== "ok":
			continue

		else:
			print("algo ocurrio con el otro cliente")
			sys.exit(0)









def main():
	if len(sys.argv) != 5:
		print("llamada del programa incorrecta")
		exit()


	while True:

		activity=dict(poller.poll())

		if clientSocket in activity:

			data=clientSocket.recv_multipart()
			fuente=data[0].decode('ascii')
			nota=data[1]
			#print("hablando en linea con {}".format(fuente))
			reproductor= reproducir(nota)




			clientSocket.send_json({"visto":"mensaje escuchado por el receptor"})










			'''
			stream = p.open(format=p.get_format_from_width(wf.getsampwidth()),
							channels=wf.getnchannels(),
							rate=wf.getframerate(),
							output=True)
			while nota != '':
				stream.write(nota)
				nota=
			'''








		elif sys.stdin.fileno() in activity:


			print("comndo?")
			command= input()
			action, *rest= command.split(' ',1)
			if action == "login":
				'''
				data={
				"op":"login",
				"nick":nickname,
				"clientIP":ipclient,
				"portclient":portclient,
				}'''
				serverSocket.send_multipart([bytes(action,'ascii'),bytes(nickname,'ascii'),bytes(ipclient,'ascii'),bytes(portclient,'ascii')])

				answer=serverSocket.recv_json()


				confirmacion= answer["result"]
				print ("inicio de sesion")
				print(confirmacion)

			'''

			elif action == "escribir":
				dest,*msg = rest[0].split(' ',1)
				data={"op":"audionote",
				"emisor":nickname,
				"dest":dest,
				"mesage":msg[0]
				}

				serverSocket.send_json(data)
				answer=serverSocket.recv()
				print(answer)

			'''

			if action == "listar":
				serverSocket.send_multipart([bytes(action,'ascii')])
				answer=serverSocket.recv_json()
				listaclients= answer["userlist"]
				print ("usuarios online")
				print(listaclients)

			if action =="audio":
				dest,*msg = rest[0].split(' ',1)
				receptor = dest
				emisor =nickname
				print (receptor)
				print("grabando")


				data= grabar()
				print("grabacion terminada Enviando.")
				serverSocket.send_multipart([bytes(action,'ascii'),bytes(receptor,'ascii'),bytes(emisor,'ascii'),bytes(data)])
				answer= serverSocket.recv_json()

				print(answer["enviado"])

			if action == "hablar":
				dest,*msg = rest[0].split(' ',1)
				receptor=dest
				print("confirmado.")

				serverSocket.send_multipart([bytes("hablar",'ascii'),bytes(receptor,'ascii')])
				receptor_linea=serverSocket.recv_json()
				print(receptor_linea["respuesta"])
				'''while receptor_linea["respuesta"] == "on":
					print("entro al ciclo de online")
					serverSocket.send_multipart([bytes("hablar",'ascii'),bytes(dest,'ascii')])
					receptor_linea=serverSocket.recv_json()

					if receptor_linea["respuesta"] == "on":
						while True:
							print("hable.")
							data= grabar()
							serverSocket.send_multipart([bytes("hablando",'ascii'),bytes(receptor,'ascii'),bytes(nickname,'ascii'),bytes(data)])
							llego= serverSocket.recv_json()

					else:
						print("algo paso con el usuario destino y se corto la llamada")
						sys.exit(0)'''

				if receptor_linea["respuesta"] == "on":
					print("el usuario destino esta en linea")
					print("se va generar un hilo para la grabacion")
					hiloparagrabar = threading.Thread(target=procesoenhilo(receptor, ))

				else:
					print("el usuario no esta disponible.")











				#answer=serverSocket.recv_multipart()

				#print(answer[0].decode('ascii'))

			else:
				print("comando no soportado")

if __name__ == '__main__':
	main()
