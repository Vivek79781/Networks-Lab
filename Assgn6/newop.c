#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <sys/time.h>

#define PACKET_SIZE_RTT 64
#define PACKETS_RTT 5
#define MAX_HOPS 30
#define TYPE_OF_SIZE 2
// #define NUM_TRIES 5
#define ODDBYTE(v)	((unsigned short)(v) << 8)

int NUM_TRIES;
int T;

int in_cksum(const unsigned short *addr, register int len, unsigned short csum) {
    register int nleft = len;
    const unsigned short *w = addr;
    register unsigned short answer;
    register int sum = csum;
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1)
        sum += ODDBYTE(*w);
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    return(answer);
}

struct icmp_packet_rtt {
    struct icmphdr header;
    char data[PACKET_SIZE_RTT - sizeof(struct icmphdr)];
};

struct ip_count {
    char ip[INET_ADDRSTRLEN];
    int count;
};


/*
To discover the intermediate nodes, use the same concept as in traceroute (its description is in the
net in many places)You should send at least 5
ICMP packets with proper headers, each 1 second apart, per intermediate node before finalizing it
*/

void sent_icmp_packet(unsigned char* buffer,int size)
{
    if(size<sizeof(struct icmphdr))
    {
        printf("Error: packet size is less than icmp header size\n");
        return;
    }
    struct icmphdr* packet=(struct icmphdr*)buffer;
    printf("\033[1;32mSent\033[0m\n");
    printf("Packet type %d",packet->type);
    printf("\tPacket code %d",packet->code);
    printf("\tPacket id %d",packet->un.echo.id);
    printf("\tPacket seq %d",packet->un.echo.sequence);
    printf("\tPacket size %d",size);

    printf("\tPacket checksum %d",packet->checksum);
    if(size>sizeof(struct icmphdr))
    printf("\tPacket data %s",buffer+sizeof(struct icmphdr));
    printf("\n");
}

void recv_icmp_packet(unsigned char* buffer,int size)
{
    if(size<sizeof(struct iphdr))
    {
        printf("Error: packet size is less than icmp header size\n");
        return;
    }
    struct iphdr* ip_header=(struct iphdr*)buffer;
    printf("\033[1;32mReceived\033[0m\n");
    // print the ip_header
    printf("\033[1;32mIP Header\033[0m\n");
    printf("IP header length %d",ip_header->ihl);
    printf("\tIP header version %d",ip_header->version);
    printf("\tIP header tos %d",ip_header->tos);
    printf("\tIP header total length %d",ip_header->tot_len);
    printf("\tIP header id %d",ip_header->id);
    printf("\tIP header frag_off %d",ip_header->frag_off);
    printf("\tIP header ttl %d",ip_header->ttl);
    printf("\tIP header protocol %d",ip_header->protocol);
    printf("\tIP header checksum %d",ip_header->check);
    printf("\tIP header saddr %s",inet_ntoa(*(struct in_addr*)&ip_header->saddr));
    printf("\tIP header daddr %s",inet_ntoa(*(struct in_addr*)&ip_header->daddr));
    printf("\n");

    if(size<sizeof(struct iphdr)+sizeof(struct icmphdr))
    {
        printf("Error: packet size is less than icmp +ip header size\n");
        return;
    }
    struct icmphdr* packet=(struct icmphdr*)(buffer+sizeof(struct iphdr));
    printf("\033[1;32mICMP Header\033[0m\n");
    printf("Packet type %d",packet->type);
    printf("\tPacket code %d",packet->code);
    printf("\tPacket id %d",packet->un.echo.id);
    printf("\tPacket seq %d",packet->un.echo.sequence);
    printf("\tPacket size %d",size);

    printf("\tPacket checksum %d",packet->checksum);
    if(size>sizeof(struct icmphdr)+sizeof(struct iphdr))
    printf("\tPacket data %s",buffer+sizeof(struct icmphdr));
    printf("\n");
}


double find_latency(char hs[],int prev[])
{
    const struct hostent *host_lat = gethostbyname(hs);
    // 1. Create a raw socket

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // set recieve timeout as 20 seconds
    struct timeval tv;
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    int new_time[TYPE_OF_SIZE];
    memset(new_time, -1, sizeof(new_time));
    struct timeval send_time[PACKETS_RTT], recv_time[PACKETS_RTT];
    double max_latency,min_latency;
    double max_bandwidth,min_bandwidth;
    max_latency = 0.0;
    min_latency = 100000000.0;
    max_bandwidth = 0.0;
    min_bandwidth = 100000000.0;

    for(int i=1;i<=TYPE_OF_SIZE;i++)
    {
        char buffer[sizeof(struct icmp_packet_rtt)+i*16];
        memset(buffer, 0, sizeof(buffer));
        struct icmp_packet_rtt *packet = (struct icmp_packet_rtt *)buffer;
        packet->header.type = ICMP_ECHO;
        packet->header.un.echo.id = getpid();
        packet->header.un.echo.sequence = i;
        packet->header.checksum = in_cksum((unsigned short *)packet, sizeof(struct icmp_packet_rtt)+i*16, 0);
        struct sockaddr_in dest_addr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        memcpy(&dest_addr.sin_addr, host_lat->h_addr_list[0], host_lat->h_length);
        
        
        double avg_time = 0.0;
        int cnt=0;
        for(int j=0;j<NUM_TRIES;j++)
        {
            gettimeofday(&send_time[i], NULL);
            sent_icmp_packet(buffer,sizeof(buffer));
            // send the packet
            if (sendto(sockfd, packet, sizeof(struct icmp_packet_rtt)+i*16, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) 
            {
                perror("Error sending packet");
                exit(EXIT_FAILURE);
            }

            // recieve the packet
            struct sockaddr_in recv_addr;
            socklen_t recv_addr_len = sizeof(recv_addr);
            char recv_buffer[sizeof(struct icmp_packet_rtt)+i*16];
            if (recvfrom(sockfd, recv_buffer, sizeof(struct icmp_packet_rtt)+i*16, 0, (struct sockaddr *)&recv_addr, &recv_addr_len) < 0) 
            {
                perror("Error receiving packet");
                exit(EXIT_FAILURE);
            }
            gettimeofday(&recv_time[i], NULL);
            struct icmp_packet_rtt *recv_packet = (struct icmp_packet_rtt *)(recv_buffer+sizeof(struct iphdr));
            // print type of icmp header of reply
            printf("%d\t", recv_packet->header.type);

            // print ip of sender of reply
            printf("%s\t", inet_ntoa(recv_addr.sin_addr));


            if (recv_packet->header.type == ICMP_ECHOREPLY && recv_packet->header.un.echo.id == getpid()) 
            {
                double elapsed_time = (recv_time[i].tv_sec - send_time[i].tv_sec) * 1000.0;
                elapsed_time += (recv_time[i].tv_usec - send_time[i].tv_usec) / 1000.0;
                avg_time += elapsed_time;
                cnt++;
                //print the header and data
                printf("Received packet from %s: icmp_seq=%d time=%.3f ms\n", inet_ntoa(recv_addr.sin_addr), recv_packet->header.un.echo.sequence, elapsed_time);

            }
            else
            {
                printf("Error: packet received is not an echo reply\n");
            }
            recv_icmp_packet(recv_buffer,sizeof(recv_buffer));

            sleep(T);
            


        }
        if(cnt==0)
        {
            printf("Error: packets received are not an echo reply\n");

            new_time[i]=-1;
            continue;
        }

        avg_time /= cnt;
        
        // find latency and bandwidth using the previous and this data
            double latency = (avg_time )/2;
            new_time[i]=avg_time;
            printf("Latency: %.3f ms for bytes =%d \n",latency,i);
            if(latency>max_latency)
                max_latency=latency;
            if(latency<min_latency)
                min_latency=latency;
            // if(bandwidth>max_bandwidth)
            //     max_bandwidth=bandwidth;
            // if(bandwidth<min_bandwidth)
            //     min_bandwidth=bandwidth;
            
        
    }



    
    for(int i=0;i<TYPE_OF_SIZE;i++)
    {
        for(int j=i+1;j<=TYPE_OF_SIZE;j++)
        {
            // calculate a bandwidth
            // printf("i=%d j=%d prev = %f new = %f\n",i,j,new_time[i],new_time[j]);
            if(new_time[i]==-1 || new_time[j]==-1 || prev[i]==-1 || prev[j]==-1)
                continue;
            if(new_time[j] - new_time[i]-(prev[j]-prev[i])<=1e-6)
                continue;
            double bandwidth = ((j-i)*8*2*16*1000)/(new_time[j] - new_time[i]-(prev[j]-prev[i]));
            if(bandwidth>max_bandwidth)
                max_bandwidth=bandwidth;
            if(bandwidth<min_bandwidth)
                min_bandwidth=bandwidth;
            // printf("Bandwidth: %.3f Mbps for bytes =%d to %d \n",bandwidth,i,j);
            

        }
    }

    printf("Max Latency: %.3f ms, Min Latency: %.3f ms, Max Bandwidth: %.3f Mbps, Min Bandwidth: %.3f Mbps\n",max_latency,min_latency,max_bandwidth,min_bandwidth);

    return (max_latency+min_latency)/2;
}


double finalize_next(int ttl,char hs[],int prev[],char source[])
{
    struct hostent *host = gethostbyname(hs);
    // 1. Create a raw socket


    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // set recieve timeout as 20 seconds
    struct timeval tv;
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // set ttl
    printf("TTL: %d host name %s\n",ttl,host->h_name);

    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        perror("Error setting TTL");
        exit(EXIT_FAILURE);
    }

    struct timeval send_time[PACKETS_RTT], recv_time[PACKETS_RTT];
    
    double min_elapsed_time = 0.0;
    double max_elapsed_time = 0.0;

    struct ip_count ip_count[PACKETS_RTT];
    //initialize ip_count
    for(int i=0;i<PACKETS_RTT;i++)
    {
        ip_count[i].count=0;
        for(int j=0;j<INET_ADDRSTRLEN;j++)
        ip_count[i].ip[j]=0;
    }
    
    int sz=0;
    
    for(int i = 0; i < PACKETS_RTT; i++) 
    {
        struct icmp_packet_rtt packet;
        memset(&packet, 0, sizeof(packet));
        packet.header.type = ICMP_ECHO;
        packet.header.un.echo.id = getpid();
        packet.header.un.echo.sequence = i;
        packet.header.checksum = in_cksum((unsigned short *)&packet, sizeof(packet), 0);
            struct sockaddr_in dest_addr;
            memset(&dest_addr, 0, sizeof(dest_addr));
            dest_addr.sin_family = AF_INET;
            // dest_addr.sin_addr.s_addr = inet_addr(host->h_addr_list[0]);
            // printf("Host: %s\n", host->h_addr_list[0]);
            // printf("Host: %s\n", host->h_name);
            memcpy(&dest_addr.sin_addr, host->h_addr_list[0], host->h_length);

                gettimeofday(&send_time[i], NULL);
                if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
                    perror("Error sending packet");
                    exit(EXIT_FAILURE);
                }

        // Receive the ICMP packet
            struct sockaddr_in recv_addr;
            socklen_t recv_addr_len = sizeof(recv_addr);
            char recv_buffer[PACKET_SIZE_RTT];
                if (recvfrom(sockfd, recv_buffer, PACKET_SIZE_RTT, 0, (struct sockaddr *)&recv_addr, &recv_addr_len) < 0) {
                    perror("Error receiving packet");
                    exit(EXIT_FAILURE);
                }
            // print the ICMP packet IP reached
            printf("%s\t", inet_ntoa(recv_addr.sin_addr));

            int available=0;
            for(int i=0;i<sz;i++)
            {
                if(strcmp(ip_count[i].ip,inet_ntoa(recv_addr.sin_addr))==0)
                {
                    ip_count[i].count++;
                    available=1;
                    break;
                }
            }
            if(available==0)
            {
                strcpy(ip_count[sz].ip,inet_ntoa(recv_addr.sin_addr));
                ip_count[sz].count=1;
                sz++;
            }
            
            sleep(5);
            // print the ICMP packet recieved type
            struct icmp_packet_rtt *icmp_header = (struct icmp_packet_rtt *)(recv_buffer + sizeof(struct iphdr));
            printf("%d\t", icmp_header->header.type);

            // print the ICMP packet recieved code
            printf("%d\t", icmp_header->header.code);

            // print the ICMP packet recieved id
            printf("%d\t", icmp_header->header.un.echo.id);

            
            struct icmp_packet_rtt *icmp_packet = (struct icmp_packet_rtt *)(recv_buffer + sizeof(struct iphdr));
            if((icmp_packet->header).type == ICMP_TIME_EXCEEDED){
                struct icmp_packet_rtt *icmp_packet2 = (struct icmp_packet_rtt *)(recv_buffer + 2*sizeof(struct iphdr) + sizeof(struct icmphdr));
                // printf("%s\n", icmp_packet2->data);
            } 
            printf("Recieving here %d\n", icmp_packet->header.un.echo.sequence);

            gettimeofday(&recv_time[i], NULL);
            double elapsed_time = (recv_time[i].tv_sec - send_time[i].tv_sec) * 1000.0;
            elapsed_time += (recv_time[i].tv_usec - send_time[i].tv_usec) / 1000.0;
            if (min_elapsed_time == 0.0 || elapsed_time < min_elapsed_time)
                min_elapsed_time = elapsed_time;
            if (elapsed_time > max_elapsed_time)
                max_elapsed_time = elapsed_time;
            
    }
    
    printf("Min elapsed time: %f\tMax elapsed time: %f\n", min_elapsed_time, max_elapsed_time);

    int max_count=0,pos=-1;
    for(int i=0;i<sz;i++)
    {
        printf("%s\t%d\n",ip_count[i].ip,ip_count[i].count);
        if(ip_count[i].count>max_count)
        {
            max_count=ip_count[i].count;
            pos=i;
        }
    }

    printf("Most frequent IP: %s\n",ip_count[pos].ip); 
    printf("The latency for this new link %s to %s is %lf\n",source,ip_count[pos].ip,find_latency(ip_count[pos].ip,prev));
    printf("TTL: %d host name %s\n",ttl,host->h_name);
    strcpy(source,ip_count[pos].ip);

    return min_elapsed_time;
        
}

/*
To estimate the latency and bandwidth of an intermediate link X-Y, send ping ICMP packets with
different amounts of data in them to Y. For each size of data, send n pings, each T seconds apart.
Use the RTT measurements and the data sizes to estimate the latency and bandwidth.
*/
// function to find latency



int main(int argc, char *argv[]) 
{
    // getting the destination details
    // struct hostent *host = gethostbyname(argv[1]);
    // if (host == NULL) {
    //     perror("Error getting host");
    //     exit(EXIT_FAILURE);
    // }

    // 1. Create a raw socket

    T=atoi(argv[3]);
    NUM_TRIES=atoi(argv[2]);

    int socket_fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (socket_fd < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // set recieve timeout as 20 seconds
    struct timeval tv;
    tv.tv_sec = 20;
    tv.tv_usec = 0;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }


    // 2. Set the TTL
    int prev[TYPE_OF_SIZE+1];
    memset(prev, 0, sizeof(prev));
    char source[50];
    char dest[50];
    strcpy(dest,argv[1]);
    strcpy(source,"source");
    for(int ttl=1; ttl <= MAX_HOPS; ttl++) 
    {
        double min_elapsed_time = 0.0;
        min_elapsed_time=finalize_next(ttl, dest,prev,source);
        printf("%d\t%f\n", ttl, min_elapsed_time);

    }
    close(socket_fd);

}