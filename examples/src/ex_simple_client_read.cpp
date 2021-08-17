#include <boost/program_options.hpp>

#include <uaplatformlayer.h>
#include <uaclient/uaclientsdk.h>

#include <LogIt.h>

using namespace boost::program_options;

struct Args
{
    std::string endpointUrl;
    std::string variableAddress;
    Log::LOG_LEVEL logLevel;
};

Args parseArgs (int argc, char* argv[])
{
    Args args;
    
    options_description options;

    std::string logLevelAsStr;

    options.add_options()
        ("help,h",        "show help")
        ("endpoint,e",     value<std::string>(&args.endpointUrl)->default_value("opc.tcp://127.0.0.1:4841"),  "OPC-UA Endpoint, e.g. opc.tcp://127.0.0.1:48020" )
        ("address,a",      value<std::string>(&args.variableAddress)->default_value("object1.state"),         "string address of the var in NS2")
        ("trace_level,t",  value<std::string>(&logLevelAsStr)->default_value("INF"),                          "Trace level, one of: ERR,WRN,INF,DBG,TRC")
        ;

    variables_map vm;
    store( parse_command_line (argc, argv, options), vm );
    notify (vm);
    if (vm.count("help"))
    {
        std::cout << options << std::endl;
        exit(0);
    }
    if (! Log::logLevelFromString( logLevelAsStr, args.logLevel ) )
    {
        std::cout << "Log level not recognized: '" << logLevelAsStr << "'" << std::endl;
        exit(1);
    }
    return args;
}

int main(int argc, char* argv[])
{
    Args args (parseArgs(argc, argv));

    Log::initializeLogging(args.logLevel);
    Log::registerLoggingComponent("open62541", args.logLevel);
     
    try
    {
        UaClientSdk::UaSession session;

        UaClientSdk::SessionConnectInfo sessionConnectInfo;
        sessionConnectInfo.sApplicationName = "client@myComputer";
        sessionConnectInfo.sApplicationUri  = "client@myComputer";
        sessionConnectInfo.sProductUri      = "client";

        UaClientSdk::SessionSecurityInfo security;

        session.connect(
            args.endpointUrl.c_str(),
            sessionConnectInfo,
            security,
            nullptr /* no cbk */ );

        ServiceSettings defaultServiceSettings;

        UaReadValueIds ids;
        ids.create(1);
        ids[0].AttributeId = OpcUa_Attributes_Value;

        UaNodeId varNodeId (args.variableAddress.c_str(), 2);
        varNodeId.copyTo(&ids[0].NodeId);

        UaDataValues output;
        output.create(1);

        UaDiagnosticInfos diagnosticInfos;

        UaStatus status = session.read(defaultServiceSettings, 0 /*max age*/, OpcUa_TimestampsToReturn_Both, ids, output, diagnosticInfos );
        std::cout << "Status of service call was: " << status.toString().toUtf8() << std::endl;
        if (status.isGood())
        {
            std::cout << "Value read status was: " << output[0].StatusCode.toString().toUtf8() << std::endl;
            std::cout << "Value read was: " << output[0].Value.toString().toUtf8() << std::endl;
        }
        

    }
    catch (const std::exception &e)
    {
        LOG(Log::ERR) << "Caught: " << e.what();
    }
}
