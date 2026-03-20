import serial
import time
import struct

class TVSerialProtocol:
    CRC_TABLE = [
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
        0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF
    ]

    def __init__(self, port="COM3", baudrate=115200):
        self.port = port
        self.baud = baudrate
        self.ser = None

    def calculate_crc(self, data: bytes):
        checksum = 0xFFFF
        for byte in data:
            uc_temp = (checksum >> 12) & 0x0F
            checksum = ((checksum << 4) & 0xFFFF) ^ self.CRC_TABLE[uc_temp ^ (byte >> 4)]
            uc_temp = (checksum >> 12) & 0x0F
            checksum = ((checksum << 4) & 0xFFFF) ^ self.CRC_TABLE[uc_temp ^ (byte & 0x0F)]
        return checksum & 0xFFFF

    def send_raw(self, data, monitor_signal=None):
        """执行物理发送，并将收到的原始数据回传给 UI 监控窗"""
        try:
            if not self.ser or not self.ser.is_open:
                self.ser = serial.Serial(self.port, self.baud, timeout=2)
            
            self.ser.reset_input_buffer()
            self.ser.write(data)
            
            time.sleep(0.6) # 5586 写入需要时间
            res = self.ser.read_all()
            
            # 关键：如果有监控信号，直接发送原始字节流到 UI
            if monitor_signal and len(res) > 0:
                monitor_signal.emit(res)
                
            success = (b"\x03\x0C\xF1" in res or b"\x03\x0C\xF2" in res)
            return success, res.hex().upper()
        except Exception as e:
            return False, str(e)

    def pack_5586_command(self, cmd_id, payload: bytes):
        length = len(payload) + 2 
        frame_head = bytes([0x07, 0x51, 0x01, length + 4, cmd_id]) 
        full_data_no_crc = frame_head + payload
        crc = self.calculate_crc(full_data_no_crc)
        return full_data_no_crc + struct.pack(">H", crc)

    def pack_hdcp_header(self, hdcp_type, total_blocks, block_size, full_data_crc):
        # 严格按照 [类型] [00] [总包数] [单包长度] [总CRC] [00 00] 打包
        payload = bytes([hdcp_type, 0x00, total_blocks]) + \
                  struct.pack(">H", block_size) + \
                  struct.pack(">H", full_data_crc) + \
                  b"\x00\x00"
        return self.pack_5586_command(0xFE, payload)

    def pack_hdcp_chunk(self, hdcp_type, block_id, chunk_data):
        payload = bytes([hdcp_type, block_id]) + chunk_data
        return self.pack_5586_command(0xFE, payload)
