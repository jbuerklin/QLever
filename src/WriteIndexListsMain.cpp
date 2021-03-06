// Copyright 2014, University of Freiburg,
// Chair of Algorithms and Data Structures.
// Author: Björn Buchhold (buchhold@informatik.uni-freiburg.de)

//! CAREFUL, THIS FILE IS NOT USUALLY USED FOR QLEVER!
//! It has been added to support various experiments evolving typical datasets
//! e.g., for use in student projects.
#include <getopt.h>
#include <libgen.h>
#include <stdlib.h>
#include <iomanip>
#include <iostream>
#include <string>

#include "engine/QueryPlanner.h"
#include "parser/SparqlParser.h"
#include "util/ReadableNumberFact.h"
#include "util/Timer.h"

using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::string;

#define EMPH_ON "\033[1m"
#define EMPH_OFF "\033[22m"

// Available options.
struct option options[] = {{"index", required_argument, NULL, 'i'},
                           {"freebase", no_argument, NULL, 'f'},
                           {NULL, 0, NULL, 0}};

// Main function.
int main(int argc, char** argv) {
  cout.sync_with_stdio(false);
  std::cout << std::endl
            << EMPH_ON << "WriteIndexListsMain, version " << __DATE__ << " "
            << __TIME__ << EMPH_OFF << std::endl
            << std::endl;

  char* locale = setlocale(LC_CTYPE, "");
  cout << "Set locale LC_CTYPE to: " << locale << endl;

  std::locale loc;
  ad_utility::ReadableNumberFacet facet(1);
  std::locale locWithNumberGrouping(loc, &facet);
  ad_utility::Log::imbue(locWithNumberGrouping);

  string indexName = "";
  bool freebase = false;

  optind = 1;
  // Process command line arguments.
  while (true) {
    int c = getopt_long(argc, argv, "i:f", options, NULL);
    if (c == -1) break;
    switch (c) {
      case 'i':
        indexName = optarg;
        break;
      case 'f':
        freebase = true;
        break;
      default:
        cout << endl
             << "! ERROR in processing options (getopt returned '" << c
             << "' = 0x" << std::setbase(16) << c << ")" << endl
             << endl;
        exit(1);
    }
  }

  if (indexName.size() == 0) {
    cout << "Missing required argument --index (-i)..." << endl;
    exit(1);
  }

  try {
    Index index;
    index.createFromOnDiskIndex(indexName);
    index.addTextFromOnDiskIndex();

    vector<string> lists;
    lists.push_back("algo*");
    bool decodeGapsAndFrequency = true;
    index.dumpAsciiLists(lists, decodeGapsAndFrequency);

    Engine engine;
    QueryExecutionContext qec(index, engine);
    ParsedQuery q;
    if (!freebase) {
      q = SparqlParser::parse("SELECT ?x WHERE {?x <is-a> <Scientist>}");
    } else {
      q = SparqlParser::parse(
          "PREFIX fb: <http://rdf.freebase.com/ns/> SELECT ?p WHERE {?p "
          "fb:people.person.profession fb:m.06q2q}");
      q.expandPrefixes();
    }
    QueryPlanner queryPlanner(&qec);
    auto qet = queryPlanner.createExecutionTree(q);
    const auto res = qet.getResult();
    AD_CHECK(res->size() > 0);
    AD_CHECK(res->_nofColumns == 1);
    string personlistFile = indexName + ".list.scientists";
    std::vector<std::array<Id, 1>>& ids =
        *static_cast<std::vector<std::array<Id, 1>>*>(res->_fixedSizeData);
    std::ofstream f(personlistFile.c_str());
    for (size_t i = 0; i < ids.size(); ++i) {
      f << ids[i][0] << ' ';
    }
    f.close();

  } catch (const std::exception& e) {
    cout << string("Caught exceptions: ") + e.what();
    return 1;
  } catch (ad_semsearch::Exception& e) {
    cout << e.getFullErrorMessage() << std::endl;
  }

  return 0;
}
