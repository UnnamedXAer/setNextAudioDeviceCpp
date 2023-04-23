#include <stdio.h>
#include <initguid.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>
// #include "DefaultAudioChanger/PolicyConfig.h"
#include "PolicyConfig.h"

HRESULT setDefaultAudioPlaybackDevice(LPCWSTR devID)
{
  IPolicyConfig *pPolicyConfig = 0;
  ERole reserved = eConsole;

  HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigClient),
                                NULL, CLSCTX_ALL, __uuidof(IPolicyConfig), (LPVOID *)&pPolicyConfig);
  if (SUCCEEDED(hr) && pPolicyConfig)
  {
    hr = pPolicyConfig->SetDefaultEndpoint(devID, reserved);
    pPolicyConfig->Release();
  }
  return hr;
}

void findAudioDevices()
{

  IMMDeviceEnumerator *enumerator = NULL;
  IMMDeviceCollection *collection = NULL;
  IMMDevice *currentDefaultDevice = NULL;
  LPWSTR currentDefaultDeviceId = NULL;

  UINT count = 0;
  HRESULT hr = 0;

  auto releaseAll = [&]()
  {
    if (enumerator)
      enumerator->Release();
    if (collection)
      collection->Release();
    if (currentDefaultDevice)
      currentDefaultDevice->Release();
  };

  CoInitialize(NULL);

  hr = CoCreateInstance(CLSID_MMDeviceEnumerator, 0, CLSCTX_ALL, IID_IMMDeviceEnumerator,
                        reinterpret_cast<void **>(&enumerator));

  if (enumerator)
  {
    hr = enumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &collection);
  }

  if (!collection)
  {
    return;
  }

  hr = collection->GetCount(&count);

  if (FAILED(hr) || count < 2)
  {
    releaseAll();
    return;
  }

  hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &currentDefaultDevice);
  if (FAILED(hr) || !currentDefaultDevice)
  {
    releaseAll();
    return;
  }

  hr = currentDefaultDevice->GetId(&currentDefaultDeviceId);
  if (FAILED(hr) || !currentDefaultDeviceId)
  {
    releaseAll();
    return;
  }

  LPWSTR deviceId;
  hr = currentDefaultDevice->GetId(&deviceId);
  if (FAILED(hr) || !deviceId)
  {
    releaseAll();
    return;
  }

  for (UINT i = 0; i < count; ++i)
  {
    IMMDevice *endpoint = NULL;
    LPWSTR deviceId;
    DWORD dwState;

    hr = collection->Item(i, &endpoint);

    if (FAILED(hr))
    {
      continue;
    }

    hr = endpoint->GetId(&deviceId);
    if (FAILED(hr))
    {
      ;
    }
    endpoint->Release();

    if (deviceId != currentDefaultDeviceId)
    {
      hr = setDefaultAudioPlaybackDevice(deviceId);
      if (FAILED(hr))
      {
        ;
      }
      continue;
    }
  }

  releaseAll();
}

int main()
{
  findAudioDevices();

  return 0;
}
