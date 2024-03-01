### 项目描述

GPK是一个基于linux的网络组包工具，用于学习计算机网络。它可以根据相关的网络协议组织网络数据包并且通过指定的网卡发送出去。

### 实现内容

- [ ] ARP组包库

- [ ] IPv4组包

- [ ] IPv6组包

- [ ] ICMPv4组包

- [ ] ICMPv6组包

- [ ] 其它
### ARP组包库

该库提供一个Arp类，它能够收/发ARP数据包、组织ARP数据包、分析ARP数据包。Arp类提供以下方法：

```c
int set_dest_mac(std::string mac);
std::string get_dest_mac(void);
int set_src_mac(std::string mac);
std::string get_src_mac(void);
void set_opcode(unsigned short opcode);
int set_sender_mac(std::string mac);
std::string get_sender_mac(void);
int set_sender_ip(std::string ip);
std::string get_sender_ip(void);
std::string get_target_ip(void);
int set_target_ip(std::string ip);
int set_target_mac(std::string mac);
std::string get_target_mac(void);

class Arp& arp_send(void);
class Arp& arp_recv(void);
  
  
  
  
```

