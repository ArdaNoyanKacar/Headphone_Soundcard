class CodecClient:
    def __init__(self, transport):
        self.transport = transport

    def help(self):
        return "help"
    
    def dump(self):
        return "dumpregs"
    
    def set_eq_profile(self, profile: str):
        return f"seteqprofile {profile}"

    def set_eq(self, b0, b1, b2, b3, b4):
        return f"seteq {int(b0)} {int(b1)} {int(b2)} {int(b3)} {int(b4)}"
    
    def set_volume(self, volume: int):
        if volume < 0:
            volume = 0
        elif volume > 100:
            volume = 100
        return f"setvolume {int(volume)}"

    def set_bass_enhance(self, enable: bool, lr_volume: int = 5, bass_level: int = 0x1F):
        onoff = "on" if enable else "off"
        if enable:
            return f"setbassenhance {onoff} {int(lr_volume)} {int(bass_level)}"
        return f"setbassenhance {onoff}"

    def set_surround_sound(self, enable: bool, width: int = 4):
        onoff = "on" if enable else "off"
        if enable:
            return f"setsurround {onoff} {int(width)}"
        return f"setsurround {onoff}"
