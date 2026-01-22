#include "Utils.hpp"
#include "WinError.hpp"

#include <mmdeviceapi.h>
#include <shellapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h> // For Microsoft::WRL::ComPtr

using Microsoft::WRL::ComPtr;

struct PropVariantRII {
	PROPVARIANT var{};
	PropVariantRII() { PropVariantInit(&var); }
	~PropVariantRII() { PropVariantClear(&var); }
};

Result<std::wstring, Error> Utils::GetDeviceIconPath(const wchar_t *deviceId) {
	Microsoft::WRL::ComPtr<IMMDeviceEnumerator> pEnum;
	HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pEnum));
	if (FAILED(hr)) {
		return WinErr(hr, "Failed to create IMMDeviceEnumerator");
	}

	Microsoft::WRL::ComPtr<IMMDevice> pDevice;
	hr = pEnum->GetDevice(deviceId, &pDevice);
	if (FAILED(hr)) {
		return WinErr(hr, "Failed to get device by ID");
	}

	Microsoft::WRL::ComPtr<IPropertyStore> pStore;
	hr = pDevice->OpenPropertyStore(STGM_READ, &pStore);
	if (FAILED(hr)) {
		return WinErr(hr, "Failed to open device property store");
	}

	PropVariantRII rii; // RAII for PropVariant

	hr = pStore->GetValue(PKEY_DeviceClass_IconPath, &rii.var);
	if (FAILED(hr)) {
		return WinErr(hr, "Failed to get device icon path property");
	}

	return std::wstring(rii.var.pwszVal);
}

HICON Utils::ExtractDeviceIcon(const std::wstring &iconPath) {
	if (iconPath.empty()) return nullptr;

	std::wstring file;
	int resourceId = 0;
	const size_t commaPos = iconPath.find(L',');

	if (commaPos != std::wstring::npos) {
		file = iconPath.substr(0, commaPos);
		resourceId = std::stoi(iconPath.substr(commaPos + 1));
	} else {
		file = iconPath;
	}

	HICON hIcon = nullptr;
	ExtractIconExW(file.c_str(), resourceId, nullptr, &hIcon, 1);
	return hIcon;
}
