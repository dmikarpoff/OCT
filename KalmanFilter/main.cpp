#include "scan_data.h"

#include <QCommandLineParser>
#include <QCoreApplication>

#include <iostream>

constexpr int kPositionalArgsNumber = 2;

int main(int argc, char** argv)
try
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Kalman filter for scans");
    parser.addHelpOption();
    parser.addPositionalArgument("source", "Source *.csv file with normalized scan description");
    parser.addPositionalArgument("destination", "Destination file name without extension. "
                                        "<destination>.csv and <destination>.png files will be generated");
    parser.process(app);
    auto positional_args = parser.positionalArguments();
    if (positional_args.size() != kPositionalArgsNumber)
    {
        std::cerr << "Unexpected number of positional arguments" << std::endl;
        std::cerr << "Expected " << kPositionalArgsNumber << ", got " << positional_args.size() << std::endl;
        return 1;
    }
    auto scan_data = ScanData::fromFile(positional_args.at(0));

    return 0;
}
catch (const std::exception& e)
{
    std::cerr << "Unhandled exception: " << e.what() << std::endl;
    return 1;
}
catch (...)
{
    std::cerr << "Unknown exception type was thrown" << std::endl;
    return 1;
}
