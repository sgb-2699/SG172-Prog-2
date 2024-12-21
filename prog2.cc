#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/netanim-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Star");

int main() {
    Config::SetDefault("ns3::OnOffApplication::PacketSize", UintegerValue(137));
    Config::SetDefault("ns3::OnOffApplication::DataRate", StringValue("14kb/s"));

    uint32_t nSpokes;
    std::cout << "Enter the number of spoke nodes in the star topology: ";
    std::cin >> nSpokes;

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
    PointToPointStarHelper star(nSpokes, pointToPoint);

    InternetStackHelper internet;
    star.InstallStack(internet);
    star.AssignIpv4Addresses(Ipv4AddressHelper("10.1.1.0", "255.255.255.0"));

    uint16_t port = 50000;
    PacketSinkHelper packetSink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
    ApplicationContainer hubApp = packetSink.Install(star.GetHub());
    hubApp.Start(Seconds(1.0));
    hubApp.Stop(Seconds(10.0));

    OnOffHelper onOffHelper("ns3::TcpSocketFactory", Address());
    onOffHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
    onOffHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
    
    for (uint32_t i = 0; i < nSpokes; ++i) {
    onOffHelper.SetAttribute("Remote", AddressValue(InetSocketAddress(star.GetHubIpv4Address(i), port)));
    ApplicationContainer app = onOffHelper.Install(star.GetSpokeNode(i));
    app.Start(Seconds(1.0));
    app.Stop(Seconds(10.0));
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("Third.tr"));
    pointToPoint.EnablePcapAll("star");

    AnimationInterface anim("Third.xml");
    anim.SetMaxPktsPerTraceFile(5000);
    anim.SetConstantPosition(star.GetHub(), 300, 300);
    double radius = 100.0;
    for (uint32_t i = 0; i < nSpokes; ++i) {
        double angle = 2 * M_PI * i / nSpokes;
        anim.SetConstantPosition(star.GetSpokeNode(i), 300 + radius * cos(angle), 300 + radius * sin(angle));
    }

    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
