import std;
import codeclipper;
import codeclipper.cli;

int main(int argc, char** argv) {
    codeclipper::platform::setupConsole();

    codeclipper::CliParser parser(argc, argv);
    if (parser.shouldExit()) {
        return parser.exitCode();
    }

    const auto config = parser.config();

    if (config.runMcpServer) {
        codeclipper::McpServer server;
        server.run(std::cin, std::cout);
        return 0;
    }

    codeclipper::ContextEngine engine(config, std::cout);
    if (auto res = engine.run(); !res) {
        std::println(std::cerr, "Execution failed with code: {}", static_cast<int>(res.error()));
        return 1;
    }

    return 0;
}