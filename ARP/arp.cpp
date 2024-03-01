#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#define debug() fprintf(stderr, "[%s]line: %d\n", __FILE__, __LINE__)



class Arp {
    private:
        struct arp_t{
        unsigned char dest[6];
        unsigned char src[6];
        unsigned short arp_type;
        unsigned short hd_type;
        unsigned short pl_type;
        unsigned char hd_size;
        unsigned char pl_size;
        unsigned short opcode;
        unsigned char sender_mac[6];
        unsigned int sender_ip;
        unsigned char target_mac[6];
        unsigned int target_ip;
        
        }__attribute__((packed));
    public:
        Arp() = default;
        Arp(char *hd_name){
            sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
            if (sock_fd < 0)
            {
                perror("socket");
                
            }
            struct ifreq ethreq = {0};
            strcpy(ethreq.ifr_ifrn.ifrn_name, hd_name);
            if (ioctl(sock_fd, SIOCGIFINDEX, &ethreq) < 0)
            {
                perror("");
            }
            src.sll_family = AF_PACKET;
            src.sll_ifindex = ethreq.ifr_ifru.ifru_ivalue;
            arp_buf.arp_type = htons(ETH_P_ARP);
            arp_buf.hd_type = htons(1);
            arp_buf.pl_type = htons(0x0800);

            eth.ifr_ifru.ifru_ivalue = ethreq.ifr_ifru.ifru_ivalue;
            strcpy(eth.ifr_ifrn.ifrn_name, ethreq.ifr_ifrn.ifrn_name);
            if (ioctl(sock_fd, SIOCGIFHWADDR, &ethreq) < 0)
            {
                perror("SIOCGIFHWADDR");
            }
            eth.ifr_ifru.ifru_hwaddr = ethreq.ifr_ifru.ifru_hwaddr;
            memcpy(arp_buf.src, ethreq.ifr_ifru.ifru_hwaddr.sa_data, 6);
            memcpy(arp_buf.sender_mac, ethreq.ifr_ifru.ifru_hwaddr.sa_data, 6);

            // unsigned char *p = (unsigned char *)eth.ifr_ifru.ifru_hwaddr.sa_data;
            // printf("%02x:%02x:%02x:%02x:%02x:%02x\n", p[0], p[1], p[2], p[3], p[4], p[5]);
        }
        //~Arp() = default;
        ~Arp()
        {
            close(sock_fd);
        }
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


    private:
        struct arp_t arp_buf = {.hd_size = 6, .pl_size = 4};
        int sock_fd;
        struct sockaddr_ll dest = {.sll_family = AF_PACKET};
        struct sockaddr_ll src;
        socklen_t len;
        struct ifreq eth;

        void ctospace(std::string& str, char c);
        in_addr_t ip_strton(std::string& str);
        void mac_strton(std::string& str, unsigned char mac[6]);
        std::string mac_ntostr(unsigned char mac[6]);
        std::string ip_ntostr(in_addr_t ip);

};
std::string Arp::mac_ntostr(unsigned char mac[6])
{
    unsigned short tmp = mac[0];
    std::string str;
    std::stringstream r;

    r << std::hex << (tmp & ((unsigned short)0x00ff));

    int i = 1;
    for (; i < 6 ;i++)
    {
        tmp = (unsigned short)mac[i];
        r <<":" << std::hex <<(tmp & ((unsigned short)0x00ff));
        
        //std::cout <<"debug: " <<tmp<< std::endl;
    }
    std::getline(r, str);
    //std::cout <<"line[mac]: "<<__LINE__<< str << std::endl;
    return str;
}

std::string Arp::ip_ntostr(in_addr_t ip)
{
    in_addr_t mask = 0xffu;
    in_addr_t tmp = (ip & mask);
    std::string str;
    std::stringstream r;

    tmp &= ((in_addr_t)0x00ff);
    r << tmp;
    

    int i = 1;
    //std::cout <<std::hex << ip << std::endl; 
    for (; i < 4; i++)
    {
        mask = mask << (8);
        //std::cout << std::hex << mask << std::endl;
        tmp = (ip & mask) >> (i * 8);
        tmp &= ((in_addr_t)0x00ff);
        r << "." << tmp;

        //std::cout <<"debug: " <<tmp<< std::endl;
        //std::cout << tmp << std::endl;
    }
    std::getline(r, str);
    //std::cout <<"line[ip]: "<<__LINE__<< str << std::endl;
    return str;



}
in_addr_t Arp::ip_strton(std::string& str)
{
    in_addr_t n, tmp = 0;
    std::stringstream r(str);
    int i = 0;
    for (; i < 4; i++)
    {
        r >> n;
        //std::cout<<n<<std::endl;
        tmp |= n << (i * 8);
    }
    return tmp;
}
void Arp::mac_strton(std::string& str, unsigned char mac[6])
{
    std::stringstream r(str);
    int i = 5;
    unsigned short tmp;
    for (; i >= 0; i--)
    {
        r >> std::hex >> tmp;
        mac[i] = tmp;
    }
    
}
void Arp::ctospace(std::string& str, char c)
{
    for (auto &t : str)
    {
        if (t == c)
        {
            t = ' ';
        }
    }
    //std::cout << str << std::endl;
}
class Arp& Arp::arp_send(void)
{
    
    if (sendto(sock_fd, (void *)&arp_buf, sizeof(arp_buf), 0, (struct sockaddr *)&src, sizeof(src)) != 42)
    {
        perror("send");
    }
    return *this;
}
class Arp& Arp::arp_recv(void)
{
    len = sizeof(src);
    if (recvfrom(sock_fd, (void *)&arp_buf, sizeof(arp_buf), 0, NULL/*(struct sockaddr *)&src*/, NULL/*&len*/) != 42)
    {
        perror("recv");
    }
    return *this;
}
int Arp::set_dest_mac(std::string mac)
{
    ctospace(mac, ':');
    mac_strton(mac, arp_buf.dest);
    return 0;

}
std::string Arp::get_dest_mac(void)
{
    return mac_ntostr(arp_buf.dest);
}
int Arp::set_src_mac(std::string mac){
    ctospace(mac, ':');
    mac_strton(mac, arp_buf.src);
    return 0;
}
std::string Arp::get_src_mac(void){
    return mac_ntostr(arp_buf.src);
}
void Arp::set_opcode(unsigned short opcode){
    arp_buf.opcode = htons(opcode);
}
int Arp::set_sender_mac(std::string mac){
    ctospace(mac, ':');
    mac_strton(mac, arp_buf.sender_mac);
    return 0;
}
std::string Arp::get_sender_mac(void){
    return mac_ntostr(arp_buf.sender_mac);
}
int Arp::set_sender_ip(std::string ip){
    ctospace(ip, '.');
    arp_buf.sender_ip = ip_strton(ip);
    return 0;
}
std::string Arp::get_sender_ip(void){
    return ip_ntostr(arp_buf.sender_ip);
}
std::string Arp::get_target_ip(void){
    return ip_ntostr(arp_buf.target_ip);
}
int Arp::set_target_ip(std::string ip){
    ctospace(ip, '.');
    arp_buf.target_ip = ip_strton(ip);
    return 0;
}
int Arp::set_target_mac(std::string mac){
    ctospace(mac, ':');
    mac_strton(mac, arp_buf.target_mac);
    return 0;
}
std::string Arp::get_target_mac(void){
    return mac_ntostr(arp_buf.target_mac);
}