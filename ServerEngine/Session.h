#pragma once
#include <string_view>

/*------------
   Session
------------*/
//
// Session은 클라이언트와의 연결을 나타냅니다
//

//class IoObject;
//class SendBuffer;
//class Service;
//class NetAddress;

//class Session : public IoObject
//{
//	enum
//	{
//		BUFFER_SIZE = 0x1000,	// 4KB
//	};
//
//public:
//	Session();
//	virtual ~Session();
//
//public:
//	void Send(std::shared_ptr<SendBuffer> sendBuffer);
//	bool Connect(HANDLE socket);
//	void Disconnect(std::string_view cause);
//
//	std::shared_ptr<Service> GetService() { return service_.lock(); }
//	void SetService(std::shared_ptr<Service> service) { service_ = service; }
//
//public:
//	NetAddress GetNetAddress() { return address_; }
//	void SetNetAddress(NetAddress address) { address_ = address; }
//
//	SOCKET GetSocket() { return socket_; }
//	bool IsConnected() { return connected_; }
//	std::shared_ptr<Session> GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }
//
//private:
//	virtual HANDLE GetHandle() override;
//	virtual void Dispatch(class IoEvent* ioEvent, int32 numOfBytes = 0) override;
//
//private:
//	bool RegisterConnect();
//	bool RegisterDisconnect();
//	virtual void RegisterRecv();
//	virtual void RegisterSend();
//
//	void ProcessConnect();
//	void ProcessDisconnect();
//	virtual void ProcessRecv(int32 numOfBytes);
//	virtual void ProcessSend(int32 numOfBytes);
//
//	void HandleError(std::string_view functionName, int32 errorCode);
//
//protected:
//	virtual void OnConnected() {}
//	virtual void OnDisconnected() {}
//	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
//	virtual void OnSend(int32 len) {}
//
//private:
//	std::weak_ptr<Service> service_;
//	SOCKET socket_ = INVALID_SOCKET;
//	NetAddress address_ = {};
//
//	std::atomic<bool> connected_ = false;
//
//	... // TODO: 더 추가 해야 함
//
//};
//
