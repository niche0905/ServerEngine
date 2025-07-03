#pragma once


class RioCore
{
private:
	template<typename K, typename V>
	using Map = std::unordered_map<K, V>;
	template<typename T>
	using Vector = std::vector<T>;

public:
	RioCore();
	~RioCore();

	bool Init();		// 간단한 초기화 WSAStartUp, RIO 초기화, CQ/RQ 생성 등
	void Shutdown();	// 정지를 위한

	RIO_CQ GetCompletionQueue() const { return _completionQueue; }
	RIO_RQ GetRequestQueue() const { return _requestQueue; }

	bool Dispatch(DWORD timeoutMs = INFINITE);	// RIO 작업 처리 루프
	// TODO: 인자가 부족하지 않나? 바꾸어야 할 수도

	// 버퍼 관리 (등록/제거)
	RIO_BUFFERID RegisterBuffer(char* buffer, DWORD size);		// TODO: char를 BYTE로 바꾸기
	void DeregisterBuffer(RIO_BUFFERID id);

private:
	bool InitRio();		// RIO 함수 테이블 로드
	bool CreateCqRq();	// CQ, RQ 생성

private:
	RIO_EXTENSION_FUNCTION_TABLE	_rio;

	HANDLE							_iocpHandle = INVALID_HANDLE_VALUE;

	RIO_CQ							_completionQueue = RIO_INVALID_CQ;
	RIO_RQ							_requestQueue = RIO_INVALID_RQ;

	Map<SOCKET, RIO_RQ>				_socketToRQ;

	Vector<RIO_BUFFERID>			_registeredBuffers;

};
