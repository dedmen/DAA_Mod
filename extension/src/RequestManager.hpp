#pragma once


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

//template <typename TO, typename FROM>
//std::unique_ptr<TO> static_unique_pointer_cast(std::unique_ptr<FROM>&& old)
//{
//    return std::unique_ptr<TO>{static_cast<TO*>(old.release())};
//    //conversion: unique_ptr<FROM>->FROM*->TO*->unique_ptr<TO>
//}

extern int (*extensionCallback)(const char* name, const char* function, const char* data);

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