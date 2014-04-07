#!/usr/bin/env python2
import Tkinter as tk
import atexit
import sys
class Application(tk.Frame):              
	def __init__(self, master=None):
		tk.Frame.__init__(self, master)   
		self.grid()                      
		self.createWidgets()

	def createWidgets(self):


		self.pitch_label = tk.Label(self, text="pitch")
		self.roll_label = tk.Label(self, text="roll")
		self.yaw_label = tk.Label(self, text="yaw")
		
		self.pitch_label.grid(row=0, column=1)
		self.roll_label.grid(row=0, column=5)
		self.yaw_label.grid(row=0, column=9)

		self.pitch_p_label = tk.Label(self, text="      P")
		self.pitch_i_label = tk.Label(self, text="      I")
		self.pitch_d_label = tk.Label(self, text="      D")

		self.roll_p_label = tk.Label(self, text="      P")
		self.roll_i_label = tk.Label(self, text="      I")
		self.roll_d_label = tk.Label(self, text="      D")

		self.yaw_p_label = tk.Label(self, text="      P")
		self.yaw_i_label = tk.Label(self, text="      I")
		self.yaw_d_label = tk.Label(self, text="      D")

		self.pitch_p_label.grid(row=1, column = 0)
		self.pitch_i_label.grid(row=1, column = 1)
		self.pitch_d_label.grid(row=1, column = 2)
		
		self.roll_p_label.grid(row=1, column = 4)
		self.roll_i_label.grid(row=1, column = 5)
		self.roll_d_label.grid(row=1, column = 6)

		self.yaw_p_label.grid(row=1, column = 8)
		self.yaw_i_label.grid(row=1, column = 9)
		self.yaw_d_label.grid(row=1, column = 10)

		self.pitch_p = tk.IntVar()
		self.pitch_i = tk.IntVar()
		self.pitch_d = tk.IntVar()
		self.roll_p = tk.IntVar()
		self.roll_i = tk.IntVar()
		self.roll_d = tk.IntVar()
		self.yaw_p = tk.IntVar()
		self.yaw_i = tk.IntVar()
		self.yaw_d = tk.IntVar()
		try:
			pid_axes = [[self.pitch_p, self.pitch_i, self.pitch_d], [self.roll_p, self.roll_i, self.roll_d], [self.yaw_p, self.yaw_i, self.yaw_d]]
			
			f = open("pid_values.txt", "r")

			for idx, line in enumerate(f):
				if idx==3:
					break #we don't touch z axis yet
				for idx2, coef in enumerate(map(int, line.split(','))):
					pid_axes[idx][idx2].set(coef)
		except:
			pass
			
			
		
		self.pitch_p_slider = tk.Scale(self, from_=100, to=0, length = 800, variable=self.pitch_p, command=self.update)
		self.pitch_i_slider = tk.Scale(self, from_=100, to=0, length = 800, variable=self.pitch_i, command=self.update)
		self.pitch_d_slider = tk.Scale(self, from_=100, to=0, length = 800, variable=self.pitch_d, command=self.update)

		self.roll_p_slider = tk.Scale(self, from_=100, to=0, length = 800, variable=self.roll_p, command=self.update)
		self.roll_i_slider = tk.Scale(self, from_=100, to=0, length = 800, variable=self.roll_i, command=self.update)
		self.roll_d_slider = tk.Scale(self, from_=100, to=0, length = 800, variable=self.roll_d, command=self.update)
	
		self.yaw_p_slider = tk.Scale(self, from_=100, to=0, length = 800, variable=self.yaw_p, command=self.update)
		self.yaw_i_slider = tk.Scale(self, from_=100, to=0, length = 800, variable=self.yaw_i, command=self.update)
		self.yaw_d_slider = tk.Scale(self, from_=100, to=0, length = 800, variable=self.yaw_d, command=self.update)
			
		self.pitch_p_slider.grid(row=2, column = 0)
		self.pitch_i_slider.grid(row=2, column = 1)
		self.pitch_d_slider.grid(row=2, column = 2)
		
		self.div1 = tk.Label(self, text="               ")
		self.div1.grid(row=1, column=3)
		
		self.roll_p_slider.grid(row=2, column=4)
		self.roll_i_slider.grid(row=2, column=5)
		self.roll_d_slider.grid(row=2, column=6)
		
		self.div2 = tk.Label(self, text="               ")
		self.div2.grid(row=1, column=7)
		
		self.yaw_p_slider.grid(row=2, column=8)
		self.yaw_i_slider.grid(row=2, column=9)
		self.yaw_d_slider.grid(row=2, column=10)

		self.save_button = tk.Button(self, text="save", command=lambda:(self.save("pid_values.txt")))
		self.save_button.grid(row=3, column = 0)
		

		self.quit_button = tk.Button(self, text="quit", command=quit)
		self.quit_button.grid(row=3, column=4)
		
		self.linked = tk.IntVar()
		self.link_pitch_roll = tk.Checkbutton(self, text="link", variable=self.linked, command=lambda: self.update(None))

		self.link_pitch_roll.grid(row=3, column=3)
		


	def update(self, arg):
		if (self.linked.get()):
			self.roll_p.set(self.pitch_p.get())
			self.roll_i.set(self.pitch_i.get())
			self.roll_d.set(self.pitch_d.get())
		
		print("pid: %d %d %d %d %d %d %d %d %d"%(self.pitch_p.get(), self.pitch_i.get(), self.pitch_d.get(),\
							self.roll_p.get(), self.roll_i.get(), self.roll_d.get(),\
							self.yaw_p.get(), self.yaw_i.get(), self.yaw_d.get()))
		sys.stdout.flush()

	def save(self, location="pid_values.txt~"):
		sys.stderr.write("saving to %s\n"%location)
		sys.stderr.flush()
		pid_axes = [[self.pitch_p, self.pitch_i, self.pitch_d], [self.roll_p, self.roll_i, self.roll_d], [self.yaw_p, self.yaw_i, self.yaw_d]]
		f = open(location, "w")

		for pid_axis in pid_axes:
			f.write("%d, %d, %d\n"%( pid_axis[0].get(), pid_axis[1].get(), pid_axis[2].get()))
		f.write("0, 0, 0\n")
	




app = Application()                
atexit.register(app.save)
app.master.title('Sample application')    
app.mainloop()                            

