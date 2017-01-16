#include <algorithm>
#include <boost/graph/graphviz.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "hmc_graphviz.h"
#include "hmc_sim.h"

const char* hmc_graphviz::get_filename_from_env(void)
{
  return getenv("HMCSIM_GRAPH_DOTFILE");
}

std::string hmc_graphviz::get_content_from_file(const char *filename)
{
  std::ifstream fh;
  fh.open(filename);
  std::stringstream fhst;
  fhst << fh.rdbuf();
  std::string str = fhst.str();
  fh.close();
  return str;
}

// https://stackoverflow.com/questions/5208811/how-to-get-port-identifiers-for-an-edge-using-boost-graph-library
void hmc_graphviz::parse_content_and_set(hmc_sim *sim, std::string content)
{
  boost::read_graphviz_detail::parser_result result;
  boost::read_graphviz_detail::parse_graphviz_from_string(content, result, false);
  for (std::vector<boost::read_graphviz_detail::edge_info>::iterator it = result.edges.begin(); it != result.edges.end(); ++it) {
    struct boost::read_graphviz_detail::node_and_port *src = &(*it).source;
    struct boost::read_graphviz_detail::node_and_port *tgt = &(*it).target;
    boost::read_graphviz_detail::properties *props = &(*it).props;
    boost::read_graphviz_detail::properties::iterator propsit = props->find("label");
    if (propsit != props->end()) {
      std::string label = propsit->second;
      std::cout << label << std::endl;
      if (label.find("-bit") != std::string::npos
          && label.find(".") != std::string::npos) {
        // Todo: parse 64-bit,0.325 ... currently ugly
        std::string::size_type sz;
        unsigned int bitwidth = std::stoi(label, &sz);
        float bitrate = std::stof(label.substr(sz + strlen("-bit,")));

        std::cout << src->name << ":" << src->location[0] << " -- " << tgt->name << ":" << tgt->location[0] << std::endl;
        std::transform(src->name.begin(), src->name.end(), src->name.begin(), ::tolower);
        if (!src->name.compare(0, strlen("host"), "host")) {
          unsigned slidId = this->extract_id_from_string(src->location[0]);
          unsigned hmcId = this->extract_id_from_string(tgt->name);
          unsigned linkId = this->extract_id_from_string(tgt->location[0]);
          if (!sim->hmc_define_slid(slidId, hmcId, linkId, bitwidth, bitrate)) {
            std::cout << "ERROR: define slid failed!" << std::endl;
            exit(-1);
          }
        }
        else {
          unsigned src_hmcId = this->extract_id_from_string(src->name);
          unsigned src_linkId = this->extract_id_from_string(src->location[0]);
          unsigned tgt_hmcId = this->extract_id_from_string(tgt->name);
          unsigned tgt_linkId = this->extract_id_from_string(tgt->location[0]);
          if (!sim->hmc_set_link_config(src_hmcId, src_linkId, tgt_hmcId, tgt_linkId, bitwidth, bitrate)) {
            std::cout << "ERROR: set link config failed!" << std::endl;
            exit(-1);
          }
        }
      }

      continue;
    }

    std::cerr << "ERROR: could not import (no bitwidth/-rate): " << src->name << ":" << src->location[0] << " -- " << tgt->name << ":" << tgt->location[0] << std::endl;
    exit(-1);
  }
}

hmc_graphviz::hmc_graphviz(hmc_sim *sim, const char *graphviz_filename)
{
  // if not set, try to find a filename, if this does not apply, potentially the graph will be setup the old way
  if (!graphviz_filename) {
    graphviz_filename = this->get_filename_from_env();
    if (!graphviz_filename) {
      std::cout << "HMCSIM_GRAPH_DOTFILE env variable not set!" << std::endl;
      return;
    }
  }

  std::string content = this->get_content_from_file(graphviz_filename);
  if (content.empty())
    return;

  this->parse_content_and_set(sim, content);
}
