#pragma once

#include <string>
#include <utility>

enum class ServiceErrorCode
{
    None = 0,
    InvalidArgument,
    NotFound,
    Unauthorized,
    Conflict,
    ExternalDependencyFailure,
    NotImplemented
};

struct ServiceError
{
    ServiceErrorCode Code = ServiceErrorCode::None;
    std::string Message;
};

template <typename TValue>
struct ServiceResult
{
    bool Success = false;
    TValue Value{};
    ServiceError Error{};

    static ServiceResult<TValue> Ok(TValue value)
    {
        ServiceResult<TValue> result;
        result.Success = true;
        result.Value = std::move(value);
        return result;
    }

    static ServiceResult<TValue> Fail(ServiceErrorCode code, std::string message)
    {
        ServiceResult<TValue> result;
        result.Success = false;
        result.Error = ServiceError{ code, std::move(message) };
        return result;
    }
};

struct VoidValue
{
};
