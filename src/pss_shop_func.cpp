/* pss_shop_func.cpp
 *
 *  Created by Rong Zhou on 05/01/09.
 *  Copyright 2009 Palo Alto Research Center Inc. All rights reserved.
 *
 *  implementation file for pss shop functions
 */
#include <sstream>
#include <algorithm>
#include "pss_shop_func.hpp"
#include "pss_exception.hpp"

using namespace std;

namespace pss {

ostream &operator << (ostream &os, const Sfunc &sfunc) {
  os << '[' << sfunc.station << ", ";
  os << sfunc.funcseq.funcinfo.name << ", ";
  os << sfunc.funcseq.funcinfo.speedval << ", ";
  os << sfunc.funcseq.funcinfo.timeunit << ']';
  return os;
}

void AttributesInSeconds(map<string, time_t> &attributes) {
  for(map<string, time_t>::iterator it = attributes.begin();
      it != attributes.end();
      ++it) {
    it->second *= 60; //convert from minutes to seconds
  }
}

void FuncInfoInSeconds(FuncInfo &funcinfo) {
  funcinfo.setuptime *= 60.0; //convert from minutes to seconds
  if(funcinfo.timeunit == "hour") {
    funcinfo.speedval /= 3600.0;
    funcinfo.timeunit = "second";
  } else if(funcinfo.timeunit == "minute") {
    funcinfo.speedval /= 60.0;
    funcinfo.timeunit = "second";
  } else if(funcinfo.timeunit != "second")
    cerr << "Error: invalid funcinfo timeunit '" << funcinfo.timeunit << "'"
         << endl;
}

void SimpleFuncInSeconds(SimpleFunc &simplefunc) {
  FuncInfoInSeconds(simplefunc.funcinfo);
  AttributesInSeconds(simplefunc.attributes);
}

void GetShopInfo(ShopInfo &shop_info, ShopModel &shop) {
  vector<Cell>::const_iterator c;
  vector<Station>::const_iterator i;
  vector<Opr>::const_iterator o;
  vector<SimpleFunc>::const_iterator j;
  vector<string>::const_iterator k, t;
  One2One mach2cell, machid2name, oprid2name;
  One2Many opr2skills;
  Sfunc sfunc;
  map<string, Seq2Sfunc>::const_iterator cl;
  Seq2Sfunc::const_iterator m;
  TintvlVec2d shopweekts, machweekts, oprweekts;
  DayTs shopdayts, machdayts, oprdayts;

  shop_info.config = shop.config;
  for(i = shop.stations.begin(); i != shop.stations.end(); ++i)
    machid2name[(*i).stationinfo.stationid] = (*i).baseinfo.name;
  for(o = shop.operators.begin(); o != shop.operators.end(); ++o) {
    if((*o).baseinfo.status == 0)  //0 means available, 2 means unavailable
      oprid2name[(*o).oprinfo.rsrcid] = (*o).baseinfo.name;
    else
      oprid2name[(*o).oprinfo.rsrcid] = ""; //means unavailable
  }
  for(c = shop.cells.begin(); c != shop.cells.end(); ++c) {
    string cell = (*c).baseinfo.name;
    shop_info.cell2config[cell] = (*c).cellconfig;
    for(t = (*c).stations.begin(); t != (*c).stations.end(); ++t)
      //mach2cell[machid2name[atoi((*t).c_str())]] = cell;
      mach2cell[machid2name[*t]] = cell;
    for(t = (*c).operators.begin(); t != (*c).operators.end(); ++t) {
      //if (oprid2name[atoi((*t).c_str())] != "")
      if(oprid2name[*t] != "")
        //shop_info.cell2opr[cell].insert(oprid2name[atoi((*t).c_str())]);
        shop_info.cell2opr[cell].insert(oprid2name[*t]);
    }
  }
  GetTmslot(shopweekts, shopdayts, shop.schds);
  for(o = shop.operators.begin(); o != shop.operators.end(); ++o) {
    opr2skills[(*o).baseinfo.name].insert((*o).skills.begin(), (*o).skills.end());
    if(!(*o).schds.empty()) {
      oprweekts.clear();
      oprdayts.once.clear();
      oprdayts.everyyear.clear();
      GetTmslot(oprweekts, oprdayts, (*o).schds);
      shop_info.opr2weekts[(*o).baseinfo.name] = oprweekts;
      shop_info.opr2dayts[(*o).baseinfo.name] = oprdayts;
    } else {
      shop_info.opr2weekts[(*o).baseinfo.name] = shopweekts;
      shop_info.opr2dayts[(*o).baseinfo.name] = shopdayts;
    }
  }
  //add a special operator "any" that is always available 24/7
  //useful when scheduling is not limited by operators
  GetTmslot247(oprweekts, oprdayts);
  shop_info.opr2weekts["any"] = oprweekts;
  shop_info.opr2dayts["any"] = oprdayts;
  for(i = shop.stations.begin(); i != shop.stations.end(); ++i) {
    sfunc.station = (*i).baseinfo.name;
    //cout << "baseinfo.name = " << (*i).baseinfo.name << endl;
    if(!(*i).schds.empty()) {
      machweekts.clear();
      machdayts.once.clear();
      machdayts.everyyear.clear();
      GetTmslot(machweekts, machdayts, (*i).schds);
      shop_info.mach2weekts[(*i).baseinfo.name] = machweekts;
      shop_info.mach2dayts[(*i).baseinfo.name] = machdayts;
    } else {
      shop_info.mach2weekts[(*i).baseinfo.name] = shopweekts;
      shop_info.mach2dayts[(*i).baseinfo.name] = shopdayts;
    }
    map<string, SimpleFunc> name2seq;
    for(j = (*i).simplefuncs.begin(); j != (*i).simplefuncs.end(); ++j) {
      string funcseq = (*j).funcinfo.name;
      name2seq[funcseq] = *j;
      SimpleFuncInSeconds(name2seq[funcseq]);
      vector<string> funcnames = (*j).names;
      // new func seq
      if(shop_info.seq2func.find(funcseq) == shop_info.seq2func.end()) {
        for(k = funcnames.begin(); k != funcnames.end(); ++k) {
          //cerr << *k << endl;
          //if (map.find(*k) == map.end())
          //    map[*k] = set<string>();
          shop_info.func2seq[*k].insert(funcseq);
          shop_info.seq2func[funcseq].insert(*k);
        }
      } else { //verify same as previously defined func seq
        for(k = funcnames.begin(); k != funcnames.end(); ++k)  {
          set<string> &funcset = shop_info.seq2func[funcseq];
          if(funcset.find(*k) == funcset.end()) {
            throw RuntimeException("Station: " + sfunc.station +
                                   "\n\tFunction step '" + *k
                                   + "' not found in previously defined sequence: "
                                   + funcseq);
          }
        }
      }
      if(mach2cell[sfunc.station] != "")
        shop_info.seq2cell[funcseq].insert(mach2cell[sfunc.station]);
      stringstream sstr;
      sstr << (*i).stationinfo.barcode << '-' << (*j).funcinfo.barcode;
      shop_info.machfuncseq2id[sfunc.station+'-'+funcseq] = sstr.str();
      sfunc.funcseq = *j;
      SimpleFuncInSeconds(sfunc.funcseq);
      if(sfunc.funcseq.funcinfo.speedval <= 0.0) {
        throw RuntimeException("Station: " + sfunc.station +
                               "\n\tSpeed value must be greater than zero: " +
                               (*j).funcinfo.name);
      }
      shop_info.rsrc2speed[sfunc.funcseq.funcinfo.speedunit] +=
        sfunc.funcseq.funcinfo.speedval;
      unsigned minbatch = static_cast<unsigned>(sfunc.funcseq.funcinfo.minbatch);
      //if (minbatch > 1)
      //    cerr << "station " << sfunc.station << " min batch = " << minbatch << endl;
      if(minbatch > 1 && minbatch >
          shop_info.unit2minbatch[mach2cell[sfunc.station]][sfunc.funcseq.funcinfo.speedunit])
        shop_info.unit2minbatch[mach2cell[sfunc.station]][sfunc.funcseq.funcinfo.speedunit]
          = minbatch;
      if(mach2cell[sfunc.station] != "") {
        shop_info.seq2mach[mach2cell[sfunc.station]][funcseq].insert(sfunc);
        set<string> &coprs = shop_info.cell2opr[mach2cell[sfunc.station]];
        set<string>::const_iterator copr;
        for(copr = coprs.begin(); copr != coprs.end(); ++copr) {
          set<string> &skills = opr2skills[*copr];
          if(includes(skills.begin(), skills.end(),
                      shop_info.seq2func[funcseq].begin(),
                      shop_info.seq2func[funcseq].end()))
            shop_info.seq2opr[mach2cell[sfunc.station]][funcseq].insert(*copr);
        }
      }
      //                    cerr << "funcseq = " << funcseq << ": ";
      //                    copy(shop_info.seq2mach[funcseq].begin(), shop_info.seq2mach[funcseq].end(), ostream_iterator<Sfunc>(cerr, " "));
      //                    cerr << endl;
    }
    if(shop_info.station2seq.find((*i).baseinfo.name) !=
        shop_info.station2seq.end()) {
      throw RuntimeException("Station: " + (*i).baseinfo.name +
                             "\n\tStation with the same name already defined");
    }
    shop_info.station2seq[(*i).baseinfo.name] = name2seq;
    name2seq.clear();
  }
  One2Many::const_iterator f;
  set<string>::const_iterator s, f2;
  for(f = shop_info.func2seq.begin(); f != shop_info.func2seq.end(); ++f) {
    shop_info.func2sameseqfunc[f->first].clear();
    for(s = f->second.begin(); s != f->second.end(); ++s) {
      for(f2 = shop_info.seq2func[*s].begin();
          f2 != shop_info.seq2func[*s].end();
          ++f2) {
        if(f->first != *f2) {
          shop_info.func2sameseqfunc[f->first].insert(*f2);
        }
      }
    }
  }

  //  for debugging:
  //            for (f = shop_info.seq2cell.begin(); f != shop_info.seq2cell.end(); ++f)
  //            {
  //                cerr << f->first << ':';
  //                copy(f->second.begin(), f->second.end(), ostream_iterator<string>(cerr, ", "));
  //                cerr << endl;
  //            }

  //            for (f = shop_info.func2seq.begin(); f != shop_info.func2seq.end(); ++f)
  //            {
  //                cerr << f->first << ':';
  //                copy(f->second.begin(), f->second.end(), ostream_iterator<string>(cerr, ", "));
  //                cerr << endl;
  //            }
  //            for (f = shop_info.seq2func.begin(); f != shop_info.seq2func.end(); ++f)
  //            {
  //                cerr << f->first << ':';
  //                copy(f->second.begin(), f->second.end(), ostream_iterator<string>(cerr, ", "));
  //                cerr << endl;
  //            }
  //            for (f = shop_info.func2sameseqfunc.begin(); f != shop_info.func2sameseqfunc.end(); ++f)
  //            {
  //                cerr << f->first << ':';
  //                copy(f->second.begin(), f->second.end(), ostream_iterator<string>(cerr, ", "));
  //                cerr << endl;
  //            }
  //            for (cl = shop_info.seq2mach.begin(); cl != shop_info.seq2mach.end(); ++cl)
  //            {
  //                cerr << "Cell '" << cl->first << "': " << endl;
  //                for (m = cl->second.begin(); m != cl->second.end(); ++m)
  //                {
  //                    cerr << m->first << ':';
  //                    copy(m->second.begin(), m->second.end(), ostream_iterator<Sfunc>(cerr, ", "));
  //                    cerr << endl;
  //                }
  //                cerr << endl;
  //            }
  //            map<string, One2Many>::const_iterator so;
  //            for (so = shop_info.seq2opr.begin(); so != shop_info.seq2opr.end(); ++so)
  //            {
  //                cerr << "Cell '" << so->first << "': " << endl;
  //                One2Many::const_iterator fo;
  //                for (fo = so->second.begin(); fo != so->second.end(); ++fo) {
  //                    cerr << fo->first << ':';
  //                    copy(fo->second.begin(), fo->second.end(), ostream_iterator<string>(cerr, ", "));
  //                    cerr << endl;
  //                }
  //            }
}

void PrintMultiFuncSeq(ostream &os, One2Many &seq2func) {
  One2Many::const_iterator s;
  string output_str;

  for(s = seq2func.begin(); s != seq2func.end(); ++s) {
    if(s->second.size() > 1) {
      os << s->first << ": ";
      SetToStr(output_str, s->second, ", ");
      os << output_str << endl;
    }
  }
}

enum OnOff { kAllOff, kAllOn, kSomeOn };

enum OnOff BatchEnabled(const map<string, CellConfig> &cell2config) {
  map<string, CellConfig>::const_iterator c;
  bool batch_enabled_all = true;
  bool batch_disabled_all = true;

  for(c = cell2config.begin(); c != cell2config.end(); ++c) {
    if(c->second.batching == false) {
      batch_enabled_all = false;
    } else {
      batch_disabled_all = false;
    }
  }
  if(batch_enabled_all)
    return kAllOn;
  if(batch_disabled_all)
    return kAllOff;
  return kSomeOn;
}

enum OnOff OprltdEnabled(const map<string, CellConfig> &cell2config) {
  map<string, CellConfig>::const_iterator c;
  bool oprltd_enabled_all = true;
  bool oprltd_disabled_all = true;

  for(c = cell2config.begin(); c != cell2config.end(); ++c) {
    if(c->second.oprlimited == false) {
      oprltd_enabled_all = false;
    } else {
      oprltd_disabled_all = false;
    }
  }
  if(oprltd_enabled_all)
    return kAllOn;
  else if(oprltd_disabled_all)
    return kAllOff;
  return kSomeOn; //at least one cell is opr limited, but not all cells
}

void OnOffMessage(string &message, enum OnOff code) {
  switch(code) {
  case kAllOff:
    message = "Off in all cells";
    break;
  case kAllOn:
    message = "On in all cells";
    break;
  default:
    message = "On in some cells";
  }
}

void PrintShopConfig(ostream &os, const ShopInfo &shop_info) {
  string on_off_msg;

  os << "Sequencing policy = " << shop_info.config.sequencepolicy << endl;
  OnOffMessage(on_off_msg, BatchEnabled(shop_info.cell2config));
  os << "Batching = " << on_off_msg << endl;
  OnOffMessage(on_off_msg, OprltdEnabled(shop_info.cell2config));
  os << "Operator limited scheduling = " << on_off_msg << endl;
  if(shop_info.config.routingpolicy.empty() ||
      shop_info.config.routingpolicy == "loadBalancing") {
    os << "Routing policy = loadBalancing (default)" << endl;
  } else {
    os << "Routing policy = " << shop_info.config.routingpolicy;
    os << ", threshold = " << shop_info.config.threshold;
    os << ", function name = " << shop_info.config.funcname << endl;
  }
}

}
