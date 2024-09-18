from tkinter import *
from tkinter import messagebox
import subprocess


command = "last | grep $( echo $( id -nu ) | cut -c -8 ) | awk 'NR==2{print $1,$5,$6,$7}'"
result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, text=True)
output_array = result.stdout.split()

result2 = subprocess.run(['id','-nu'], stdout=subprocess.PIPE, text=True)
output_Usuario = result2.stdout.strip()

mes = ""

if output_array[1] == "Jan" : mes = "Enero"
if output_array[1] == "Feb" : mes = "Febrero"
if output_array[1] == "Mar" : mes = "Marzo"
if output_array[1] == "Apr" : mes = "Abril"
if output_array[1] == "May" : mes = "Mayo"
if output_array[1] == "Jun" : mes = "Junio"
if output_array[1] == "Jul" : mes = "Julio"
if output_array[1] == "Aug" : mes = "Agosto"
if output_array[1] == "Sep" : mes = "Septiembre"
if output_array[1] == "Oct" : mes = "Octubre"
if output_array[1] == "Nov" : mes = "Noviembre"
if output_array[1] == "Dec" : mes = "Diciembre"


mensaje = '''
Este equipo se encuentra protegido por los mecanismos de seguridad definidos en el Sistema de Gestión de la Seguridad de la Información implantado por EDUCAMADRID en cumplimiento a lo dispuesto en la Ley Orgánica 3/2018, de 5 de diciembre, de Protección de Datos Personales y garantía de los derechos digitales, y en el Real Decreto 311/2022, de 3 de mayo, por el que regula el Esquema Nacional de Seguridad.

Con la finalidad de lograr el cumplimiento de la normativa indicada,todas las actividades pueden ser supervisadas para el análisis y/o investigación de uso indebido o no autorizado, con plenas garantías del derecho a la intimidad, honor e imagen del usuario.

El acceso a este equipo está limitado a personas autorizadas y solo para la finalidad prevista. Queda prohibido todo uso no autorizado.'''

ws = Tk()

ws.title("Información de inicio de Sesión")

canvas= Canvas(ws, width= 710, height= 450, bg="#28A6D8")
#Ventana de bienvenida.
canvas.create_text(30, 50, anchor="w", text="Bienvenido", fill="white", font=('Helvetica 25 bold') )
canvas.create_text(30, 80, anchor="w", text="Usuario: "+output_Usuario, fill="white", font=('Helvetica 12'), justify="left")
canvas.create_text(30, 100, anchor="w", text="Última sesión activa: "+mes+" "+output_array[2] +", hora:" +output_array[3], fill="white", font=('Helvetica 12'), justify="left")
canvas.create_text(30, 240, anchor="w", text=mensaje, fill="white", font=('Helvetica 12'), justify="left", width=650)
canvas.pack()
ws.mainloop()
