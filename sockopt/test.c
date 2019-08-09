#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/in.h>
#include <string.h>
#include <errno.h>

#define SOCKET_OPS_BASE 128
#define SOCKET_OPS_SET (SOCKET_OPS_BASE)
#define SOCKET_OPS_GET (SOCKET_OPS_BASE)
#define SOCKET_OPS_MAX (SOCKET_OPS_BASE + 1)

#define UMSG "----------user------------"
#define UMSG_LEN sizeof("----------user------------")

char kmsg[64];

int main(void)
{
    int sockfd;
    int len;
    int ret;

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sockfd < 0) {
        printf("can not create a socket\n");
        return -1;
    }

    /*call function recv_msg()*/
    ret = setsockopt(sockfd, IPPROTO_IP, SOCKET_OPS_SET, UMSG, UMSG_LEN);
    printf("setsockopt: ret = %d. msg = %s\n", ret, UMSG);
    len = sizeof(char) * 64;

    /*call function send_msg()*/
    ret = getsockopt(sockfd, IPPROTO_IP, SOCKET_OPS_GET, kmsg, &len);
    printf("getsockopt: ret = %d. msg = %s\n", ret, kmsg);
    if (ret != 0) {
        printf("getsockopt error: errno = %d, errstr = %s\n", errno, strerror(errno));
    }

    close(sockfd);
    return 0;
}