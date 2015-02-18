#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <uv.h>

#include <cassandra.h>


void print_error(CassFuture* future) {
   CassString message = cass_future_error_message(future);
   fprintf(stderr, "Error: %.*s\n", (int)message.length, message.data);
}


CassCluster* create_cluster() {
   CassCluster* cluster = cass_cluster_new();
   cass_cluster_set_contact_points(cluster, "132.176.69.181");
   return cluster;
}

CassError connect_session(CassSession* session, const CassCluster* cluster) {
   CassError rc = CASS_OK;
   CassFuture* future = cass_session_connect(session, cluster);
   cass_future_wait(future);
   rc = cass_future_error_code(future);
   if (rc != CASS_OK) {
      print_error(future);
   }
   cass_future_free(future);
   return rc;
}

void select_from_paging(CassSession* session) {
   cass_bool_t has_more_pages = cass_false;
   const CassResult* result = NULL;
   CassString query = cass_string_init("SELECT * FROM keyspace_r3.myrelation100000");
   CassStatement* statement = cass_statement_new(query, 0);
   cass_statement_set_paging_size(statement, 100);
   do {
      CassIterator* iterator;
      CassFuture* future = cass_session_execute(session, statement);
      
      if (cass_future_error_code(future) != 0) {
         print_error(future);
         break;
      }
      
     result = cass_future_get_result(future);
     iterator = cass_iterator_from_result(result);
     cass_future_free(future);

     while (cass_iterator_next(iterator)) {
        const CassRow* row = cass_iterator_get_row(iterator);
        printf(".");
     }
   
     has_more_pages = cass_result_has_more_pages(result);
     if (has_more_pages) {
        cass_statement_set_paging_state(statement, result); 
     }
     cass_iterator_free(iterator);
     cass_result_free(result);
   } while (has_more_pages);
   cass_statement_free(statement);
}

int main() {
   CassCluster* cluster = create_cluster();
   CassSession* session = cass_session_new();
   CassFuture* close_future = NULL;
   
   if (connect_session(session, cluster) != CASS_OK) {
      cass_cluster_free(cluster);
      cass_session_free(session);
      return -1;
   }
   

   select_from_paging(session);
   close_future = cass_session_close(session);
   cass_future_wait(close_future);
   cass_future_free(close_future);
   cass_cluster_free(cluster);
   cass_session_free(session);
   return 0;
}