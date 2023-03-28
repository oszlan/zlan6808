## 概述
通过ModbusTCP协议与Zlan6808-3进行通信，并依照其指令集，使用16进制命令对其进行控制，实现了如下功能：
- 批量读取DI1-DI8的输入状态
- 控制指定DO的开闭
- 其他通用的进制转换、命令发送并接收回包等功能

## Overview:
Communication with Zlan6808-3 is achieved through the ModbusTCP protocol, 
using a set of instructions to control it with hexadecimal commands. 
The following functions have been implemented:
- Batch reading of input status of DI1-DI8
- Control of specified DO open/closed
- Other common functions such as number system conversion, command transmission and reception of feedback.
