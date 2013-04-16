/*
 * Author 			: Sahil Kumar
 * Date 				: 16/4/2013
 * Assignment 	: 4
 */

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

int main (int argc, char *argv[])
{
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  
  //Creating Nodes
  NodeContainer nodes;
  nodes.Create (2);		

	//Point to Point Link
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue (argv[1]));

  //Installing Devices
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

   //Setting Stack
  InternetStackHelper stack;
  stack.Install (nodes);

   //Assigning Addresses
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // First Instance of Client-Server Application
  UdpEchoServerHelper echoServer_1 (11);

  ApplicationContainer serverApps_1 = echoServer_1.Install (nodes.Get (1));
  serverApps_1.Start (Seconds (1.0));
  serverApps_1.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient_1 (interfaces.GetAddress (1), 11);
  echoClient_1.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient_1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient_1.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps_1 = echoClient_1.Install (nodes.Get (0));
  clientApps_1.Start (Seconds (2.0));
  clientApps_1.Stop (Seconds (10.0));

  // Second Instance of Client-Server Application
  UdpEchoServerHelper echoServer_2 (12);

  ApplicationContainer serverApps_2 = echoServer_2.Install (nodes.Get (1));
  serverApps_2.Start (Seconds (1.0));
  serverApps_2.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient_2(interfaces.GetAddress (1), 12);
  echoClient_2.SetAttribute ("MaxPackets", UintegerValue (10));
  echoClient_2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient_2.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps_2 = echoClient_2.Install (nodes.Get (0));
  clientApps_2.Start (Seconds (2.0));
  clientApps_2.Stop (Seconds (10.0));
 
 //Storing the Trace
  pointToPoint.EnableAsciiAll("ques_1_b.tr");
  pointToPoint.EnablePcapAll("ques_1_b");
  
  //To monitor the flow
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
  
  //Simulating
	Simulator::Stop (Seconds(11.0));
  Simulator::Run ();
  
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
     
  Simulator::Destroy ();
  return 0;
}
