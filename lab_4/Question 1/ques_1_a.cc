#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"
#include<string>
#include<iostream>
#include<map>
using namespace ns3;

void throughput();

int main (int argc, char *argv[])
{
  
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
  //Creating Nodes
  NodeContainer nodes;
  nodes.Create (2);			

  //Attributes For point to point
  //Takes the input from the command line
  PointToPointHelper pointToPoint;
  pointToPoint.SetChannelAttribute ("Delay", StringValue (argv[1]));
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  
  //Installing Devices
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

   //Setting Stack
  InternetStackHelper stack;
  stack.Install (nodes);

  //Addresses
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  //Server Instance
  UdpEchoServerHelper echoServer(11);
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  //Client Instance
  UdpEchoClientHelper echoClient(interfaces.GetAddress (1), 11);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
	
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  //Storing the Trace
  pointToPoint.EnableAsciiAll("ques_1_a.tr");
  pointToPoint.EnablePcapAll("ques_1_a");
 
  //To monitor the flow
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  
  //Simulating	
  Simulator::Stop (Seconds(11.0));
  Simulator::Run();
  
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if ((t.sourceAddress=="10.1.1.1" ))
      {
          std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
          std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
      }
     }
  Simulator::Destroy();  
  return 0;
}


