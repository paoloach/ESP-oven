#include <stdlib.h>
#include "Connections.h"


Connection   Connection::slots[MAX_CONN];
Connection * Connection::endSlot = Connection::slots + MAX_CONN;

void Connection::init(){
  for(Connection * iter=slots; iter !=endSlot; iter++ ){
    iter->tcpPcb=nullptr;
  }
}

void Connection::releseSlot(Connection * c) {
  c->tcpPcb = nullptr;
  c->httpResponse.release();
  c->http.init();
}

Connection * Connection::getEmptySlot() {
  for(Connection * iter=slots; iter !=endSlot; iter++ ){
    if (iter->tcpPcb==nullptr){
      return iter;
    }
  }
  return nullptr;
}
