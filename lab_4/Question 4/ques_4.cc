#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-remote-channel.h"
#include "ns3/csma-net-device.h"
#include "string"
#include<iostream>
#include<fstream>

using namespace ns3;

static double node2_bytes = 0;
static double node3_bytes = 0;
std::ofstream fout1,fout2;

void Packet_node2(std::string ,Ptr<const Packet> , const Address&);
void Packet_node3(std::string ,Ptr<const Packet> , const Address&);
void calculate_throughput(double);

int main (int argc, char *argv[])
{
  std::string tcpType = "NewReno";
  //Setting Socket Type
  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue(TypeId::LookupByName ("ns3::Tcp" + tcpType)));
  
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));

  fout1.open("connection_1.dat");
  fout2.open("connection_2.dat");

  NodeContainer p2pNodes;
  p2pNodes.Create (4);		
	
	//Point To Point Helper
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1.5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("10ms"));
  
  NetDeviceContainer p2pDevice20, p2pDevice30, p2pDevice01;
  
  //creatiing n2-n0 link
  p2pDevice20 = pointToPoint.Install (p2pNodes.Get (2),p2pNodes.Get (0));

  //creating n3-n0 link
  pointToPoint.SetChannelAttribute ("Delay", StringValue (argv[1]));
  p2pDevice30 = pointToPoint.Install (p2pNodes.Get (3),p2pNodes.Get (0));

  //creating n0-n1 link
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("10ms"));
  p2pDevice01 = pointToPoint.Install (p2pNodes.Get (0),p2pNodes.Get (1));
  
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue(uint32_t(10)));	//setting maximum limit on no. of packets
  
  //setting stack for nodes
  InternetStackHelper stack;
  stack.Install (p2pNodes);
  
  Ipv4AddressHelper address;
  Ipv4InterfaceContainer p2pInterface20, p2pInterface30, p2pInterface01;

  //assigning address to n0-n1 interface
  address.SetBase ("10.1.1.0", "255.255.255.0");
  p2pInterface20 = address.Assign (p2pDevice20);

  //assigning address to n0-n3 interface
  address.SetBase ("10.1.2.0", "255.255.255.0");
  p2pInterface30 = address.Assign (p2pDevice30);
  
  //assigning address to n1-n2 interface
  address.SetBase ("10.1.3.0", "255.255.255.0");
  p2pInterface01 = address.Assign (p2pDevice01);

  //setting first source at node 2
  OnOffHelper source1("ns3::TcpSocketFactory",InetSocketAddress(p2pInterface01.GetAddress(1),9));
  source1.SetAttribute ("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=5]")); 
  source1.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  
  source1.SetAttribute("DataRate", DataRateValue(DataRate("1.5Mbps")));
  source1.SetAttribute("PacketSize", UintegerValue(2000));

  ApplicationContainer apps;
  apps.Add (source1.Install (p2pNodes.Get (2)));
  
  //setting second source at node 3
  OnOffHelper source2("ns3::TcpSocketFactory",InetSocketAddress(p2pInterface01.GetAddress(1),10));
  source2.SetAttribute ("OnTime",  StringValue("ns3::ConstantRandomVariable[Constant=5]")); 
  source2.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

  source2.SetAttribute("DataRate", DataRateValue(DataRate("1.5Mbps")));
  source2.SetAttribute("PacketSize", UintegerValue(2000));

  apps.Add (source2.Install (p2pNodes.Get (3)));
  
  //setting first sink at node 1
  PacketSinkHelper sink1("ns3::TcpSocketFactory",InetSocketAddress(p2pInterface01.GetAddress(1),9));
  apps.Add (sink1.Install (p2pNodes.Get (1)));
  
  //setting second sink at node 1
  PacketSinkHelper sink2("ns3::TcpSocketFactory",InetSocketAddress(p2pInterface01.GetAddress(1),10));
  apps.Add (sink2.Install (p2pNodes.Get (1)));
  
  // assigning the start and stop times for the application
  apps.Start(Seconds (0.0));
  apps.Stop(Seconds (5.0));

  //enabling dynamic routing
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  std::string context = "/NodeList/1/ApplicationList/0/$ns3::PacketSink/Rx";
  Config::Connect(context, MakeCallback(&Packet_node2));
  
  context = "/NodeList/1/ApplicationList/1/$ns3::PacketSink/Rx";
  Config::Connect(context, MakeCallback(&Packet_node3));

  pointToPoint.EnableAsciiAll("q4.tr");
  pointToPoint.EnablePcapAll("q4");
  
  //scheduling the calculation of throughput of both links while the application is still running
  for(double k=0;k<=5;k = k + 0.05)
  { 
    Simulator::Schedule (Seconds(k), &calculate_throughput, k);
  }  

  Simulator::Stop(Seconds(10));
  std::ofstream con_1,con_2;
  con_1.open("through_1.dat",std::ios::app);
  con_2.open("through_2.dat",std::ios::app);
  Simulator::Run ();
  con_1 << argv[1] << " " <<(node2_bytes* 8 / 1000000)/5<< std::endl;
  con_2 << argv[1] << " " << (node3_bytes* 8 / 1000000)/5 << std::endl;
  
  Simulator::Destroy ();
  con_2.close();
  con_1.close();
  return 0;
}

//processing the packet received from node 2
void Packet_node2( std::string context,Ptr<const Packet> p, const Address& addr)
{
 node2_bytes += p->GetSize();	
}

//processing the packet received from node 3
void Packet_node3( std::string context, Ptr<const Packet> p, const Address& addr)
{
 node3_bytes += p->GetSize();	
}

//calculating the throughput for both links 1 and 2
void calculate_throughput(double k)
{
  fout1 << k << " " << (node2_bytes* 8 / 1000000)/5 << std::endl;
  fout2 << k << " " << (node3_bytes* 8 / 1000000)/5 << std::endl;
}
