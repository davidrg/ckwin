#ifndef KERM_TRACK_H
#define KERM_TRACK_J

#include <windows.h>

#include "conn_profile.h"

// linked list of kermit instances we're tracking. These
// are all either instances the we launched, or instances
// that advertised their existance to us.
class KermitInstance {
public:
	enum Status {
		S_READY,
		S_IDLE,
		S_CONNECTING,
		S_CONNECTED
	};

	// Insert a new instance
	KermitInstance(HWND hWnd, DWORD processId, 
		ConnectionProfile *profile);

	// Accessor methods
	Status status() { return _status; }
	void setStatus(Status s) { _status = s; }

	HWND hwnd() { return _hWnd; }
	void setHwnd(HWND hWnd) { _hWnd = hWnd; }

	DWORD processId() { return _processId; }
	void setProcessId(DWORD processId) { _processId = processId; }

	//HANDLE process() { return _hProcess; }
	ConnectionProfile* profile() { return _profile; }
	void setProfile(ConnectionProfile* profile) { _profile = profile; }

	int instanceId() { return _instanceId; }

	// Get elapsed connect time in seconds.
	int elapsedConnectTime();

	SYSTEMTIME connectTime() { return _connectTime; }

	void setConnected();
	void setDisconnected();

	// Finds the next instance with the same ConnectionProfile.
	KermitInstance *nextInstanceWithSameProfile(BOOL connectedOnly);

	KermitInstance *nextInstance();

	// Ask the kermit instance to disconnect
	void requestDisconnect();

	// Let the kermit instance know the connection manager is terminating
	void notifyOfExit();

	// Ask the kermit instance to run the specified script
	void takeScript(LPTSTR scriptFile, HWND hwndMainWindow);

	// --- Handy functions for doing things ---

	// Gets the next instance thats ready to be taken over with a new
	// connection. May return nothing at all.
	static KermitInstance *nextAvailableInstance();

	// Indicates this should be the next instance that
	// nextAvailableInstance() returns.
	static void setPriorityInstanceIdAvailable(int id);

	// Gets the ID that will be handed out to the next C-Kermit instance
	// we track
	static int nextInstanceId() { return _nextInstanceId; }

	// Get an instance by its instance Id. If the instance Id is 0, null is
	// returned.
	static KermitInstance *getInstance(int id);

	// Or get one by process Id
	static KermitInstance *getInstanceByProcessId(int processId);

	// Get by profile. This compared profiles by name rather than
	// pointer value.
	static KermitInstance *getInstanceByProfile(ConnectionProfile *profile);

	static KermitInstance *firstInstance();

	static void removeInstance(int id);
	
	static BOOL anyInstances();

private:
	static int _nextInstanceId;
	static KermitInstance *_firstInstance;
	static int _priorityInstanceId;

	// Next item in the linked list
	KermitInstance *_nextInstance;

	//HANDLE _hProcess;
	HWND _hWnd;
	ConnectionProfile *_profile;
	Status _status;
	DWORD _processId;
	int _instanceId;

	// In UTC
	SYSTEMTIME _connectTime;
	SYSTEMTIME _disconnectTime;
};


#endif /* KERM_TRACK_H */