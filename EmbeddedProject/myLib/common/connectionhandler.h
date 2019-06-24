/*
  FILE: connectionhandler.h
  Created on: 9/19/2016, by Tom Diaz
*/

#ifndef CONNECTIONHANDLER_H
#define CONNECTIONHANDLER_H

/*class ConnectionSource
{
public:
	enum Value
	{
		RS232,
		STLink,
		None,
	};
};*/

//-------------------------------------------------------------ConnectionHandler
class ConnectionHandler : public x_Handler
{
public:
  void fsm();
  void SetState(ConnectionSource::Value s);
  ConnectionSource::Value GetState();
  void Ping(ConnectionSource::Value c);
  ConnectionSource::Value PHY();
  ConnectionSource::Value Connected();
private:
  void extraInit();
  void handleTimeout();
  ConnectionSource::Value connected;
  bool connPast;
  ConnectionSource::Value phyState;
  uint16_t count;
  uint16_t pingCount;
};

#endif /* CONNECTIONHANDLER_H */
