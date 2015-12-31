#ifndef __HTTP_RESPONSE__H__
#define  __HTTP_RESPONSE__H__

class Connection;
struct tcp_pcb;

class HttpResponse {
public:
  HttpResponse();
  ~HttpResponse(){delete []startBuffer;}
  bool sendOk(Connection * c, const char * response);
  void sendNotFound(Connection * c);
  void sendBadRequest(Connection * c);
  void release();
private:
  void addToBuffer(const char * data, int size);
  void resizeTo(int newMaxSize);

  char * startBuffer;
  int  used;
  int  maxSize;
};


#endif
