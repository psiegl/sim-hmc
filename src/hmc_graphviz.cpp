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

#define boost_graphviz boost::read_graphviz_detail

// https://stackoverflow.com/questions/5208811/how-to-get-port-identifiers-for-an-edge-using-boost-graph-library
void hmc_graphviz::parse_content_and_set(hmc_sim *sim, std::string content)
{
  boost_graphviz::parser_result result;
  boost_graphviz::parse_graphviz_from_string(content, result, false);
  for (std::vector<boost_graphviz::edge_info>::iterator it = result.edges.begin(); it != result.edges.end(); ++it) {
    struct boost_graphviz::node_and_port *src = &(*it).source;
    struct boost_graphviz::node_and_port *tgt = &(*it).target;
    boost_graphviz::properties *props = &(*it).props;
    boost_graphviz::properties::iterator propsit = props->find("label");
    if (propsit != props->end()) {
      std::string label = propsit->second;
      if (label.find(",") != std::string::npos
          && label.find("BR") != std::string::npos) {
        std::string::size_type sz;
        unsigned int lanes = std::stoi(label, &sz); // ToDo
        switch (lanes) {
        case HMCSIM_FULL_LINK_WIDTH:
        case HMCSIM_HALF_LINK_WIDTH:
        case HMCSIM_QUARTER_LINK_WIDTH:
          break;
        default:
          std::cerr << "ERROR: lanes supports only ";
          std::cerr << HMCSIM_FULL_LINK_WIDTH << " (FULL), ";
          std::cerr << HMCSIM_HALF_LINK_WIDTH << " (HALF), ";
          std::cerr << HMCSIM_QUARTER_LINK_WIDTH << " (QUARTER); ";
          std::cerr << "current: " << lanes;
          std::cerr << std::endl;
          exit(-1);
        }

        float bitrate = std::stof(label.substr(sz + strlen(",BR"))); // ToDo
        if (!(similar_floats(bitrate, HMCSIM_BR12_5)
              || similar_floats(bitrate, HMCSIM_BR15)
              || similar_floats(bitrate, HMCSIM_BR25)
              || similar_floats(bitrate, HMCSIM_BR28)
              || similar_floats(bitrate, HMCSIM_BR30))) {
          std::cerr << "ERROR: bitrate supports only ";
          std::cerr << HMCSIM_BR12_5 << " (BR12.5), ";
          std::cerr << HMCSIM_BR15 << " (BR15), ";
          std::cerr << HMCSIM_BR25 << " (BR25), ";
          std::cerr << HMCSIM_BR28 << " (BR28), ";
          std::cerr << HMCSIM_BR30 << " (BR30); ";
          std::cerr << "current: " << bitrate;
          std::cerr << std::endl;
          exit(-1);
        }

        std::string src_name(src->name);
        std::transform(src_name.begin(), src_name.end(), src_name.begin(), ::tolower);
        if (!src_name.compare(0, strlen("host"), "host")) {
          unsigned slidId = this->extract_id_from_string(src->location[0]);
          unsigned hmcId = this->extract_id_from_string(tgt->name);
          unsigned linkId = this->extract_id_from_string(tgt->location[0]);
          if (!sim->hmc_define_slid(slidId, hmcId, linkId, lanes, bitrate)) {
            std::cout << "ERROR: define slid failed!" << std::endl;
            exit(-1);
          }
        }
        else {
          unsigned src_hmcId = this->extract_id_from_string(src->name);
          unsigned src_linkId = this->extract_id_from_string(src->location[0]);
          unsigned tgt_hmcId = this->extract_id_from_string(tgt->name);
          unsigned tgt_linkId = this->extract_id_from_string(tgt->location[0]);
          if (!sim->hmc_set_link_config(src_hmcId, src_linkId, tgt_hmcId, tgt_linkId, lanes, bitrate)) {
            std::cout << "ERROR: set link config failed!" << std::endl;
            exit(-1);
          }
        }
        std::cout << "HMC_GRAPHVIZ: " << src->name << ":" << src->location[0] << " -- " << tgt->name << ":" << tgt->location[0] << std::endl;
      }

      continue;
    }

    std::cerr << "ERROR: could not import (no bitwidth/-rate): " << src->name << ":" << src->location[0] << " -- " << tgt->name << ":" << tgt->location[0] << std::endl;
    exit(-1);
  }
}

hmc_graphviz::hmc_graphviz (hmc_sim *sim, const char *graphviz_filename)
{
  // if not set, try to find a filename, if this does not apply, potentially the graph will be setup the old way
  if (!graphviz_filename) {
    graphviz_filename = this->get_filename_from_env();
    if (!graphviz_filename) {
      std::cout << "HMC_GRAPHVIZ: HMCSIM_GRAPH_DOTFILE env variable not set!" << std::endl;
      return;
    }
  }
  std::cout << "HMC_GRAPHVIZ: " << graphviz_filename << std::endl;

  std::string content = this->get_content_from_file(graphviz_filename);
  if (content.empty())
    return;

  this->parse_content_and_set(sim, content);
}
