#ifndef CODECLIPPER_MCPSERVER_HXX
#define CODECLIPPER_MCPSERVER_HXX

#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

namespace codeclipper {

    class McpServer {
    public:
        void run(std::istream& input, std::ostream& output);

    private:
        using json = nlohmann::json;

        void handleMessage(const json& message, std::ostream& output);
        void sendResponse(std::ostream& output, const json& id, const json& result);
        void sendError(std::ostream& output, const json& id, int code, const std::string& message);

        json handleInitialize(const json& params);
        json handleListTools();
        json handleCallTool(const json& params);
    };
}

#endif //CODECLIPPER_MCPSERVER_HXX