#include "../include/audiocapture.h"
#include "../include/debug.h"
#include "../include/config.h"
#include <Functiondiscoverykeys_devpkey.h>

const IID IID_IAudioCaptureClient(__uuidof(IAudioCaptureClient));
const CLSID CLSID_MMDeviceEnumerator(__uuidof(MMDeviceEnumerator));
const IID IID_IMMDeviceEnumerator(__uuidof(IMMDeviceEnumerator));
const IID IID_IAudioClient(__uuidof(IAudioClient));

std::mutex CADataCapture::sm_mutexWait;

unsigned int __stdcall CADataCapture::ExInitialService_(PVOID pParam)
{
	((CADataCapture*)pParam)->ExInitial_();
	return 0;
}

CADataCapture::CADataCapture() : IMMNotificationClient()
{
	m_pAudioClient_ = NULL;
	m_pCaptureClient_ = NULL;
	m_pEnumerator_ = NULL;
	m_pDevice_ = NULL;
	m_pwfx_ = NULL;
	m_numFramesAvailable_ = 0;
	m_packetLength_ = 0;
	m_flags_ = 0;
	m_changing_ = false;
	m_start_ = false;
	m_role_ = ERole_enum_count;
}

CADataCapture::~CADataCapture()
{
	m_pEnumerator_->UnregisterEndpointNotificationCallback(this);
	CoTaskMemFree(m_pwfx_);
	if (m_pCaptureClient_ != NULL) m_pCaptureClient_->Release();
	if (m_pAudioClient_ != NULL) m_pAudioClient_->Release();
	if (m_pDevice_ != NULL) m_pDevice_->Release();
	if (m_pEnumerator_ != NULL) m_pEnumerator_->Release();
	CoUninitialize();
}

HRESULT CADataCapture::Initial()
{

	HRESULT hr;
	hr = CoInitialize(NULL);
	if (RPC_E_CHANGED_MODE == hr) {
		LOG_ERROR_CODE("Faild to CoInitialize", hr);
		exit(1);
	}

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&m_pEnumerator_);

	if (FAILED(hr)) {
		LOG_ERROR_CODE("Faild to CoCreateInstance", hr);
		exit(1);
	}
	m_pEnumerator_->RegisterEndpointNotificationCallback(this);

	return S_OK;
}

HRESULT CADataCapture::ExInitial_()
{
	HRESULT hr;

	hr = CoInitialize(NULL);
	if (RPC_E_CHANGED_MODE == hr) {
		LOG_ERROR_CODE("Faild to CoInitialize", hr);
		exit(1);
	}

	// get default output audio endpoint
	hr = m_pEnumerator_->GetDefaultAudioEndpoint(eRender, eMultimedia, &m_pDevice_);
	if (FAILED(hr)) {
		LOG_ERROR_CODE("Faild to GetDefaultAudioEndpoint", hr);
		exit(1);
	}

	// activates device
	hr = m_pDevice_->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&m_pAudioClient_);
	if (FAILED(hr)) {
		LOG_ERROR_CODE("Faild to Activate Decive", hr);
		exit(1);
	}

	// gets audio format
	hr = m_pAudioClient_->GetMixFormat(&m_pwfx_);
	if (FAILED(hr)) {
		LOG_ERROR_CODE("Faild to GetMixFormat", hr);
		exit(1);
	}
	/*printf("\nGetMixFormat...\n");
	std::cout << "wFormatTag      : " << m_pwfx_->wFormatTag << std::endl
		<< "nChannels       : " << m_pwfx_->nChannels << std::endl
		<< "nSamplesPerSec  : " << m_pwfx_->nSamplesPerSec << std::endl
		<< "nAvgBytesPerSec : " << m_pwfx_->nAvgBytesPerSec << std::endl
		<< "nBlockAlign     : " << m_pwfx_->nBlockAlign << std::endl
		<< "wBitsPerSample  : " << m_pwfx_->wBitsPerSample << std::endl
		<< "cbSize          : " << m_pwfx_->cbSize << std::endl << std::endl;*/
	
	hr = m_pAudioClient_->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, REFTIMES_PER_SEC, 0, m_pwfx_, NULL);
	if (FAILED(hr)) {
		// Compatibility with the Nahimic audio driver
		// https://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/bd8cd9f2-974f-4a9f-8e9c-e83001819942/iaudioclient-initialize-failure
		LOG_WARN("Faild to Initialize Audio Client");
		m_pwfx_->nChannels = 2;
		m_pwfx_->nBlockAlign = (2 * m_pwfx_->wBitsPerSample) / 8;
		m_pwfx_->nAvgBytesPerSec = m_pwfx_->nSamplesPerSec * m_pwfx_->nBlockAlign;
		hr = m_pAudioClient_->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, REFTIMES_PER_SEC, 0, m_pwfx_, NULL);
		if (FAILED(hr)) {
			// stereo waveformat didnt work either, throw an error
			LOG_ERROR_CODE("Faild to Initialize Audio Client", hr);
			exit(1);
		}
	}

	hr = m_pAudioClient_->GetService(IID_IAudioCaptureClient, (void**)&m_pCaptureClient_);
	if (FAILED(hr)) {
		LOG_ERROR_CODE("Faild to GetService", hr);
		exit(1);
	}
	return S_OK;
}

// 防止调用ExInitial中无效的m_pAudioClient_->Initialize造成主程序阻塞
bool CADataCapture::StartExInitialService()
{
	unsigned int _id;
	HANDLE _handle = (HANDLE)_beginthreadex(NULL, 0, ExInitialService_, this, 0, &_id);
	DWORD _ret = WaitForSingleObject(_handle, 1500);
	if (WAIT_OBJECT_0 != _ret)
	{
		CloseHandle(_handle);
		return false;
	}
	return true;
}


HRESULT CADataCapture::Start()
{
	HRESULT hr = m_pAudioClient_->Start();
	if (FAILED(hr))
	{
		LOG_ERROR_CODE("Failed to Start", hr);
		exit(1);
	}
	m_start_ = true;
	return hr;
}

HRESULT CADataCapture::Stop()
{
	 HRESULT hr = m_pAudioClient_->Stop();
	if (FAILED(hr))
	{
		LOG_ERROR_CODE("Failed to Stop", hr);
		exit(1);
	}
	m_start_ = false;
	return hr;
}

HRESULT CADataCapture::Reset()
{
	bool _start = m_start_;
	HRESULT hr = Stop();
	hr = m_pAudioClient_->Reset();
	if (FAILED(hr))
	{
		LOG_ERROR_CODE("Failed to Reset", hr);
		exit(1);
	}
	if(_start)
		hr = Start();
	return hr;
}

HRESULT CADataCapture::GetNextPacketSize()
{
	return m_pCaptureClient_->GetNextPacketSize(&m_packetLength_);
}

HRESULT CADataCapture::GetBuffer(float** dataBuff)
{
	return m_pCaptureClient_->GetBuffer((BYTE**)dataBuff, &m_numFramesAvailable_, &m_flags_, NULL, NULL);
}

HRESULT CADataCapture::ReleaseBuffer()
{
	return m_pCaptureClient_->ReleaseBuffer(m_numFramesAvailable_);
}

bool CADataCapture::IsChanging()
{
	return m_changing_;
}

bool CADataCapture::ReStart()
{
	bool _ret = StartExInitialService();

	if (m_start_ && _ret)
	{
		Start();
	}
	return _ret;
}

UINT32 CADataCapture::GetNumFramesAvailable()
{
	return m_numFramesAvailable_;
}

UINT32 CADataCapture::GetPacketLength()
{
	return m_packetLength_;
}

DWORD CADataCapture::GetFlags()
{
	return m_flags_;
}

HRESULT STDMETHODCALLTYPE CADataCapture::QueryInterface(REFIID riid, void** ppvObject)
{
	if (riid == __uuidof(IUnknown))
	{
		AddRef();
		*ppvObject = static_cast<IUnknown*>(this);
	}
	else if (riid == __uuidof(IMMNotificationClient))
	{
		AddRef();
		*ppvObject = static_cast<IMMNotificationClient*>(this);
	}
	else
	{
		*ppvObject = nullptr;
		return E_NOINTERFACE;
	}

	return S_OK;
}

ULONG STDMETHODCALLTYPE CADataCapture::AddRef(void)
{
	return InterlockedIncrement(&m_referenceCount_);
}

ULONG STDMETHODCALLTYPE CADataCapture::Release(void)
{
	auto count = InterlockedDecrement(&m_referenceCount_);
	if (count == 0)
	{
		delete this;
	}
	return count;
}

HRESULT STDMETHODCALLTYPE CADataCapture::OnDefaultDeviceChanged(
	_In_ EDataFlow flow,
	_In_ ERole role,
	_In_ LPCWSTR pwstrDefaultDeviceId
)
{	
	if (!m_pEnumerator_)
	{
		return S_OK;
	}
	if (m_role_ == ERole_enum_count)
	{
		m_role_ = role;
	}
	else
	{
		if (m_role_ != role)
		{
			return S_OK;
		}
	}
	//LOG_INFO("Device Changed!");
	bool device_reboot_flag = false;
	ReadAdvancedConfig(&device_reboot_flag);
	if (device_reboot_flag)
	{
		LOG_INFO("Exit on device changed!");
		exit(0);
	}
	m_changing_ = true;
	sm_mutexWait.lock();

	char before_device[MAX_CHAR_LENGTH/2] = "Unknow";
	char after_device[MAX_CHAR_LENGTH/2] = "Unknow";

	IPropertyStore* pProperties = NULL;
	PROPVARIANT varName;
	PropVariantInit(&varName);
	HRESULT hr = m_pDevice_->OpenPropertyStore(STGM_READ, &pProperties);
	if (hr == S_OK) 
	{
		hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName);
		TCHAR2Char(varName.pwszVal, before_device);
	}
	
	m_pDevice_->Release();
	hr = m_pEnumerator_->GetDevice(pwstrDefaultDeviceId, &m_pDevice_);
	if (FAILED(hr))
	{
		LOG_ERROR_CODE("GetDevice Error", hr);
		m_changing_ = false;
		sm_mutexWait.unlock();
		exit(1);
	}

	hr = m_pDevice_->OpenPropertyStore(STGM_READ, &pProperties);
	if (hr == S_OK)
	{
		hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName);
		TCHAR2Char(varName.pwszVal, after_device);
	}

	char targetString[MAX_CHAR_LENGTH];
	snprintf(targetString,
		sizeof(targetString),
		"Device Changed:{%s}->{%s}",
		before_device,
		after_device);
	LOG_INFO(targetString);

	ReStart();
	sm_mutexWait.unlock();
	m_changing_ = false;
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CADataCapture::OnDeviceStateChanged(_In_  LPCWSTR pwstrDeviceId, _In_  DWORD dwNewState)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CADataCapture::OnDeviceAdded(_In_  LPCWSTR pwstrDeviceId)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CADataCapture::OnDeviceRemoved(_In_  LPCWSTR pwstrDeviceId)
{
	return S_OK;
}

HRESULT STDMETHODCALLTYPE CADataCapture::OnPropertyValueChanged(_In_  LPCWSTR pwstrDeviceId, _In_  const PROPERTYKEY key)
{
	return S_OK;
}