import sys
import csv
import tkinter as tk
from tkinter import ttk, filedialog
import socket
import threading
import serial
import serial.tools.list_ports

APPNAME = "ESP32-C6-MPU6050 V1.0"
RATES = [10, 30, 50, 60, 100, 200, 250, 500, 1000, 2000, 5000] # ms

class DataClient:
    def __init__(self, host=None, port=None, serial_port=None, baudrate=115200):
        self.host = host
        self.port = port
        self.serial_port = serial_port
        self.baudrate = baudrate
        self.client = None
        self.connected = False
        self.lock = threading.Lock()

    def connect_serial(self):
        try:
            self.client = serial.Serial(self.serial_port, self.baudrate, timeout=3)
            self.connected = True
        except Exception as e:
            print(f"Serial connection failed: {e}")

    def list_ports(self):
        return [port.device for port in serial.tools.list_ports.comports()]

    def send_command(self, command):
        with self.lock:
            if not self.connected:
                print("Not Connected")
                return
            try:
                self.client.write((command + '\n').encode())
            except Exception as e:
                self.connected = False
                print(f"Send failed: {e}")

    def receive_data(self, callback):
        if self.connected and self.client:
            while self.connected:
                try:
                    data = self.client.readline().decode().strip()

                    if not data:
                        continue
                    
                    callback(data)  
                    
                except Exception as e:
                    self.connected = False
                    print(f"Not connected: {e}")
                    break


    def close(self):
        if self.client:
            try:
                self.client.close()
            except:
                print("Cannot Close the Interface")
            self.connected = False
            
class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(APPNAME)
        self.geometry("1000x740")

        self.data_client = DataClient()
        self.rate_var    = tk.IntVar(value=RATES[0])
        self.port_var    = tk.StringVar()

        self.is_running     = False
        self.record_file    = None
        self.record_writer  = None

        self.max_points     = 100

        self.accel_range = 20      
        self.veloc_range = 100    
        self.displ_range = 10     

        self.plot_width     = 1200
        self.plot_height    = 300

        # Init buffers for all 3 plot types
        self.accel_x = [0] * self.max_points
        self.accel_y = [0] * self.max_points
        self.accel_z = [0] * self.max_points

        self.vel_x = [0] * self.max_points
        self.vel_y = [0] * self.max_points
        self.vel_z = [0] * self.max_points

        self.disp_x = [0] * self.max_points
        self.disp_y = [0] * self.max_points
        self.disp_z = [0] * self.max_points

        self.bind("<Configure>", self.on_resize)
        self.protocol("WM_DELETE_WINDOW", self.on_closing)

        self.create_ui()
        self.draw_static_elements()

    def create_ui(self):
        tools_frame = ttk.Frame(self)
        tools_frame.pack(fill=tk.X, padx=10, pady=5, side=tk.TOP)
        
        port_frame = ttk.Frame(tools_frame)
        port_frame.pack(side=tk.LEFT)

        self.status_label = ttk.Label(port_frame, text="Disconnected", foreground="red")
        self.status_label.pack(side=tk.LEFT, padx=10)

        ttk.Label(port_frame, text="Port:").pack(side=tk.LEFT)
        self.port_box = ttk.Combobox(port_frame, textvariable=self.port_var, width=10)
        self.port_box.pack(side=tk.LEFT)
        self.update_ports()

        btn_frame = ttk.Frame(tools_frame)
        btn_frame.pack(side=tk.LEFT, padx=10)

        self.connect_button = ttk.Button(btn_frame, text="CONNECT", command=self.toggle_connect_device)
        self.connect_button.pack(side=tk.LEFT, padx=5)

        self.start_button = ttk.Button(btn_frame, text="START", command=self.toggle_start_stop)
        self.start_button.pack(side=tk.LEFT, padx=5)

        self.reset_button = ttk.Button(btn_frame, text="RESET", command=self.toggle_reset)
        self.reset_button.pack(side=tk.LEFT, padx=5)

        config_frame = ttk.Frame(self)
        config_frame.pack(fill=tk.X, padx=10)

        ttk.Label(config_frame, text="Rate:").pack(side=tk.LEFT, padx=5)
        self.rate_combo = ttk.Combobox(config_frame, values=RATES, textvariable=self.rate_var, width=6, state="readonly")
        self.rate_combo.pack(side=tk.LEFT)
        self.rate_combo.current(0)

        ttk.Label(config_frame, text="accel_noise_floor:").pack(side=tk.LEFT, padx=5)
        self.noise_entry = ttk.Entry(config_frame, width=10)
        self.noise_entry.insert(0, "0.5")
        self.noise_entry.pack(side=tk.LEFT)

        ttk.Label(config_frame, text="a,g,d,s:").pack(side=tk.LEFT, padx=5)
        self.mpu_config_entry = ttk.Entry(config_frame, width=20)
        self.mpu_config_entry.insert(0, "0,0,3,0")
        self.mpu_config_entry.pack(side=tk.LEFT)

        ttk.Button(config_frame, text="Set Rate", command=self.send_rate).pack(side=tk.LEFT, padx=5)
        ttk.Button(config_frame, text="Set Noise", command=self.send_noise).pack(side=tk.LEFT, padx=5)
        ttk.Button(config_frame, text="Set Config", command=self.send_config).pack(side=tk.LEFT, padx=5)

        self.plot_frame = tk.Frame(self)
        self.plot_frame.pack(fill=tk.BOTH, expand=True)

        self.canvas_displ = tk.Canvas(self.plot_frame, bg="black", height=self.plot_height)
        self.canvas_accel = tk.Canvas(self.plot_frame, bg="black", height=self.plot_height)
        self.canvas_veloc = tk.Canvas(self.plot_frame, bg="black", height=self.plot_height)

        self.canvas_displ.pack(fill=tk.X)
        self.canvas_accel.pack(fill=tk.X)
        self.canvas_veloc.pack(fill=tk.X)

    def draw_canvas_base(self, canvas, label, y_range):
        canvas.delete("all")
        canvas.create_text(10, 10, text=label, anchor="nw", fill="white")
        for y in [-y_range, -y_range // 2, 0, y_range // 2, y_range]:
            y_screen = self.plot_height / 2 - (y / y_range * self.plot_height / 2)
            canvas.create_line(40, y_screen, self.plot_width, y_screen, fill="gray", dash=(2, 2))
            canvas.create_text(35, y_screen, text=str(y), fill="white", anchor="e", font=("Arial", 10))
            
    def redraw_canvas(self):
        def scale(data, y_range):
            return [self.plot_height / 2 - (y / y_range * self.plot_height / 2) for y in data]

        def get_coords(data):
            x_coords = [
                40 + i * (self.plot_width - 40) / self.max_points for i in range(len(data))
            ]
            return x_coords

        def flatten(xs, ys):
            return [coord for pair in zip(xs, ys) for coord in pair]

        x_coords = get_coords(self.accel_x)

        # Displacement
        self.canvas_displ.coords(self.line_x_disp, *flatten(x_coords, scale(self.disp_x, self.displ_range)))
        self.canvas_displ.coords(self.line_y_disp, *flatten(x_coords, scale(self.disp_y, self.displ_range)))
        self.canvas_displ.coords(self.line_z_disp, *flatten(x_coords, scale(self.disp_z, self.displ_range)))

        # Acceleration
        self.canvas_accel.coords(self.line_x_disp, *flatten(x_coords, scale(self.accel_x, self.accel_range)))
        self.canvas_accel.coords(self.line_y_disp, *flatten(x_coords, scale(self.accel_y, self.accel_range)))
        self.canvas_accel.coords(self.line_z_disp, *flatten(x_coords, scale(self.accel_z, self.accel_range)))

        # Velocity
        self.canvas_veloc.coords(self.line_x_disp, *flatten(x_coords, scale(self.vel_x, self.veloc_range)))
        self.canvas_veloc.coords(self.line_y_disp, *flatten(x_coords, scale(self.vel_y, self.veloc_range)))
        self.canvas_veloc.coords(self.line_z_disp, *flatten(x_coords, scale(self.vel_z, self.veloc_range)))
   
    def update_ports(self):
        self.port_box.config(state="normal")
        ports = self.data_client.list_ports()
        self.port_var.set(ports[0] if ports else "No ports")
        self.port_box["values"] = ports

    def draw_static_elements(self):
        self.draw_canvas_base(self.canvas_displ, "Displacement", self.displ_range)
        self.draw_canvas_base(self.canvas_accel, "Acceleration", self.accel_range)
        self.draw_canvas_base(self.canvas_veloc, "Velocity", self.veloc_range)

        # Displacement lines
        self.line_x_disp = self.canvas_displ.create_line(0, 0, 0, 0, fill="red", width=2)
        self.line_y_disp = self.canvas_displ.create_line(0, 0, 0, 0, fill="green", width=2)
        self.line_z_disp = self.canvas_displ.create_line(0, 0, 0, 0, fill="blue", width=2)

        # Acceleration lines
        self.line_x_acc = self.canvas_accel.create_line(0, 0, 0, 0, fill="red", width=2)
        self.line_y_acc = self.canvas_accel.create_line(0, 0, 0, 0, fill="green", width=2)
        self.line_z_acc = self.canvas_accel.create_line(0, 0, 0, 0, fill="blue", width=2)

        # Velocity lines
        self.line_x_vel = self.canvas_veloc.create_line(0, 0, 0, 0, fill="red", width=2)
        self.line_y_vel = self.canvas_veloc.create_line(0, 0, 0, 0, fill="green", width=2)
        self.line_z_vel = self.canvas_veloc.create_line(0, 0, 0, 0, fill="blue", width=2)

    def toggle_connect_device(self):
        def connect_thread():
            if not self.data_client.connected:
                selected_port = self.port_var.get()
                if selected_port and selected_port != "No ports":
                    self.data_client.serial_port = selected_port
                    self.data_client.connect_serial()

                self.update_status()

                if self.data_client.connected:
                    self.connect_button.config(text="DISCONNECT")
                    threading.Thread(target=self.data_client.receive_data, args=(self.update_plot,), daemon=True).start()
            else:
                self.data_client.send_command("stop")
                self.data_client.close()
                self.update_status()
                self.connect_button.config(text="CONNECT")
                self.start_button.config(text="START")
                self.is_running = False

        threading.Thread(target=connect_thread, daemon=True).start()

    def update_status(self):
        status = "Connected" if self.data_client.connected else "Disconnected"
        self.status_label.config(text=status, foreground="green" if self.data_client.connected else "red")

    def toggle_reset(self):
        self.data_client.send_command("stop")
        self.data_client.send_command("reset")
        self.start_button.config(text="START")
        
    def toggle_start_stop(self):
        if not self.data_client.connected:
            return
        if self.is_running:
            self.data_client.send_command("stop")
            self.is_running = False
        else:
            self.start_button.config(text="STOP")
            self.send_rate()
            self.data_client.send_command("start")
            self.is_running = True

    def send_rate(self):
        rate = self.rate_var.get()
        self.data_client.send_command(f"set_rate:{rate}")

    def send_noise(self):
        noise = self.noise_entry.get()
        self.data_client.send_command(f"set_accel_noise_floor:{noise}")

    def send_config(self):
        cfg = self.mpu_config_entry.get()
        self.data_client.send_command(f"set_mpu6050_config:{cfg}")
            
    def update_plot(self, data):
        try:
            if "Acceleration:" in data:
                values = list(map(float, data.split("Acceleration:")[1].strip().split(",")))
                self.accel_x = self.accel_x[1:] + [values[0]]
                self.accel_y = self.accel_y[1:] + [values[1]]
                self.accel_z = self.accel_z[1:] + [values[2]]

            elif "Velocity:" in data:
                values = list(map(float, data.split("Velocity:")[1].strip().split(",")))
                self.vel_x = self.vel_x[1:] + [values[0]]
                self.vel_y = self.vel_y[1:] + [values[1]]
                self.vel_z = self.vel_z[1:] + [values[2]]

            elif "Displacement:" in data:
                values = list(map(float, data.split("Displacement:")[1].strip().split(",")))
                self.disp_x = self.disp_x[1:] + [values[0]]
                self.disp_y = self.disp_y[1:] + [values[1]]
                self.disp_z = self.disp_z[1:] + [values[2]]
                
        except:
            print("No data to plot")
            return

        self.redraw_canvas = getattr(self, "redraw_canvas", lambda: None)
        self.redraw_canvas()
 
    def on_resize(self, event):
        if event.width < 300 or event.height < 200: return
        self.plot_width = event.width
        self.draw_static_elements()
        self.redraw_canvas()

    def on_closing(self):
        if self.record_file:
            self.record_file.close()
        self.data_client.close()
        self.destroy()
        sys.exit()

if __name__ == "__main__":
    app = App()
    app.mainloop()
