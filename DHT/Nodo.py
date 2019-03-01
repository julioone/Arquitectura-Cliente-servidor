import zmq
import random
import string
import sys
import time
import hashlib
import json
import numpy
#atributos de todos los nodos
nodo=json.dumps({'ip':'','port':'','id':'',
	'limiteinferior_keys':'','ip_sucesor':'','files':{}
	})
#cargo en nodo un json
nodo = json.loads(nodo)

context =zmq.Context()
#cada nodo tiene un socket para solicitar y escuchar
socket_escucha = context.socket(zmq.REP)
socket_peticion =context.socket(zmq.REQ)
#funcion que ingresa el id y pone el socketBin en modo escucha
def generar_id(ip,socket):
	global nodo
	identificador =raw_input('ingrese id del nodo')
	nodo['ip'],nodo['port'] = ip.split(':')
	nodo['id']= str(hashlib.sha1(str(identificador)).hexdigest())
	socket.bind('tcp://*:' + nodo['port'])
def solicitud(solicitud,origen,destino,mensaje):
	data = json.dumps({'tipoSolicitud':solicitud,'origen':origen,'destino':destino,'mensaje':mensaje})
	return data
def limite_de_llaves(ip_limite):
	global limite_superior,limite_superior_ip,limite_inferior,limite_inferior_ip
	if ip_limite:
		nodo['limiteinferior_keys'] =''
		nodo['ip_sucesor']=''
		print 'otro nodo se ingregra'
		#solicitud devuelve un json con etiquetas
		#tipoSolicitud,origen,destino,msg
		crear_solicitud_add = solicitud('add',nodo['ip'] + ':' + nodo['port'],ip_limite,
			{'origen':nodo['ip'] + ':' + nodo['port'],'id':nodo['id']})
		solicitud_add = json.loads(crear_solicitud_add)
		socket_peticion.connect('tcp://' + solicitud_add['destino'])
		print json.dumps(solicitud_add, indent=2, sort_keys=True)
		socket_peticion.send(crear_solicitud_add)
		answer=socket_peticion.recv()
		print answer
	else:
		nodo['limiteinferior_keys']=nodo['id']
		nodo['ip_sucesor'] = nodo['ip'] + ':' + nodo['port']

#funcion para ubicar donde va posicionado el nuevo nodo dependiendo de su id
def comparacion_ubicacion(mi_id, id_ultima_key, id_a_ingresar):
	#si la ultima llave es mayor a mi id
	if id_ultima_key > mi_id:
		# entonces si el id del nuevo nodo es mayor a la ultima llave del nodo actual 
		#O id a ingresar es mayor a 0 y id es menor que mi id retorne 0

		if id_a_ingresar > id_ultima_key or (id_a_ingresar >= 0 and id_a_ingresar < mi_id):
			return 0
		else:
			return -1
	#si mi ultima llave es menor a mi id 
	else:
	   #si el id a ingres es menor a mi id y id a ingresar es mayor que mi ultima llave 
	   #O mi id es igual a mi ultima llave retorne 0
	   if (id_a_ingresar <= mi_id and id_a_ingresar > id_ultima_key) or (mi_id == id_ultima_key):
			return 0
	   else:
		   return -1

def add_nodo(nodo,solicitud, socket_peticion):
	print 'un nodo ha solicitado unirse'
	json.dumps(solicitud,indent=2,sort_keys=True)
	check = comparacion_ubicacion(nodo['id'],nodo['limiteinferior_keys'],solicitud['mensaje']['id'])
	str(check)
	if check == 0:
		crear_solicitud_act_anillo = solicitud(
			'actualizar',nodo['ip'] + ':' + nodo['port'], solicitud['mensaje']['origen'],{
			'limiteinferior_keys': nodo['limiteinferior_keys'],
			'ip_sucesor': nodo['ip_sucesor']}
			)
		crear_solicitud_act_anillo_to_json= json.loads(crear_solicitud_act_anillo)
		print 'actualizacion para' + 'tcp://' + crear_solicitud_act_anillo_to_json['destino']
		time.sleep(2)
		socket_peticion.connect('tcp://' + crear_solicitud_act_anillo_to_json['destino'])
		socket_peticion.send(crear_solicitud_act_anillo)
		answer=socket_peticion.recv()
		print answer
		nodo['limiteinferior_keys'] = solicitud['msg']['id']
		nodo['ip_sucesor']= solicitud['msg']['origen']

		print 'INFO nodo:'
		print 'ip:' + nodo['ip'] + ':' + nodo['port']
		print 'id:' + nodo['id']
		print 'ip sucesor' + nodo['ip_sucesor']
		print 'ultima llave que contiene' + nodo['limiteinferior_keys']
		print 'archivos:'
		for file in nodo['files']:
			print file

	elif check == -1:
		crear_solicitud_add = solicitud('add',nodo['ip'] + ':' + nodo['port'],nodo['ip_sucesor'],
			{'origen':solicitud['msg']['origen'],'id':solicitud['msg']['id']})

		solicitud_add_to_json = json.loads(crear_solicitud_add)
		socket_peticion.connect('tcp://' + solicitud_add_to_json['destino'])
		socket_peticion.send(crear_solicitud_add)
		answer = socket_peticion.recv()
		print answer

def actualizar_llaves(nodo,solicitud):
	print json.dumps(solicitud, indent=2, sort_keys=True)
	nodo['limiteinferior_keys'] = solicitud['msg']['limiteinferior_keys']
	nodo['ip_sucesor'] = solicitud['msg']['ip_sucesor']
	print 'se han actualizacdo las llaves ahora este nodo tiene'
	print 'INFO nodo:'
	print 'ip:' + nodo['ip'] + ':' + nodo['port']
	print 'id:' + nodo['id']
	print 'ip sucesor' + nodo['ip_sucesor']
	print 'ultima llave que contiene:' + nodo['limiteinferior_keys']
	print 'archivos:'
	for file in nodo['files']:
		print file

def reconectar_anillo(nodo,info_nodo_a_salir,socket_peticion):
	#si el nodo que queria abandonar su limite de llaves es igual a su id 
	#es porque es el unico nodo
	if nodo['limiteinferior_keys'] == info_nodo_a_salirnodo['id']:
		nodo['limiteinferior_keys'] == info_nodo_a_salirnodo['limiteinferior_keys']
		nodonodo['ip_sucesor'] == info_nodo_a_salirnodo['ip_sucesor']
		print 'INFO nodo:'
		print 'ip:' + nodo['ip'] + ':' + nodo['port']
		print 'id:' + nodo['id']
		print 'ip sucesor' + nodo['ip_sucesor']
		print 'ultima llave que contiene' + nodo['limiteinferior_keys']
		print 'archivos:'
		for file in nodo['files']:
			print file

	else:
		crear_solicitud_reconectar = solicitud('reconectar',nodo['ip'] + ':' + nodo['port'],
			nodo['ip_sucesor'],info_nodo_a_salir)
		solicitud_reconectar_to_json = json.loads(crear_solicitud_reconectar)
		socket_peticion.connect('tcp://' + solicitud_reconectar_to_json['destino'])
		socket_peticion.send(crear_solicitud_reconectar)
		answer = socket_peticion.recv()
		print answer


def transferencia(nodo, solicitud_salir):
	contador=0
	for file in solicitud_salir['msg']:
		nodo['files'][file] = solicitud_salir['msg'][file]
		contador+1
	print 'se han transferido' + contador + 'archivos'




def main():
	global nodo, socket_escucha
	mi_ip = ip_otro_nodo =''
	if len(sys.argv) == 3:
		print 'intentando conectar a'+sys.argv[2]
		ip_otro_nodo=sys.argv[2]
	elif len(sys.argv) == 2:
		print ('primer nodo que contiene todo el anillo')
	else:
		print ('ejecute: python nodo.py <ip:port>')
	#if para el ciclo
	if len(sys.argv) >=2:
		mi_ip = sys.argv[1]
		#lee id y escucha
		generar_id(mi_ip,socket_escucha)
		limite_de_llaves(ip_otro_nodo)
		print ('ip nodo:' +nodo['ip'] + ':' + nodo['port'])
		print ('id:'+nodo['id'])


		try:
			while True:
				print 'esperando solicitud'
				message = socket_escucha.recv()
				message_to_json= json.loads(str(message))
				socket_escucha.connect('tcp://' + message_to_json['origen'])
				if message_to_json['tipoSolicitud'] == 'add':
					print 'nodo se esta anadiendo al anillo'
					socket_escucha.send(nodo['ip'] + ':' + nodo['port'] + 'solicitud add recibida')
					socket_peticion =context.socket(zmq.REQ)
					add_nodo(nodo,message_to_json,socket_peticion)

				#cuando el nodo se va ingresar debajo d eun nodo se envia una slicitud actualizar	
				elif message_to_json['tipoSolicitud'] == "actualizar":
					print 'actualizando llaves de cada nodo'
					socket_peticion = context.socket(zmq.REQ)
					socket.send(nodo['ip'] + ':' + nodo['port'] + 'solicitud de actualizacion recibida')
					actualizar_llaves(nodo,message_to_json)
				elif message_to_json['tipoSolicitud'] == 'reconectar':
					print 'espere, este nodo se esta reconectando a otro nodo en el anillo'
					socket_peticion = context(zmq.REQ)
					socket_peticion.send(nodo['ip'] + ':' + nodo['port'] + 'archivos aceptados')


				elif message_to_json['tipoSolicitud'] == 'salir':
					print 'un nodo quiere salir, consiguiendo datos...'
					socket_peticion = context.socket(zmq.REQ)
					socket_peticion.send(nodo['ip'] + ':' + nodo['port'] + 'solicitud para salir aceptada.')
					print 'pasando los datos y archivos.'
					transferencia(nodo,message_to_json)
				else:
					print message
				print 'salio'

		except KeyboardInterrupt:
			print ''
			if (nodo['ip'] + ':' + nodo['port']) != nodo['ip_sucesor']:
				crear_solicitud_salir = solicitud('salir',nodo['ip'] + ':' + nodo['port'],
					nodo['ip_sucesor'],nodo['files'])
				solicitud_salir_to_json = json.loads(crear_solicitud_salir)
				socket_peticion = context.socket(zmq.REQ)
				socket_peticion.send(crear_solicitud_salir)
				answer = socket_peticion.recv()
				print (answer)
				#se debe guardar la informacion que tenia el nodo que quiere salir en un json
				reconectar_anillo(nodo,{
					'nodo_ip': nodo['ip'],'nodo_port': nodo['port'],
					'nodo_id': nodo['id'],'limiteinferior_keys':nodo['limiteinferior_keys'],
					'ip_sucesor': nodo['ip_sucesor']},socket_peticion)
				print 'este nodo ha cedido su espacio de llaves para salir'
				exit(0)




if __name__ == '__main__':
	main()

