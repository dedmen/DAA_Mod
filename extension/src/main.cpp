#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "DNSRequest.hpp"
#include "HTTPRequest.hpp"

int (*extensionCallback)(const char* name, const char* function, const char* data);

extern "C" {
    __declspec(dllexport) void __stdcall RVExtension(char* output, int outputSize, const char* function);
    __declspec(dllexport) int __stdcall RVExtensionArgs(char* output, int outputSize, const char* function, const char** argv, int argc);
    __declspec(dllexport) void __stdcall RVExtensionVersion(char* output, int outputSize);
    __declspec(dllexport) void RVExtensionRegisterCallback(int (*callbackProc)(const char* name, const char* function, const char* data));
}


class IAsyncRequest
{
public:
    virtual ~IAsyncRequest() = default;
    virtual void AddOnDoneHandler(std::function<void()> handler) = 0;
    virtual bool IsDone() = 0;
    virtual std::string_view GetResult() = 0;
    virtual std::string_view GetHandle() = 0;
};

class AsyncHTTPRequest : public IAsyncRequest
{
    std::shared_ptr<HTTPRequest> request;
    std::string handle;
public:
    AsyncHTTPRequest(std::shared_ptr<HTTPRequest> request, std::string handle) : request(std::move(request)), handle(std::move(handle))
    {}

    bool IsDone() override { return request->IsDone(); };
    std::string_view GetResult() override { return request->GetResult(); }
    std::string_view GetHandle() override { return handle; }
    void AddOnDoneHandler(std::function<void()> handler) override { request->AddOnDoneHandler(std::move(handler)); }
};

class AsyncDNSRequest : public IAsyncRequest
{
    std::shared_ptr<DNSRequest> request;
    std::string handle;
    std::optional<std::string> result;
public:
    AsyncDNSRequest(std::shared_ptr<DNSRequest> request, std::string handle) : request(std::move(request)), handle(std::move(handle))
    {}

    bool IsDone() override { return request->IsDone(); }

    std::string_view GetResult() override
    {
        if (!result)
            result = request->GetResultTXT();
        return *result;
    }

    std::string_view GetHandle() override { return handle; }
    void AddOnDoneHandler(std::function<void()> handler) override { request->AddOnDoneHandler(std::move(handler)); }
};

template <typename TO, typename FROM>
std::unique_ptr<TO> static_unique_pointer_cast(std::unique_ptr<FROM>&& old)
{
    return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
    //conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
}

class RequestManager
{
    std::vector<std::unique_ptr<IAsyncRequest>> requests;
    std::jthread workerThread;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic_uint16_t jobsDone; // jobs that are completed and waiting to be handled
    std::atomic_bool callbacksAllowed = false; // If we callback too soon during game startup, we might loose it because the EH is not registered and Arma will clear all stored callbacks

    void Run()
    {
        const auto stoptoken = workerThread.get_stop_token();
        while (!stoptoken.stop_requested())
        {
            std::unique_lock lk(mtx);
            cv.wait(lk, [this] { return jobsDone != 0; });

            if (stoptoken.stop_requested())
                return;

            if (!callbacksAllowed) // can't do yet, will do soon
                continue;

            auto completedJobs = std::ranges::remove_if(requests, [](const std::unique_ptr<IAsyncRequest>& req) { return req->IsDone(); });

            for (const auto& it : completedJobs)
            {
                --jobsDone;
                const auto handle = it->GetHandle();
                if (!handle.empty())
                    extensionCallback("DAA", it->GetHandle().data(), it->GetResult().data());
            }

            requests.erase(completedJobs.begin(), completedJobs.end());
        }
    }

    void OnDoneHandler()
    {
        std::unique_lock lk(mtx);
        ++jobsDone;
        cv.notify_one();
    }

public:
    RequestManager() : workerThread([this] { Run(); }) {}

    ~RequestManager()
    {
        StopThread();
    }

    void PushRequest(std::shared_ptr<HTTPRequest> request, std::string_view handle)
    {
        std::unique_lock lk(mtx);
        request->AddOnDoneHandler([this]() { OnDoneHandler(); });
        requests.emplace_back(std::make_unique<AsyncHTTPRequest>(std::move(request), std::string(handle)));
    }

    void PushRequest(std::shared_ptr<DNSRequest> request, std::string_view handle)
    {
        std::unique_lock lk(mtx);
        request->AddOnDoneHandler([this]() { OnDoneHandler(); });
        requests.emplace_back(std::make_unique<AsyncDNSRequest>(std::move(request), std::string(handle)));
    }

    void CallbacksReady()
    {
        if (callbacksAllowed) return;
        callbacksAllowed = true;
        cv.notify_all(); 
    }

    void StopThread()
    {
        workerThread.get_stop_source().request_stop();
        jobsDone = 500;
        cv.notify_all();
    }
};

RequestManager GRequestManager;

void __stdcall RVExtension(char* output, int outputSize, const char* function)
{
    const std::string_view func(function);

    if (func == "callbackReady")
    {
        GRequestManager.CallbacksReady();
    }
}

std::string_view unquote(std::string_view input)
{
    return input.substr(1, input.length() - 2); // "\"test\"" -> "test"
}

int __stdcall RVExtensionArgs(char* output, int outputSize, const char* function, const char** argv, int argc)
{
    const std::string_view func(function);

    auto makeHTTPRequest = [](HTTPRequest::RequestType type, const char** argv, int argc)
    {
        auto request = std::make_shared<HTTPRequest>(unquote(argv[1]), type);
        request->AddHeader("daa-api-auth-token", "872a1622-58ec-4d3a-91d2-13021fbe5368");

        if (argc >= 4)
        {
            auto headers = unquote(argv[3]);
            for (auto& header : Util::SplitString(headers, ';'))
            {
                auto headerSplit = Util::SplitString(header, ':');
                if (headerSplit.size() == 2)
                    request->AddHeader(headerSplit[0], headerSplit[1]);
            }
        }

        request->AddHeader("Content-Type", "text/plain");
        request->SetPostData(std::string(unquote(argv[2])));
        GRequestManager.PushRequest(request, unquote(argv[0]));
        request->StartRequest();
    };

    if (func == "post" && argc == 3) // "post", ["handle", "https://url", "postData", "header:value;header2:value2"]
    {
        makeHTTPRequest(HTTPRequest::RequestType::POST, argv, argc);
    }
    else if (func == "put" && argc >= 3) // "put", ["handle", "https://url", "postData", "header:value;header2:value2"]
    {
        makeHTTPRequest(HTTPRequest::RequestType::PUT, argv, argc);
    }
    else if (func == "get" && argc == 2) // "get", ["handle", "https://url"]
    {
        auto request = std::make_shared<HTTPRequest>(unquote(argv[1]), HTTPRequest::RequestType::GET);
        request->AddHeader("daa-api-auth-token", "872a1622-58ec-4d3a-91d2-13021fbe5368");
        GRequestManager.PushRequest(request, unquote(argv[0]));
        request->StartRequest();
    }
    else if (func == "dns" && argc == 2) // "dns", ["handle", "domain"] // only TXT
    {
        auto request = std::make_shared<DNSRequest>(unquote(argv[1]), DNSRequest::QueryType::TXT);
        GRequestManager.PushRequest(request, unquote(argv[0]));
        request->StartRequest();
    }

    return 0;
}

void __stdcall RVExtensionVersion(char* output, int outputSize)
{
    std::strncpy(output, "DAA Extension v1.0", outputSize - 1);
}

void RVExtensionRegisterCallback(int (*callbackProc)(const char* name, const char* function, const char* data))
{
    extensionCallback = callbackProc;
}
