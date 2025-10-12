#pragma once

class IoObject;
class Session;
class SendBuffer;

enum class IoEventType : uint8
{
	None,			// default

	Connect,		// Connect는 비동기 연결의 결과를 나타냅니다
	Disconnect,		// Disconnect는 비동기 연결 해제의 결과를 나타냅니다
	Accept,			// Accept는 비동기 수신 연결의 결과를 나타냅니다
	Recv,			// Recv는 비동기 수신의 결과를 나타냅니다
	Send,			// Send는 비동기 송신의 결과를 나타냅니다

	End

};

/*-----------
   IoEvent
-----------*/
//
// IoEvent는 비동기 입출력의 결과를 나타냅니다
//

class IoEvent : public OVERLAPPED
{
public:
	IoEvent(IoEventType type = IoEventType::None);

	void Init();

public:
	IoEventType					eventType_;
	std::shared_ptr<IoObject>	owner_;

};

/*----------------
   ConnectEvent
----------------*/

class ConnectEvent : public IoEvent
{
public:
	ConnectEvent() : IoEvent(IoEventType::Connect) {}

};

/*-------------------
   DisconnectEvent
-------------------*/

class DisconnectEvent : public IoEvent
{
public:
	DisconnectEvent() : IoEvent(IoEventType::Disconnect) {}

};

/*---------------
   AcceptEvent
---------------*/

class AcceptEvent : public IoEvent
{
public:
	AcceptEvent() : IoEvent(IoEventType::Accept) {}

public:
	std::shared_ptr<Session>	newSession_ = nullptr;

};

/*--------------
   RecvEvent
--------------*/

class RecvEvent : public IoEvent
{
public:
	RecvEvent() : IoEvent(IoEventType::Recv) {}

};

/*--------------
   SendEvent
--------------*/

class SendEvent : public IoEvent
{
public:
	SendEvent() : IoEvent(IoEventType::Send) {}

public:
	std::vector<std::shared_ptr<SendBuffer>>	sendBuffers_;

};
