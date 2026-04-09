#pragma once

#include "TestClient/Application/TestClientModels.h"
#include "TestClient/Transport/TestClientConnection.h"

#include <cstdint>
#include <string>

class TestClientService
{
public:
    bool Connect(const std::string& host, uint16_t port, std::string& errorMessage);
    void Disconnect();
    bool IsConnected() const;

    ClientOperationResult RegisterLocal(const ClientRegisterRequest& request);
    ClientOperationResult LoginLocal(const ClientLoginRequest& request);
    ClientOperationResult LoginSocial(const ClientSocialLoginRequest& request);
    ClientOperationResult ValidateGameSession(const std::string& gameSessionToken);
    ClientOperationResult SubmitMatchResult(const ClientMatchRequest& request);
    ClientOperationResult QueryPlayerStats(int64_t userId);
    ClientOperationResult QueryMonitoringSnapshot();

private:
    template <typename TResponse>
    bool ReceiveTypedPacket(uint16_t expectedOpcode, TResponse& response, std::string& errorMessage);

    TestClientConnection m_connection;
};
