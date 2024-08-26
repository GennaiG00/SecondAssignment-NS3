#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/multi-model-spectrum-channel.h"
#include "ns3/non-communicating-net-device.h"
#include "ns3/on-off-helper.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/packet-sink.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/spectrum-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/string.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/wifi-net-device.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;

int main(int argc, char* argv[])
{
        bool udp = true;
        bool yans = true;
        double distance = 1.0;
        double simulationTime = 20.0; // seconds
        std::string wifiType = "ns3::SpectrumWifiPhy";
        std::string errorModelType = "ns3::NistErrorRateModel";
        bool enablePcap = false;

        CommandLine cmd(__FILE__);
        cmd.AddValue("simulationTime", "Simulation time in seconds", simulationTime);
        cmd.AddValue("udp", "UDP if set to 1, TCP otherwise", udp);
        cmd.AddValue("distance", "meters separation between nodes", distance);
        cmd.AddValue("wifiType", "select ns3::SpectrumWifiPhy or ns3::YansWifiPhy", wifiType);
        cmd.AddValue("errorModelType", "select ns3::NistErrorRateModel or ns3::YansErrorRateModel", errorModelType);
        cmd.AddValue("enablePcap", "enable pcap output", enablePcap);
        cmd.AddValue("yans", "use YansWifiPhy instead of SpectrumWifiPhy", yans);
        cmd.Parse(argc, argv);

        std::cout << "wifiType: " << wifiType << " distance: " << distance
                  << "m; time: " << simulationTime << "; TxPower: 16 dBm (40 mW)" << std::endl;

        uint32_t payloadSize;
        if (udp)
        {
            payloadSize = 972; // 1000 bytes IPv4
        }
        else
        {
            payloadSize = 1448; // 1500 bytes IPv6
            Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(payloadSize));
        }

        NodeContainer wifiStaNode1, wifiApNode1, wifiStaNode2, wifiApNode2;

        wifiStaNode1.Create(1);
        wifiApNode1.Create(1);
        wifiStaNode2.Create(1);
        wifiApNode2.Create(1);

        NetDeviceContainer staDevice1, staDevice2, apDevice1, apDevice2;

        WifiHelper wifi1, wifi2;
        wifi1.SetStandard(WIFI_STANDARD_80211n);
        wifi2.SetStandard(WIFI_STANDARD_80211n);
        WifiMacHelper mac1, mac2;
        Ssid ssid1 = Ssid("ns380211n");
        Ssid ssid2 = Ssid("ns380211n_2");

        if(!yans)
        {
            std::cout << "Using Spectrum Channel" << std::endl;
            SpectrumWifiPhyHelper spectrumPhy1, spectrumPhy2;

            Ptr<MultiModelSpectrumChannel> spectrumChannel1 = CreateObject<MultiModelSpectrumChannel>();

            Ptr<FriisPropagationLossModel> lossModel1 = CreateObject<FriisPropagationLossModel>();
            lossModel1->SetFrequency(2401);
            spectrumChannel1->AddPropagationLossModel(lossModel1);

            Ptr<ConstantSpeedPropagationDelayModel> delayModel1 = CreateObject<ConstantSpeedPropagationDelayModel>();
            spectrumChannel1->SetPropagationDelayModel(delayModel1);

            spectrumPhy1.SetChannel(spectrumChannel1);
            spectrumPhy1.SetErrorRateModel(errorModelType);
            spectrumPhy1.Set("ChannelSettings", StringValue("{1, 0, BAND_2_4GHZ, 0}"));

            spectrumPhy2.SetChannel(spectrumChannel1);
            spectrumPhy2.SetErrorRateModel(errorModelType);
            spectrumPhy2.Set("ChannelSettings", StringValue("{2, 0, BAND_2_4GHZ, 0}"));

            wifi1.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("HtMcs4"), "ControlMode", StringValue("HtMcs4"));
            wifi2.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("HtMcs4"), "ControlMode", StringValue("HtMcs4"));

            mac1.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid1));
            mac2.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid2));

            staDevice1 = wifi1.Install(spectrumPhy1, mac1, wifiStaNode1);
            staDevice2 = wifi2.Install(spectrumPhy2, mac2, wifiStaNode2);

            mac1.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid1));
            mac2.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid2));

            apDevice1 = wifi1.Install(spectrumPhy1, mac1, wifiApNode1);
            apDevice2 = wifi2.Install(spectrumPhy2, mac2, wifiApNode2);
        }else{
            std::cout << "Using Yans Channel" << std::endl;
            YansWifiChannelHelper channelHelper = YansWifiChannelHelper::Default();
            YansWifiPhyHelper yansPhy1, yansPhy2;
            yansPhy1.SetChannel(channelHelper.Create());
            yansPhy2.SetChannel(channelHelper.Create());

            wifi1.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("HtMcs4"), "ControlMode", StringValue("HtMcs4"));
            wifi2.SetRemoteStationManager("ns3::ConstantRateWifiManager", "DataMode", StringValue("HtMcs4"), "ControlMode", StringValue("HtMcs4"));

            mac1.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid1));
            mac2.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid2));

            staDevice1 = wifi1.Install(yansPhy1, mac1, wifiStaNode1);
            staDevice2 = wifi2.Install(yansPhy2, mac2, wifiStaNode2);

            mac1.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid1));
            mac2.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid2));

            apDevice1 = wifi1.Install(yansPhy1, mac1, wifiApNode1);
            apDevice2 = wifi2.Install(yansPhy2, mac2, wifiApNode2);
        }


        MobilityHelper mobility1, mobility2;
        Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator>();
        Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator>();

        positionAlloc1->Add(Vector(0.0, 1.0, 0.0));
        positionAlloc1->Add(Vector(distance, 0.0, 0.0));
        positionAlloc2->Add(Vector(0.0, 1.0, 0.0));
        positionAlloc2->Add(Vector(distance, 1.0, 0.0));


        mobility1.SetPositionAllocator(positionAlloc1);
        mobility1.SetMobilityModel("ns3::ConstantPositionMobilityModel");

        mobility2.SetPositionAllocator(positionAlloc2);
        mobility2.SetMobilityModel("ns3::ConstantPositionMobilityModel");

        mobility1.Install(wifiApNode1);
        mobility1.Install(wifiStaNode1);

        mobility2.Install(wifiApNode2);
        mobility2.Install(wifiStaNode2);

        InternetStackHelper stack1, stack2;
        stack1.Install(wifiApNode1);
        stack1.Install(wifiStaNode1);
        stack2.Install(wifiApNode2);
        stack2.Install(wifiStaNode2);

        Ipv4AddressHelper address1, address2;
        address1.SetBase("192.168.1.0", "255.255.255.0");
        address2.SetBase("192.168.2.0", "255.255.255.0");

        Ipv4InterfaceContainer staInterface1 = address1.Assign(staDevice1);
        Ipv4InterfaceContainer apInterface1 = address1.Assign(apDevice1);

        Ipv4InterfaceContainer staInterface2 = address2.Assign(staDevice2);
        Ipv4InterfaceContainer apInterface2 = address2.Assign(apDevice2);

        std::cout << "sta1 " << staInterface1.GetAddress(0) << std::endl;
        std::cout << "sta2 " << staInterface2.GetAddress(0) << std::endl;
        std::cout << "ap1 " << apInterface1.GetAddress(0) << std::endl;
        std::cout << "ap2 " << apInterface2.GetAddress(0) << std::endl;

        ApplicationContainer serverApp1, serverApp2;

        PointToPointHelper pointToPoint;
        pointToPoint.SetDeviceAttribute("DataRate", StringValue("1Gbps"));
        pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

        // Install Point-to-Point devices on sta1 and sta2
        NetDeviceContainer p2pDevices;
        p2pDevices = pointToPoint.Install(wifiStaNode1.Get(0), wifiStaNode2.Get(0));

        // Assign IP addresses to the Point-to-Point link
        Ipv4AddressHelper p2pAddress;
        p2pAddress.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer p2pInterfaces = p2pAddress.Assign(p2pDevices);

        // Print the IP addresses assigned to the Point-to-Point link
        std::cout << "p2p address 1: " << p2pInterfaces.GetAddress(0) << std::endl;
        std::cout << "p2p address 2: " << p2pInterfaces.GetAddress(1) << std::endl;

        // Continue with existing code...

        // Example application setup for Point-to-Point link (e.g., UDP)
        uint16_t p2pPort = 8080;
        UdpServerHelper p2pServer(p2pPort);
        ApplicationContainer p2pServerApp = p2pServer.Install(wifiStaNode2.Get(0)); // Server on sta2
        p2pServerApp.Start(Seconds(1.0));
        p2pServerApp.Stop(Seconds(simulationTime + 1));

        UdpClientHelper p2pClient(p2pInterfaces.GetAddress(1), p2pPort); // Client on sta1
        p2pClient.SetAttribute("MaxPackets", UintegerValue(1000));
        p2pClient.SetAttribute("Interval", TimeValue(Time("0.000001")));
        p2pClient.SetAttribute("PacketSize", UintegerValue(1024));

        ApplicationContainer p2pClientApp = p2pClient.Install(wifiStaNode1.Get(0));
        p2pClientApp.Start(Seconds(2.0));
        p2pClientApp.Stop(Seconds(simulationTime + 1));

        if (udp)
        {
            std::cout << "Using UDP" << std::endl;
            uint16_t port1 = 9;
            uint16_t port2 = 9;

            UdpServerHelper server1(port1);
            UdpServerHelper server2(port2);

            serverApp1 = server1.Install(wifiStaNode1.Get(0));
            serverApp2 = server2.Install(wifiStaNode2.Get(0));

            serverApp1.Start(Seconds(0.0));
            serverApp1.Stop(Seconds(simulationTime + 1));
            serverApp2.Start(Seconds(0.0));
            serverApp2.Stop(Seconds(simulationTime + 1));

            UdpClientHelper client1(staInterface1.GetAddress(0), port1);
            client1.SetAttribute("MaxPackets", UintegerValue(54541));
            client1.SetAttribute("Interval", TimeValue(Time("0.0001")));
            client1.SetAttribute("PacketSize", UintegerValue(payloadSize));

            UdpClientHelper client2(staInterface2.GetAddress(0), port2);
            client2.SetAttribute("MaxPackets", UintegerValue(54541));
            client2.SetAttribute("Interval", TimeValue(Time("0.0001")));
            client2.SetAttribute("PacketSize", UintegerValue(payloadSize));

            ApplicationContainer clientApp1 = client1.Install(wifiApNode1.Get(0));
            ApplicationContainer clientApp2 = client2.Install(wifiApNode2.Get(0));

            clientApp1.Start(Seconds(1.0));
            clientApp1.Stop(Seconds(simulationTime + 1));
            clientApp2.Start(Seconds(1.0));
            clientApp2.Stop(Seconds(simulationTime + 1));
        }
        else
        {
            std::cout << "Using TCP" << std::endl;
            uint16_t port1 = 50000;
            uint16_t port2 = 50000;

            Address apLocalAddress1(InetSocketAddress(Ipv4Address::GetAny(), port1));
            PacketSinkHelper packetSinkHelper1("ns3::TcpSocketFactory", apLocalAddress1);
            serverApp1 = packetSinkHelper1.Install(wifiStaNode1.Get(0));

            Address apLocalAddress2(InetSocketAddress(Ipv4Address::GetAny(), port2));
            PacketSinkHelper packetSinkHelper2("ns3::TcpSocketFactory", apLocalAddress2);
            serverApp2 = packetSinkHelper2.Install(wifiStaNode2.Get(0));

            serverApp1.Start(Seconds(0.0));
            serverApp1.Stop(Seconds(simulationTime + 1));
            serverApp2.Start(Seconds(0.0));
            serverApp2.Stop(Seconds(simulationTime + 1));

            OnOffHelper onOffHelper1("ns3::TcpSocketFactory", Ipv4Address::GetAny());
            onOffHelper1.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
            onOffHelper1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
            onOffHelper1.SetAttribute("PacketSize", UintegerValue(payloadSize));
            onOffHelper1.SetAttribute("DataRate", DataRateValue(DataRate("100Mb/s")));

            OnOffHelper onOffHelper2("ns3::TcpSocketFactory", Ipv4Address::GetAny());
            onOffHelper2.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
            onOffHelper2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
            onOffHelper2.SetAttribute("PacketSize", UintegerValue(payloadSize));
            onOffHelper2.SetAttribute("DataRate", DataRateValue(DataRate("100Mb/s")));

            ApplicationContainer clientApp1, clientApp2;

            AddressValue remoteAddress1(InetSocketAddress(staInterface1.GetAddress(0), port1));
            onOffHelper1.SetAttribute("Remote", remoteAddress1);
            clientApp1.Add(onOffHelper1.Install(wifiApNode1.Get(0)));

            AddressValue remoteAddress2(InetSocketAddress(staInterface2.GetAddress(0), port2));
            onOffHelper2.SetAttribute("Remote", remoteAddress2);
            clientApp2.Add(onOffHelper2.Install(wifiApNode2.Get(0)));

            clientApp1.Start(Seconds(1.0));
            clientApp1.Stop(Seconds(simulationTime + 1));
            clientApp2.Start(Seconds(1.0));
            clientApp2.Stop(Seconds(simulationTime + 1));
        }

        Ptr<FlowMonitor> flowMonitor;
        FlowMonitorHelper flowHelper;
        flowMonitor = flowHelper.InstallAll();

        Simulator::Stop(Seconds(simulationTime + 1));
        Simulator::Run();

        flowMonitor->CheckForLostPackets();
        Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowHelper.GetClassifier());
        std::map<FlowId, FlowMonitor::FlowStats> stats = flowMonitor->GetFlowStats();

        for (auto i = stats.begin(); i != stats.end(); ++i)
        {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
            std::cout << "Flow ID: " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
            std::cout << "Tx Packets: " << i->second.txPackets << "\n";
            std::cout << "Rx Packets: " << i->second.rxPackets << "\n";
            std::cout << "Tx Bytes: " << i->second.txBytes << "\n";
            std::cout << "Rx Bytes: " << i->second.rxBytes << "\n";
            std::cout << "Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1024 << " Kbps\n";
            std::cout << "Delay: " << i->second.delaySum.GetSeconds() / i->second.rxPackets << " s\n";
            std::cout << "Lost Packets: " << i->second.lostPackets << "\n";
            std::cout << "Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / 1024 << " Kbps\n";
            std::cout << "-------------------------------\n";
        }

        Simulator::Destroy();
    return 0;
}
