import tkinter as tk
from tkinter import ttk, messagebox
from serial.tools import list_ports

from codec_client import CodecClient
from serial_client import SerialClient


class SoundCardApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Sound Card Controller BUKREK")
        self.geometry("800x600")

        self.serial = SerialClient()
        self.codec = CodecClient(self.serial)

        # Styles (big title)
        self.style = ttk.Style(self)
        self.style.configure("Title.TLabel", font=("Segoe UI", 22, "bold"), foreground="#0078D7")

        # Top: Port/baud/connect
        port_frame = ttk.Frame(self); port_frame.pack(fill="x", padx=8, pady=6)
        ttk.Label(port_frame, text="Port:").pack(side="left")
        self.combo_port = ttk.Combobox(port_frame, width=18, values=self.enum_ports())
        self.combo_port.pack(side="left", padx=4)
        ttk.Button(port_frame, text="Refresh", command=self.refresh_ports).pack(side="left", padx=(4,10))
        ttk.Label(port_frame, text="Baud:").pack(side="left")
        self.entry_baud = ttk.Entry(port_frame, width=8); self.entry_baud.insert(0, "9600")
        self.entry_baud.pack(side="left", padx=4)
        self.btn_connect = ttk.Button(port_frame, text="Connect", command=self.toggle_conn)
        self.btn_connect.pack(side="left", padx=8)
        # Big title on the right
        ttk.Label(port_frame, text="BUKREK SOUNDCARD", style="Title.TLabel").pack(side="right", padx=(8,0))

        # Quick Control
        frame_mid = ttk.LabelFrame(self, text="Quick Control"); frame_mid.pack(fill="x", padx=8, pady=6)

        # EQ Profile
        ttk.Label(frame_mid, text="EQ Profile:").grid(row=0, column=0, padx=4, pady=4, sticky="w")
        self.combo_eq = ttk.Combobox(frame_mid, width=16, values=[
            "flat","rock","pop","classical","rap","jazz","edm","vocal",
            "bright","warm","bassboost","trebleboost","maxsmile","midspike"
        ])
        self.combo_eq.set("flat")
        self.combo_eq.grid(row=0, column=1, padx=4, pady=4, sticky="w")
        ttk.Button(frame_mid, text="Apply EQ Profile", command=self.apply_eq_profile).grid(row=0, column=2, padx=6, pady=4, sticky="w")

        # Bass Enhance (compact, aligned with Surround)
        self.bass_enable_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(frame_mid, text="Bass Enhance", variable=self.bass_enable_var)\
            .grid(row=1, column=0, sticky="w", padx=6, pady=2)
        bass_params = ttk.Frame(frame_mid); bass_params.grid(row=1, column=1, sticky="w", pady=2)
        ttk.Label(bass_params, text="LR(0..63):").pack(side="left")
        self.entry_bass_lr = ttk.Entry(bass_params, width=6); self.entry_bass_lr.insert(0, "5"); self.entry_bass_lr.pack(side="left", padx=(2,8))
        ttk.Label(bass_params, text="Level(0..127):").pack(side="left")
        self.entry_bass_level = ttk.Entry(bass_params, width=6); self.entry_bass_level.insert(0, "31"); self.entry_bass_level.pack(side="left", padx=(2,0))
        ttk.Button(frame_mid, text="Apply Bass Enhance", command=self.apply_bass_enhance)\
            .grid(row=1, column=2, padx=6, pady=2, sticky="w")

        # Surround (aligned with Bass row)
        self.sur_enable_var = tk.BooleanVar(value=False)
        ttk.Checkbutton(frame_mid, text="Surround", variable=self.sur_enable_var)\
            .grid(row=2, column=0, sticky="w", padx=6, pady=2)
        sur_params = ttk.Frame(frame_mid); sur_params.grid(row=2, column=1, sticky="w", pady=2)
        ttk.Label(sur_params, text="Width(0..7):").pack(side="left")
        self.entry_sur_width = ttk.Entry(sur_params, width=6); self.entry_sur_width.insert(0, "4"); self.entry_sur_width.pack(side="left", padx=(2,0))
        ttk.Button(frame_mid, text="Apply Surround", command=self.apply_surround)\
            .grid(row=2, column=2, padx=6, pady=2, sticky="w")

        # Volume
        ttk.Label(frame_mid, text="Volume %").grid(row=3, column=0, sticky="w", padx=6)
        self.scale_volume = ttk.Scale(frame_mid, from_=0, to=100, orient="horizontal"); self.scale_volume.set(60)
        self.scale_volume.grid(row=3, column=1, sticky="we", padx=6, columnspan=1)
        ttk.Button(frame_mid, text="Set Volume", command=self.apply_volume).grid(row=3, column=2, padx=6, sticky="w")
        # Layout weights for compact alignment
        frame_mid.columnconfigure(0, weight=0)
        frame_mid.columnconfigure(1, weight=1)
        frame_mid.columnconfigure(2, weight=0)

        # 5-Band EQ Sliders with green bars under them
        frame_eq = ttk.LabelFrame(self, text="5-Band EQ (-12..+12 dB)"); frame_eq.pack(fill="x", padx=8, pady=6)
        band_labels = ["B0","B1","B2","B3","B4"]
        self.eq_vars = [tk.IntVar(value=0) for _ in range(5)]
        self.eq_canvases = []
        self.eq_rects = []
        for i, lab in enumerate(band_labels):
            ttk.Label(frame_eq, text=lab).grid(row=0, column=i, pady=(4,0))
            # cell holds the slider and its slim green bar in the same column
            cell = ttk.Frame(frame_eq)
            cell.grid(row=1, column=i, padx=10, pady=(2,4), sticky="n")
            # vertical slider
            s = tk.Scale(cell, from_=12, to=-12, resolution=1, length=140,
                         variable=self.eq_vars[i], orient="vertical",
                         command=lambda val, idx=i: self.update_eq_bar(idx, float(val)))
            s.pack(side="left")
            # slim green bar (fills upward)
            bar_h = 140
            c = tk.Canvas(cell, width=10, height=bar_h, highlightthickness=0, bg="#f0f0f0")
            c.pack(side="left", padx=(4,0))
            c.create_rectangle(1, 1, 9, bar_h-1, outline="#c0c0c0")
            rect = c.create_rectangle(2, bar_h-2, 8, bar_h-2, fill="#28a745", outline="")
            self.eq_canvases.append(c)
            self.eq_rects.append(rect)
            # initialize bar
            self.update_eq_bar(i, self.eq_vars[i].get())
        ttk.Button(frame_eq, text="Apply EQ", command=self.apply_eq_sliders).grid(row=2, column=0, columnspan=5, pady=6)

        # Bottom: Console + manual command
        frame_bot = ttk.Frame(self); frame_bot.pack(fill="both", expand=True, padx=8, pady=6)
        self.txt = tk.Text(frame_bot, height=10, wrap="none")
        self.txt.pack(fill="both", expand=True, side="top")
        cmd_row = ttk.Frame(frame_bot); cmd_row.pack(fill="x", side="bottom", pady=(6,0))
        ttk.Label(cmd_row, text="Command:").pack(side="left")
        self.entry_cmd = ttk.Entry(cmd_row); self.entry_cmd.pack(side="left", fill="x", expand=True, padx=6)
        self.entry_cmd.bind("<Return>", lambda e: self.on_send_manual())
        ttk.Button(cmd_row, text="Send", command=self.on_send_manual).pack(side="left")

        # Periodic check for received data
        self.after(50, self.drain_serial)

    def update_eq_bar(self, idx, val):
        try:
            val = float(val)
        except Exception:
            val = 0.0
        # map -12..+12 -> 0..1
        level = max(0.0, min(1.0, (val + 12.0) / 24.0))
        c = self.eq_canvases[idx]
        rect = self.eq_rects[idx]
        BAR_H = int(c["height"])
        # fill from bottom up
        top = int((1.0 - level) * (BAR_H - 3)) + 2
        c.coords(rect, 2, top, 8, BAR_H - 2)

    def enum_ports(self):
        return [port.device for port in list_ports.comports()]

    def refresh_ports(self):
        self.combo_port["values"] = self.enum_ports()

    def toggle_conn(self):
        if self.serial.is_open():
            self.serial.close()
            self.btn_connect.config(text="Connect")
            self.log("Disconnected")
        else:
            port = (self.combo_port.get() or "").strip()
            if not port:
                messagebox.showerror("Error", "Please select a valid COM port")
                return
            try:
                baud = int((self.entry_baud.get() or "9600").strip())
                self.serial.open(port, baud, newline="\r\n")
                self.btn_connect.config(text="Disconnect")
                self.log(f"Connected to {port} at {baud} baud")
            except Exception as e:
                messagebox.showerror("Error", f"Failed to open port: {e}")

    def log(self, msg: str):
        self.txt.insert("end", msg + "\r\n"); self.txt.see("end")

    def send_cmd(self, cmd: str):
        if self.serial.is_open():
            self.serial.write_line(cmd)
            self.log(f"> {cmd}")
        else:
            self.log("Error: Not connected"); return

    def on_send_manual(self):
        cmd = (self.entry_cmd.get() or "").strip()
        if cmd:
            self.send_cmd(cmd)
            self.entry_cmd.delete(0, "end")

    # Codec controllers
    def apply_eq_profile(self):
        name = (self.combo_eq.get() or "").strip().lower()
        if name:
            self.send_cmd(self.codec.set_eq_profile(name))
        else:
            self.log("Error: Please select an EQ profile")

    def apply_eq_sliders(self):
        vals = [int(var.get()) for var in self.eq_vars]
        self.send_cmd(self.codec.set_eq(*vals))

    def apply_bass_enhance(self):
        enable = self.bass_enable_var.get()
        try:
            lr = int(self.entry_bass_lr.get())
            level = int(self.entry_bass_level.get())
        except ValueError:
            lr, level = 5, 31
        self.send_cmd(self.codec.set_bass_enhance(enable, lr, level))

    def apply_surround(self):
        enable = self.sur_enable_var.get()
        try:
            width = int(self.entry_sur_width.get())
        except ValueError:
            width = 4
        # If your codec_client uses set_surround(), call that:
        # self.send_cmd(self.codec.set_surround(enable, width))
        self.send_cmd(self.codec.set_surround_sound(enable, width))

    def apply_volume(self):
        vol = int(self.scale_volume.get())
        self.send_cmd(self.codec.set_volume(vol))

    def drain_serial(self):
        try:
            # read_lines() returns a list of lines
            for line in self.serial.read_lines():
                self.log(f"< {line.strip()}")
        except Exception:
            pass
        self.after(100, self.drain_serial)


if __name__ == "__main__":
    app = SoundCardApp()
    app.mainloop()