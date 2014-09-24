////////////////////////////////////////////////////////////////////////////////
/// @brief allow to register AQL Queries in a central place in memory
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2014 triagens GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Copyright 2014, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_AQL_QUERY_REGISTRY_H
#define ARANGOD_AQL_QUERY_REGISTRY_H 1

#include "Basics/Common.h"
#include "Basics/ReadWriteLock.h"
#include "Aql/Query.h"

namespace triagens {
  namespace aql {

// -----------------------------------------------------------------------------
// --SECTION--                                           the QueryRegistry class
// -----------------------------------------------------------------------------

    class QueryRegistry {

      public:

// -----------------------------------------------------------------------------
// --SECTION--                                          constructors/destructors
// -----------------------------------------------------------------------------

        QueryRegistry () {
        }

        ~QueryRegistry () {
          for (auto& m : _queries) {
            for (auto& q : m.second) {
              QueryInfo*& p = q.second;
              delete p->_query;
            }
          }
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert, this inserts the query <query> for the vocbase <vocbase> 
/// and the id <id> into the registry. It is in error if there is already
/// a query for this <vocbase> and <id> combination and an exception will
/// be thrown in that case. The time to live <ttl> is in seconds and the
/// query will be deleted if it is not opened for that amount of time.
////////////////////////////////////////////////////////////////////////////////

        void insert (TRI_vocbase_t* vocbase,
                     QueryId id,
                     Query* query,
                     double ttl = 3600.0);

////////////////////////////////////////////////////////////////////////////////
/// @brief open, find a query in the registry, if none is found, a nullptr
/// is returned, otherwise, ownership of the query is transferred to the
/// caller, however, the registry retains the entry and will open will
/// succeed only once. If an already open query with the given id is
/// found, an exception is thrown.
/// An open query can directly be destroyed by the destroy method.
/// Note that an open query will not expire, so users should please
/// protect against leaks. If an already open query is found, an exception
/// is thrown.
////////////////////////////////////////////////////////////////////////////////

        Query* open (TRI_vocbase_t* vocbase, QueryId id);

////////////////////////////////////////////////////////////////////////////////
/// @brief close, return a query to the registry, if the query is not found,
/// an exception is thrown. If the ttl is negative (the default is), the 
/// original ttl is taken again.
////////////////////////////////////////////////////////////////////////////////
        
        void close (TRI_vocbase_t* vocbase, QueryId id, double ttl = -1.0);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy, this removes the entry from the registry and calls
/// delete on the Query*. It is allowed to call this regardless of whether
/// the query is open or closed. No check is performed that this call comes
/// from the same thread that has opened it!
////////////////////////////////////////////////////////////////////////////////

        void destroy (TRI_vocbase_t* vocbase, QueryId id);

////////////////////////////////////////////////////////////////////////////////
/// @brief expireQueries, this deletes all expired queries from the registry
////////////////////////////////////////////////////////////////////////////////

        void expireQueries ();

      private:

// -----------------------------------------------------------------------------
// --SECTION--                                                   private members
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief a struct for all information regarding one query in the registry
////////////////////////////////////////////////////////////////////////////////

        struct QueryInfo {
          TRI_vocbase_t* _vocbase;  // name of vocbase
          QueryId _id;               // id of the query
          Query* _query;             // the actual query pointer
          bool _isOpen;              // flag indicating whether or not the query
                                     // is in use
          double _timeToLive;        // in seconds
          double _expires;           // UNIX UTC timestamp of expiration
        };

////////////////////////////////////////////////////////////////////////////////
/// @brief _queries, the actual map of maps for the registry, the bool
/// indicates
////////////////////////////////////////////////////////////////////////////////

        std::unordered_map<TRI_vocbase_t*, std::unordered_map<QueryId, QueryInfo*>> _queries;

////////////////////////////////////////////////////////////////////////////////
/// @brief _lock, the read/write lock for access
////////////////////////////////////////////////////////////////////////////////

        basics::ReadWriteLock _lock;

    };

  }   // namespace triagens::aql
}   // namespace triagens

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:


