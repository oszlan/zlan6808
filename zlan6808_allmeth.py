import socket #导入socket模块
import struct

s = socket.socket()	#创建套接字
host = '192.168.1.200' #主机IP
port = 4196
try:
    s.connect((host,port)) #初始化连接
except Exception as e:
    print("连接失败，请检查网络连接或主机IP地址是否正确。")
    print(e)
global DI1,DI2,DI3,DI4,DI5,DI6,DI7,DI8
global DO1,DO2,DO3,DO4,DO5,DO6,DO7,DO8

'''
根据传入的字节数组生成CRC16校验码，自动添加在末两字节
'''
def gen_crc16(data: bytes):
    crc = 0xFFFF
    for i in range(len(data)):
        crc ^= data[i]
        for j in range(8):
            if (crc & 0x0001):
                crc >>= 1
                crc ^= 0xA001
            else:
                crc >>= 1
    crc_h = (crc & 0xff00) >> 8
    crc_l = crc & 0x00ff
    return data + bytes([crc_l, crc_h])

'''
发送指令接收回包，回包为长度不定的16进制字节串，末两字节为CRC校验码
'''
def send_cmd(cmd):
    scmd = struct.pack("%dB" % (len(cmd)), *cmd)
    s.send(scmd)
    print((s.recv(1024)).hex())
    return (s.recv(1024)).hex()

'''
批量读取DI1到8输入状态，其中第四字节转二进制后，从右至左分别代表DI1到DI8输入状态，1为有输入，2为无输入
'''
def read_di():
    data = gen_cmd('0x01','0x01','0x00','0x00','0x00','0x08') #遍历DI指令
    result = gen_crc16(bytes(data))
    print("CRC:",result)
    repo = send_cmd(result)
    di_byte = bytes.fromhex(repo[6:8])
    di_bin = bin(int(di_byte.hex(), 16))[2:].zfill(8)
    DI = [di_bin[i] for i in range(7, -1, -1)]
    print("DI状态：", *DI)
    print(di_bin)

'''
把可阅读的0x00格式命令转换为发送格式
'''
def gen_cmd(*args):
    cmd = []
    for arg in args:
        cmd.append(int(arg, 16))
    return cmd

'''
设置DO状态
'''
def set_do(do, swtich):
    data = gen_cmd('0x01', '0x05', '0x00', '0x00', '0x00', '0x00')
    if do in ['DO1', 'DO2', 'DO3', 'DO4', 'DO5', 'DO6', 'DO7', 'DO8']:
        do_num = int(do[2])
        if swtich == 'on':
            data[3] = 16 + do_num
            data[5] = 255
        elif swtich == 'off':
            data[3] = 16 + do_num
            data[5] = 0
        else:
            print("wrong switch")
    else:
        print("wrong DO")
    result = gen_crc16(bytes(data))
    print("CRC:", result)
    repo = send_cmd(result)
    return repo


'''
循环读取DI状态，测试用
'''
if __name__ == '__main__':
    while True:
            read_di()
