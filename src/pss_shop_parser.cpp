/*  pss_shop_parser.cpp
 *
 *  Created by Rong Zhou on 04/07/16.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file pss shop parser
 */
#include <fstream>
#include "pss_shop_parser.hpp"
#include "pss_parser_utils.hpp"

using namespace std;
using namespace rapidxml;

namespace pss {
//Helper function
void ParseStationInfo(xml_node<> *node, StationInfo &stationinfo) {
  ParseAttr<int>(node, "Barcode", stationinfo.barcode);
  ParseStrAttr(node, "StationID", stationinfo.stationid);
  ParseBoolAttr(node, "HighCapacity", stationinfo.highcapacity);
  ParseAttr<double>(node, "mttf", stationinfo.mttf);
  ParseAttr<double>(node, "mttr", stationinfo.mttr);
}

//Helper function
void ParseFuncInfo(xml_node<> *node, FuncInfo &funcinfo) {
  ParseStrAttr(node, "Name", funcinfo.name);
  ParseAttr<double>(node, "OperatorDemand", funcinfo.oprdemand);
  ParseAttr<double>(node, "SetupTime", funcinfo.setuptime);
  ParseAttr<double>(node, "SpeedValue", funcinfo.speedval);
  ParseStrAttr(node, "SpeedUnit", funcinfo.speedunit);
  ParseAttr<double>(node, "SpeedVariation", funcinfo.speedvar);
  ParseStrAttr(node, "TimeUnit", funcinfo.timeunit);
  ParseAttr<int>(node, "Quality", funcinfo.quality);
  ParseAttr<int>(node, "MinBatch", funcinfo.minbatch);
  ParseAttr<int>(node, "Barcode", funcinfo.barcode);
}

//Helper function
void ParseSimpleFunc(xml_node<> *node, SimpleFunc &simplefunc) {
  ParseFuncInfo(node, simplefunc.funcinfo);
  //Parse SimpleFunctions
  ParseVector<string>(node, "SimpleFunction", simplefunc.names, ParseNameAttr);
  xml_node<> *attrs = node->first_node("Attributes");
  for(xml_node<> *attr = attrs->first_node("Attr");
      attr;
      attr = attr->next_sibling("Attr")) {
    string name;
    ParseStrAttr(attr, "Name", name);
    int value;
    ParseAttr<int>(attr, "Value", value);
    simplefunc.attributes[name] = (time_t)value;
  }
}

//Helper function
void ParseStation(xml_node<> *node, Station &station) {
  ParseBaseInfo(node, station.baseinfo);
  ParseStationInfo(node, station.stationinfo);
  xml_node<> *geometry = node->first_node("Geometry");
  ParseGeometry(geometry, station.geometry);
  ParseSchedule(node, station.schds);
  //Parse SimpleFunctionSequences
  ParseVector<SimpleFunc>(node, "SimpleFunctionSequence", station.simplefuncs,
                          ParseSimpleFunc);
}

//Helper function
void ParseOperatorSkill(xml_node<> *node, string &skill) {
  ParseStrAttr(node, "Skill", skill);
}

//Helper function
void ParseOperator(xml_node<> *node, Opr &opr) {
  ParseBaseInfo(node, opr.baseinfo);
  ParseStrAttr(node, "ResourceID", opr.oprinfo.rsrcid);
  ParseAttr<int>(node, "Barcode", opr.oprinfo.barcode);
  ParseSchedule(node, opr.schds);
  //Parse operator skills
  ParseVector<string>(node, "OperatorSkill", opr.skills, ParseOperatorSkill);
}

//Helper function
void ParseConfig(xml_node<> *node, Config &config) {
  assert(node);
  xml_node<> *cfgnode = node->first_node("ShopConfiguration");
  assert(cfgnode);
  ParseStrAttr(cfgnode, "CellAssignmentPolicy", config.cellpolicy);
  ParseStrAttr(cfgnode, "SequencingPolicy", config.sequencepolicy);
  ParseAttr<int>(cfgnode, "BatchLimit", config.batchlimit);
  ParseStrAttr(cfgnode, "RoutingPolicy", config.routingpolicy);
  ParseAttr<int>(cfgnode, "Threshold", config.threshold);
  ParseStrAttr(cfgnode, "FunctionName", config.funcname);
}

//Helper function
void ParseCellConfig(xml_node<> *node, CellConfig &cellconfig) {
  assert(node);
  xml_node<> *cfgnode = node->first_node("CellConfiguration");
  assert(cfgnode);
  ParseAttr<int>(cfgnode, "ControlPolicy", cellconfig.policy);
  ParseAttr<int>(cfgnode, "ControlParameter", cellconfig.parameter);
  ParseBoolAttr(cfgnode, "Batching", cellconfig.batching);
  ParseBoolAttr(cfgnode, "OperatorLimited", cellconfig.oprlimited);
  ParseBoolAttr(cfgnode, "UseOperatorSkills", cellconfig.useoprskills);
  ParseBoolAttr(cfgnode, "UseOperatorSchedules", cellconfig.useoprschds);
}

//Helper function
void ParseCellStations(xml_node<> *node, vector<string> &stations) {
  assert(node);
  xml_node<> *stnnode = node->first_node("Stations");
  assert(stnnode);
  //Parse station references
  ParseVector<string>(stnnode, "Reference", stations, ParseIdAttr);
}

//Helper function
void ParseCellOperators(xml_node<> *node, vector<string> &operators) {
  assert(node);
  xml_node<> *oprnode = node->first_node("Operators");
  assert(oprnode);
  //Parse operator references
  ParseVector<string>(oprnode, "Reference", operators, ParseIdAttr);
}

//Helper function
void ParseCell(xml_node<> *node, Cell &cell) {
  assert(node);
  ParseStrAttr(node, "CellId", cell.cellid);
  ParseBaseInfo(node, cell.baseinfo);
  ParseCellConfig(node, cell.cellconfig);
  ParseCellStations(node, cell.stations);
  ParseCellOperators(node, cell.operators);
}

int ShopParser::ParseFile(char const *path) {
  ifstream file(path);
  //read the entire file into buf
  std::vector<char> buf((istreambuf_iterator<char>(file)),
                        istreambuf_iterator<char>());
  buf.push_back('\0');
  xml_document<> doc;
  //Parse the string pointed to by 'buf'
  doc.parse<0>(&buf[0]);
  //Obtain the shop node
  xml_node<> *shpnode = doc.first_node("Shop");
  ParseBaseInfo(shpnode, shopmodel_.baseinfo);
  ParseStrAttr(shpnode, "Creator", shopmodel_.creator);
  ParseStrAttr(shpnode, "Version", shopmodel_.version);
  //Parse shop configuration
  ParseConfig(shpnode, shopmodel_.config);
  //Parse schedule
  ParseSchedule(shpnode, shopmodel_.schds);
  //Parse stations
  ParseVector<Station>(shpnode, "Station", shopmodel_.stations, ParseStation);
  //Parse operators
  ParseVector<Opr>(shpnode, "Operator", shopmodel_.operators, ParseOperator);
  //Parse cells
  ParseVector<Cell>(shpnode, "Cell", shopmodel_.cells, ParseCell);
  return 0;
}

}