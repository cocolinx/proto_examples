#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrf_modem.h> 
#include <nrf_modem_at.h> 
#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>

LOG_MODULE_REGISTER(main_tcp, CONFIG_LOG_DEFAULT_LEVEL);

static K_SEM_DEFINE(tcpsem_start, 0, 1);

static int socknum;
static uint8_t rxpktbuf[64];
static bool isconnected = false;

int main(void)
{
    int err;
    err = nrf_modem_lib_init();
    if(err < 0) {
        LOG_ERR("Unable to initialize modem lib. (err: %d)", err);
        return 0;
    }

    LOG_INF("=====TCP EXAMPLE=====");
    
    int ret;
    struct sockaddr_in sa;
    struct zsock_addrinfo *res = NULL;  
    struct zsock_addrinfo hints = {
        .ai_family   = AF_INET,
        .ai_socktype = SOCK_STREAM
    };

    err = lte_lc_connect();
    if(err < 0) {
        LOG_ERR("Failed to connect lte %d", err);
        return 0;
    }

    socknum = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socknum < 0) {
        LOG_ERR("Failed to create socket %d", -errno);
        return 0;
    }

    sa.sin_family = AF_INET;
    sa.sin_port = htons(7777);

    err = zsock_getaddrinfo("echo.cocolinx.com", NULL, &hints, &res); /* get ip address */
    if(err) {
        LOG_ERR("Failed to get addr info %d", err);
        return 0;
    }
    sa.sin_addr = ((struct sockaddr_in *)res->ai_addr)->sin_addr;
    zsock_freeaddrinfo(res);

	err = zsock_connect(socknum, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
	if(err < 0) {
		LOG_ERR("Failed to connect %d", -errno);
        return 0;
	}
    LOG_INF("tcp connected...");
    isconnected = true;

    k_sem_give(&tcpsem_start);

    int txcnt = 5;
    while(txcnt--){
        uint8_t txbuf[] = "this is test for tcp";
        ret = zsock_send(socknum, txbuf, sizeof(txbuf), 0);
        if(ret < 0) {
            LOG_ERR("Failed to send %d", -errno);
            break;
        }
        LOG_INF("send %d bytes...%d", ret, txcnt);
        k_msleep(5000);
    }

    isconnected = false;
    LOG_INF("tcp close...");
    LOG_INF("main close...");

    return 0;
}

static void tcp_thread(void)
{
    int ret;
    struct zsock_pollfd fds[1]; 

    fds[0].fd = socknum;
    fds[0].events = ZSOCK_POLLIN;

    k_sem_take(&tcpsem_start, K_FOREVER);
    LOG_INF("udp poll start...");

    while(isconnected) {
        ret = zsock_poll(fds, 1, 1000);
        if (ret < 0) {
            LOG_ERR("poll() failed: (%d)", -errno);
            ret = -EIO;
            break;
	    }
        if (ret == 0) {
            continue;   /* poll() timeout */
        }
        if ((fds[0].revents & ZSOCK_POLLHUP) == ZSOCK_POLLHUP) {
            LOG_ERR("POLLHUP");
            break;
	    }
        if ((fds[0].revents & ZSOCK_POLLNVAL) == ZSOCK_POLLNVAL) {
            LOG_ERR("POLLNVAL");
            break;
        }
        if ((fds[0].revents & ZSOCK_POLLERR) == ZSOCK_POLLERR) {
            LOG_WRN("POLLERR");
            break;
        }
        if ((fds[0].revents & ZSOCK_POLLIN) == ZSOCK_POLLIN) {
            ret = zsock_recv(fds[0].fd, (void *)rxpktbuf, sizeof(rxpktbuf), 0);
            if (ret < 0) {
                LOG_ERR("recv() failed: (%d)", -errno);
                break;
            }
            else if(ret > 0) {
                LOG_INF("recv(%d bytes): %.*s", ret, ret, rxpktbuf);
            }
            else {
                LOG_INF("TCP Connection Finished");
                break;
            }
        } 
    }

    isconnected = false;
    (void)zsock_close(socknum);
    (void)lte_lc_power_off();
    LOG_INF("tcp thread close...");
    return;
}

K_THREAD_DEFINE(tcp_thread_id, 2048, tcp_thread, NULL, NULL, NULL, 1, 0, 0);
