#include "kerm_track.h"
#include "ipc_messages.h"

// Statics
KermitInstance *KermitInstance::_firstInstance = NULL;
int KermitInstance::_nextInstanceId = 1;
int KermitInstance::_priorityInstanceId = 1;

KermitInstance::KermitInstance(HWND hWnd, DWORD processId, 
		ConnectionProfile *profile) {
	
	_processId = processId;
	_hWnd = hWnd;
	_profile = profile;
	_status = S_IDLE;
	_instanceId = KermitInstance::_nextInstanceId++;

	// We're going to be the new last item in the list
	_nextInstance = NULL;

	// Now go put this instance at the end of the list.
	// Or at the start of the list if the list doesn't
	// exist yet...
	if (KermitInstance::_firstInstance == NULL) {
		// We're the first item in the list!
		KermitInstance::_firstInstance = this;
	} else {
		// Go find the last item in the list.
		KermitInstance *inst = _firstInstance;
		while (inst->_nextInstance != NULL) {
			inst = inst->_nextInstance;
		}
		inst->_nextInstance = this;
	}
}

KermitInstance* KermitInstance::nextAvailableInstance() {
	KermitInstance *inst;

	if (_priorityInstanceId != 0) {
		inst = getInstance(_priorityInstanceId);
		if (inst == NULL) {
			// Instance doesn't exist. Quit before it could
			// be assigned work?
			_priorityInstanceId = 0;
		}  else if (inst->status() != KermitInstance::S_READY) {
			// Found the instance but its not ready anymore.
			// Can't take it over.
			_priorityInstanceId = 0;
		} else {
			// Found it and its still ready. Good to go!
			return inst;
		}
		
	}

	inst = KermitInstance::_firstInstance;

	if (inst == NULL) return NULL;
	
	while (inst->_nextInstance != NULL) {
		if (inst->status() == KermitInstance::S_READY) {
			return inst;
		}
		inst = inst->_nextInstance;
	}
	return NULL;
}

void KermitInstance::setPriorityInstanceIdAvailable(int id) {

	KermitInstance::_priorityInstanceId = id;
}

KermitInstance* KermitInstance::getInstance(int id) {
	if (id == NULL) return NULL;

	KermitInstance *inst = KermitInstance::_firstInstance;

	if (inst == NULL) return NULL;
	
	while (inst != NULL) {
		if (inst->instanceId() == id) {
			return inst;
		}
		inst = inst->_nextInstance;
	}
	return NULL;
}

KermitInstance* KermitInstance::getInstanceByProcessId(int processId) {
	KermitInstance *inst = KermitInstance::_firstInstance;

	if (inst == NULL) return NULL;


	while (inst != NULL) {
		if (inst->processId() == processId) {
			return inst;
		}
		inst = inst->_nextInstance;
	}
	return NULL;
}

KermitInstance* KermitInstance::getInstanceByProfile(ConnectionProfile *profile) {
	KermitInstance *inst = KermitInstance::_firstInstance;

	if (inst == NULL) return NULL;

	while (inst != NULL) {
		if (inst->profile() != NULL && 
			inst->profile()->name() == profile->name()) {

			return inst;
		}
		inst = inst->_nextInstance;
	}
	return NULL;
}

void KermitInstance::setConnected() {
	setStatus(S_CONNECTED);
	GetSystemTime(&_connectTime);
}

void KermitInstance::setDisconnected() {
	setStatus(S_IDLE);
	GetSystemTime(&_disconnectTime);
}

void KermitInstance::removeInstance(int id) {
	KermitInstance *inst = KermitInstance::_firstInstance;

	if (inst == NULL) return;

	if (inst->instanceId() == id) {
		// We're deleting the head of the list. The next node
		// becomes the new head.
		_firstInstance = inst->_nextInstance;
		delete inst;
		return;
	}
	
	// else the one we're deleting is deeper in the list

	KermitInstance *next = inst->_nextInstance;

	while (next != NULL) {
		if (next->instanceId() == id) {
			inst->_nextInstance = next->_nextInstance;
			delete next;
			return;
		}
		inst = next;
		next = next->_nextInstance;
	}
}


KermitInstance* KermitInstance::nextInstanceWithSameProfile(BOOL connectedOnly) {
	KermitInstance *inst = this->_nextInstance;
	
	if (inst == NULL) return NULL;

	while (inst != NULL) {
		if ((!connectedOnly && inst->status() == KermitInstance::S_CONNECTING)
			|| inst->status() == KermitInstance::S_CONNECTED) {

			if (inst->profile()->name() == profile()->name()) {
				return inst;
			}
		}
		inst = inst->_nextInstance;
	}
	return NULL;

}

int KermitInstance::elapsedConnectTime() {

	if (status() != S_CONNECTED) {
		return 0;
	}

	// This is a bit complicated.
	SYSTEMTIME stNow;
	FILETIME ftConnected, ftNow;
	__int64 result = 0;

	GetSystemTime(&stNow);

	if (SystemTimeToFileTime(&_connectTime, &ftConnected) &&
		SystemTimeToFileTime(&stNow, &ftNow)) {
		
		ULARGE_INTEGER ulConnected, ulNow;

		ulConnected.LowPart = ftConnected.dwLowDateTime;
		ulConnected.HighPart = ftConnected.dwHighDateTime;

		ulNow.LowPart = ftNow.dwLowDateTime;
		ulNow.HighPart = ftNow.dwHighDateTime;

		result = ulNow.QuadPart - ulConnected.QuadPart;

		// Result is currently in 100-nanosecond intervals. Thats
		// not really the level of detail we need here.

		result = result / 10000000;
	}

	// Now that we're working in units of 1 second rather than
	// 100 nanosecond, a 32bit integer is more than large enough
	// to represent how long an instance has been connected.
	return (int)result;
}


void KermitInstance::requestDisconnect() {
	PostMessage(hwnd(), OPT_KERMIT_HANGUP, 0, 0);
}

void KermitInstance::notifyOfExit() {
	PostMessage(hwnd(), OPT_DIALER_EXIT, 0, 0);
}

void KermitInstance::takeScript(LPTSTR scriptFile, HWND hwndMainWindow) {
	ATOM atom = GlobalAddAtom(scriptFile);

	SendMessage(hwnd(), OPT_DIALER_HWND, instanceId(), (LPARAM)hwndMainWindow);
	SendMessage(hwnd(), OPT_DIALER_CONNECT, 0, (LPARAM)atom);
}


KermitInstance *KermitInstance::firstInstance() {
	return KermitInstance::_firstInstance;
}

KermitInstance *KermitInstance::nextInstance() {
	return this->_nextInstance;
}