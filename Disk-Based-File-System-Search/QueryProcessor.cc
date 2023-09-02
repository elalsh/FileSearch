/*
 * Copyright ©2023 Justin Hsia.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include "./QueryProcessor.h"

#include <iostream>
#include <algorithm>
#include <list>
#include <string>
#include <vector>

extern "C" {
  #include "./libhw1/CSE333.h"
}

using std::list;
using std::sort;
using std::string;
using std::vector;

namespace hw3 {

QueryProcessor::QueryProcessor(const list<string>& index_list, bool validate) {
  // Stash away a copy of the index list.
  index_list_ = index_list;
  array_len_ = index_list_.size();
  Verify333(array_len_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader* [array_len_];
  itr_array_ = new IndexTableReader* [array_len_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::const_iterator idx_iterator = index_list_.begin();
  for (int i = 0; i < array_len_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = fir.NewDocTableReader();
    itr_array_[i] = fir.NewIndexTableReader();
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (int i = 0; i < array_len_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

// This structure is used to store a index-file-specific query result.
typedef struct {
  DocID_t doc_id;  // The document ID within the index file.
  int     rank;    // The rank of the result so far.
} IdxQueryResult;

// Read processes the query against a single index table
// Arguments:
// - index_file: the Index Table to qury against
// - query: list of qury strings
static vector<IdxQueryResult>
GetIndxQuery(const IndexTableReader* const index_file,
             const vector<string>& query);

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string>& query) const {
  Verify333(query.size() > 0);

  // STEP 1.
  // (the only step in this file)
  vector<QueryProcessor::QueryResult> final_result;
  // query against each index
  for (int i = 0; i < array_len_; i++) {
    vector<IdxQueryResult> curr_res = GetIndxQuery(itr_array_[i], query);

    // Add the index query results of single index file to final result
    QueryResult result;
    string str;
    for (const auto& elems : curr_res) {
      dtr_array_[i]->LookupDocID(elems.doc_id, &str);
      result.document_name = str;
      result.rank = elems.rank;
      final_result.push_back(result);
    }
  }

  // Sort the final results.
  sort(final_result.begin(), final_result.end());
  return final_result;
}

static vector<IdxQueryResult>
GetIndxQuery(const IndexTableReader* const index_file,
             const vector<string>& query) {
  vector<IdxQueryResult> curr_res;
  DocIDTableReader* doc_table = (*index_file).LookupWord(query[0]);
    if (doc_table == nullptr) {
      return curr_res;
    }
  // we have found the words in docid table
  list<DocIDElementHeader> doc_pos_list = doc_table->GetDocIDList();
  IdxQueryResult idqres;
  for (const auto& elem : doc_pos_list) {
    idqres.doc_id = elem.doc_id;
    idqres.rank = elem.num_positions;
    curr_res.push_back(idqres);
  }
  delete doc_table;  // free the Doc Id Table we are reading

  // consider the other query words
  // bool failed = false;
  for (int j = 1; j < static_cast<int>(query.size()); j++) {
    // look up word in index
    DocIDTableReader* doc_table = (*index_file).LookupWord(query[j]);
    if (doc_table == nullptr) {
      curr_res.clear();
      return curr_res;
    }
    list<DocPositionOffset_t> positions;
    // iterate through current doc_id and rank we have and update accordingly
    for (auto it = curr_res.begin(); it != curr_res.end(); ) {
      if (doc_table ->LookupDocID(it->doc_id, &positions)) {
        it->rank += positions.size();
        it++;
      } else {
        curr_res.erase(it);
      }
    }
    delete doc_table;
  }
  return curr_res;
}

}  // namespace hw3
