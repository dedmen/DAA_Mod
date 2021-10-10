#include "HTTPRequest.hpp"

#include <Windows.h>
#include <winhttp.h>
#pragma comment(lib, "Winhttp.lib")

#include <format>
#include "Util.hpp"


struct HTTPRequest::InFlightRequest
{
    bool isDone = false;
    uint16_t errorCode = 0;
    std::string result;
    Signal<void()> OnDone;

private:
    std::wstring QueryName;
    std::string requestData;
    std::wstring headerData;

    HINTERNET hSession = nullptr;
    HINTERNET hConnect = nullptr;
    HINTERNET hRequest = nullptr;

    RequestType reqType;
    std::wstring requestURL;

    std::wstring domain;
    std::wstring subURL;
    std::string resultTemp;

    void SetDone()
    {
        isDone = true;
        if (hRequest) WinHttpCloseHandle(hRequest);
        hRequest = nullptr;

        requestData.shrink_to_fit(); // should be empty now
        resultTemp.clear();
        resultTemp.shrink_to_fit();

        OnDone();
    }

    static void CALLBACK generalCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
    {
        auto myself = reinterpret_cast<InFlightRequest*>(dwContext);

        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE) // first
        {
            // dwStatusInformationLength == bytes sent
            // send post data here, even on GET we do this, requestData will just be empty

            // We consume requestData
            myself->requestData.erase(0, dwStatusInformationLength);

            if (myself->requestData.empty()) // no data to send anymore, wait for response
            {
                WinHttpReceiveResponse(hInternet, nullptr);
                return;
            }

            // post data, send data
            WinHttpWriteData(hInternet, myself->requestData.data(), myself->requestData.size(), nullptr); // will trigger WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE

            return;
        }

        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE) // second for post requests
        {
            // if there is lots of post data, we could write here again
            DWORD dataSent = *static_cast<DWORD*>(lpvStatusInformation);
            // We consume requestData
            myself->requestData.erase(0, dataSent);


            if (myself->requestData.empty()) // no data to send anymore, wait for response
            {
                WinHttpReceiveResponse(hInternet, nullptr);
                return;
            }

            // post data, send more data
            WinHttpWriteData(hInternet, myself->requestData.data(), myself->requestData.size(), nullptr); // will trigger WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE
            return;
        }

        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE) // second
        {
            DWORD statusCode = 0;
            DWORD statusCodeSize = sizeof(DWORD);

            WinHttpQueryHeaders(hInternet,
                                WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                                WINHTTP_HEADER_NAME_BY_INDEX,
                                &statusCode,
                                &statusCodeSize,
                                WINHTTP_NO_HEADER_INDEX);

            myself->errorCode = statusCode;

            // read data
            WinHttpQueryDataAvailable(hInternet, nullptr); // will trigger WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE
            return;
        }

        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE)
        {
            DWORD dataAvail = *static_cast<DWORD*>(lpvStatusInformation);
            if (dataAvail == 0) // 0 to read, we are done
            {
                myself->SetDone();
                return;
            }

            myself->resultTemp.resize(dataAvail + 1);

            WinHttpReadData(myself->hRequest, myself->resultTemp.data(), dataAvail, nullptr);
            return;
        }

        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_READ_COMPLETE)
        {
            // lpvStatusInformation == resultTemp.data()
            myself->result += std::string_view(static_cast<char*>(lpvStatusInformation), dwStatusInformationLength);

            // check if there is more
            WinHttpQueryDataAvailable(hInternet, nullptr); // will trigger WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE
            return;
        }

        if (dwInternetStatus == WINHTTP_CALLBACK_STATUS_REQUEST_ERROR)
        {
            auto res = static_cast<WINHTTP_ASYNC_RESULT*>(lpvStatusInformation);

            if (res->dwResult == ERROR_WINHTTP_OPERATION_CANCELLED)
                return; // out InFlightReques might already be deallocated

            //__debugbreak();
        }
        //__debugbreak();
    }

public:
    InFlightRequest(std::string_view url, RequestType type): reqType(type), requestURL(Util::UTF8ToUTF16(url))
    {
        // Use WinHttpOpen to obtain a session handle.
        hSession = WinHttpOpen(L"DAA Arma Extension/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);

        WinHttpSetStatusCallback(hSession, generalCallback, WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS, 0);
        LPVOID context = this;
        WinHttpSetOption(hSession, WINHTTP_OPTION_CONTEXT_VALUE, &context, sizeof(uintptr_t));
    }

    void SetPostData(std::string data)
    {
        requestData = std::move(data);
    }

    void AddHeader(std::string_view key, std::string_view value)
    {
        //if (!headerData.empty())
        headerData.append(Util::UTF8ToUTF16(std::format("{}: {}\r\n", key, value)));
    }

    void StartRequest()
    {
        URL_COMPONENTS urlComponents{};
        urlComponents.dwStructSize = sizeof(URL_COMPONENTS);

        domain.resize(256);
        urlComponents.lpszHostName = domain.data();
        urlComponents.dwHostNameLength = domain.size();

        subURL.resize(512);
        urlComponents.lpszUrlPath = subURL.data();
        urlComponents.dwUrlPathLength = subURL.size();

        WinHttpCrackUrl(requestURL.data(), requestURL.size(), 0, &urlComponents);

        domain.resize(urlComponents.dwHostNameLength);
        domain.shrink_to_fit();
        subURL.resize(urlComponents.dwUrlPathLength);
        subURL.shrink_to_fit();

        // Specify an HTTP server.
        if (hSession)
        {
            hConnect = WinHttpConnect(hSession, domain.data(), urlComponents.nPort, 0);
            //WinHttpSetStatusCallback(hConnect, generalCallback, WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS, 0);
        }

        if (!hConnect)
        {
            errorCode = 418; // I like tea
            SetDone();
            return;
        }

        std::wstring_view verb;
        switch (reqType)
        {
        case RequestType::GET: verb = L"GET";
            break;
        case RequestType::POST: verb = L"POST";
            break;
        case RequestType::PUT: verb = L"PUT";
            break;
        }


        hRequest = WinHttpOpenRequest(hConnect, verb.data(), subURL.empty() ? L"/" : subURL.data(), nullptr,  WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, urlComponents.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);

        if (!hRequest)
        {
            errorCode = 418; // I like tea
            SetDone();
            return;
        }

        //WinHttpSetStatusCallback(hRequest, generalCallback, WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS, 0);

        //LPVOID context = this;
        if (!WinHttpSendRequest(hRequest, headerData.data(), headerData.size(), requestData.data(), requestData.size(), requestData.size(), 0))
        {
            //auto err = GetLastError();
            //__debugbreak();
        }
    }

    void WaitForDone()
    {
        if (hSession && hConnect && hRequest)
            WinHttpReceiveResponse(hRequest, nullptr);
    }

    ~InFlightRequest()
    {
        if (hRequest) WinHttpCloseHandle(hRequest);
        if (hConnect) WinHttpCloseHandle(hConnect);
        if (hSession) WinHttpCloseHandle(hSession);
    }
};


HTTPRequest::HTTPRequest(std::string_view URL, RequestType type)
{
    _currentRequest = std::make_unique<InFlightRequest>(URL, type);
}

HTTPRequest::~HTTPRequest() = default;

void HTTPRequest::SetPostData(std::string data)
{
    if (_currentRequest)
        _currentRequest->SetPostData(std::move(data));
}

void HTTPRequest::AddHeader(std::string_view key, std::string_view value)
{
    if (_currentRequest)
        _currentRequest->AddHeader(key, value);
}

void HTTPRequest::StartRequest()
{
    _currentRequest->StartRequest();
}

bool HTTPRequest::IsDone() const
{
    if (!_currentRequest)
        return false;
    return _currentRequest->isDone;
}

bool HTTPRequest::IsSuccess() const
{
    if (!IsDone())
        return false;
    return _currentRequest->errorCode == 200;
}

void HTTPRequest::AddOnDoneHandler(std::function<void()> handler)
{
    _currentRequest->OnDone.Connect(std::move(handler));
}

const std::string& HTTPRequest::GetResult() const
{
    return _currentRequest->result;
}
