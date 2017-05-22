/*  pss_parser_utils.hpp
 *
 *  Created by Rong Zhou on 04/15/09.
 *  Copyright 2009-16 Palo Alto Research Center Inc. All rights reserved.
 *
 *  header file for parser utility data types and functions
 */

#ifndef PSS_PARSER_UTILS_HPP_INCLUDED_
#define PSS_PARSER_UTILS_HPP_INCLUDED_

#include <sstream>
#include "rapidxml.hpp"
#include "pss_parser.hpp"

namespace pss {

template<typename T>
void ParseVector(rapidxml::xml_node<> *node,
                 char const *name,
                 std::vector<T>  &v,
                 void(*func)(rapidxml::xml_node<> *, T &)) {
  for(rapidxml::xml_node<> *n = node->first_node(name);
      n;
      n = n->next_sibling(name)) {
    T t;
    func(n, t);
    v.push_back(t);
  }
}

template<typename T>
void ParseVector2(rapidxml::xml_node<> *node,
                  char const *name,
                  std::vector<T>  &v,
                  void(*func)(rapidxml::xml_node<> *, std::vector<T> &)) {
  for(rapidxml::xml_node<> *n = node->first_node(name);
      n;
      n = n->next_sibling(name)) {
    std::vector<T> t;
    func(n, t);
    v.insert(v.end(), t.begin(), t.end());
  }
}

template<typename T>
void ParseAttr(rapidxml::xml_node<> *node, char const *name, T &attr) {
  std::stringstream ss;
  rapidxml::xml_attribute<> *attribute = node->first_attribute(name);
  if(attribute) {
    ss << attribute->value();
    ss >> attr;
  }
}

void ParseStrAttr(rapidxml::xml_node<> *node, char const *name, std::string &attr);
void ParseBoolAttr(rapidxml::xml_node<> *node, char const *name, bool &attr);
void ParseNameAttr(rapidxml::xml_node<> *node, std::string &name);
void ParseIdAttr(rapidxml::xml_node<> *node, std::string &id);
void ParseBaseInfo(rapidxml::xml_node<> *node, BaseInfo &baseinfo);
void ParseSchedule(rapidxml::xml_node<> *node, std::vector<Schedule> &schds);
void ParseGeometry(rapidxml::xml_node<> *node, Geometry &geometry);
void ParseDate(rapidxml::xml_node<> *node, Date &date);
void ParseQuantity(rapidxml::xml_node<> *node, Quantity &quantity);

}

#endif // PSS_PARSER_UTILS_HPP_INCLUDED_
