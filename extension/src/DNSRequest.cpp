#include "DNSRequest.hpp"
#include <Windows.h>
#include <WinDNS.h>
//#include <urlmon.h>
//#pragma comment(lib,"urlmon.lib")
#pragma comment(lib, "Dnsapi.lib")

#include "Util.hpp"

struct DNSRequest::InFlightRequest {
    std::wstring QueryName;
    DNS_QUERY_REQUEST request {};
    DNS_QUERY_RESULT results {};
    DNS_QUERY_CANCEL cancel {};

    Signal<void()> OnDone;

    bool isDone = false;

    void Cancel() {
        if (!isDone) {
            // Windows 7 compatibility - We know that DNSApi.dll was already loaded for DNSQuery usage
            if (const auto dnsModule = GetModuleHandleA("Dnsapi.dll"))
            {
                if (const auto dnsCancelQueryAddr = GetProcAddress(dnsModule, "DnsCancelQuery"))
                {
                    using dnsCancelQueryT = DNS_STATUS(WINAPI*)(_In_ PDNS_QUERY_CANCEL pCancelHandle);
                    const auto dnsCancelQuery = reinterpret_cast<dnsCancelQueryT>(dnsCancelQueryAddr);
                    dnsCancelQuery(&cancel);
                    isDone = true;
                    return;
                }
            }
        }
        isDone = true;
    }

    ~InFlightRequest()
    {
        Cancel();
        if (results.pQueryRecords)
            DnsRecordListFree(results.pQueryRecords, DnsFreeRecordList);
    }
};

void WINAPI DnsQueryCompletionRoutine(PVOID pQueryContext, PDNS_QUERY_RESULT pQueryResults) {
    const auto context = static_cast<DNSRequest::InFlightRequest*>(pQueryContext);
    context->isDone = true;
    context->OnDone();
}

DNSRequest::DNSRequest(std::string_view domain, QueryType type) {
    _currentRequest = std::make_unique<InFlightRequest>();
    auto& requestData = _currentRequest->request;

    requestData.Version = DNS_QUERY_REQUEST_VERSION1;
    _currentRequest->QueryName = Util::UTF8ToUTF16(domain);
    requestData.QueryName = _currentRequest->QueryName.data();

    switch (type) {
        case QueryType::TXT:    requestData.QueryType = DNS_TYPE_TEXT; break;
    }

    requestData.QueryOptions = DNS_QUERY_TREAT_AS_FQDN | DNS_QUERY_BYPASS_CACHE;
    requestData.pDnsServerList = nullptr;
    requestData.InterfaceIndex = 0;
    requestData.pQueryCompletionCallback = DnsQueryCompletionRoutine;
    requestData.pQueryContext = _currentRequest.get();

    _currentRequest->results.Version = DNS_QUERY_REQUEST_VERSION1;
}

DNSRequest::~DNSRequest() {
    if (_currentRequest)  {
        _currentRequest->Cancel();
    }
}

void DNSRequest::StartRequest()
{
    if (const auto dnsModule = GetModuleHandleA("Dnsapi.dll"))
    {
        if (const auto dnsQueryAddr = GetProcAddress(dnsModule, "DnsQueryEx"))
        {
            using dnsQueryExT = DNS_STATUS(WINAPI*)(
                _In_        PDNS_QUERY_REQUEST  pQueryRequest,
                _Inout_     PDNS_QUERY_RESULT   pQueryResults,
                _Inout_opt_ PDNS_QUERY_CANCEL   pCancelHandle);

            const auto dnsQueryEx = reinterpret_cast<dnsQueryExT>(dnsQueryAddr);

            const auto result = dnsQueryEx(&_currentRequest->request, &_currentRequest->results, &_currentRequest->cancel);

            if (result != DNS_REQUEST_PENDING) // this can happen, but shouldn't
                _currentRequest->isDone = true;

            return;
        }
    }

    // Fallback to old sync DnsQuery

    _currentRequest->results.QueryStatus = DnsQuery_W(_currentRequest->QueryName.data(), _currentRequest->request.QueryType, _currentRequest->request.QueryOptions, nullptr, &_currentRequest->results.pQueryRecords, nullptr);
    _currentRequest->results.QueryOptions = _currentRequest->request.QueryOptions;
    _currentRequest->isDone = true; // DnsQuery is synchronous
}

bool DNSRequest::IsDone() const {
    if (!_currentRequest)
        return true;

    return _currentRequest->isDone;
}

bool DNSRequest::IsSuccess() const {
    if (!_currentRequest)
        return false;

    if (_currentRequest->results.QueryStatus != ERROR_SUCCESS)
        return false;

    if (!_currentRequest->results.pQueryRecords)
        return false;

    return true;
}

void DNSRequest::AddOnDoneHandler(std::function<void()> handler)
{
    if (_currentRequest)
        _currentRequest->OnDone.Connect(std::move(handler));
}

std::string DNSRequest::GetResultTXT() const
{
    if (!IsSuccess()) return {};

    if (_currentRequest->results.pQueryRecords->wType != DNS_TYPE_TEXT) return {};

    const auto& result = reinterpret_cast<PDNS_RECORDW>(_currentRequest->results.pQueryRecords)->Data.TXT;

    std::string resultStr;

    for (size_t i = 0; i < result.dwStringCount; ++i)
        resultStr.append(Util::UTF16ToUTF8(result.pStringArray[i]));

    return resultStr;
}
