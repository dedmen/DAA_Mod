#include <string_view>
#include <thread>
#include <utility>
#include <vector>
#include <ranges>

#include "DNSRequest.hpp"
#include "HTTPRequest.hpp"
#include "RequestManager.hpp"

int (*extensionCallback)(const char* name, const char* function, const char* data);

extern "C" {
    __declspec(dllexport) void __stdcall RVExtension(char* output, unsigned int outputSize, const char* function);
    __declspec(dllexport) int __stdcall RVExtensionArgs(char* output, unsigned int outputSize, const char* function, const char** argv, unsigned int argc);
    __declspec(dllexport) void __stdcall RVExtensionVersion(char* output, unsigned int outputSize);
    __declspec(dllexport) void RVExtensionRegisterCallback(int (*callbackProc)(const char* name, const char* function, const char* data));
}

RequestManager GRequestManager;
std::vector<std::pair<std::string, std::string>> GlobalHTTPHeaders;

void MakeHTTPRequest(HTTPRequest::RequestType type, const char** argv, unsigned int argc)
{
    const auto request = std::make_shared<HTTPRequest>(Util::UnQuote(argv[1]), type);
    bool hasContentType = false;

    auto HeadersIndex = -1;
    auto PostDataIndex = -1;

    switch (type)
    {
    case HTTPRequest::RequestType::GET: // [handle, url(, headers)]
        HeadersIndex = argc >= 3 ? 2 : -1;
        break;
    case HTTPRequest::RequestType::POST: // [handle, url, postData(, headers)]
        [[fallthrough]];
    case HTTPRequest::RequestType::PUT:
        HeadersIndex = argc >= 4 ? 3 : -1;
        PostDataIndex = 2;
        break;
    }

    // Headers
    {
        auto addHeader = [request, &hasContentType](std::string_view headerKey, std::string_view headerValue)
            {
                request->AddHeader(headerKey, headerValue);

                switch (FnvHash{}.AddStringCI(headerKey))
                {
                case "content-type"_fnvHash: hasContentType = true; break;
                default:;
                }
            };

        if (HeadersIndex != -1)
        {
            for (const auto [headerKey, headerValue] : Util::SplitHeaderString(argv[HeadersIndex]))
                addHeader(headerKey, headerValue);
        }

        for (const auto& [headerKey, headerValue] : GlobalHTTPHeaders)
            addHeader(headerKey, headerValue);
    }

    // Post Data
    if (PostDataIndex != -1)
    {
        if (!hasContentType)
            request->AddHeader("Content-Type", "text/plain");
        request->SetPostData(Util::ParseSQFString(argv[PostDataIndex]));
    }

    GRequestManager.PushRequest(request, Util::UnQuote(argv[0]));
    request->StartRequest();
}

void __stdcall RVExtension(char* output, unsigned int outputSize, const char* function)
{
    const std::string_view func(function);

    if (func == "callbackReady")
    {
        GRequestManager.CallbacksReady();
    }
}

int __stdcall RVExtensionArgs(char* output, unsigned int outputSize, const char* function, const char** argv, unsigned int argc)
{
    const std::string_view func(function);

    bool hadError = false;

    auto setError = [&output, outputSize, &hadError](std::string_view error)
    {
        const auto len = std::min(static_cast<size_t>(outputSize - 1), error.length());
        memcpy(output, error.data(), len);
        output[len] = 0;
        hadError = true;
    };


    switch (FnvHash{}.AddStringCI(func))
    {
        case "post"_fnvHash: // "post", ["handle", "https://url", "postData", "header:value;header2:value
        {
            if (argc < 3)
            {
                setError("POST request wrong number of arguments, requires atleast 3 [handle, url, postData(, headers)]");
                break;
            }

            MakeHTTPRequest(HTTPRequest::RequestType::POST, argv, argc);
        } break;
        case "put"_fnvHash: // "put", ["handle", "https://url", "postData", "header:value;header2:value2"]
        {
            if (argc < 3)
            {
                setError("PUT request wrong number of arguments, requires atleast 3 [handle, url, postData(, headers)]");
                break;
            }
            MakeHTTPRequest(HTTPRequest::RequestType::PUT, argv, argc);
        } break;
        case "get"_fnvHash: // "get", ["handle", "https://url"(, headers)]
        {
            if (argc < 2)
            {
                setError("GET request wrong number of arguments, requires 2 [handle, url(, headers)]");
                break;
            }

            MakeHTTPRequest(HTTPRequest::RequestType::GET, argv, argc);
        } break;
        case "dns"_fnvHash: // "dns", ["handle", "domain"] // only TXT
        {
            if (argc != 2)
            {
                setError("DNS request wrong number of arguments, requires 2 [handle, domain]");
                break;
            }
            const auto request = std::make_shared<DNSRequest>(Util::UnQuote(argv[1]), DNSRequest::QueryType::TXT);
            GRequestManager.PushRequest(request, Util::UnQuote(argv[0]));
            request->StartRequest();
        } break;
        case "addglobalheader"_fnvHash: // "addGlobalHeader", ["headerName:headerValue"]
        {
            if (argc != 1)
            {
                setError("AddGlobalHeader request wrong number of arguments, requires 1 ['headerName:headerValue']");
                break;
            }

            for (const auto [headerKey, headerValue] : Util::SplitHeaderString(argv[0]))
                GlobalHTTPHeaders.emplace_back(headerKey, headerValue);
        } break;
        case "clearglobalheaders"_fnvHash: // "clearGlobalHeaders", []
        {
            if (argc != 0)
            {
                setError("ClearGlobalHeaders request wrong number of arguments, requires 0 []");
                break;
            }

            GlobalHTTPHeaders.clear();
        } break;

        default:
        {
            setError("Invalid function");
            break;
        }
    }

    return hadError ? 1 : 0;
}

void __stdcall RVExtensionVersion(char* output, unsigned int outputSize)
{
    std::strncpy(output, "DAA Extension v1.0", outputSize - 1);
}

void RVExtensionRegisterCallback(int (*callbackProc)(const char* name, const char* function, const char* data))
{
    extensionCallback = callbackProc;
}
