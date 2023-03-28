#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bitset>

using namespace std;

class Zlan6808
{
    private:
        int sock;
        string address;
        int port;
        struct sockaddr_in server;
        unsigned char DI1, DI2, DI3, DI4, DI5, DI6, DI7, DI8;
        unsigned char DO1, DO2, DO3, DO4, DO5, DO6, DO7, DO8;

    public:
        Zlan6808(string address, int port)
        {
            this->address = address;
            this->port = port;
            sock = -1;
            memset(&server, 0, sizeof(server));
        }

        bool Connect()
        {
            //Create socket
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock == -1)
            {
                perror("Could not create socket");
                return false;
            }
            cout << "Socket created\n";

            server.sin_addr.s_addr = inet_addr(address.c_str());
            server.sin_family = AF_INET;
            server.sin_port = htons(port);

            //Connect to remote server
            if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
            {
                perror("connect failed. Error");
                return false;
            }

            cout << "Connected\n";
            return true;
        }

        /*
        根据传入的字节数组生成CRC16校验码，自动添加在末两字节
        */
        unsigned char* GenCrc16(unsigned char* data, int len)
        {
            unsigned short crc = 0xFFFF;
            for (int i = 0; i < len; i++)
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
            unsigned char* crc_h = new unsigned char[2];
            unsigned char* crc_l = new unsigned char[2];
            crc_h[0] = (unsigned char)((crc & 0xff00) >> 8);
            crc_l[0] = (unsigned char)(crc & 0x00ff);
            unsigned char* crc_res = new unsigned char[2];
            crc_res[0] = crc_l[0];
            crc_res[1] = crc_h[0];
            return crc_res;
        }

        /*
        发送指令接收回包，回包为长度不定的16进制字节串，末两字节为CRC校验码
        */
        unsigned char* SendCmd(unsigned char* cmd, int len)
        {
            if (send(sock, cmd, len, 0) < 0)
            {
                puts("Send failed");
                return NULL;
            }

            unsigned char buffer[1024] = {0};
            int count = recv(sock, buffer, 1024, 0);
            unsigned char* result = new unsigned char[count];
            memcpy(result, buffer, count);
            return result;
        }

        /*
        批量读取DI1到8输入状态，其中第四字节转二进制后，从右至左分别代表DI1到DI8输入状态，1为有输入，2为无输入
        */
        void ReadDi()
        {
            unsigned char data[] = {0x01, 0x01, 0x00, 0x00, 0x00, 0x08};
            int len = sizeof(data)/sizeof(data[0]);
            unsigned char* crc_res = GenCrc16(data, len);
            cout << "CRC:" << hex << (int)crc_res[0] << " " << (int)crc_res[1] << endl;
            unsigned char* repo = SendCmd(crc_res, len+2);
            unsigned char di_byte[] = {repo[6]};
            bitset<8> di_bin(di_byte[0]);
            DI1 = di_bin[7];
            DI2 = di_bin[6];
            DI3 = di_bin[5];
            DI4 = di_bin[4];
            DI5 = di_bin[3];
            DI6 = di_bin[2];
            DI7 = di_bin[1];
            DI8 = di_bin[0];
            cout << "DI状态：" << hex << (int)DI1 << " " << (int)DI2 << " " << (int)DI3 << " " << (int)DI4 << " " << (int)DI5 << " " << (int)DI6 << " " << (int)DI7 << " " << (int)DI8 << endl;
            cout << di_bin << endl;
        }

        /*
        把可阅读的0x00格式命令转换为发送格式
        */
        unsigned char* GenCmd(unsigned char* args, int len)
        {
            return args;
        }

        /*
        设置DO状态
        */
        unsigned char* SetDo(string do_num, string swtich)
        {
            unsigned char data[] = {0x01, 0x05, 0x00, 0x00, 0x00, 0x00};
            int len = sizeof(data)/sizeof(data[0]);
            if (do_num.substr(0, 2) == "DO" && stoi(do_num.substr(2)) >= 1 && stoi(do_num.substr(2)) <= 8)
            {
                data[3] = (unsigned char)(16 + stoi(do_num.substr(2)));
                data[5] = swtich == "on" ? (unsigned char)255 : swtich == "off" ? (unsigned char)0 : throw invalid_argument("开闭指令错误");
            }
            else
            {
                throw invalid_argument("DO接口名错误");
            }
            unsigned char* crc_res = GenCrc16(data, len);
            cout << "CRC:" << hex << (int)crc_res[0] << " " << (int)crc_res[1] << endl;
            unsigned char* repo = SendCmd(crc_res, len+2);
            return repo;
        }
};

int main(int argc, char* argv[])
{
    Zlan6808 zlan6808("192.168.1.200", 4196);
    zlan6808.Connect();
    while (true)
    {
        zlan6808.ReadDi();
    }
    return 0;
}
 char* argv[])
{
    Zlan6808 zlan6808("192.168.1.200", 4196);
    zlan6808.Connect();
    while (true)
    {
        zlan6808.ReadDi();
}
