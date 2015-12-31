#ifndef __Connections__H__
#define __Connections__H__

#include "config.h"
#include "http.h"
#include "HttpResponse.h"


struct Connection {
public:
  struct tcp_pcb * tcpPcb;
  HttpResponse httpResponse;
  HTTP http;
  static Connection  * getEmptySlot();
  static void releseSlot(Connection * );
  static const Connection * end(){
    return endSlot;
  }
  static void init();
private:
  static Connection slots[MAX_CONN];
  static Connection * endSlot;
};


#endif
