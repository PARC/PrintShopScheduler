/*  pss_shop_file.hpp
 *
 *  Created by Rong Zhou on 04/15/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  data types for pss shop
 */

#ifndef PSS_SHOP_FILE_HPP_INCLUDED_
#define PSS_SHOP_FILE_HPP_INCLUDED_

#include "pss_parser.hpp"

namespace pss {

struct Config {
  std::string cellpolicy;
  std::string sequencepolicy;
  int batchlimit;
  std::string routingpolicy;
  int threshold;
  std::string funcname;
};

struct StationInfo {
  int barcode;
  std::string stationid;
  bool highcapacity;
  double mttf; //mean time to failure
  double mttr; //mean time to repair
};

struct FuncInfo {
  std::string name;
  double oprdemand;
  double setuptime;
  double speedval;
  std::string speedunit;
  double speedvar;
  std::string timeunit;
  int quality;
  int minbatch;
  int barcode;
};

struct PriceInfo {
  int perjob;
  int perunit;
  int pertime;
};

struct SimpleFunc {
  pss::FuncInfo funcinfo;
  pss::PriceInfo priceinfo;
  std::vector<std::string> names;
  std::map<std::string, time_t> attributes;
};

struct Station {
  pss::BaseInfo baseinfo;
  pss::StationInfo stationinfo;
  pss::Geometry geometry;
  std::vector<pss::Schedule> schds;
  pss::PriceInfo priceinfo;
  std::vector<pss::SimpleFunc> simplefuncs;
};

struct OprInfo {
  std::string rsrcid;
  int barcode;
};

struct Opr {
  pss::BaseInfo baseinfo;
  pss::OprInfo oprinfo;
  std::vector<pss::Schedule> schds;
  std::vector<std::string> skills;
};

struct CellConfig {
  int policy;
  int parameter;
  bool batching;
  bool oprlimited;
  bool useoprskills;
  bool useoprschds;
};

struct Cell {
  std::string cellid;
  pss::BaseInfo baseinfo;
  pss::CellConfig cellconfig;
  std::vector<std::string> stations;
  std::vector<std::string> operators;
};

struct ShopModel {
  pss::BaseInfo baseinfo;
  std::string creator;
  std::string version;
  pss::Config config;
  std::vector<pss::Schedule> schds;
  std::vector<pss::Station> stations;
  std::vector<pss::Opr> operators;
  std::vector<pss::Cell> cells;
};

} // namespace pss

#endif // PSS_SHOP_FILE_HPP_INCLUDED_
