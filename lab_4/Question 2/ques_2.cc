#include <iostream>
#include<fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/service-flow.h"
#include "ns3/flow-monitor-module.h"
#include<iostream>
#include<map>

using namespace ns3;

void packet_loss(FlowMonitorHelper ,Ptr<FlowMonitor>,double );
 std::ofstream fout;
 
int 
main (int argc, char *argv[])
{
	
  Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents",BooleanValue(true));
  //Log component
	LogComponentEnable("OnOffApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("PacketSink", LOG_LEVEL_INFO);
  
  //Loss Data File
  fout.open("loss.dat");
  
  //Creating Nodes
  NodeContainer p2pNodes;
  p2pNodes.Create(5);

	//Point to Point Helper
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

	//Net Device Containers
  NetDeviceContainer p2pDevice01, p2pDevice03, p2pDevice12, p2pDevice34, p2pDevice42;

  //Creating n0-n1 link
  p2pDevice01 = pointToPoint.Install (p2pNodes.Get (0),p2pNodes.Get (1));

  //Creating n0-n3 link
  p2pDevice03 = pointToPoint.Install (p2pNodes.Get (0),p2pNodes.Get (3));

  //Creating n1-n2 link 
  p2pDevice12 = pointToPoint.Install (p2pNodes.Get (1),p2pNodes.Get (2));

  //Creating n4-n2 link
  p2pDevice42 = pointToPoint.Install (p2pNodes.Get (2),p2pNodes.Get (4));

  //Creating n3-n4 link
  //Setting the Link Value
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("0.5Mbps"));
  p2pDevice34 = pointToPoint.Install (p2pNodes.Get (3),p2pNodes.Get (4));

	//Internet Stack
  InternetStackHelper stack;
  stack.Install (p2pNodes);

	//Ipv4AddressHelper
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pInterface01, p2pInterface03, p2pInterface12, p2pInterface34, p2pInterface42;

  //Assigning address to n0-n1 interface
  address.SetBase ("10.1.1.0", "255.255.255.0");
  p2pInterface01 = address.Assign (p2pDevice01);

  //Assigning address to n0-n3 interface
  address.SetBase ("10.1.2.0", "255.255.255.0");
  p2pInterface03 = address.Assign (p2pDevice03);
  
  //Assigning address to n1-n2 interface
  address.SetBase ("10.1.3.0", "255.255.255.0");
  p2pInterface12 = address.Assign (p2pDevice12);

  //Assigning address to n3-n4 interface
  address.SetBase ("10.1.4.0", "255.255.255.0");
  p2pInterface34 = address.Assign (p2pDevice34);

  //Assigning address to n4-n2 interface
  address.SetBase ("10.1.5.0", "255.255.255.0");
  p2pInterface42 = address.Assign (p2pDevice42);

	//Global Routing table Helper
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	int16_t sourcePort = 9;
	//Sending data from node 0 to node 2 
  
  OnOffHelper source1("ns3::UdpSocketFactory",InetSocketAddress(p2pInterface12.GetAddress(1),sourcePort));
  source1.SetAttribute ("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=1]")); 
  source1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  source1.SetAttribute("DataRate", DataRateValue(DataRate("0.9Mbps")));
  source1.SetAttribute("PacketSize", UintegerValue(50));

  ApplicationContainer sourceApps1 = source1.Install(p2pNodes.Get(0));
  sourceApps1.Start(Seconds(1.0));
  sourceApps1.Stop(Seconds(3.5));

	//Sending data from node 0 to node 1 
  OnOffHelper source2("ns3::UdpSocketFactory",InetSocketAddress(p2pInterface01.GetAddress(1),sourcePort));
  source2.SetAttribute ("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=1]")); 
  source2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  source2.SetAttribute("DataRate", DataRateValue(DataRate("0.3Mbps")));
  source2.SetAttribute("PacketSize", UintegerValue(50));

  ApplicationContainer sourceApps2 = source2.Install(p2pNodes.Get(0));
  sourceApps2.Start(Seconds(1.5));
  sourceApps2.Stop(Seconds(3.0));

	//Creating Sink to the source packets at 2
  int16_t sinkPort = 9;
  PacketSinkHelper sink1("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),sinkPort));
  ApplicationContainer sinkApps1 = sink1.Install(p2pNodes.Get(2));
  sinkApps1.Start(Seconds(1.0));
  sinkApps1.Stop(Seconds(3.5));

	//Creating Sink to the source packets at 1
  PacketSinkHelper sink2("ns3::UdpSocketFactory",InetSocketAddress(Ipv4Address::GetAny(),sinkPort));
  ApplicationContainer sinkApps2 = sink2.Install(p2pNodes.Get(1));
  sinkApps2.Start(Seconds(1.5));
  sinkApps2.Stop(Seconds(3.0));

	//Pointer to node 2
  Ptr<Node> n2 = p2pNodes.Get(2);
  Ptr<FlowMonitor> monitor;
  FlowMonitorHelper helper;
  monitor = helper.InstallAll();
  monitor->CheckForLostPackets();

	//The link between 0 and 1 goes down
  Ptr<Node> node = p2pNodes.Get(0);
  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
  uint32_t interface = ipv4->GetInterfaceForPrefix(Ipv4Address("10.1.1.0") ,Ipv4Mask("255.255.255.0"));
  Simulator::Schedule(Seconds (2.0), &Ipv4::SetDown, ipv4,interface);
  Simulator::Schedule(Seconds (2.7), &Ipv4::SetUp, ipv4, interface);

  for(double k=1; k<=3.5;k = k + 0.05)
  { 
    Simulator::Schedule (Seconds(k), &packet_loss, helper, monitor, k);
  }
  
  pointToPoint.EnableAsciiAll("ques_2.tr");
  pointToPoint.EnablePcapAll("ques_2"); 

  Simulator::Stop (Seconds (3.5));
  Simulator::Run ();
  Simulator::Destroy();
  return 0;
}

void packet_loss(FlowMonitorHelper flowmon,Ptr<FlowMonitor> monitor,double k)
{
  double total=0;
  
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
 double j=0,m=0;
 for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
  {		
                j += i->second.txPackets;
                m += i->second.lostPackets;
  }
  if(j!=0)
	  total = m/j ;
  fout<< k<< " "<< total << std::endl;
}
