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
Lorem Ipsum is simply dummy text of the printing and typesetting industry.
Lorem Ipsum has been the industry's standard dummy text ever since the 1500s,
when an unknown printer took a galley of type and scrambled it to make a type
specimen book. It has survived not only five centuries.'''
ws = Tk()

ws.title("Información de inicio de Sesión")

canvas= Canvas(ws, width= 700, height= 450, bg="#28A6D8")
#Ventana de bienvenida.
canvas.create_text(30, 50, anchor="w", text="Bienvenido", fill="white", font=('Helvetica 25 bold') )
canvas.create_text(30, 80, anchor="w", text="Usuario: "+output_Usuario, fill="white", font=('Helvetica 12'), justify="left")
canvas.create_text(30, 100, anchor="w", text="Última sesión activa: "+mes+" "+output_array[2] +", hora:" +output_array[3], fill="white", font=('Helvetica 12'), justify="left")
canvas.create_text(30, 150, anchor="w", text=mensaje, fill="white", font=('Helvetica 12'), justify="left")
canvas.pack()
ws.mainloop()
