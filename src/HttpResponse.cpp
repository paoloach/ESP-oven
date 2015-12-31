#include <string.h>
#include "lwip/tcp.h"

#include "HttpResponse.h"
#include "Connections.h"


const char statusLineOK[] = "HTTP/1.1 200 OK\r\n";
const char statusLineBadRequest[] = "HTTP/1.1 400 Bad request\r\n";
const char statusLineNotFound[] = "HTTP/1.1 404 Not Found\r\n";
const char contentLengthHeader[] = "Content-Length:";

HttpResponse::HttpResponse() {
  maxSize=100;
  startBuffer = new char[maxSize];
  used=0;
}

void HttpResponse::release() {
  delete []startBuffer;
  maxSize=100;
  startBuffer = new char[maxSize];
  used=0;
}

void HttpResponse::sendNotFound(Connection * c){
  addToBuffer(statusLineNotFound, sizeof(statusLineNotFound)-1);
  addToBuffer(contentLengthHeader,sizeof(contentLengthHeader)-1);
  addToBuffer("0\r\n",3);
  addToBuffer("\r\n",2);
  printf("[%s,%d] send %d bytes: \n%s",__FILE__, __LINE__, used, startBuffer );
  err_t error = tcp_write(c->tcpPcb, startBuffer, used,0);;
  if (error != ERR_OK){
    printf("[%s,%d] Error writing: %d",__FILE__, __LINE__, error );
  }
}

void HttpResponse::sendBadRequest(Connection * c){
  addToBuffer(statusLineBadRequest, sizeof(statusLineBadRequest)-1);
  addToBuffer(contentLengthHeader,sizeof(contentLengthHeader)-1);
  addToBuffer("0\r\n",3);
  addToBuffer("\r\n",2);
  printf("[%s,%d] send %d bytes: \n%s",__FILE__, __LINE__, used, startBuffer );
  err_t error = tcp_write(c->tcpPcb, startBuffer, used,0);;
  if (error != ERR_OK){
    printf("[%s,%d] Error writing: %d",__FILE__, __LINE__, error );
  }
}

bool HttpResponse::sendOk(Connection * c, const char * response){
  char buffer[10];
  int bufferLen;
  int dataLen=0;
  if (response!=nullptr){
    dataLen=strlen(response);
  }
  addToBuffer(statusLineOK, sizeof(statusLineOK)-1);
  addToBuffer(contentLengthHeader,sizeof(contentLengthHeader)-1);
  bufferLen = sprintf(buffer, "%d\r\n", dataLen);
  addToBuffer(buffer,bufferLen);
  addToBuffer("\r\n",2);
  if (dataLen > 0){
    addToBuffer(response, dataLen);
  }
  printf("[%s,%d] send %d bytes: \n%s",__FILE__, __LINE__, used, startBuffer );
  err_t error = tcp_write(c->tcpPcb, startBuffer, used,0);;
  if (error != ERR_OK){
    printf("[%s,%d] Error writing: %d",__FILE__, __LINE__, error );
    return false;
  }
  return true;
}

void HttpResponse::resizeTo(int newMaxSize) {
  char * newBuffer = new char[newMaxSize];
  memcpy(newBuffer, startBuffer, used);
  delete []startBuffer;
  startBuffer = newBuffer;
  maxSize = newMaxSize;
}

void HttpResponse::addToBuffer(const char * data, int size) {
  if (used + size+1 > maxSize){
    resizeTo( (1+(used+size+1)/100)*100);
  }
  memcpy(startBuffer+used, data, size);

  used += size;
  startBuffer[used]=0;
}
