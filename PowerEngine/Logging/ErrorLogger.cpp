#include "ErrorLogger.h"
#include <comdef.h>

void ErrorLogger::Log(std::string message)
{
	std::string error_message = "Message: " + message;
	MessageBoxA(NULL, error_message.c_str(), "Message", MB_ICONINFORMATION);
}

void ErrorLogger::Log(HRESULT hr, std::string message)
{
	_com_error error(hr);
	if (FAILED(hr))
	{
		std::wstring error_message = L"Error: " + StringHelper::StringToWide(message) + L"\n" + error.ErrorMessage();
		MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
	}
	else if (S_OK(hr))
	{
		std::wstring error_message = L"Message: " + StringHelper::StringToWide(message) + L"\n" + error.ErrorMessage();
		MessageBoxW(NULL, error_message.c_str(), L"Message", MB_ICONINFORMATION);
	}
}

void ErrorLogger::Log(HRESULT hr, std::wstring message)
{
	_com_error error(hr);
	std::wstring error_message = L"Error: " + message + L"\n" + error.ErrorMessage();
	MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
}

void ErrorLogger::Log(COMException& exception)
{
	std::wstring error_message = exception.what();
	MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
}
