#pragma once

#include <functional>
#include <memory>
#include <string_view>

class DNSRequest : public std::enable_shared_from_this<DNSRequest>
{
public:
    struct InFlightRequest;

    enum class QueryType
    {
        TXT
    };

    DNSRequest(std::string_view domain, QueryType type);
    ~DNSRequest();

    void StartRequest();

    /// Did the async request complete yet?
    bool IsDone() const;

    // Did we complete successfully and have results ready
    bool IsSuccess() const;

    void AddOnDoneHandler(std::function<void()> handler);

    // Get result as TXT request. 
    std::string GetResultTXT() const;

private:
    // Stores all data for a request that is in progress
    std::unique_ptr<InFlightRequest> _currentRequest;
};