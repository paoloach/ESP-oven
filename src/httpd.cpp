#include <vector>
#include <string.h>

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "netif/etharp.h"

#include "httpd.h"
#include "config.h"
#include "http.h"
#include "Connections.h"
#include "UrlMatcher.h"
#include "HttpResponse.h"

struct tcp_pcb * tcpPcb;
extern int degree;
extern int threshold;
extern bool enable;
extern bool restart;

static err_t  newConnection(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
//static void getRoot(Connection * c);
static void getWhoAreYou(Connection *c);
static void getTemperature(Connection * c);
static void getThreshold(Connection * c);
static void setTemperature(Connection * c);
static void setEnable(Connection * c);
static void setRestart(Connection * c);

std::vector<UrlMatcher> matchersGet;
std::vector<UrlMatcher> matchersPut;
std::vector<UrlMatcher> matchersEmpty;

void std::__throw_bad_function_call() {
  while(1);
}

inline bool isLastPBuf(const struct pbuf * p)  {
  return p->len == p->tot_len;
}

void initHttpd(void){
  err_t error;

  matchersGet.reserve(4);
  matchersGet.emplace_back("/", [](Connection *c){c->httpResponse.sendOk(c,"I am ESP-OVEN");});
  matchersGet.emplace_back("/whoareyou", getWhoAreYou);
  matchersGet.emplace_back("/temperature", getTemperature);
  matchersGet.emplace_back("/threshold", getThreshold);

  matchersPut.emplace_back("/threshold", setTemperature);
  matchersPut.emplace_back("/enable", setEnable);
  matchersPut.emplace_back("/restart", setRestart);

  tcpPcb = tcp_new();
  Connection::init();

  error =  tcp_bind(tcpPcb, IP_ADDR_ANY, HTTP_PORT);
  if (error != ERR_OK){
    printf("tcp_bind error: %d\n", error);
    return;
  }

  struct tcp_pcb * tcpPcbl = tcp_listen(tcpPcb);
  if (tcpPcbl == NULL){
    printf("tcp_listern error: not enough memory !\n");
    return;
  }

  tcpPcb = tcpPcbl;

  tcp_accept(tcpPcb, newConnection);
}

static err_t  newConnection(void *arg, struct tcp_pcb *newpcb, err_t err) {
  err_t error;

  struct Connection * newConnection = Connection::getEmptySlot();
  if (newConnection == NULL){
    printf("too many connections\n" );
    error = tcp_close(newpcb);
    if (error != ERR_OK){
      printf("Unable to close the connection: %d\n", error);
    }
  }
  newConnection->tcpPcb=newpcb;
  tcp_arg(newpcb, newConnection);
  tcp_sent(newpcb, sent);
  tcp_recv(newpcb,recv );
  tcp_accepted(newpcb);
  return ERR_OK;
}

err_t sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
  Connection * connection= (Connection *)arg;
  Connection::releseSlot(connection);
  return tcp_close(tpcb);
}

static err_t recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err){
  err_t error;
  Connection * connection= (Connection *)arg;
  if (p == NULL){
    error = tcp_close(tpcb);
    if (error != ERR_OK){
      printf("Unable to close the connection: %d\n", error);
    }
    connection->tcpPcb=NULL;
    return error;
  }

  while(1){
    connection->http.feed((const char *)p->payload, p->len);
    if (connection->http.error()){
      printf("ERROR parsing data\n");
      tcp_close(tpcb);
      connection->tcpPcb = nullptr;
    } else if (connection->http.needMoreData()){
      if (isLastPBuf(p)){
        return ERR_OK;
      }
      p = p->next;
    } else {
      break;
    }
  }

  std::vector<UrlMatcher> * matchers;
  switch (connection->http.getMethod()) {
    case Method::GET:
      matchers = &matchersGet;
      break;
    case Method::PUT:
      matchers = &matchersPut;
      break;
    default:
      matchers = &matchersEmpty;
  }
  bool found=false;
  for(auto & matcher: (*matchers)){
    if (matcher==connection->http.getUrl()){
      found=true;
      matcher(connection);
    }
  }
  if (!found){
    connection->httpResponse.sendNotFound(connection);
  }

  tcp_recved(tpcb, p->len);
  pbuf_free(p);
  printf("[%s:%d]\n", __FILE__, __LINE__);
  return ERR_OK;
}


void getWhoAreYou(Connection * c){
  printf("Respond at \\whoareyou\n");
  if (!c->httpResponse.sendOk(c,"I am ESP-OVEN")){
    tcp_close(c->tcpPcb);
  }
}

void getTemperature(Connection * c){
  char buffer[10];
  sprintf(buffer,"%d",degree);
  printf("Respond at \\temperature\n");
  if (!c->httpResponse.sendOk(c,buffer)){
    tcp_close(c->tcpPcb);
  }
}
void getThreshold(Connection * c){
  char buffer[10];
  sprintf(buffer,"%d",threshold);
  printf("Respond at \\threshold\n");
  if (!c->httpResponse.sendOk(c,buffer)){
    tcp_close(c->tcpPcb);
  }
}

void setTemperature(Connection * c){
  const char * body = c->http.getBody();
  const char * iter=body;
  const char * end = body + c->http.getBodyLength();
  printf("Body: %s\n",body );
  for(;iter < end; iter++){
    if (!isdigit(*iter)){
      printf("ERROR: body is not a number\n" );
      c->httpResponse.sendBadRequest(c);
      tcp_close(c->tcpPcb);
      return;
    }
  }
  threshold = atoi(body);
  printf("new threshold: %d\n",threshold );
  if (!c->httpResponse.sendOk(c,nullptr)){
    tcp_close(c->tcpPcb);
  }
}

void setEnable(Connection * c){
  const char * body = c->http.getBody();
  printf("Body: %s\n",body );
  if (strcmp(body, "1")){
    enable=true;
  }
  if (strcmp(body, "true")){
    enable=true;
  }
  if (strcmp(body, "0")){
    enable=false;
  }
  if (strcmp(body, "false")){
    enable=false;
  }

  printf("enable: %d\n",enable );
  if (!c->httpResponse.sendOk(c,nullptr)){
    tcp_close(c->tcpPcb);
  }
}

void setRestart(Connection * c){
  const char * body = c->http.getBody();
  printf("Body: %s\n",body );
  if (strcmp(body, "1")){
    restart=true;
  }
  if (strcmp(body, "true")){
    restart=true;
  }
  if (strcmp(body, "0")){
    restart=false;
  }
  if (strcmp(body, "false")){
    restart=false;
  }

  printf("restart: %d\n",restart );
  if (!c->httpResponse.sendOk(c,nullptr)){
    tcp_close(c->tcpPcb);
  }
}
