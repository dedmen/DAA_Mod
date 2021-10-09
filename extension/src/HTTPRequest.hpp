#pragma once

#include <future>
#include <memory>
#include <string_view>

#include "Util.hpp"

class HTTPRequest : public std::enable_shared_from_this<HTTPRequest>
{
public:
    struct InFlightRequest;

    enum class RequestType
    {
        GET,
        POST
    };

    HTTPRequest(std::string_view URL, RequestType type);
    ~HTTPRequest();

    void SetPostData(std::string data);

    void StartRequest();

    /// Did the async request complete yet?
    bool IsDone() const;

    // Did we complete successfully and have results ready
    bool IsSuccess() const;

    void AddOnDoneHandler(std::function<void()> handler);
    
    const std::string& GetResult() const;

private:
    // Stores all data for a request that is in progress
    std::unique_ptr<InFlightRequest> _currentRequest;
};
