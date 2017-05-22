/*  pss_shop_func.hpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  header file for data types and functions for pss shop functions
 */

#ifndef PSS_SHOP_FUNC_HPP_INCLUDED_
#define PSS_SHOP_FUNC_HPP_INCLUDED_

#include <iostream>
#include "pss_utils.hpp"
#include "pss_shop_file.hpp"

namespace pss {

struct Sfunc {
  std::string station;
  SimpleFunc funcseq;
};

struct LtSfunc {
  bool operator()(const Sfunc &sfunc1, const Sfunc &sfunc2) const {
    return sfunc1.station < sfunc2.station;
  }
};

typedef std::set<Sfunc, LtSfunc> SfuncSet;

typedef std::map<std::string,  SfuncSet> Seq2Sfunc;

struct ShopInfo {
  std::map<std::string, pss::Seq2Sfunc> seq2mach;
  std::map<std::string, pss::TintvlVec2d> mach2weekts;
  std::map<std::string, pss::DayTs> mach2dayts;
  std::map<std::string, std::map<std::string, SimpleFunc> > station2seq;
  pss::One2Many cell2opr;
  std::map<std::string, pss::One2Many> seq2opr;
  std::map<std::string, pss::TintvlVec2d> opr2weekts;
  std::map<std::string, pss::DayTs> opr2dayts;
  std::map<std::string, pss::CellConfig> cell2config;
  std::map<std::string, pss::Rsrc2Qty> unit2minbatch;
  std::map<std::string, double> rsrc2speed;
  pss::One2Many func2seq;
  pss::One2Many seq2func;
  pss::One2Many func2sameseqfunc;
  pss::One2One machfuncseq2id;
  pss::One2Many seq2cell;
  Config config;
};

void GetShopInfo(ShopInfo &shop_info, ShopModel &shop);

void PrintShopConfig(std::ostream &os, const ShopInfo &shop_info);

void PrintMultiFuncSeq(std::ostream &os, pss::One2Many &seq2func);

bool BatchEnabled(std::map<std::string, CellConfig> &cell2config);

bool OprltdEnabled(std::map<std::string, CellConfig> &cell2config);

}

#endif // PSS_SHOP_FUNC_HPP_INCLUDED_
