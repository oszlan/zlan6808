using System.Net.Sockets;
using System;
using System.Text;

namespace Zlan6808
{
    public class Zlan6808
    {
        private Socket s;
        private string host;
        private int port;
        private byte[] DI1, DI2, DI3, DI4, DI5, DI6, DI7, DI8;
        private byte[] DO1, DO2, DO3, DO4, DO5, DO6, DO7, DO8;

        public Zlan6808(string host, int port)
        {
            this.host = host;
            this.port = port;
            s = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        }

        public void Connect()
        {
            try
            {
                s.Connect(host, port);
            }
            catch (Exception e)
            {
                Console.WriteLine("连接失败，请检查网络连接或主机IP地址是否正确。");
                Console.WriteLine(e);
            }
        }

        /*
        根据传入的字节数组生成CRC16校验码，自动添加在末两字节
        */
        private byte[] GenCrc16(byte[] data)
        {
            ushort crc = 0xFFFF;
            for (int i = 0; i < data.Length; i++)
            {
                crc ^= data[i];
                for (int j = 0; j < 8; j++)
                {
                    if ((crc & 0x0001) == 1)
                    {
                        crc >>= 1;
                        crc ^= 0xA001;
                    }
                    else
                    {
                        crc >>= 1;
                    }
                }
            }
            byte[] crc_h = BitConverter.GetBytes((ushort)(crc & 0xff00));
            byte[] crc_l = BitConverter.GetBytes((ushort)(crc & 0x00ff));
            Array.Reverse(crc_h);
            Array.Reverse(crc_l);
            byte[] crc = new byte[2];
            crc[0] = crc_l[0];
            crc[1] = crc_h[0];
            return data.Concat(crc).ToArray();
        }

        /*
        发送指令接收回包，回包为长度不定的16进制字节串，末两字节为CRC校验码
        */
        private byte[] SendCmd(byte[] cmd)
        {
            s.Send(cmd);
            byte[] buffer = new byte[1024];
            int count = s.Receive(buffer);
            byte[] result = new byte[count];
            Array.Copy(buffer, result, count);
            return result;
        }

        /*
        批量读取DI1到8输入状态，其中第四字节转二进制后，从右至左分别代表DI1到DI8输入状态，1为有输入，2为无输入
        */
        public void ReadDi()
        {
            byte[] data = GenCmd(0x01, 0x01, 0x00, 0x00, 0x00, 0x08); //遍历DI指令
            byte[] result = GenCrc16(data);
            Console.WriteLine("CRC:" + BitConverter.ToString(result));
            byte[] repo = SendCmd(result);
            string di_bin = Convert.ToString(repo[6], 2).PadLeft(8, '0');
            DI1 = new byte[] { Convert.ToByte(di_bin[7].ToString()) };
            DI2 = new byte[] { Convert.ToByte(di_bin[6].ToString()) };
            DI3 = new byte[] { Convert.ToByte(di_bin[5].ToString()) };
            DI4 = new byte[] { Convert.ToByte(di_bin[4].ToString()) };
            DI5 = new byte[] { Convert.ToByte(di_bin[3].ToString()) };
            DI6 = new byte[] { Convert.ToByte(di_bin[2].ToString()) };
            DI7 = new byte[] { Convert.ToByte(di_bin[1].ToString()) };
            DI8 = new byte[] { Convert.ToByte(di_bin[0].ToString()) };
            Console.WriteLine("DI状态：" + BitConverter.ToString(DI1) + " " + BitConverter.ToString(DI2) + " " + BitConverter.ToString(DI3) + " " + BitConverter.ToString(DI4) + " " + BitConverter.ToString(DI5) + " " + BitConverter.ToString(DI6) + " " + BitConverter.ToString(DI7) + " " + BitConverter.ToString(DI8));
            Console.WriteLine(di_bin);
        }

        /*
        把可阅读的0x00格式命令转换为发送格式
        */
        private byte[] GenCmd(params byte[] args)
        {
            return args;
        }

        /*
        设置DO状态
        */
        public byte[] SetDo(string do_num, string swtich)
        {
            byte[] data = GenCmd(0x01, 0x05, 0x00, 0x00, 0x00, 0x00);
            if (do_num.StartsWith("DO") && int.TryParse(do_num.Substring(2), out int do_num_int))
            {
                data[3] = (byte)(16 + do_num_int);
                data[5] = swtich == "on" ? (byte)255 : swtich == "off" ? (byte)0 : throw new ArgumentException("开闭指令错误");
            }
            else
            {
                throw new ArgumentException("DO接口名错误");
            }
            byte[] result = GenCrc16(data);
            Console.WriteLine("CRC:" + BitConverter.ToString(result));
            byte[] repo = SendCmd(result);
            return repo;
        }
    }
}
